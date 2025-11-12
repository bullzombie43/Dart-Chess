import 'dart:collection';
import 'dart:io';
import 'dart:math';

typedef Bitboard = int;

bool isOnBoard(int square) => square >= 0 && square < 64;
int rankOf(int square) => square ~/ 8;
int fileOf(int square) => square % 8;
Bitboard squareBB(int square) => 1 << square;
String indexToSquare(int index) {
  int rank = index ~/ 8;
  int file = index % 8;
  String fileChar = String.fromCharCode('a'.codeUnitAt(0) + file);
  String rankChar = (rank + 1).toString();
  return "$fileChar$rankChar";
}
String moveNotation(Move move) {
  String from = indexToSquare(startingSquare(move));
  String to = indexToSquare(endingSquare(move));
  String promo = isPromo(move) ? '=${movePromotionPiece(move)!.charValue}' : '';
  return '$from$to$promo';
}


enum Piece {
  whitePawn("P", PieceColor.WHITE, "wP.svg", 1),
  whiteKnight("N", PieceColor.WHITE, "wN.svg", 2),
  whiteBishop("B", PieceColor.WHITE, "wB.svg", 3),
  whiteRook("R", PieceColor.WHITE, "wR.svg", 4),
  whiteQueen("Q", PieceColor.WHITE, "wQ.svg",5 ),
  whiteKing("K", PieceColor.WHITE, "wK.svg", 6),
  blackPawn("p", PieceColor.BLACK, "bP.svg", 1),
  blackKnight("n", PieceColor.BLACK, "bN.svg", 2),
  blackBishop("b", PieceColor.BLACK, "bB.svg", 3),
  blackRook("r", PieceColor.BLACK, "bR.svg", 4),
  blackQueen("q", PieceColor.BLACK, "bQ.svg", 5),
  blackKing("k", PieceColor.BLACK, "bK.svg", 6);

  final String charValue;
  final PieceColor color;
  final String asset;
  final int pieceInt;
  
  bool get isKing => this == Piece.blackKing || this == Piece.whiteKing;
  bool get isRook => this == Piece.blackRook || this == Piece.whiteRook;
  bool get isKnight => this == Piece.blackKnight || this == Piece.whiteKnight;
  bool get isBishop => this == Piece.blackBishop || this == Piece.whiteBishop;
  bool get isQueen => this == Piece.blackQueen || this == Piece.whiteQueen;
  bool get isPawn => this == Piece.blackPawn || this == Piece.whitePawn;

  static Piece? fromChar(String c){
    switch (c) {
      case 'P': return Piece.whitePawn;
      case 'N': return Piece.whiteKnight;
      case 'B': return Piece.whiteBishop;
      case 'R': return Piece.whiteRook;
      case 'Q': return Piece.whiteQueen;
      case 'K': return Piece.whiteKing;
      case 'p': return Piece.blackPawn;
      case 'n': return Piece.blackKnight;
      case 'b': return Piece.blackBishop;
      case 'r': return Piece.blackRook;
      case 'q': return Piece.blackQueen;
      case 'k': return Piece.blackKing;
      default: return null;
    }
  }

  const Piece(this.charValue, this.color, this.asset, this.pieceInt);
}  

class GameState {
  static const List<int> directionOffsets = [8,-8,-1,1,7,-7,9,-9, 6, 10, 15, 17, -6, -10, -15, -17]; 

  static const List<int> knightOffsets = [17, 15, 10, 6, -17, -15, -10, -6]; //I know this is redundant but Im lazy so reasons
  static const List<int> kingOffsets = [8, -8, 1, -1, 7, -7, 9, -9];

  static final List<List<int>> numSquaresToEdge = _precomputedMoveData();

  // For white
  static const int wKingStart = 4;
  static const List<int> wKingsidePath = [5, 6];
  static const List<int> wQueensidePath = [1, 2, 3];

  // For black
  static const int bKingStart = 60;
  static const List<int> bKingsidePath = [61, 62];
  static const List<int> bQueensidePath = [57, 58, 59];

  final Map<Piece,Bitboard> positions;
  final Map<Piece,Bitboard> attacks;


  final List<Bitboard> knightAttacks = List.filled(64, 0);
  final List<Bitboard> kingAttacks = List.filled(64, 0);
  final List<List<Bitboard>> pawnAttacks =
      List.generate(2, (_) => List.filled(64, 0)); // [color][square]

  final List<MoveState> moveHistory = [];

  int? enPassantSquare;
  bool? whiteCanEnPassant;

  ListQueue<Move> premoveWhite = ListQueue();
  ListQueue<Move> premoveBlack = ListQueue();

  bool whiteToMove = true;

  bool whiteCanCastleKingside = true;
  bool blackCanCastleKingside = true;
  bool whiteCanCastleQueenside = true;
  bool blackCanCastleQueenside = true;


  PieceColor get turn => whiteToMove ? PieceColor.WHITE : PieceColor.BLACK;

  static List<List<int>> _precomputedMoveData(){
    final data = List.generate(64, (_) => List.filled(8, 0));

    for (int file = 0; file < 8; file++) {
      for (int rank = 0; rank < 8; rank++) {
        int squareIndex = rank * 8 + file;

        int north = 7 - rank;
        int south = rank;
        int west = file;
        int east = 7 - file;
        int northWest = min(north, west);
        int northEast = min(north, east);
        int southWest = min(south, west);
        int southEast = min(south, east);

        data[squareIndex] = [
          north,
          south,
          west,
          east,
          northWest,
          southEast,
          northEast,
          southWest,
        ];
      }
    }

    return data;
  }

  GameState() :
    positions = {
      Piece.whitePawn :   0x000000000000FF00,
      Piece.whiteKnight : 0x0000000000000042,
      Piece.whiteBishop : 0x0000000000000024,
      Piece.whiteRook :   0x0000000000000081,
      Piece.whiteQueen :  0x0000000000000008,
      Piece.whiteKing :   0x0000000000000010,
      Piece.blackPawn :   0x00FF000000000000,
      Piece.blackKnight : 0x4200000000000000,
      Piece.blackBishop : 0x2400000000000000,
      Piece.blackRook :   0x8100000000000000,
      Piece.blackQueen :  0x0800000000000000,
      Piece.blackKing :   0x1000000000000000
    },

    attacks = {
      Piece.whitePawn :   0x000000000000FF00,
      Piece.whiteKnight : 0x0000000000000042,
      Piece.whiteBishop : 0x0000000000000024,
      Piece.whiteRook :   0x0000000000000081,
      Piece.whiteQueen :  0x0000000000000008,
      Piece.whiteKing :   0x0000000000000010,
      Piece.blackPawn :   0x00FF000000000000,
      Piece.blackKnight : 0x4200000000000000,
      Piece.blackBishop : 0x2400000000000000,
      Piece.blackRook :   0x8100000000000000,
      Piece.blackQueen :  0x0800000000000000,
      Piece.blackKing :   0x1000000000000000
    } {
      initAttackTables();
    }

