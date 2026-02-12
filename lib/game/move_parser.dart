import 'package:chess_ui/game/chess_engine.dart';

/// Parser for UCI (Universal Chess Interface) move notation
/// 
/// UCI format examples:
/// - Normal moves: "e2e4", "g1f3"
/// - Promotions: "e7e8q" (queen), "e7e8r" (rook), "e7e8b" (bishop), "e7e8n" (knight)
/// - Castling: "e1g1" (white kingside), "e1c1" (white queenside)
class MoveParser {
  /// Parse a UCI move string to a Move object
  /// 
  /// Requires the current board state to determine:
  /// - Which piece is moving (from square)
  /// - If it's a capture (destination square has opponent piece)
  /// - If it's castling (king moving 2 squares)
  /// - If it's en passant (pawn capturing diagonally to empty square)
  /// 
  /// Throws [FormatException] if UCI string is invalid
  static Move parseUci(String uci, ChessBoard board, ChessEngine engine) {
    if (uci.length < 4 || uci.length > 5) {
      throw FormatException('UCI move must be 4-5 characters, got: $uci');
    }
    
    // Parse squares (e.g., "e2e4" -> from="e2", to="e4")
    final fromSquare = _squareToIndex(uci.substring(0, 2));
    final toSquare = _squareToIndex(uci.substring(2, 4));
    
    // Get piece at source square
    final pieceType = board.getPieceAt(fromSquare);
    if (pieceType == PieceType.none) {
      throw FormatException('No piece at source square: ${uci.substring(0, 2)}');
    }
    
    // Get piece at destination square (for captures)
    final capturedPieceType = board.getPieceAt(toSquare);
    final capturedPiece = capturedPieceType.value;
    
    // Determine if it's a capture
    final isCapture = capturedPieceType != PieceType.none;
    
    // Check for promotion (5th character: q, r, b, n)
    int promotedPiece = 0;
    if (uci.length == 5) {
      promotedPiece = _parsePromotionPiece(uci[4], pieceType.isWhite);
    }
    
    // Determine if it's castling (king moving 2 squares horizontally)
    final isCastling = _isCastling(pieceType, fromSquare, toSquare);
    
    // Determine if it's en passant
    final isEnPassant = _isEnPassant(
      pieceType,
      fromSquare,
      toSquare,
      board,
      isCapture,
    );
    
    return Move(
      piece: pieceType.value,
      fromSquare: fromSquare,
      toSquare: toSquare,
      capturedPiece: capturedPiece,
      promotedPiece: promotedPiece,
      isEnPassant: isEnPassant,
      isCastling: isCastling,
    );
  }
  
  /// Convert algebraic notation to square index (0-63)
  /// e.g., "e2" -> 12, "a1" -> 0, "h8" -> 63
  static int _squareToIndex(String square) {
    if (square.length != 2) {
      throw FormatException('Square must be 2 characters, got: $square');
    }
    
    final file = square[0].toLowerCase();
    final rank = square[1];
    
    if (file.compareTo('a') < 0 || file.compareTo('h') > 0) {
      throw FormatException('Invalid file: $file');
    }
    if (rank.compareTo('1') < 0 || rank.compareTo('8') > 0) {
      throw FormatException('Invalid rank: $rank');
    }
    
    final fileIndex = file.codeUnitAt(0) - 'a'.codeUnitAt(0);
    final rankIndex = int.parse(rank) - 1;
    
    return rankIndex * 8 + fileIndex;
  }
  
  /// Parse promotion piece character to piece value
  static int _parsePromotionPiece(String promoChar, bool isWhite) {
    switch (promoChar.toLowerCase()) {
      case 'q':
        return isWhite ? PieceType.wQueen.value : PieceType.bQueen.value;
      case 'r':
        return isWhite ? PieceType.wRook.value : PieceType.bRook.value;
      case 'b':
        return isWhite ? PieceType.wBishop.value : PieceType.bBishop.value;
      case 'n':
        return isWhite ? PieceType.wKnight.value : PieceType.bKnight.value;
      default:
        throw FormatException('Invalid promotion piece: $promoChar (must be q, r, b, or n)');
    }
  }
  
  /// Check if move is castling
  static bool _isCastling(PieceType piece, int fromSquare, int toSquare) {
    if (piece != PieceType.wKing && piece != PieceType.bKing) {
      return false;
    }
    
    // Castling: king moves exactly 2 squares horizontally
    final fileDiff = (toSquare % 8) - (fromSquare % 8);
    return fileDiff.abs() == 2;
  }
  
  /// Check if move is en passant
  static bool _isEnPassant(
    PieceType piece,
    int fromSquare,
    int toSquare,
    ChessBoard board,
    bool isCapture,
  ) {
    // En passant only for pawns
    if (piece != PieceType.wPawn && piece != PieceType.bPawn) {
      return false;
    }
    
    // Must be a capture
    if (!isCapture) {
      return false;
    }
    
    // Pawn must be moving diagonally
    final rankDiff = (toSquare ~/ 8) - (fromSquare ~/ 8);
    final fileDiff = (toSquare % 8) - (fromSquare % 8);
    
    // For white: moving up (rank increases), for black: moving down (rank decreases)
    if (piece == PieceType.wPawn && rankDiff != 1) return false;
    if (piece == PieceType.bPawn && rankDiff != -1) return false;
    
    // Must be diagonal (file changes by 1)
    if (fileDiff.abs() != 1) return false;
    
    // Destination square should be empty (en passant captures the pawn behind)
    final destPiece = board.getPieceAt(toSquare);
    if (destPiece != PieceType.none) {
      return false;
    }
    
    // Check if board has en passant square set
    // Note: This requires checking the board's en passant state
    // For now, we'll rely on the capture flag and diagonal move
    return true;
  }
  
  /// Parse multiple UCI moves from a list
  static List<Move> parseUciList(List<String> uciMoves, ChessBoard board, ChessEngine engine) {
    final moves = <Move>[];
    
    for (final uci in uciMoves) {
      try {
        moves.add(parseUci(uci, board, engine));
      } catch (e) {
        // Log error but continue parsing
        print('Error parsing UCI move "$uci": $e');
      }
    }
    
    return moves;
  }
}
