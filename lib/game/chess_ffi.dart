import 'dart:ffi' as ffi;
import 'dart:io' show Platform, Directory;

import 'package:path/path.dart' as path;

// ============================================================================
// LOAD NATIVE LIBRARY
// ============================================================================

ffi.DynamicLibrary _loadLibrary() {
  if (Platform.isMacOS) {
    // In production (Flutter app), library is in the app bundle
    return ffi.DynamicLibrary.open('libchess_bridge.dylib');
  } else if (Platform.isAndroid) {
    return ffi.DynamicLibrary.open('libchess_bridge.so');
  } else if (Platform.isLinux) {
    return ffi.DynamicLibrary.open('libchess_bridge.so');
  } else if (Platform.isWindows) {
    return ffi.DynamicLibrary.open('chess_bridge.dll');
  } else if (Platform.isIOS) {
    return ffi.DynamicLibrary.process();
  }
  
  throw UnsupportedError('Platform ${Platform.operatingSystem} not supported');
}

final _nativeLib = _loadLibrary();

// ============================================================================
// C TYPES (matching chess_bridge.h)
// ============================================================================

// Opaque pointer types
typedef ChessBoardHandle = ffi.Pointer<ffi.Void>;
typedef ChessEngineHandle = ffi.Pointer<ffi.Void>;

// CMove struct (must match C layout exactly!)
final class CMove extends ffi.Struct {
  @ffi.Uint8()
  external int piece;

  @ffi.Uint8()
  external int fromSquare;

  @ffi.Uint8()
  external int toSquare;

  @ffi.Uint8()
  external int capturedPiece;

  @ffi.Uint8()
  external int promotedPiece;

  @ffi.Uint8()
  external int isEnPassant;

  @ffi.Uint8()
  external int isCastling;
}

// ============================================================================
// C FUNCTION SIGNATURES
// ============================================================================

// Engine functions
typedef ChessEngineCreateNative = ChessEngineHandle Function();
typedef ChessEngineCreateDart = ChessEngineHandle Function();

typedef ChessEngineDestroyNative = ffi.Void Function(ChessEngineHandle);
typedef ChessEngineDestroyDart = void Function(ChessEngineHandle);

typedef ChessEngineGenerateLegalMovesNative = ffi.Int32 Function(
  ChessEngineHandle,
  ChessBoardHandle,
  ffi.Pointer<CMove>,
  ffi.Int32,
);
typedef ChessEngineGenerateLegalMovesDart = int Function(
  ChessEngineHandle,
  ChessBoardHandle,
  ffi.Pointer<CMove>,
  int,
);

// Board functions
typedef ChessBoardCreateNative = ChessBoardHandle Function();
typedef ChessBoardCreateDart = ChessBoardHandle Function();

typedef ChessBoardCreateFromFenNative = ChessBoardHandle Function(
    ffi.Pointer<ffi.Char>);
typedef ChessBoardCreateFromFenDart = ChessBoardHandle Function(
    ffi.Pointer<ffi.Char>);

typedef ChessBoardDestroyNative = ffi.Void Function(ChessBoardHandle);
typedef ChessBoardDestroyDart = void Function(ChessBoardHandle);

typedef ChessBoardGetFenNative = ffi.Pointer<ffi.Char> Function(
    ChessBoardHandle);
typedef ChessBoardGetFenDart = ffi.Pointer<ffi.Char> Function(
    ChessBoardHandle);

typedef ChessBoardSetFenNative = ffi.Uint8 Function(
  ChessBoardHandle,
  ffi.Pointer<ffi.Char>,
);
typedef ChessBoardSetFenDart = int Function(
  ChessBoardHandle,
  ffi.Pointer<ffi.Char>,
);

typedef ChessBoardGetPieceAtNative = ffi.Uint8 Function(
  ChessBoardHandle,
  ffi.Int32,
);
typedef ChessBoardGetPieceAtDart = int Function(ChessBoardHandle, int);

typedef ChessBoardGetSideToMoveNative = ffi.Uint8 Function(ChessBoardHandle);
typedef ChessBoardGetSideToMoveDart = int Function(ChessBoardHandle);

typedef ChessBoardIsInCheckNative = ffi.Uint8 Function(ChessBoardHandle);
typedef ChessBoardIsInCheckDart = int Function(ChessBoardHandle);

typedef ChessBoardMakeMoveNative = ffi.Void Function(
  ChessBoardHandle,
  ffi.Pointer<CMove>,
);
typedef ChessBoardMakeMoveDart = void Function(
  ChessBoardHandle,
  ffi.Pointer<CMove>,
);

typedef ChessBoardUndoMoveNative = ffi.Void Function(ChessBoardHandle);
typedef ChessBoardUndoMoveDart = void Function(ChessBoardHandle);