  GameState.fromValues({
    Map<Piece, Bitboard>? positions,
    bool? whiteToMove,
    this.enPassantSquare,
    this.whiteCanEnPassant,
  }) :
    positions = positions ?? {
      Piece.whitePawn :   0x000000000000FF00,
      Piece.whiteKnight : 0x0000000000000042,
      Piece.whiteBishop : 0x0000000000000024,
      Piece.whiteRook :   0x0000000000000081,
      Piece.whiteQueen :  0x0000000000000008,
      Piece.whiteKing :   0x0000000000000010,
      Piece.blackPawn :   0x00FF000000000000,
      Piece.blackKnight : 0x4200000000000000,
      Piece.blackBishop : 0x2400000000000000,
      Piece.blackRook :   0x8100000000000000,
      Piece.blackQueen :  0x0800000000000000,
      Piece.blackKing :   0x1000000000000000
    },
    attacks = {
      Piece.whitePawn :   0x000000000000FF00,
      Piece.whiteKnight : 0x0000000000000042,
      Piece.whiteBishop : 0x0000000000000024,
      Piece.whiteRook :   0x0000000000000081,
      Piece.whiteQueen :  0x0000000000000008,
      Piece.whiteKing :   0x0000000000000010,
      Piece.blackPawn :   0x00FF000000000000,
      Piece.blackKnight : 0x4200000000000000,
      Piece.blackBishop : 0x2400000000000000,
      Piece.blackRook :   0x8100000000000000,
      Piece.blackQueen :  0x0800000000000000,
      Piece.blackKing :   0x1000000000000000
    },
    whiteToMove = whiteToMove ?? true
  ;

  GameState clone() {
    final clonedPositions = <Piece, Bitboard>{};
    for (final entry in positions.entries) {
      clonedPositions[entry.key] = entry.value.toInt();
    }

    return GameState.fromValues(
      positions: clonedPositions,
      whiteToMove: whiteToMove,
      enPassantSquare: enPassantSquare,
      whiteCanEnPassant: whiteCanEnPassant,
    );
  }

void initAttackTables() {
  for (int square = 0; square < 64; square++) {
    knightAttacks[square] = _computeKnightAttack(square);
    kingAttacks[square] = _computeKingAttack(square);
    pawnAttacks[PieceColor.WHITE.value][square] = _computePawnAttack(square, PieceColor.WHITE);
    pawnAttacks[PieceColor.BLACK.value][square] = _computePawnAttack(square, PieceColor.BLACK);
  }
}

Bitboard _computeKnightAttack(int square) {
  final int rank = rankOf(square);
  final int file = fileOf(square);
  Bitboard attacks = 0;

  const knightMoves = [
    [2, 1], [1, 2], [-1, 2], [-2, 1],
    [-2, -1], [-1, -2], [1, -2], [2, -1]
  ];

  for (final move in knightMoves) {
    int r = rank + move[0];
    int f = file + move[1];
    if (r >= 0 && r < 8 && f >= 0 && f < 8) {
      attacks |= squareBB(r * 8 + f);
    }
  }
  return attacks;
}

Bitboard _computeKingAttack(int square) {
  final int rank = rankOf(square);
  final int file = fileOf(square);
  Bitboard attacks = 0;

  for (int dr = -1; dr <= 1; dr++) {
    for (int df = -1; df <= 1; df++) {
      if (dr == 0 && df == 0) continue;
      int r = rank + dr;
      int f = file + df;
      if (r >= 0 && r < 8 && f >= 0 && f < 8) {
        attacks |= squareBB(r * 8 + f);
      }
    }
  }
  return attacks;
}

Bitboard rookAttacks(int square, Bitboard occupied) {
  Bitboard attacks = 0;
  const directions = [8, -8, 1, -1]; // up, down, right, left

  for (final dir in directions) {
    int s = square;
    while (true) {
      s += dir;
      if (s < 0 || s >= 64) break;
      // stop wrap-around (left/right edges)
      if ((dir == 1 && fileOf(s) == 0) || (dir == -1 && fileOf(s) == 7)) break;

      attacks |= squareBB(s);
      if ((occupied & squareBB(s)) != 0) break; // blocked
    }
  }
  return attacks;
}

Bitboard bishopAttacks(int square, Bitboard occupied) {
  Bitboard attacks = 0;
  const directions = [9, 7, -9, -7]; // diagonals

  for (final dir in directions) {
    int s = square;
    while (true) {
      s += dir;
      if (s < 0 || s >= 64) break;
      if ((dir == 9 && fileOf(s) == 0) || (dir == -9 && fileOf(s) == 7) ||
          (dir == 7 && fileOf(s) == 7) || (dir == -7 && fileOf(s) == 0)) break;

      attacks |= squareBB(s);
      if ((occupied & squareBB(s)) != 0) break;
    }
  }
  return attacks;
}

Bitboard queenAttacks(int square, Bitboard occupied) =>
    rookAttacks(square, occupied) | bishopAttacks(square, occupied);

Bitboard _computePawnAttack(int square, PieceColor color) {
  final int rank = rankOf(square);
  final int file = fileOf(square);
  Bitboard attacks = 0;

  int forward = color == PieceColor.WHITE ? 1 : -1;

  for (int df in [-1, 1]) {
    int r = rank + forward;
    int f = file + df;
    if (r >= 0 && r < 8 && f >= 0 && f < 8) {
      attacks |= squareBB(r * 8 + f);
    }
  }
  return attacks;
}

  void restore(GameState other) {
    positions
      ..clear()
      ..addAll(other.positions);
    whiteToMove = other.whiteToMove;
    enPassantSquare = other.enPassantSquare;
    whiteCanEnPassant = other.whiteCanEnPassant;
  }

  void printSingleBitBoard(Bitboard bitboard){
    for (int rank = 7; rank >= 0; rank--) {
      for (int file = 0; file <= 7; file++) {
        int square = rank * 8 + file;
        Bitboard mask = 1 << square;
        bool hasPiece = (bitboard & mask) != 0;

        stdout.write(hasPiece ? '1 ' : '. ');
      }
      print('');
    }
    print('');
  }

  void printBoard(){
    Map<Piece, Bitboard> bitboards = positions;

    for (int rank = 7; rank >= 0; rank--) {
      for (int file = 0; file <= 7; file++) {
        int square = rank * 8 + file;
        String s = ".";
        for(MapEntry board in bitboards.entries){
          Bitboard mask = 1 << square;
          bool hasPiece = (board.value & mask) != 0;

          if(hasPiece){
            s = board.key.charValue;
            break;
          }
        }

        stdout.write('$s ');

      }
      print('');
    }
    
    print('');
  }

  void safeMakeMove(Move move){
    if(!isTurnColor(movePiece(move).color)){
      setPreMove(move);
      return;
    } else {
      if(whiteToMove && premoveWhite.isNotEmpty){
        move = premoveWhite.removeFirst();
      }

      if(!whiteToMove && premoveBlack.isNotEmpty){
        move = premoveBlack.removeFirst();
      }
    }

    if (!isLegalMove(move)){
      return;
    } 

    makeMove(move);

    if(whiteToMove && premoveWhite.isNotEmpty){
      safeMakeMove(premoveWhite.first); //Don't remove here, it will remove in the call
    }

    if(!whiteToMove && premoveBlack.isNotEmpty){
      safeMakeMove(premoveBlack.first); //Don't remove here, it will remove in the call
    }
  }

