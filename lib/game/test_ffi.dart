import 'package:chess_ui/game/chess_engine.dart';

void testChessFFI() {
  print('\n=== Testing Chess Engine ===\n');

  // Create engine and board
  final engine = ChessEngine();
  final board = ChessBoard();

  print('✓ Starting position:');
  print('  FEN: ${board.getFen()}');
  print('  Side to move: ${board.getSideToMove()}');
  print('  In check: ${board.isInCheck()}');

  // Generate moves
  final moves = engine.generateLegalMoves(board);
  print('\n✓ Generated ${moves.length} legal moves');
  print('  First 5 moves: ${moves.take(5).map((m) => m.toUCI()).join(", ")}');

  // Make a move (e2e4)
  final e2e4 = moves.firstWhere((m) => m.fromSquare == 12 && m.toSquare == 28);
  print('\n✓ Making move: ${e2e4.toUCI()}');
  board.makeMove(e2e4);

  print('  New FEN: ${board.getFen()}');
  print('  Side to move: ${board.getSideToMove()}');

  // Generate black's moves
  final blackMoves = engine.generateLegalMoves(board);
  print('  Black has ${blackMoves.length} legal moves');

  // Undo
  board.undoMove();
  print('\n✓ After undo: ${board.getFen()}');

  // Test FEN loading
  print('\n✓ Testing FEN loading...');
  final board2 = ChessBoard.fromFen('rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1');
  print('  Loaded: ${board2.getFen()}');
  print('  Side to move: ${board2.getSideToMove()}');

  // Perft test
  print('\n✓ Running perft tests...');
  final totalStopWatch = Stopwatch()..start();
  for (int depth = 1; depth <= 6; depth++) {
    final stopwatch = Stopwatch()..start();
    final nodes = engine.perft(board, depth);
    stopwatch.stop();

    final seconds = stopwatch.elapsedMilliseconds / 1000.0;
    print('  Perft($depth) = $nodes');
    print("  Time:  ${seconds.toStringAsFixed(3)} s");
    print("  Speed: ${(nodes / seconds).toStringAsFixed(0)} nodes/s\n");
  }
  totalStopWatch.stop();

  final totalSeconds = totalStopWatch.elapsedMilliseconds / 1000.0;
  print("");
  print(" Total Time:  ${totalSeconds.toStringAsFixed(3)} s");


  // Cleanup
  board.dispose();
  board2.dispose();
  engine.dispose();

  print('\n=== All tests passed! ===\n');
}

void main(){
  testChessFFI();
}