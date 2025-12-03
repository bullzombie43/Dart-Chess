import 'dart:ffi' as ffi;
import 'package:ffi/ffi.dart';
import 'package:chess_ui/game/chess_ffi.dart' as ffi_bindings;

class Move {
  final int piece;
  final int fromSquare;
  final int toSquare;
  final int capturedPiece;
  final int promotedPiece;
  final bool isEnPassant;
  final bool isCastling;

  Move({
    required this.piece,
    required this.fromSquare,
    required this.toSquare,
    required this.capturedPiece,
    required this.promotedPiece,
    required this.isEnPassant,
    required this.isCastling,
  });

  // Convert from C struct
  factory Move.fromCMove(ffi_bindings.CMove cmove) {
    return Move(
      piece: cmove.piece,
      fromSquare: cmove.fromSquare,
      toSquare: cmove.toSquare,
      capturedPiece: cmove.capturedPiece,
      promotedPiece: cmove.promotedPiece,
      isEnPassant: cmove.isEnPassant != 0,
      isCastling: cmove.isCastling != 0,
    );
  }

  ffi.Pointer<ffi_bindings.CMove> _toCMove(){
    final ptr = calloc<ffi_bindings.CMove>();
    ptr.ref.piece = piece;
    ptr.ref.fromSquare = fromSquare;
    ptr.ref.toSquare = toSquare;
    ptr.ref.capturedPiece = capturedPiece;
    ptr.ref.promotedPiece = promotedPiece;
    ptr.ref.isEnPassant = isEnPassant ? 1 : 0;
    ptr.ref.isCastling = isCastling ? 1 : 0;

    return ptr;
  }

  String toUCI(){
    final cmove = _toCMove();
    final cptr = ffi_bindings.chessMoveToString(cmove);
    final uci = cptr.cast<Utf8>().toDartString(); //cast from a char* basically

    ffi_bindings.chessFreeString(cptr); //Free everything so no leaks
    calloc.free(cmove);

    return uci;
  }

  @override
  String toString() => toUCI();
}

enum Color{
  white(ffi_bindings.colorWhite),
  black(ffi_bindings.colorBlack);

  final int value;
  const Color(this.value);

  static Color fromValue(int value){
    return values.firstWhere((color) => color.value == value);
  }
}

enum PieceType{
  wPawn(ffi_bindings.pieceWPawn, 'P'),
  wKnight(ffi_bindings.pieceWKnight, 'N'),
  wBishop(ffi_bindings.pieceWBishop, 'B'),
  wRook(ffi_bindings.pieceWRook, 'R'),
  wQueen(ffi_bindings.pieceWQueen, 'Q'),
  wKing(ffi_bindings.pieceWKing, 'K'),
  bPawn(ffi_bindings.pieceBPawn, 'p'),
  bKnight(ffi_bindings.pieceBKnight, 'n'),
  bBishop(ffi_bindings.pieceBBishop, 'b'),
  bRook(ffi_bindings.pieceBRook, 'r'),
  bQueen(ffi_bindings.pieceBQueen, 'q'),
  bKing(ffi_bindings.pieceBKing, 'k'),
  none(ffi_bindings.pieceNone, '.');

  final int value;
  final String symbol;
  const PieceType(this.value, this.symbol);

  static PieceType fromValue(int value) {
    return values.firstWhere((p) => p.value == value, orElse: () => PieceType.none);
  }

  bool get isWhite => value >= ffi_bindings.pieceWPawn && value <= ffi_bindings.pieceWKing;
  bool get isBlack => value >= ffi_bindings.pieceBPawn && value <= ffi_bindings.pieceBKing;

  bool get isPawn => value == ffi_bindings.pieceWPawn || value == ffi_bindings.pieceBPawn;
  bool get isKnight => value == ffi_bindings.pieceWKnight || value == ffi_bindings.pieceBKnight;
  bool get isBishop => value == ffi_bindings.pieceWBishop || value == ffi_bindings.pieceBBishop;
  bool get isRook => value == ffi_bindings.pieceWRook || value == ffi_bindings.pieceBRook;
  bool get isQueen => value == ffi_bindings.pieceWQueen || value == ffi_bindings.pieceBQueen;
  bool get isKing => value == ffi_bindings.pieceWKing || value == ffi_bindings.pieceBKing;
}