  void makeMove(Move move){
    Bitboard bitboard = positions[movePiece(move)]!;

    //Check there is a piece at starting spot
    Bitboard startMask = 1 << startingSquare(move);
    bool hasPiece = (bitboard & startMask) != 0;

    if(!hasPiece){
      throw Exception("No Piece at starting square");
    }

    moveHistory.add(
      MoveState(
        move: move,
        capturedPiece: getPieceAt(endingSquare(move)),
        enPassantSquare: enPassantSquare,
        whiteCanEnPassant: whiteCanEnPassant,
        whiteCanCastleKingside: whiteCanCastleKingside,
        whiteCanCastleQueenside: whiteCanCastleQueenside,
        blackCanCastleKingside: blackCanCastleKingside,
        blackCanCastleQueenside: blackCanCastleQueenside,
        wasPromotion: isPromo(move)
      ),
    );

    //print("MAKING MOVE: Saving old EP square: $enPassantSquare");

    if(isEnPassant(move) && isCapture(move)) {
      int capturedPawnSquare = (movePiece(move) == Piece.whitePawn)
        ? endingSquare(move) - 8
        : endingSquare(move) + 8;
      removeCapturedPiece(capturedPawnSquare);
    } else if(isCapture(move)){
      removeCapturedPiece(endingSquare(move));
    }

    if (movePiece(move) == Piece.whitePawn && startingSquare(move) ~/ 8 == 1 && endingSquare(move) ~/ 8 == 3) {
      enPassantSquare = startingSquare(move) + 8; // square behind the pawn
      whiteCanEnPassant = false;
    } else if (movePiece(move) == Piece.blackPawn && startingSquare(move) ~/ 8 == 6 && endingSquare(move) ~/ 8 == 4) {
      enPassantSquare = startingSquare(move) - 8;
      whiteCanEnPassant = true;
    } else {
      enPassantSquare = null;
      whiteCanEnPassant = null;
    }

    //Castling Flah Check
    if(movePiece(move).isRook){
      if(movePiece(move).color == PieceColor.WHITE && startingSquare(move) == 0){
        whiteCanCastleQueenside = false;
      } else if(movePiece(move).color == PieceColor.WHITE && startingSquare(move) == 7){
        whiteCanCastleKingside = false;
      } else if(movePiece(move).color == PieceColor.BLACK && startingSquare(move) == 56){
        blackCanCastleQueenside = false;
      } else if(movePiece(move).color == PieceColor.WHITE && startingSquare(move) == 63){
        blackCanCastleKingside = false;
      }
    }

    if(movePiece(move).isKing){
      if(movePiece(move).color == PieceColor.WHITE){
        whiteCanCastleKingside = false;
        whiteCanCastleQueenside = false;
      } else {
        blackCanCastleKingside = false;
        blackCanCastleQueenside = false;
      }
    }

    if (isCastle(move)) {
      castleMove(move);
    } 

    Bitboard newBitboard = bitboard & (~startMask);

    Bitboard endMask = 1 << endingSquare(move);

    // --- Handle promotion ---
    if (isPromo(move)) {
      // Remove pawn from its bitboard (already done in newBitboard)
      positions[movePiece(move)] = newBitboard; // update pawn board

      // Add the promoted piece to its bitboard
      Bitboard promoBitboard = positions[movePromotionPiece(move)] ?? 0;
      positions[movePromotionPiece(move)!] = promoBitboard | endMask;

      // Remove pawn bit from its bitboard entirely (already handled)
    } else {
      // Normal move
      newBitboard = newBitboard | endMask;
      positions[movePiece(move)] = newBitboard;
    }

    whiteToMove = !whiteToMove;
  }

  void unmakeMove() {
    final last = moveHistory.removeLast();
    final move = last.move;

    // Revert side to move
    whiteToMove = !whiteToMove;

    // Restore flags
    enPassantSquare = last.enPassantSquare;
    whiteCanCastleKingside = last.whiteCanCastleKingside;
    whiteCanCastleQueenside = last.whiteCanCastleQueenside;
    blackCanCastleKingside = last.blackCanCastleKingside;
    blackCanCastleQueenside = last.blackCanCastleQueenside;
    whiteCanEnPassant = last.whiteCanEnPassant;

    //print("UNMAKING MOVE: Restoring EP square to: ${enPassantSquare}");

    // Undo piece movement
    Piece movedPiece = movePiece(move);
    Bitboard bb = positions[movedPiece]!;
    bb &= ~(1 << endingSquare(move)); // remove from end
    bb |= 1 << startingSquare(move);  // restore start
    positions[movedPiece] = bb;

    // Restore captured piece
    if (last.capturedPiece != null) {
      Piece cap = last.capturedPiece!;
      positions[cap] = positions[cap]! | (1 << endingSquare(move));
    }

    // Handle undo special moves (castle, en passant, etc.)
    if (isCastle(move)) {
      if (endingSquare(move) - startingSquare(move) == 2) {
        // kingside
        int rookStart = movePiece(move).color == PieceColor.WHITE ? 7 : 63;
        int rookEnd = movePiece(move).color == PieceColor.WHITE ? 5 : 61;
        undoRookForCastle(movePiece(move).color, rookStart, rookEnd);
      } else if (endingSquare(move) - startingSquare(move) == -2) {
        // queenside
        int rookStart = movePiece(move).color == PieceColor.WHITE ? 0 : 56;
        int rookEnd = movePiece(move).color == PieceColor.WHITE ? 3 : 59;
        undoRookForCastle(movePiece(move).color, rookStart, rookEnd);
      }
    }

      // Undo en passant
    if (isEnPassant(move) && isCapture(move)) {
        int capturedPawnSquare = movePiece(move).color == PieceColor.WHITE
            ? endingSquare(move) - 8
            : endingSquare(move) + 8;
        final Piece capturedPawn = movePiece(move).color == PieceColor.WHITE
            ? Piece.blackPawn
            : Piece.whitePawn;
        positions[capturedPawn] =
            positions[capturedPawn]! | (1 << capturedPawnSquare);
    }

    //undo promotion
    if (isPromo(move)) {
      // 1️⃣ Remove the promoted piece from the board
      Bitboard promotedBitboard = positions[movePromotionPiece(move)]!;
      Bitboard endMask = 1 << endingSquare(move);
      promotedBitboard &= ~endMask; // clear promoted piece bit
      positions[movePromotionPiece(move)!] = promotedBitboard;

      // 2️⃣ Restore the pawn on its starting square
      final Piece pawn = (movePiece(move).color == PieceColor.WHITE)
          ? Piece.whitePawn
          : Piece.blackPawn;

      Bitboard pawnBitboard = positions[pawn]!;
      Bitboard startMask = 1 << startingSquare(move);
      pawnBitboard |= startMask; // put pawn back
      positions[pawn] = pawnBitboard;
    }
  }

  void undoRookForCastle(PieceColor color, int start, int end) {
    final Piece rook = color == PieceColor.WHITE ? Piece.whiteRook : Piece.blackRook;
    Bitboard rookBoard = positions[rook]!;

    rookBoard &= ~(1 << end);
    rookBoard |= 1 << start;

    positions[rook] = rookBoard;
  }

  void removeCapturedPiece(int square) {
    positions.forEach((piece, bb) {
      if ((bb & (1 << square)) != 0) {
        positions[piece] = bb & ~(1 << square);
        if(piece == Piece.whiteRook && square == 7) whiteCanCastleKingside = false;
        if(piece == Piece.whiteRook && square == 0) whiteCanCastleQueenside = false;
        if(piece == Piece.blackRook && square == 56) blackCanCastleQueenside = false;
        if(piece == Piece.blackRook && square == 63) blackCanCastleKingside = false;
      }
    });
  }

