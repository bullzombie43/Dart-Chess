import 'dart:async';
import 'dart:convert';
import 'dart:io';

/// Events from UCI orchestrator
enum UciEventType {
  engineReady,
  movePlayed,
  engineError,
  matchStarted,
  matchFinished,
}

class UciMoveEvent {
  final String uciMove;
  final bool isWhite;
  final int moveNumber;
  
  UciMoveEvent({
    required this.uciMove,
    required this.isWhite,
    required this.moveNumber,
  });
}

class UciEvent {
  final UciEventType type;
  final String? message;
  final UciMoveEvent? moveEvent;
  
  UciEvent({
    required this.type,
    this.message,
    this.moveEvent,
  });
}

/// Direct UCI protocol communication for real-time engine matches
class UciOrchestrator {
  Process? _engine1Process;
  Process? _engine2Process;
  final StreamController<UciEvent> _eventController = StreamController.broadcast();
  
  Stream<UciEvent> get events => _eventController.stream;
  
  Map<String, String> _engine1Options = {};
  Map<String, String> _engine2Options = {};
  
  bool _isMatchActive = false;
  int _moveNumber = 0;
  bool _whiteToMove = true;
  String _currentPosition = 'startpos';
  List<String> _moveHistory = [];
  
  /// Start a match between two engines
  /// 
  /// [engine1Path] and [engine2Path] are paths to engine executables
  /// [engine1Options] and [engine2Options] are UCI options (e.g., {"Skill Level": "5"})
  Future<void> startMatch({
    required String engine1Path,
    required String engine2Path,
    Map<String, String>? engine1Options,
    Map<String, String>? engine2Options,
  }) async {
    if (_isMatchActive) {
      await stopMatch();
    }
    
    _engine1Options = engine1Options ?? {};
    _engine2Options = engine2Options ?? {};
    _moveNumber = 0;
    _whiteToMove = true;
    _currentPosition = 'startpos';
    _moveHistory = [];
    
    try {
      // Start both engines
      _engine1Process = await Process.start(engine1Path, [], runInShell: true);
      _engine2Process = await Process.start(engine2Path, [], runInShell: true);
      
      // Initialize engines
      await _initializeEngine(_engine1Process!, 1);
      await _initializeEngine(_engine2Process!, 2);
      
      _isMatchActive = true;
      _eventController.add(UciEvent(type: UciEventType.matchStarted));
      
      // Start the game - white (engine1) moves first
      await _requestMove(_engine1Process!, true);
      
    } catch (e) {
      _eventController.add(UciEvent(
        type: UciEventType.engineError,
        message: 'Failed to start match: $e',
      ));
      await stopMatch();
      rethrow;
    }
  }
  
  /// Initialize an engine with UCI protocol
  Future<void> _initializeEngine(Process process, int engineNum) async {
    final options = engineNum == 1 ? _engine1Options : _engine2Options;
    
    // Send UCI command
    process.stdin.writeln('uci');
    
    // Wait for uciok
    await _waitForResponse(process, 'uciok');
    
    // Set options
    for (final entry in options.entries) {
      process.stdin.writeln('setoption name ${entry.key} value ${entry.value}');
    }
    
    // Wait for readyok
    process.stdin.writeln('isready');
    await _waitForResponse(process, 'readyok');
    
    _eventController.add(UciEvent(
      type: UciEventType.engineReady,
      message: 'Engine $engineNum ready',
    ));
    
    // Start new game
    process.stdin.writeln('ucinewgame');
  }
  
  /// Wait for a specific response from engine
  Future<void> _waitForResponse(Process process, String expected) async {
    final completer = Completer<void>();
    late StreamSubscription subscription;
    final buffer = StringBuffer();
    
    subscription = process.stdout
        .transform(utf8.decoder)
        .transform(const LineSplitter())
        .listen((line) {
      final trimmed = line.trim();
      buffer.writeln(trimmed);
      
      // Check for expected response
      if (trimmed == expected) {
        subscription.cancel();
        completer.complete();
      }
      // Also check for error responses
      else if (trimmed.toLowerCase().contains('error') || 
               trimmed.toLowerCase().contains('unknown')) {
        subscription.cancel();
        completer.completeError(Exception('Engine error: $trimmed'));
      }
    });
    
    // Timeout after 10 seconds (increased for slower engines)
    await completer.future.timeout(
      const Duration(seconds: 10),
      onTimeout: () {
        subscription.cancel();
        throw TimeoutException(
          'Engine did not respond with $expected. Output so far:\n${buffer.toString()}'
        );
      },
    );
  }
  
  /// Request a move from an engine
  Future<void> _requestMove(Process process, bool isWhite) async {
    // Build position command
    String positionCmd = 'position $_currentPosition';
    if (_moveHistory.isNotEmpty) {
      positionCmd += ' moves ${_moveHistory.join(' ')}';
    }
    process.stdin.writeln(positionCmd);
    
    // Request move (with time control - adjust as needed)
    process.stdin.writeln('go movetime 1000'); // 1 second per move
    
    // Listen for bestmove
    late StreamSubscription subscription;
    subscription = process.stdout
        .transform(utf8.decoder)
        .transform(const LineSplitter())
        .listen((line) {
      if (line.startsWith('bestmove')) {
        subscription.cancel();
        _handleBestMove(line, isWhite);
      }
    });
  }
  
  /// Handle bestmove response from engine
  void _handleBestMove(String line, bool isWhite) {
    // Parse "bestmove e2e4" or "bestmove e2e4 ponder e7e5"
    final parts = line.split(' ');
    if (parts.length < 2) {
      _eventController.add(UciEvent(
        type: UciEventType.engineError,
        message: 'Invalid bestmove response: $line',
      ));
      return;
    }
    
    final move = parts[1];
    if (move == 'none' || move == '(none)') {
      // Game over
      _eventController.add(UciEvent(type: UciEventType.matchFinished));
      return;
    }
    
    // Add move to history
    _moveHistory.add(move);
    _moveNumber++;
    
    // Emit move event
    _eventController.add(UciEvent(
      type: UciEventType.movePlayed,
      moveEvent: UciMoveEvent(
        uciMove: move,
        isWhite: isWhite,
        moveNumber: _moveNumber,
      ),
    ));
    
    // Switch to other engine
    _whiteToMove = !_whiteToMove;
    final nextEngine = _whiteToMove ? _engine1Process : _engine2Process;
    
    if (nextEngine != null && _isMatchActive) {
      _requestMove(nextEngine, _whiteToMove);
    }
  }
  
  /// Stop the current match
  Future<void> stopMatch() async {
    _isMatchActive = false;
    
    // Send quit to engines
    _engine1Process?.stdin.writeln('quit');
    _engine2Process?.stdin.writeln('quit');
    
    // Wait for processes to exit
    await Future.wait([
      _engine1Process?.exitCode ?? Future.value(0),
      _engine2Process?.exitCode ?? Future.value(0),
    ]);
    
    _engine1Process = null;
    _engine2Process = null;
    
    _eventController.add(UciEvent(type: UciEventType.matchFinished));
  }
  
  /// Check if match is active
  bool get isMatchActive => _isMatchActive;
  
  /// Dispose resources
  void dispose() {
    stopMatch();
    _eventController.close();
  }
}
