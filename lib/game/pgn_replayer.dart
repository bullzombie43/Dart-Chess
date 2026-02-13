import 'dart:async';
import 'dart:convert';
import 'dart:io';

import 'package:chess_ui/game/chess_engine.dart';
import 'package:chess_ui/game/game_controller.dart';
import 'package:chess_ui/game/move_parser.dart';

/// Events emitted during game replay
enum ReplayEventType {
  replayStarted,
  moveApplied,
  replayPaused,
  replayResumed,
  replayStopped,
  replayFinished,
}

class ReplayEvent {
  final ReplayEventType type;
  final int? moveNumber;
  final String? moveUci;
  final String? rawMove;
  final String? fenBefore;
  final String? fenAfter;
  final String? error;

  ReplayEvent({
    required this.type,
    this.moveNumber,
    this.moveUci,
    this.rawMove,
    this.fenBefore,
    this.fenAfter,
    this.error,
  });
}

/// Replays completed games from PGN files with configurable delay
class PgnReplayer {
  final GameController controller;
  final StreamController<ReplayEvent> _eventController =
      StreamController.broadcast();

  Stream<ReplayEvent> get events => _eventController.stream;

  Duration _moveDelay = const Duration(milliseconds: 500);
  bool _isReplaying = false;
  bool _isPaused = false;
  Completer<void>? _pauseCompleter;
  Timer? _replayTimer;

  PgnReplayer(this.controller);

  // #region agent log
  void _debugLog(
    String hypothesisId,
    String location,
    String message,
    Map<String, dynamic> data,
  ) {
    final payload = <String, dynamic>{
      'id': 'log_${DateTime.now().millisecondsSinceEpoch}_${location.hashCode}',
      'timestamp': DateTime.now().millisecondsSinceEpoch,
      'location': location,
      'message': message,
      'data': data,
      'runId': 'pre-fix',
      'hypothesisId': hypothesisId,
    };

    try {
      final file = File(
        '/Users/justin/VSCODE PROJECTS/chess_ui/.cursor/debug.log',
      );
      file.writeAsStringSync(
        '${jsonEncode(payload)}\n',
        mode: FileMode.append,
        flush: true,
      );
    } catch (_) {
      // Swallow all logging errors to avoid impacting app behavior.
    }
  }
  // #endregion agent log

  /// Set the delay between moves during replay
  void setMoveDelay(Duration delay) {
    _moveDelay = delay;
  }