  Piece? getPieceAt(int index){
    if(index < 0) return null;

    for(MapEntry entry in positions.entries){
      //Check there is a piece at starting spot
      Bitboard startMask = 1 << index;
      bool hasPiece = (entry.value & startMask) != 0;

      if(hasPiece){
        return entry.key;
      }
    } 

    return null;
  }

  List<Move> generatePsuedoLegalMoves(int index){
    List<Move> legalMoves = [];


    if(getPieceAt(index) == null) return legalMoves;

    Piece piece = getPieceAt(index)!;

    if(isSlidingPiece(piece)){
        legalMoves = generateSlidingMoves(piece, index);
    } else if(piece == Piece.blackKnight || piece == Piece.whiteKnight){
        legalMoves = generateKnightMoves(piece, index);
    } else if (piece == Piece.blackPawn || piece == Piece.whitePawn) {
        legalMoves = generatePawnMoves(piece, index);
    } else if (piece == Piece.blackKing || piece == Piece.whiteKing){
        legalMoves = generateKingMoves(piece, index);
    }

    return legalMoves;
  }

  List<Move> generateFullLegalMoves(int index){
    List<Move> psuedoLegalMoves = generatePsuedoLegalMoves(index);
    List<Move> fullLegalMoves = List.empty(growable: true);

    for(Move m in psuedoLegalMoves){
      makeMove(m);
      
      if(!isKingInCheck(turn.opposite)){
        fullLegalMoves.add(m);
      }

      unmakeMove();
    }

    return fullLegalMoves;
  }

  bool isLegalMove(Move move){
    List<Move> legalMoves = generateFullLegalMoves(startingSquare(move));

    return legalMoves.contains(move);
  }
  
  bool isSlidingPiece(Piece piece){
    return piece == Piece.blackBishop || piece == Piece.blackRook || piece == Piece.blackQueen
      || piece == Piece.whiteBishop || piece == Piece.whiteRook || piece == Piece.whiteQueen;
  }

  bool isSameColor(Piece? a, Piece? b) {

    if(a == null || b == null){
      return false;
    }

    final isWhiteA = a.name.startsWith('white');
    final isWhiteB = b.name.startsWith('white');

    return isWhiteA == isWhiteB;
  }

  bool isOppositeColor(Piece? a, Piece? b) {

    if(a == null || b == null) return false;

    final isWhiteA = a.name.startsWith('white');
    final isWhiteB = b.name.startsWith('white');

    return isWhiteA != isWhiteB;
  }

  List<Move> generateSlidingMoves(Piece piece, int index){
    int startDirIndex = piece == Piece.blackBishop || piece == Piece.whiteBishop ? 4 : 0;
    int endDirIndex = piece == Piece.blackRook || piece == Piece.whiteRook ? 4 : 8;

    List<Move> legalMoves = [];


    for(int directionIndex = startDirIndex; directionIndex < endDirIndex; directionIndex++){
      for(int n = 0; n < numSquaresToEdge[index][directionIndex]; n++){
        int targetSquare = index + directionOffsets[directionIndex] * (n+1);

        if(targetSquare < 0){
          break;
        }
        
        if(isSameColor(piece, getPieceAt(targetSquare))){
          break;
        }

        if(isOppositeColor(piece, getPieceAt(targetSquare))){
          legalMoves.add(makeMoveInt(from: index, to: targetSquare, movingPiece: piece, color: piece.color, isCapture: true));
          break;
        }

        legalMoves.add(makeMoveInt(from: index, to: targetSquare, movingPiece: piece, color: piece.color, isCapture: false));
      }
    }

    return legalMoves;
  }

  List<Move> generateKnightMoves(Piece piece, int index){
    int startDirIndex = 8;
    int endDirIndex = 16;

    List<Move> legalMoves = [];

    for(int directionIndex = startDirIndex; directionIndex < endDirIndex; directionIndex++){
      int targetSquare = index + directionOffsets[directionIndex];

      if(targetSquare < 0 || targetSquare > 63) continue;

      // Check file wrapping
      int fileDiff = (targetSquare % 8) - (index % 8);
      if (fileDiff.abs() > 2) continue; // illegal wrap

      if(isSameColor(piece, getPieceAt(targetSquare))) continue;

      legalMoves.add(makeMoveInt(from: index, to: targetSquare, movingPiece: piece, color: piece.color, isCapture: isOppositeColor(piece, getPieceAt(targetSquare))));
    }

    return legalMoves;
  }
  
  List<Move> generatePawnMoves(Piece piece, int index){
    List<Move> legalMoves = [];

    if (piece == Piece.whitePawn){
      int rank = index ~/ 8;
      if(rank == 1 && getPieceAt(index+16) == null && getPieceAt(index+8) == null) legalMoves.add(makeMoveInt(from: index, to: index+16, movingPiece: piece, color: piece.color, isCapture: false));

      if(getPieceAt(index + 8) == null) legalMoves.add(makeMoveInt(from: index, to: index+8, movingPiece: piece, color: piece.color, isCapture: false));


      if(isOppositeColor(piece, getPieceAt(index+9)) || (index + 9 == enPassantSquare && whiteCanEnPassant != null && whiteCanEnPassant!)){
        bool canEnPassant = enPassantSquare != null && index + 9 == enPassantSquare && whiteCanEnPassant == true;

         // Check file wrapping
        int fileDiff = ((index+9) % 8) - (index % 8);
        if (fileDiff.abs() == 1){ // prevent illegal wrap
          legalMoves.add(makeMoveInt(from: index, to: index+9, movingPiece: piece, color: piece.color, isCapture: true, isEP: canEnPassant));
        } 
      }


      if(isOppositeColor(piece, getPieceAt(index+7)) || (index + 7 == enPassantSquare && whiteCanEnPassant != null && whiteCanEnPassant!)){
        bool canEnPassant = enPassantSquare != null && index + 7 == enPassantSquare && whiteCanEnPassant == true;

         // Check file wrapping
        int fileDiff = ((index+7) % 8) - (index % 8);
        if (fileDiff.abs() == 1){ // prevent illegal wrap
          legalMoves.add(makeMoveInt(from: index, to: index+7, movingPiece: piece, color: piece.color, isCapture: true, isEP: canEnPassant));
        } 
      }

    } else if (piece == Piece.blackPawn){
      int rank = index ~/ 8;
      if(rank == 6 && getPieceAt(index-16) == null && getPieceAt(index-8) == null) legalMoves.add(makeMoveInt(from: index, to: index-16, movingPiece: piece, color: piece.color, isCapture: false));

      if(getPieceAt(index - 8) == null) legalMoves.add(makeMoveInt(from: index, to: index-8, movingPiece: piece, color: piece.color, isCapture: false));

      if(isOppositeColor(piece, getPieceAt(index-9)) || (index-9 == enPassantSquare && whiteCanEnPassant != null && !whiteCanEnPassant!)){
        bool canEnPassant = enPassantSquare != null && index - 9 == enPassantSquare && whiteCanEnPassant == false;

         // Check file wrapping
        int fileDiff = ((index-9) % 8) - (index % 8);
        if (fileDiff.abs() == 1){ // prevent illegal wrap
          legalMoves.add(makeMoveInt(from: index, to: index-9, movingPiece: piece, color: piece.color, isCapture: true, isEP: canEnPassant));
        } 
      }

      if(isOppositeColor(piece, getPieceAt(index-7)) || (index - 7 == enPassantSquare && whiteCanEnPassant != null && !whiteCanEnPassant!)){
        bool canEnPassant = enPassantSquare != null && index - 7 == enPassantSquare && whiteCanEnPassant == false;

         // Check file wrapping
        int fileDiff = ((index-7) % 8) - (index % 8);
        if (fileDiff.abs() == 1){ // prevent illegal wrap
          legalMoves.add(makeMoveInt(from: index, to: index-7, movingPiece: piece, color: piece.color, isCapture: true, isEP: canEnPassant));
        } 
      }
    }

    List<Move> legalMovesAndPromotion = [];
      
      for(Move m in legalMoves){
        if(isPromotionRank(endingSquare(m), moveColor(m)) && moveColor(m) == PieceColor.WHITE){
          for (Piece promo in [Piece.whiteQueen, Piece.whiteRook, Piece.whiteBishop, Piece.whiteKnight]) {
            legalMovesAndPromotion.add(makeMoveInt(from: startingSquare(m), to: endingSquare(m), movingPiece: movePiece(m), color: moveColor(m), isCapture: isCapture(m), isEP: isEnPassant(m), isCastle: isCastle(m), promotionPiece: promo));
          }
        } else if(isPromotionRank(endingSquare(m), moveColor(m)) && moveColor(m) == PieceColor.BLACK){
          for (Piece promo in [Piece.blackQueen, Piece.blackRook, Piece.blackBishop, Piece.blackKnight]) {
            legalMovesAndPromotion.add(makeMoveInt(from: startingSquare(m), to: endingSquare(m), movingPiece: movePiece(m), color: moveColor(m), isCapture: isCapture(m), isEP: isEnPassant(m), isCastle: isCastle(m), promotionPiece: promo));
          }
        } else {
          legalMovesAndPromotion.add(m);
        }
      }

    return legalMovesAndPromotion;
  }