// Utility functions
typedef ChessMoveToStringNative = ffi.Pointer<ffi.Char> Function(
    ffi.Pointer<CMove>);
typedef ChessMoveToStringDart = ffi.Pointer<ffi.Char> Function(
    ffi.Pointer<CMove>);

typedef ChessFreeStringNative = ffi.Void Function(ffi.Pointer<ffi.Char>);
typedef ChessFreeStringDart = void Function(ffi.Pointer<ffi.Char>);

typedef ChessPerftNative = ffi.Uint64 Function(ChessEngineHandle, ChessBoardHandle, ffi.Int32);
typedef ChessPerftDart = int Function(ChessEngineHandle, ChessBoardHandle, int);

// ============================================================================
// DART BINDINGS (looked up from native library)
// ============================================================================

final chessEngineCreate = _nativeLib
    .lookup<ffi.NativeFunction<ChessEngineCreateNative>>('engine_create')
    .asFunction<ChessEngineCreateDart>();

final chessEngineDestroy = _nativeLib
    .lookup<ffi.NativeFunction<ChessEngineDestroyNative>>(
        'engine_destroy')
    .asFunction<ChessEngineDestroyDart>();

final chessEngineGenerateLegalMoves = _nativeLib
    .lookup<ffi.NativeFunction<ChessEngineGenerateLegalMovesNative>>(
        'engine_generate_legal_moves')
    .asFunction<ChessEngineGenerateLegalMovesDart>();

final chessBoardCreate = _nativeLib
    .lookup<ffi.NativeFunction<ChessBoardCreateNative>>('board_create')
    .asFunction<ChessBoardCreateDart>();

final chessBoardCreateFromFen = _nativeLib
    .lookup<ffi.NativeFunction<ChessBoardCreateFromFenNative>>(
        'board_create_from_fen')
    .asFunction<ChessBoardCreateFromFenDart>();

final chessBoardDestroy = _nativeLib
    .lookup<ffi.NativeFunction<ChessBoardDestroyNative>>(
        'board_destroy')
    .asFunction<ChessBoardDestroyDart>();

final chessBoardGetFen = _nativeLib
    .lookup<ffi.NativeFunction<ChessBoardGetFenNative>>('board_get_fen')
    .asFunction<ChessBoardGetFenDart>();

final chessBoardSetFen = _nativeLib
    .lookup<ffi.NativeFunction<ChessBoardSetFenNative>>('board_set_fen')
    .asFunction<ChessBoardSetFenDart>();

final chessBoardGetPieceAt = _nativeLib
    .lookup<ffi.NativeFunction<ChessBoardGetPieceAtNative>>(
        'board_get_piece_at')
    .asFunction<ChessBoardGetPieceAtDart>();

final chessBoardGetSideToMove = _nativeLib
    .lookup<ffi.NativeFunction<ChessBoardGetSideToMoveNative>>(
        'board_get_side_to_move')
    .asFunction<ChessBoardGetSideToMoveDart>();

final chessBoardIsInCheck = _nativeLib
    .lookup<ffi.NativeFunction<ChessBoardIsInCheckNative>>(
        'board_is_in_check')
    .asFunction<ChessBoardIsInCheckDart>();

final chessBoardMakeMove = _nativeLib
    .lookup<ffi.NativeFunction<ChessBoardMakeMoveNative>>(
        'board_make_move')
    .asFunction<ChessBoardMakeMoveDart>();

final chessBoardUndoMove = _nativeLib
    .lookup<ffi.NativeFunction<ChessBoardUndoMoveNative>>(
        'board_undo_move')
    .asFunction<ChessBoardUndoMoveDart>();

final chessMoveToString = _nativeLib
    .lookup<ffi.NativeFunction<ChessMoveToStringNative>>(
        'chess_move_to_string')
    .asFunction<ChessMoveToStringDart>();

final chessFreeString = _nativeLib
    .lookup<ffi.NativeFunction<ChessFreeStringNative>>('chess_free_string')
    .asFunction<ChessFreeStringDart>();

final chessPerft = _nativeLib
    .lookup<ffi.NativeFunction<ChessPerftNative>>('chess_perft')
    .asFunction<ChessPerftDart>();

// ============================================================================
// CONSTANTS (matching C constants)
// ============================================================================

const int colorWhite = 1;
const int colorBlack = 2;

const int pieceWPawn = 0;
const int pieceWKnight = 1;
const int pieceWBishop = 2;
const int pieceWRook = 3;
const int pieceWQueen = 4;
const int pieceWKing = 5;
const int pieceBPawn = 6;
const int pieceBKnight = 7;
const int pieceBBishop = 8;
const int pieceBRook = 9;
const int pieceBQueen = 10;
const int pieceBKing = 11;
const int pieceNone = 12;

const int maxLegalMoves = 256;