class ChessBoard{
  late ffi_bindings.ChessBoardHandle _handle;
  bool _disposed = false;

  /// Create a new board in starting position
  ChessBoard(){
    _handle = ffi_bindings.chessBoardCreate();
  }

  /// Create a board from FEN string
  ChessBoard.fromFen(String fen){
    final fenPtr = fen.toNativeUtf8(); //basicall just casting to char*
    _handle = ffi_bindings.chessBoardCreateFromFen(fenPtr.cast());
    malloc.free(fenPtr);

    //I think we actually throw an exception in the c bridge rather than returning nullptr but wtv
    //TODO: SWITCH TO RETURNING NULLPTR AND ERROR CODES
    if (_handle == ffi.nullptr) { 
      throw ArgumentError('Invalid FEN string: $fen');
    }
  }

  void _checkDisposed() {
    if (_disposed) {
      throw StateError('ChessEngine has been disposed');
    }
  }

  /// Get current position as FEN string
  String getFen(){
    final cPtr = ffi_bindings.chessBoardGetFen(_handle);
    final fen = cPtr.cast<Utf8>().toDartString();
    ffi_bindings.chessFreeString(cPtr);

    return fen;
  }

  /// Set position from FEN string
  void setFen(String fen){
    _checkDisposed();
    final fenPtr = fen.toNativeUtf8();
    final success = ffi_bindings.chessBoardSetFen(_handle, fenPtr.cast());
    malloc.free(fenPtr);

    if(success == 0){ //We don't actually return error codes but just throw so this is useless for now
      throw ArgumentError('Invalid FEN string: $fen');
    }
  }

  /// Get piece at a square (0-63)
  PieceType getPieceAt(int square){
    _checkDisposed();

    if (square < 0 || square > 63) {
      throw RangeError('Square must be 0-63, got $square'); //Just check here for extra safety + better errors
    }

    final piece = ffi_bindings.chessBoardGetPieceAt(_handle, square);
    return PieceType.fromValue(piece);
  }

  /// Get side to move
  Color getSideToMove() {
    _checkDisposed();
    final side = ffi_bindings.chessBoardGetSideToMove(_handle);

    return Color.fromValue(side);
  }

  /// Check if current side is in check
  bool isInCheck(){
    _checkDisposed();

    return ffi_bindings.chessBoardIsInCheck(_handle) != 0;
  }

  /// Make a move
  void makeMove(Move move){
    _checkDisposed();
    final cmove = move._toCMove();
    ffi_bindings.chessBoardMakeMove(_handle, cmove);

    calloc.free(cmove);
  }

  /// Undo the last move
  void undoMove(){
    _checkDisposed();
    ffi_bindings.chessBoardUndoMove(_handle);
  }

  /// Clean up native resources
  void dispose(){
    if(!_disposed){
      ffi_bindings.chessBoardDestroy(_handle);
      _disposed = true;
    }
  }

}

class ChessEngine {
  late ffi_bindings.ChessEngineHandle _handle;
  bool _disposed = false;

  ChessEngine() {
    _handle = ffi_bindings.chessEngineCreate();
  }

  void _checkDisposed() {
    if (_disposed) {
      throw StateError('ChessEngine has been disposed');
    }
  }

  /// Generate all legal moves for the current position
  List<Move> generateLegalMoves(ChessBoard board){
    _checkDisposed();
    board._checkDisposed();

    final movesPtr = calloc<ffi_bindings.CMove>(ffi_bindings.maxLegalMoves);
    final count = ffi_bindings.chessEngineGenerateLegalMoves(
      _handle,
      board._handle,
      movesPtr,
      ffi_bindings.maxLegalMoves
    );

    final moves = List<Move>.empty(growable: true);
    for (int i = 0; i < count; i++) {
      moves.add(Move.fromCMove(movesPtr[i]));
    }

    calloc.free(movesPtr);
    return moves;
  }

  /// Run perft test (for debugging)
  int perft(ChessBoard board, int depth){
    _checkDisposed();
    board._checkDisposed();
    return ffi_bindings.chessPerft(_handle, board._handle, depth);
  }

  /// Clean up native resources
  void dispose(){
    if(!_disposed){
      ffi_bindings.chessEngineDestroy(_handle);
      _disposed = true;
    }
  } 
  
}