  bool isPromotionRank(int square, PieceColor side) =>
    side == PieceColor.WHITE ? square ~/ 8 == 7 : square ~/ 8 == 0;

  //Check for if is in Check outside of this
  List<Move> generateCastleMoves(PieceColor color){
    List<Move> legalMoves = [];

    if(isKingInCheck(color)) return legalMoves;

    if (whiteCanCastleKingside && color == PieceColor.WHITE && getPieceAt(7) == Piece.whiteRook) {
      if (wKingsidePath.every((s) => getPieceAt(s) == null) &&
          wKingsidePath.every((s) => !isSquareAttacked(s, PieceColor.BLACK))) {
        legalMoves.add(makeMoveInt(from: wKingStart, to: 6, movingPiece: Piece.whiteKing, color: color, isCastle: true));
      }
    }

    if (whiteCanCastleQueenside && color == PieceColor.WHITE && getPieceAt(0) == Piece.whiteRook) {
      if (wQueensidePath.every((s) => getPieceAt(s) == null) &&
          wQueensidePath.getRange(1, 3).every((s) => !isSquareAttacked(s, PieceColor.BLACK))) {
        legalMoves.add(makeMoveInt(from: wKingStart, to: 2, movingPiece: Piece.whiteKing, color: color, isCastle: true));
      }
    }

    if (blackCanCastleKingside && color == PieceColor.BLACK && getPieceAt(63) == Piece.blackRook) {
      if (bKingsidePath.every((s) => getPieceAt(s) == null) &&
          bKingsidePath.every((s) => !isSquareAttacked(s, PieceColor.WHITE))) {
        legalMoves.add(makeMoveInt(from: bKingStart, to: 62, movingPiece: Piece.blackKing, color: color, isCastle: true));
      }
    }

    if (blackCanCastleQueenside && color == PieceColor.BLACK && getPieceAt(56) == Piece.blackRook) {
      if (bQueensidePath.every((s) => getPieceAt(s) == null) &&
          bQueensidePath.getRange(1, 3).every((s) => !isSquareAttacked(s, PieceColor.WHITE))) {
        legalMoves.add(makeMoveInt(from: bKingStart, to: 58, movingPiece: Piece.blackKing, color: color, isCastle: true));
      }
    }

    return legalMoves;
  }

  void castleMove(Move kingMove){
    int rookStart, rookEnd;

    if (endingSquare(kingMove) - startingSquare(kingMove) == 2) {
      // King-side castling
      rookStart = startingSquare(kingMove) + 3; // rook originally on h-file
      rookEnd = startingSquare(kingMove) + 1;   // rook moves next to king
    } else if (endingSquare(kingMove) - startingSquare(kingMove) == -2) {
      // Queen-side castling
      rookStart = startingSquare(kingMove) - 4; // rook originally on a-file
      rookEnd = startingSquare(kingMove) - 1;   // rook moves next to king
    } else {
      throw Exception("Invalid castling move");
    }

    // Determine which rook piece
    Piece rookPiece = getPieceAt(rookStart)!;

    // Move the rook on the bitboards
    Bitboard rookBitboard = positions[rookPiece]!;
    rookBitboard &= ~(1 << rookStart); // remove from start
    rookBitboard |= 1 << rookEnd;      // add to end
    positions[rookPiece] = rookBitboard;
  }

  List<Move> generateKingMoves(Piece piece, int index){
    List<Move> legalMoves = [];

    for(int directionIndex = 0; directionIndex < 8; directionIndex++){
      int targetSquare = index + directionOffsets[directionIndex];

      if(targetSquare < 0 || targetSquare > 63){
        continue;
      }

      // Check file wrapping
      int fileDiff = (targetSquare % 8) - (index % 8);
      if (fileDiff.abs() > 2) continue; // illegal wrap
      
      if(isSameColor(piece, getPieceAt(targetSquare))){
        continue;
      }

      if(isOppositeColor(piece, getPieceAt(targetSquare))){
        legalMoves.add(makeMoveInt(from: index, to: targetSquare, movingPiece: piece, color: piece.color, isCapture: true));
        continue;
      }

      legalMoves.add(makeMoveInt(from: index, to: targetSquare, movingPiece: piece, color: piece.color, isCapture: false));
    }

    legalMoves += generateCastleMoves(piece.color);

    return legalMoves;
  }

