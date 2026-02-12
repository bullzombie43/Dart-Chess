import 'dart:async';
import 'package:chess_ui/game/cutechess_manager.dart';
import 'package:chess_ui/game/game_controller.dart';
import 'package:chess_ui/game/game_mode.dart';
import 'package:chess_ui/game/move_parser.dart';
import 'package:chess_ui/game/pgn_replayer.dart';
import 'package:chess_ui/game/pgn_watcher.dart';
import 'package:chess_ui/game/uci_orchestrator.dart';

/// Match events emitted by MatchManager
enum MatchEventType {
  matchStarted,
  gameStarted,
  movePlayed,
  gameFinished,
  matchFinished,
  error,
}

class MatchEvent {
  final MatchEventType type;
  final String? message;
  final int? gameNumber;
  final String? moveUci;
  final String? result;
  
  MatchEvent({
    required this.type,
    this.message,
    this.gameNumber,
    this.moveUci,
    this.result,
  });
}

/// Unified match management for engine vs engine matches
/// 
/// Supports two modes:
/// - Visualization: Real-time moves via UCI orchestrator
/// - Testing: ELO testing via cutechess with PGN replay
class MatchManager {
  final GameController controller;
  final StreamController<MatchEvent> _eventController = StreamController.broadcast();
  
  Stream<MatchEvent> get events => _eventController.stream;
  
  MatchMode _currentMode = MatchMode.visualization;
  UciOrchestrator? _uciOrchestrator;
  CutechessManager? _cutechessManager;
  PGNWatcher? _pgnWatcher;
  PgnReplayer? _pgnReplayer;
  
  bool _isActive = false;
  Duration _replayDelay = const Duration(milliseconds: 500);
  
  MatchManager(this.controller);
  
  /// Start a match with specified mode
  Future<void> startMatch({
    required MatchMode mode,
    required String engine1,
    required String engine2,
    String? engine1Path,
    String? engine2Path,
    Map<String, String>? engine1Options,
    Map<String, String>? engine2Options,
    int? rounds,
    int? stockfishSkill,
    String? projectRoot,
  }) async {
    if (_isActive) {
      await stopMatch();
    }
    
    _currentMode = mode;
    _isActive = true;
    
    // Reset board
    controller.resetGame();
    controller.setGameMode(GameMode.engineVsEngine);
    controller.setMatchStatus(MatchStatus.active);
    
    try {
      if (mode == MatchMode.visualization) {
        await _startVisualizationMatch(
          engine1Path: engine1Path ?? engine1,
          engine2Path: engine2Path ?? engine2,
          engine1Options: engine1Options,
          engine2Options: engine2Options,
        );
      } else {
        await _startTestingMatch(
          engine1: engine1,
          stockfishSkill: stockfishSkill ?? 1,
          rounds: rounds ?? 100,
          projectRoot: projectRoot,
        );
      }
      
      _eventController.add(MatchEvent(type: MatchEventType.matchStarted));
    } catch (e) {
      _isActive = false;
      _eventController.add(MatchEvent(
        type: MatchEventType.error,
        message: 'Failed to start match: $e',
      ));
      rethrow;
    }
  }
  
  /// Start visualization mode match (real-time UCI)
  Future<void> _startVisualizationMatch({
    required String engine1Path,
    required String engine2Path,
    Map<String, String>? engine1Options,
    Map<String, String>? engine2Options,
  }) async {
    _uciOrchestrator = UciOrchestrator();
    
    // Listen to UCI events
    _uciOrchestrator!.events.listen((uciEvent) {
      switch (uciEvent.type) {
        case UciEventType.movePlayed:
          if (uciEvent.moveEvent != null) {
            _applyUciMove(uciEvent.moveEvent!.uciMove);
            _eventController.add(MatchEvent(
              type: MatchEventType.movePlayed,
              moveUci: uciEvent.moveEvent!.uciMove,
            ));
          }
          break;
        case UciEventType.matchFinished:
          _eventController.add(MatchEvent(type: MatchEventType.matchFinished));
          _isActive = false;
          controller.setMatchStatus(MatchStatus.finished);
          break;
        case UciEventType.engineError:
          _eventController.add(MatchEvent(
            type: MatchEventType.error,
            message: uciEvent.message,
          ));
          break;
        default:
          break;
      }
    });
    
    await _uciOrchestrator!.startMatch(
      engine1Path: engine1Path,
      engine2Path: engine2Path,
      engine1Options: engine1Options,
      engine2Options: engine2Options,
    );
  }
  
