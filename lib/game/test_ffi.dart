import 'chess_ffi.dart';
import 'dart:ffi' as ffi;
import 'package:ffi/ffi.dart';

void testChessFFI() {
  print('Testing Chess FFI...');
  
  // Create engine
  final engine = chessEngineCreate();
  print('✓ Engine created: $engine');
  
  // Create board
  final board = chessBoardCreate();
  print('✓ Board created: $board');
  
  // Get FEN
  final fenPtr = chessBoardGetFen(board);
  final fen = fenPtr.cast<Utf8>().toDartString();
  print('✓ Starting FEN: $fen');
  chessFreeString(fenPtr);
  
  // Get side to move
  final side = chessBoardGetSideToMove(board);
  print('✓ Side to move: ${side == colorWhite ? "White" : "Black"}');
  
  // Generate moves
  final movesPtr = calloc<CMove>(maxLegalMoves);
  final count = chessEngineGenerateLegalMoves(engine, board, movesPtr, maxLegalMoves);
  print('✓ Generated $count legal moves');
  
  // Print first move
  if (count > 0) {
    final moveStr = chessMoveToString(movesPtr);
    print('✓ First move: ${moveStr.cast<Utf8>().toDartString()}');
    chessFreeString(moveStr);
  }
  
  // Cleanup
  calloc.free(movesPtr);
  chessBoardDestroy(board);
  chessEngineDestroy(engine);
  
  print('✓ All tests passed!');
}

void main(){
  testChessFFI();
}