  /// Returns true if [square] is attacked by side [attackingColor].
  bool isSquareAttacked(int targetSquare, PieceColor attackingColor) {
    Bitboard occupied = 0;
    for (final bitboard in positions.values){
      occupied |= bitboard;
    }

    // Pawns
    if (attackingColor == PieceColor.WHITE) {
      if ((pawnAttacks[PieceColor.BLACK.index][targetSquare] & positions[Piece.whitePawn]!) != 0) return true;
    } else {
      if ((pawnAttacks[PieceColor.WHITE.index][targetSquare] & positions[Piece.blackPawn]!) != 0) return true;
    }

    // Knights
    if ((knightAttacks[targetSquare] &
        (attackingColor == PieceColor.WHITE
            ? positions[Piece.whiteKnight]!
            : positions[Piece.blackKnight]!)) != 0) {
      return true;
    }

    // Kings
    if ((kingAttacks[targetSquare] &
        (attackingColor == PieceColor.WHITE
            ? positions[Piece.whiteKing]!
            : positions[Piece.blackKing]!)) != 0){
      return true;
    }

    // Bishops / Queens (diagonals)
    Bitboard bishopLike = bishopAttacks(targetSquare, occupied) &
        ((attackingColor == PieceColor.WHITE
            ? (positions[Piece.whiteBishop]! | positions[Piece.whiteQueen]!)
            : (positions[Piece.blackBishop]! | positions[Piece.blackQueen]!)));
    if (bishopLike != 0){
      return true;
    } 

    // Rooks / Queens (files + ranks)
    Bitboard rookLike = rookAttacks(targetSquare, occupied) &
        ((attackingColor == PieceColor.WHITE
            ? (positions[Piece.whiteRook]! | positions[Piece.whiteQueen]!)
            : (positions[Piece.blackRook]! | positions[Piece.blackQueen]!)));
    if (rookLike != 0){
       return true;
    }

    return false;
  }

  bool isKingInCheck(PieceColor color){
    int kingSquare = findKingSquare(color);
    return isSquareAttacked(kingSquare, color.opposite);
  }

  int findKingSquare(PieceColor color) {
    for (int square = 0; square < 64; square++) {
      final Piece? piece = getPieceAt(square);

      if (piece != null && piece.color == color && piece.isKing) {
        return square;
      }
    }
    throw Exception('No king found for $color');
  }

  bool isCheckmate(PieceColor color) {
    if (!isKingInCheck(color)) return false;

    // For each piece of this color, try all legal moves.
    for (int square = 0; square < 64; square++) {
      final Piece? piece = getPieceAt(square);
      if (piece == null || piece.color != color) continue;

      final moves = generatePsuedoLegalMoves(square);
      for (final move in moves) {
        // Simulate move
        makeMove(move);
        bool stillInCheck = isKingInCheck(color);
        unmakeMove();

        if (!stillInCheck) {
          return false; // Escaped check → not checkmate
        }
      }
    }

    return true; // No escape found
  }

  bool isTurnColor(PieceColor color){
    if(whiteToMove){
      return color == PieceColor.WHITE;
    } else {
      return color == PieceColor.BLACK;
    }
  }

  void setPreMove(Move premove){
    if(moveColor(premove) == PieceColor.WHITE) {
      premoveWhite.contains(premove) ? removePremove(premove) : premoveWhite.add(premove);
    } else {
      premoveBlack.contains(premove) ? removePremove(premove) : premoveBlack.add(premove);
    }
  }

  //Removes this premove and any premoves after it
  void removePremove(Move premove){    
    ListQueue<Move> original = moveColor(premove) == PieceColor.WHITE ? premoveWhite : premoveBlack;

    while(original.isNotEmpty){
      Move move = original.removeFirst();
      if(move == premove){
        break;
      }
    }
  }

  void updateAttacks(){
    for(MapEntry entry in positions.entries){
      updateAttack(entry.key);
    }
  }

  void updateAttack(Piece changedPiece){
    Bitboard bitboard = positions[changedPiece]!;
    List<int> pieceIndexes = [];

    List<int> attackIndexes = [];

    for (int i = 0; i < 64; i++) {
      if ((bitboard & (1 << i)) != 0) {
        pieceIndexes.add(i);
      }
    }
    
    for(int index in pieceIndexes){
      if(changedPiece.isBishop || changedPiece.isRook || changedPiece.isQueen){
        int startDirIndex = changedPiece == Piece.blackBishop || changedPiece == Piece.whiteBishop ? 4 : 0;
        int endDirIndex = changedPiece == Piece.blackRook || changedPiece == Piece.whiteRook ? 4 : 8;

        for(int directionIndex = startDirIndex; directionIndex < endDirIndex; directionIndex++){
          for(int n = 0; n < numSquaresToEdge[index][directionIndex]; n++){
            int targetSquare = index + directionOffsets[directionIndex] * (n+1);

            if(targetSquare < 0){
              break;
            }
            
            if(isSameColor(changedPiece, getPieceAt(targetSquare))){
              break;
            }

            if(isOppositeColor(changedPiece, getPieceAt(targetSquare))){
              attackIndexes.add(targetSquare);
              break;
            }

            attackIndexes.add(targetSquare);
          }
        }    
      } else if (changedPiece.isKnight){
          for(int offset in knightOffsets){
            int targetSquare = index + offset;
            if(targetSquare < 0 || targetSquare > 63) continue;

            // Check file wrapping
            int fileDiff = (targetSquare % 8) - (index % 8);
            if (fileDiff.abs() > 2) continue; // illegal wrap

            if(isSameColor(changedPiece, getPieceAt(targetSquare))) continue;

            attackIndexes.add(targetSquare);
          }
      } else if (changedPiece.isKing){
          for(int offset in kingOffsets){
            int targetSquare = index + offset;
            if(targetSquare < 0 || targetSquare > 63) continue;

            // Check file wrapping
            int fileDiff = (targetSquare % 8) - (index % 8);
            if (fileDiff.abs() > 2) continue; // illegal wrap

            if(isSameColor(changedPiece, getPieceAt(targetSquare))) continue;

            attackIndexes.add(targetSquare);
          }
      } else if (changedPiece.isPawn){
          int rank = index ~/ 8;
          int file = index % 8;

          int forward = changedPiece.color == PieceColor.WHITE ? 1 : -1;

          // left capture
          if (file > 0) {
            int targetSquare = (rank + forward) * 8 + (file - 1);
            attackIndexes.add(targetSquare);
          }

          // right capture
          if (file < 7) {
            int targetSquare = (rank + forward) * 8 + (file + 1);
            attackIndexes.add(targetSquare);
          }
      }
    }

    attacks[changedPiece] = listToBitboard(attackIndexes);
  }

  void generateAllAttacks(){
    for(MapEntry entry in positions.entries){
      updateAttack(entry.key);
    }
  }

  Bitboard listToBitboard(List<int> squares) {
    Bitboard bitboard = 0;
    for (final square in squares) {
      bitboard |= 1 << square;
    }
    return bitboard;
  }

  void printAttackBoards(){
    for(MapEntry entry in attacks.entries){
      print("Piece: ${entry.key}");

      printSingleBitBoard(entry.value);
    }
  }
  
  List<Move> generateAllMovesForColor(PieceColor color) {
    final List<Move> allMoves = [];

    for (final entry in positions.entries) {
      final Piece piece = entry.key;
      if (piece.color != color) continue;

      Bitboard bb = entry.value;
      while (bb != 0) {
        final int square = bitScanForward(bb);
        // clear the lowest set bit
        bb = bb & (bb - 1);
        final moves = generateFullLegalMoves(square);
        allMoves.addAll(moves);
      }
    }

    return allMoves;
  }

  int bitScanForward(Bitboard bb) {
    if (bb == 0) return -1;
    for (int i = 0; i < 64; i++) {
      if ((bb & (1 << i)) != 0) return i;
    }
    throw Exception("returned -1"); // this should never happen;
  }