  /// Start testing mode match (cutechess + PGN replay)
  Future<void> _startTestingMatch({
    required String engine1,
    required int stockfishSkill,
    required int rounds,
    String? projectRoot,
  }) async {
    _cutechessManager = CutechessManager();
    final pgnPath = '${projectRoot ?? '/Users/justin/VSCODE PROJECTS/chess_ui'}/logs/${engine1}_vs_sf$stockfishSkill.pgn';
    _pgnWatcher = PGNWatcher(pgnPath);
    _pgnReplayer = PgnReplayer(controller);
    _pgnReplayer!.setMoveDelay(_replayDelay);
    
    // Listen to cutechess events
    _cutechessManager!.events.listen((event) {
      if (event is GameStartedEvent) {
        _eventController.add(MatchEvent(
          type: MatchEventType.gameStarted,
          gameNumber: event.gameNumber,
        ));
      } else if (event is GameFinishedEvent) {
        _eventController.add(MatchEvent(
          type: MatchEventType.gameFinished,
          gameNumber: event.gameNumber,
          result: event.result,
        ));
      } else if (event is MatchCompleteEvent) {
        _eventController.add(MatchEvent(type: MatchEventType.matchFinished));
        _isActive = false;
        controller.setMatchStatus(MatchStatus.finished);
      }
    });
    
    // Listen to PGN watcher for completed games
    _pgnWatcher!.moves.listen((gameMoves) {
      // Replay the completed game
      _pgnReplayer!.replayGame(gameMoves.moves);
    });
    
    // Start cutechess match
    await _cutechessManager!.startMatch(
      engineName: engine1,
      stockfishSkill: stockfishSkill,
      projectRoot: projectRoot,
    );
    
    // Start PGN watcher
    _pgnWatcher!.start();
  }
  
  /// Apply a UCI move to the board
  void _applyUciMove(String uciMove) {
    try {
      final move = MoveParser.parseUci(
        uciMove,
        controller.board,
        controller.engine,
      );
      controller.makeMove(move);
    } catch (e) {
      _eventController.add(MatchEvent(
        type: MatchEventType.error,
        message: 'Failed to parse move "$uciMove": $e',
      ));
    }
  }
  
  /// Stop the current match
  Future<void> stopMatch() async {
    if (!_isActive) return;
    
    _isActive = false;
    
    if (_currentMode == MatchMode.visualization) {
      await _uciOrchestrator?.stopMatch();
      _uciOrchestrator?.dispose();
      _uciOrchestrator = null;
    } else {
      await _pgnReplayer?.stopReplay();
      _pgnWatcher?.stop();
      await _cutechessManager?.stop();
      _pgnReplayer?.dispose();
      _pgnReplayer = null;
      _pgnWatcher = null;
      _cutechessManager = null;
    }
    
    controller.setMatchStatus(MatchStatus.idle);
  }
  
  /// Set replay delay for testing mode
  void setReplayDelay(Duration delay) {
    _replayDelay = delay;
    _pgnReplayer?.setMoveDelay(delay);
  }
  
  /// Pause replay (testing mode only)
  void pauseReplay() {
    _pgnReplayer?.pauseReplay();
  }
  
  /// Resume replay (testing mode only)
  void resumeReplay() {
    _pgnReplayer?.resumeReplay();
  }
  
  /// Check if match is active
  bool get isActive => _isActive;
  
  /// Get current match mode
  MatchMode get currentMode => _currentMode;
  
  /// Dispose resources
  void dispose() {
    stopMatch();
    _eventController.close();
  }
}
