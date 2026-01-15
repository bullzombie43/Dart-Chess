import 'dart:async';
import 'dart:io';

class PGNWatcher {
  final String pgnPath;
  Timer? _timer;
  int _lastGameCount = 0;
  
  final StreamController<GameMoves> _movesController = StreamController.broadcast();
  Stream<GameMoves> get moves => _movesController.stream;
  
  PGNWatcher(this.pgnPath);
  
  void start() {
    _timer = Timer.periodic(const Duration(milliseconds: 500), (_) => _checkFile());
  }
  
  Future<void> _checkFile() async {
    final file = File(pgnPath);
    if (!await file.exists()) return;
    
    try {
      final content = await file.readAsString();
      final games = _parseAllGames(content);
      
      if (games.length > _lastGameCount) {
        _lastGameCount = games.length;
        final lastGame = games.last;
        final moves = _extractMoves(lastGame);
        final result = _extractResult(lastGame);
        
        _movesController.add(GameMoves(
          gameNumber: games.length,
          moves: moves,
          result: result,
        ));
      }
    } catch (e) {
      print('Error reading PGN: $e');
    }
  }
  
  List<String> _parseAllGames(String content) {
    // Split on [Event but keep the delimiter
    final games = <String>[];
    final lines = content.split('\n');
    StringBuffer currentGame = StringBuffer();
    
    for (final line in lines) {
      if (line.startsWith('[Event ') && currentGame.isNotEmpty) {
        games.add(currentGame.toString());
        currentGame = StringBuffer();
      }
      currentGame.writeln(line);
    }
    
    if (currentGame.isNotEmpty) {
      games.add(currentGame.toString());
    }
    
    return games.where((g) => g.trim().isNotEmpty).toList();
  }
  
  List<String> _extractMoves(String pgn) {
    final lines = pgn.split('\n');
    
    // Find movetext (lines that don't start with [ and aren't empty)
    final movetext = lines
        .where((line) => !line.startsWith('[') && line.trim().isNotEmpty)
        .join(' ');
    
    // Remove move numbers (1. 2. etc), result markers, and extra whitespace
    final cleaned = movetext
        .replaceAll(RegExp(r'\d+\.+'), ' ')
        .replaceAll(RegExp(r'\{[^}]*\}'), ' ') // Remove comments
        .replaceAll(RegExp(r'1-0|0-1|1/2-1/2|\*'), ' ')
        .trim();
    
    // Split into individual moves
    return cleaned
        .split(RegExp(r'\s+'))
        .where((m) => m.isNotEmpty)
        .toList();
  }
  
  String _extractResult(String pgn) {
    final resultMatch = RegExp(r'\[Result "([^"]+)"\]').firstMatch(pgn);
    return resultMatch?.group(1) ?? '*';
  }
  
  void stop() {
    _timer?.cancel();
    _movesController.close();
  }
}

class GameMoves {
  final int gameNumber;
  final List<String> moves;
  final String result;
  
  GameMoves({
    required this.gameNumber,
    required this.moves,
    required this.result,
  });
  
  @override
  String toString() => 'Game $gameNumber: ${moves.length} moves, result: $result';
}