  int moveGenerationTest(int depth){
    if(depth == 0){
      return 1;
    }

    List<Move> moves = generateAllMovesForColor(turn);
    int numPositions = 0;

    for(Move move in moves){
      makeMove(move);
      numPositions += moveGenerationTest(depth-1);
      unmakeMove();
    }

    return numPositions;
  }

  MoveStats moveGenerationTestWithBreakdown(int depth) {
    final stats = MoveStats();

    if (depth == 0) {
      stats.total = 1;
      return stats;
    }

    final moves = generateAllMovesForColor(turn);

    for (final move in moves) {
      makeMove(move);
      final childStats = moveGenerationTestWithBreakdown(depth - 1);
      if (enPassantSquare != null) {

      }
      unmakeMove();

      stats.add(childStats);

      // If we're at the leaf depth, count move types here
      if (depth == 1) {
        if (isCapture(move)) stats.captures++;
        if (isPromo(move)) stats.promotions++;
        if (isCastle(move)) stats.castles++;
        if (isEnPassant(move)) stats.enPassant++;
      }
    }

    return stats;
  }

  int moveGenerationTestWithEPDebug(int depth) {
    if (depth == 0) return 1;

    final moves = generateAllMovesForColor(turn);
    int numPositions = 0;

    for (final move in moves) {

      // DEBUG: print ep moves
      if (isEnPassant(move)) {
        print("Depth $depth: en passant move from ${(startingSquare(move))} "
            "to ${indexToSquare(endingSquare(move))}, EP square: $enPassantSquare");
      }

      makeMove(move);
      numPositions += moveGenerationTestWithEPDebug(depth - 1);
      unmakeMove();
    }

    return numPositions;
  }

  void setBoardToFen(String fen){
    final parts = fen.split(" ");
    if(parts.length != 6){
      throw Exception("Fen should have 6 parts");
    }

    final piecePlacement = parts[0];
    final sideToMove = parts[1];
    final castlingAbility = parts[2];
    final enPassantTargetSquare = parts[3];
    final halfMoveClock = parts[4];
    final fullMoveCounter = parts[5];

    //Piece Placement
    _parsePiecePlacement(piecePlacement);

    //Side to Move
    whiteToMove = sideToMove == "w";

    //En Passant
    if(enPassantTargetSquare == "-") {
      enPassantSquare = null;
    } else {
      enPassantSquare = squareIndexFromAlgebraic(enPassantTargetSquare);
    }

    //Castling Ability
    whiteCanCastleKingside = castlingAbility.contains("K");
    whiteCanCastleQueenside = castlingAbility.contains("Q");
    blackCanCastleKingside = castlingAbility.contains("k");
    blackCanCastleQueenside = castlingAbility.contains("q");

    //halfMoveClock (we don't use for now)

    //Full Move Counter (we don't use for now)



  }

  String getFen(){
    List<String> parts = ["", "", "", "", "", ""];

    //Piece Placement
    parts[0] = generateFenPiecePlacement();

    //Side to Move
    parts[1] = whiteToMove ? "w" : "b";

    //Castling Ability
    parts[2] = whiteCanCastleKingside ? "${parts[2]}K" : parts[2];
    parts[2] = whiteCanCastleQueenside ? "${parts[2]}Q" : parts[2];
    parts[2] = blackCanCastleKingside ? "${parts[2]}k" : parts[2];
    parts[2] = blackCanCastleQueenside ? "${parts[2]}q" : parts[2];
    parts[2] = parts[2] == "" ? "-" : parts[2];

    //En Passant
    parts[3] = enPassantSquare != null ? indexToSquare(enPassantSquare!) : "-";

    //halfMoveClock (we don't use for now)

    parts[4] = "0";

    //Full Move Counter (we don't use for now)

    parts[5] = "1";

    return parts.join(" ");
  }

  void _parsePiecePlacement(String placement) {
    // Clear all bitboards
    for (final piece in positions.keys) {
      positions[piece] = 0;
    }

    final ranks = placement.split('/');
    if (ranks.length != 8) {
      throw ArgumentError('Invalid FEN: should have 8 ranks');
    }

    int squareIndex = 56; // a8 (top-left), descending down to a1

    for (final rank in ranks) {
      for (final char in rank.split('')) {
        if (RegExp(r'[1-8]').hasMatch(char)) {
          // skip empty squares
          squareIndex += int.parse(char);
        } else {
          final piece = Piece.fromChar(char);
          if (piece != null) {
            positions[piece] = positions[piece]! | (1 << squareIndex);
          }
          squareIndex++;
        }
      }
      squareIndex -= 16; // move down one rank (8 squares + skip to next rank)
    }
  }

  String generateFenPiecePlacement() {
    final buffer = StringBuffer();

    // Loop ranks from 8 down to 1 (since FEN goes top → bottom)
    for (int rank = 7; rank >= 0; rank--) {
      int emptyCount = 0;

      for (int file = 0; file < 8; file++) {
        int square = rank * 8 + file;
        Piece? piece = getPieceAt(square);

        if (piece == null) {
          emptyCount++;
        } else {
          if (emptyCount > 0) {
            buffer.write(emptyCount);
            emptyCount = 0;
          }
          buffer.write(piece.charValue);
        }
      }

      // Write trailing empties if any
      if (emptyCount > 0) buffer.write(emptyCount);

      if (rank > 0) buffer.write('/');
    }

    return buffer.toString();
}

  int squareIndexFromAlgebraic(String notation) {
    if (notation.length != 2) {
      throw ArgumentError('Invalid square notation: $notation');
    }

    final fileChar = notation[0];
    final rankChar = notation[1];

    // file: 'a' → 0, 'b' → 1, ..., 'h' → 7
    final file = fileChar.codeUnitAt(0) - 'a'.codeUnitAt(0);
    if (file < 0 || file > 7) {
      throw ArgumentError('Invalid file: $fileChar');
    }

    // rank: '1' → 0, '2' → 1, ..., '8' → 7
    final rank = int.parse(rankChar) - 1;
    if (rank < 0 || rank > 7) {
      throw ArgumentError('Invalid rank: $rankChar');
    }

    // index = rank * 8 + file
    return rank * 8 + file;
  }
}


enum PieceColor {
  WHITE(0),
  BLACK(1);

  final int value;

  const PieceColor(this.value);

  PieceColor get opposite => this == WHITE ? BLACK : WHITE;
}

typedef Move = int;

// Bit layout: [ 0..5 from | 6..11 to | 12..15 flags | 16..19 promoType ]
const int FROM_MASK = 0x3F;       // bits 0–5
const int TO_MASK = 0xFC0;        // bits 6–11
const int FLAGS_MASK = 0xF000;    // bits 12–15
const int PROMO_MASK = 0xF0000;   // bits 16–19

// Flag bits
const int FLAG_CAPTURE           = 1 << 0;  // 0001
const int FLAG_DOUBLE_PAWN_PUSH  = 1 << 1;  // 0010
const int FLAG_EN_PASSANT        = 1 << 2;  // 0100
const int FLAG_CASTLE            = 1 << 3;  // 1000
const int FLAG_PROMOTION         = 1 << 4;  // 1_0000 (needs extra bit)

// Color bit
const int COLOR_BIT = 1 << 28;