  /// Replay a game from a list of UCI moves
  ///
  /// Resets the board to starting position, then applies moves with delay
  Future<void> replayGame(List<String> uciMoves) async {
    if (_isReplaying) {
      await stopReplay();
    }

    _isReplaying = true;
    _isPaused = false;

    // Reset board to starting position
    controller.resetGame();
    await Future.delayed(const Duration(milliseconds: 100));

    _debugLog(
      'H2',
      'pgn_replayer.dart:replayGame',
      'after resetGame',
      {
        'startFen': controller.board.getFen(),
        'movesCount': uciMoves.length,
      },
    );

    _eventController.add(ReplayEvent(type: ReplayEventType.replayStarted));

    // Parse all moves first (handles both UCI and algebraic notation)
    // We need to apply moves as we parse to get correct board state for next move
    final moves = <Move>[];
    final tempBoard = ChessBoard(); // Use a temporary board for parsing
    final tempEngine = ChessEngine();

    for (var i = 0; i < uciMoves.length; i++) {
      final moveStr = uciMoves[i];
      final fenBefore = tempBoard.getFen();
      try {
        final move = MoveParser.parseMove(
          moveStr,
          tempBoard,
          tempEngine,
        );
        if (move != null) {
          moves.add(move);
          // Apply move to temp board for next move parsing
          tempBoard.makeMove(move);
          _eventController.add(ReplayEvent(
            type: ReplayEventType.moveApplied,
            moveNumber: moves.length,
            moveUci: move.toUCI(),
            rawMove: moveStr,
            fenBefore: fenBefore,
            fenAfter: tempBoard.getFen(),
          ));
        } else {
          final errorMessage = 'Could not parse move: $moveStr';
          print(errorMessage);
          _eventController.add(ReplayEvent(
            type: ReplayEventType.replayStopped,
            moveNumber: i + 1,
            moveUci: null,
            rawMove: moveStr,
            fenBefore: fenBefore,
            fenAfter: null,
            error: errorMessage,
          ));
        }
      } catch (e) {
        final errorMessage = 'Error parsing move "$moveStr": $e';
        print(errorMessage);
        _eventController.add(ReplayEvent(
          type: ReplayEventType.replayStopped,
          moveNumber: i + 1,
          moveUci: null,
          rawMove: moveStr,
          fenBefore: fenBefore,
          fenAfter: null,
          error: errorMessage,
        ));
      }
    }

    // Clean up temp resources
    tempBoard.dispose();
    tempEngine.dispose();

    // Apply moves with delay
    for (int i = 0; i < moves.length; i++) {
      if (!_isReplaying) break;

      // Wait for pause to be resumed
      while (_isPaused && _isReplaying) {
        if (_pauseCompleter != null) {
          await _pauseCompleter!.future;
        } else {
          await Future.delayed(const Duration(milliseconds: 100));
        }
      }

      if (!_isReplaying) break;

      // Apply the move
      controller.makeMove(moves[i]);

      // Wait before next move (except for last move)
      if (i < moves.length - 1) {
        await Future.delayed(_moveDelay);
      }
    }

    if (_isReplaying) {
      _isReplaying = false;
      _eventController.add(ReplayEvent(type: ReplayEventType.replayFinished));
    }
  }

  /// Replay a game from PGN text
  ///
  /// Extracts moves from PGN and replays them
  Future<void> replayGameFromPgn(String pgnText) async {
    // Extract moves from PGN (similar to PGNWatcher logic)
    final lines = pgnText.split('\n');
    final movetext = lines
        .where((line) => !line.startsWith('[') && line.trim().isNotEmpty)
        .join(' ');

    // Remove move numbers, result markers, and comments
    final cleaned = movetext
        .replaceAll(RegExp(r'\d+\.+'), ' ')
        .replaceAll(RegExp(r'\{[^}]*\}'), ' ')
        .replaceAll(RegExp(r'1-0|0-1|1/2-1/2|\*'), ' ')
        .trim();

    // Split into individual moves
    final moves =
        cleaned.split(RegExp(r'\s+')).where((m) => m.isNotEmpty).toList();

    await replayGame(moves);
  }

  /// Pause the current replay
  void pauseReplay() {
    if (_isReplaying && !_isPaused) {
      _isPaused = true;
      _pauseCompleter = Completer<void>();
      _eventController.add(ReplayEvent(type: ReplayEventType.replayPaused));
    }
  }

  /// Resume a paused replay
  void resumeReplay() {
    if (_isReplaying && _isPaused) {
      _isPaused = false;
      _pauseCompleter?.complete();
      _pauseCompleter = null;
      _eventController.add(ReplayEvent(type: ReplayEventType.replayResumed));
    }
  }

  /// Stop the current replay
  Future<void> stopReplay() async {
    if (_isReplaying) {
      _isReplaying = false;
      _isPaused = false;
      _pauseCompleter?.complete();
      _pauseCompleter = null;
      _replayTimer?.cancel();
      _replayTimer = null;
      _eventController.add(ReplayEvent(type: ReplayEventType.replayStopped));
    }
  }

  /// Check if a replay is currently in progress
  bool get isReplaying => _isReplaying;

  /// Check if replay is paused
  bool get isPaused => _isPaused;

  /// Dispose resources
  void dispose() {
    stopReplay();
    _eventController.close();
  }
}
