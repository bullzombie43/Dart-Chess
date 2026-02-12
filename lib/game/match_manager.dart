import 'dart:async';
import 'dart:io';
import 'package:chess_ui/game/cutechess_manager.dart';
import 'package:chess_ui/game/engine_path_resolver.dart';
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
  EnginePathResolver? _pathResolver;
  
  bool _isActive = false;
  Duration _replayDelay = const Duration(milliseconds: 500);
  
  MatchManager(this.controller, {String? projectRoot}) {
    _pathResolver = EnginePathResolver(projectRoot: projectRoot);
  }
  
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
          engine2: engine2,
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
    // Resolve engine paths if they're names
    final resolvedPath1 = _pathResolver?.resolveEnginePath(engine1Path) ?? engine1Path;
    final resolvedPath2 = _pathResolver?.resolveEnginePath(engine2Path) ?? engine2Path;
    
    // Validate engine paths
    if (!(_pathResolver?.isEngineValid(resolvedPath1) ?? true)) {
      throw Exception('Engine 1 path is invalid or not executable: $resolvedPath1');
    }
    if (!(_pathResolver?.isEngineValid(resolvedPath2) ?? true)) {
      throw Exception('Engine 2 path is invalid or not executable: $resolvedPath2');
    }
    
    // Get engine options if not provided
    final options1 = engine1Options ?? _pathResolver?.getEngineOptions(engine1Path) ?? {};
    final options2 = engine2Options ?? _pathResolver?.getEngineOptions(engine2Path) ?? {};
    
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
      engine1Path: resolvedPath1,
      engine2Path: resolvedPath2,
      engine1Options: options1,
      engine2Options: options2,
    );
  }
  
  /// Start testing mode match (cutechess + PGN replay)
  Future<void> _startTestingMatch({
    required String engine1,
    required String engine2,
    required int stockfishSkill,
    required int rounds,
    String? projectRoot,
  }) async {
    _cutechessManager = CutechessManager();
    
    // Resolve engine paths and options for both sides
    final resolver = _pathResolver ?? EnginePathResolver(projectRoot: projectRoot);
    final resolvedPath1 = resolver.resolveEnginePath(engine1);
    final resolvedPath2 = resolver.resolveEnginePath(engine2);

    if (!resolver.isEngineValid(resolvedPath1)) {
      throw Exception('Engine 1 path is invalid or not executable: $resolvedPath1');
    }
    if (!resolver.isEngineValid(resolvedPath2)) {
      throw Exception('Engine 2 path is invalid or not executable: $resolvedPath2');
    }

    final engine1Options = resolver.getEngineOptions(engine1);
    final engine2Options = resolver.getEngineOptions(engine2);

    // Prepare unique PGN/log file paths per test run
    final root = projectRoot ?? resolver.projectRoot;
    final logsDir = '$root/logs';
    await Directory(logsDir).create(recursive: true);

    final baseName = _buildLogBaseName(engine1, engine2);
    final pgnPath = '$logsDir/$baseName.pgn';
    final logPath = '$logsDir/$baseName.log';

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
    
    // Start cutechess match with both selected engines
    await _cutechessManager!.startEngineVsEngineMatch(
      whiteName: engine1,
      whiteCommand: resolvedPath1!,
      whiteOptions: engine1Options,
      blackName: engine2,
      blackCommand: resolvedPath2!,
      blackOptions: engine2Options,
      pgnPath: pgnPath,
      logPath: logPath,
      rounds: rounds,
      projectRoot: root,
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

  /// Build a safe, unique base name for PGN/log files based on engine names.
  String _buildLogBaseName(String engine1, String engine2) {
    String sanitize(String name) {
      final replaced = name.replaceAll(RegExp(r'[^a-zA-Z0-9]+'), '_');
      return replaced.replaceAll(RegExp(r'^_+|_+$'), '');
    }

    final safe1 = sanitize(engine1);
    final safe2 = sanitize(engine2);
    final timestamp = DateTime.now()
        .toIso8601String()
        .replaceAll(':', '-')
        .replaceAll('.', '-');

    return '${safe1}_vs_${safe2}_$timestamp';
  }
}