/// movingPiece: 1=pawn, 2=knight, 3=bishop, 4=rook, 5=queen, 6=king
Move makeMoveInt({
  required int from,
  required int to,
  required Piece movingPiece,     // 1–6
  required PieceColor color,    // side to move
  bool isCapture = false,
  bool isEP = false,
  bool isCastle = false,
  Piece? promotionPiece,       // 0 = none, 1–4 = Q,R,B,N
}) {
  int c = color == PieceColor.BLACK ? 1 : 0;
  int flags = 0;
  if (isCapture) flags |= FLAG_CAPTURE;
  if (isEP) flags |= FLAG_EN_PASSANT;
  if (isCastle) flags |= FLAG_CASTLE;
  if (promotionPiece != null) flags |= FLAG_PROMOTION;

  int promoInt = 0;
  if(promotionPiece == Piece.blackQueen || promotionPiece == Piece.whiteQueen){
    print("Entered p2");
    promoInt = 1;
  }
  if(promotionPiece == Piece.blackRook || promotionPiece == Piece.whiteRook) promoInt = 2;
  if(promotionPiece == Piece.blackBishop || promotionPiece == Piece.whiteBishop) promoInt = 3;
  if(promotionPiece == Piece.blackKnight || promotionPiece == Piece.whiteKnight) promoInt = 4;

  return (from & 0x3F)
       | ((to & 0x3F) << 6)
       | ((flags & 0xFF) << 12)
       | ((promoInt & 0xF) << 20)
       | ((movingPiece.pieceInt & 0xF) << 24)
       | (c << 28);
}

//Extract Data
int startingSquare(Move m) => m & 0x3F;
int endingSquare(Move m) => (m >> 6) & 0x3F;
int moveFlags(Move m) => (m >> 12) & 0xFF;
int movePromotion(Move m) => (m >> 20) & 0xF;
int movePieceType(Move m) => (m >> 24) & 0xF;
PieceColor moveColor(Move m) => ((m >> 28) & 1) == 0 ? PieceColor.WHITE : PieceColor.BLACK;

Piece? movePromotionPiece(Move m) {
  if (!isPromo(m)) return null;

  final promo = movePromotion(m);
  final color = moveColor(m);

  switch (promo) {
    case 1: return color == PieceColor.WHITE ? Piece.whiteQueen : Piece.blackQueen;
    case 2: return color == PieceColor.WHITE ? Piece.whiteRook : Piece.blackRook;
    case 3: return color == PieceColor.WHITE ? Piece.whiteBishop : Piece.blackBishop;
    case 4: return color == PieceColor.WHITE ? Piece.whiteKnight : Piece.blackKnight;
    default: throw Exception("Invalid promotion type: $promo");
  }
}

Piece movePiece(Move m) {
  final type = movePieceType(m);
  final color = moveColor(m);

  switch (type) {
    case 1: return color == PieceColor.WHITE ? Piece.whitePawn : Piece.blackPawn;
    case 2: return color == PieceColor.WHITE ? Piece.whiteKnight : Piece.blackKnight;
    case 3: return color == PieceColor.WHITE ? Piece.whiteBishop : Piece.blackBishop;
    case 4: return color == PieceColor.WHITE ? Piece.whiteRook : Piece.blackRook;
    case 5: return color == PieceColor.WHITE ? Piece.whiteQueen : Piece.blackQueen;
    case 6: return color == PieceColor.WHITE ? Piece.whiteKing : Piece.blackKing;
    default: throw Exception("There was no piece in this move for some reason");
  }
}

/// Helpers
// Flag helpers
bool isCapture(Move m) => (moveFlags(m) & FLAG_CAPTURE) != 0;
bool isEnPassant(Move m) => (moveFlags(m) & FLAG_EN_PASSANT) != 0;
bool isDoublePawnPush(Move m) => (moveFlags(m) & FLAG_DOUBLE_PAWN_PUSH) != 0;
bool isCastle(Move m) => (moveFlags(m) & FLAG_CASTLE) != 0;
bool isPromo(Move m) => (moveFlags(m) & FLAG_PROMOTION) != 0;


class MoveState {
  final Move move;
  final Piece? capturedPiece;
  final int? enPassantSquare;
  final bool? whiteCanEnPassant;
  final bool whiteCanCastleKingside;
  final bool whiteCanCastleQueenside;
  final bool blackCanCastleKingside;
  final bool blackCanCastleQueenside;
  final bool wasPromotion;

  MoveState({
    required this.move,
    this.capturedPiece,
    this.enPassantSquare,
    this.whiteCanEnPassant,
    required this.whiteCanCastleKingside,
    required this.whiteCanCastleQueenside,
    required this.blackCanCastleKingside,
    required this.blackCanCastleQueenside,
    required this.wasPromotion
  });
}

class MoveStats {
  int total = 0;
  int captures = 0;
  int promotions = 0;
  int castles = 0;
  int enPassant = 0;

  void add(MoveStats other) {
    total += other.total;
    captures += other.captures;
    promotions += other.promotions;
    castles += other.castles;
    enPassant += other.enPassant;
  }

  @override
  String toString() {
    return '''
      Normal moves: ${total - captures - promotions - castles - enPassant}
      Captures: $captures
      Promotions: $promotions
      Castles: $castles
      En Passant: $enPassant
      Total: $total
      ''';
  }
}

void perftTest(GameState game) {
  for (int depth = 1; depth <= 6; depth++) {
    final stopwatch = Stopwatch()..start();

    final count = game.moveGenerationTest(depth);

    stopwatch.stop();

    final seconds = stopwatch.elapsedMilliseconds / 1000.0;
    print("Depth $depth:");
    print("  Moves: $count");
    print("  Time:  ${seconds.toStringAsFixed(3)} s");
    print("  Speed: ${(count / seconds).toStringAsFixed(0)} nodes/s\n");
  }
}

void perftTestWithBreakdown(GameState game){
  for (int depth = 1; depth <= 6; depth++) {
    final stopwatch = Stopwatch()..start();

    final stats = game.moveGenerationTestWithBreakdown(depth);

    stopwatch.stop();

    final seconds = stopwatch.elapsedMilliseconds / 1000.0;
    print("Depth $depth:");
    print("  Moves: ${stats.total}");
    print("  Captures: ${stats.captures}");
    print("  EP: ${stats.enPassant}");
    print("  Castles: ${stats.castles}");
    print("  Promotions: ${stats.promotions}");
    print("  Time:  ${seconds.toStringAsFixed(3)} s");
    print("  Speed: ${(stats.total / seconds).toStringAsFixed(0)} nodes/s\n");
  }
}

void dividePerft(GameState game, int depth) {
  final moves = game.generateAllMovesForColor(game.turn);

  print('Divide perft at depth $depth:');
  int total = 0;

  for (final move in moves) {
    game.makeMove(move);

    final stats = game.moveGenerationTest(depth - 1);


    print("Fen: ${game.getFen()}");
    game.unmakeMove();

    print(
      '${moveNotation(move)} -> ${stats.toString()}'
    );

    total += stats;
  }

  print('Total positions at depth $depth: $total');
}


void main(){
    GameState game = GameState();

    Move move = makeMoveInt(from: 11, to: 10, movingPiece: Piece.whitePawn, color: PieceColor.WHITE, isCapture: false);

    print("Starting");

    game.printBoard();

    //game.setBoardToFen("r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10");

    game.printBoard();

    print(game.getFen());

    //dividePerft(game, 4);
    perftTest(game);

    print("Finished");
  }

