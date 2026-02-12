import 'dart:convert';
import 'dart:io';

import 'package:chess_ui/game/chess_engine.dart';

/// Parser for UCI (Universal Chess Interface) move notation
/// 
/// UCI format examples:
/// - Normal moves: "e2e4", "g1f3"
/// - Promotions: "e7e8q" (queen), "e7e8r" (rook), "e7e8b" (bishop), "e7e8n" (knight)
/// - Castling: "e1g1" (white kingside), "e1c1" (white queenside)
class MoveParser {
  // #region agent log
  static void _debugLog(
    String hypothesisId,
    String location,
    String message,
    Map<String, dynamic> data,
  ) {
    final payload = <String, dynamic>{
      'id':
          'log_${DateTime.now().millisecondsSinceEpoch}_${location.hashCode}',
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
        jsonEncode(payload) + '\n',
        mode: FileMode.append,
        flush: true,
      );
    } catch (_) {
      // Swallow all logging errors to avoid impacting app behavior.
    }
  }
  // #endregion agent log

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
  
  /// Parse algebraic notation to a Move object
  /// 
  /// Supports standard algebraic notation:
  /// - "e4" - pawn move
  /// - "Nf3" - knight move
  /// - "Qxf5" - queen capture
  /// - "O-O" or "0-0" - kingside castling
  /// - "O-O-O" or "0-0-0" - queenside castling
  /// - "e8=Q" - promotion
  /// - "exd5" - pawn capture with file disambiguation
  /// 
  /// Uses legal move generation to disambiguate and find the correct move
  static Move? parseAlgebraic(String alg, ChessBoard board, ChessEngine engine) {
    // Clean the notation (remove check/checkmate markers, extra spaces, and NAGs)
    var cleaned = alg.trim();
    // Remove numeric annotation glyphs like $1, $3 etc.
    cleaned = cleaned.replaceAll(RegExp(r'\$\d+'), '');
    cleaned = cleaned
        .replaceAll(RegExp(r'[+#]'), '') // Remove check/checkmate markers
        .replaceAll(RegExp(r'\s+'), '');
    
    if (cleaned.isEmpty) return null;

    final isPieceMove = RegExp(r'^[KQRBN]').hasMatch(cleaned);
    final isPawnCapture = !isPieceMove && cleaned.contains('x');

    if (isPieceMove || isPawnCapture) {
      _debugLog(
        'H1',
        'move_parser.dart:parseAlgebraic',
        'parseAlgebraic entry',
        {
          'alg': alg,
          'cleaned': cleaned,
          'fen': board.getFen(),
        },
      );
    }
    
    // Handle castling
    if (cleaned == 'O-O' || cleaned == '0-0' || cleaned.toLowerCase() == 'o-o') {
      return _findCastlingMove(board, engine, true); // Kingside
    }
    if (cleaned == 'O-O-O' || cleaned == '0-0-0' || cleaned.toLowerCase() == 'o-o-o') {
      return _findCastlingMove(board, engine, false); // Queenside
    }
    
    // Get all legal moves for current position
    final legalMoves = engine.generateLegalMoves(board);
    final currentTurn = board.getSideToMove();
    
    // First, try strict matching
    for (final move in legalMoves) {
      if (_matchesAlgebraic(cleaned, move, board, currentTurn)) {
        if (isPieceMove || isPawnCapture) {
          _debugLog(
            'H1',
            'move_parser.dart:parseAlgebraic',
            'strict match success',
            {
              'alg': alg,
              'cleaned': cleaned,
              'uci': move.toUCI(),
              'fromSquare': move.fromSquare,
              'toSquare': move.toSquare,
            },
          );
        }
        return move;
      }
    }
    
    // Fallback: be more permissive and match primarily on destination (and promotion),
    // but still enforce the SAN piece type (e.g. \"N\" for knights) and any
    // disambiguation hints (file/rank) to avoid silently choosing the wrong piece.
    String? promotionChar;
    String workingAlg = cleaned;
    if (workingAlg.contains('=')) {
      final parts = workingAlg.split('=');
      if (parts.length == 2) {
        workingAlg = parts[0];
        promotionChar = parts[1];
      }
    }
    
    final hasCaptureSymbol =
        workingAlg.contains('x') || workingAlg.contains('X');

    // Split into prefix (piece/disambiguation/capture) and destination.
    final prefix = workingAlg.length > 2
        ? workingAlg.substring(0, workingAlg.length - 2)
        : '';
    final prefixWithoutCapture = prefix.replaceAll(RegExp(r'[xX]'), '');

    // Determine expected piece type and optional disambiguation from SAN prefix.
    // Examples:
    // - \"Nf3\"   -> piece 'N', no disamb
    // - \"Neg5\"  -> piece 'N', file disamb 'e'
    // - \"N1d2\"  -> piece 'N', rank disamb '1'
    // - \"e4\"    -> pawn (no piece letter)
    String expectedPieceChar = '';
    int? fileHint;
    int? rankHint;

    final piecePrefixMatch = RegExp(r'^[KQRBN]').firstMatch(workingAlg);
    if (piecePrefixMatch != null) {
      expectedPieceChar = piecePrefixMatch.group(0)!;

      // Derive disambiguation hints similarly to _matchesAlgebraic.
      if (prefixWithoutCapture.length > 1) {
        final disamb = prefixWithoutCapture.substring(1);
        for (var i = 0; i < disamb.length; i++) {
          final c = disamb[i].toLowerCase();
          if (c.compareTo('a') >= 0 && c.compareTo('h') <= 0) {
            fileHint = c.codeUnitAt(0) - 'a'.codeUnitAt(0);
          } else if (c.compareTo('1') >= 0 && c.compareTo('8') <= 0) {
            rankHint = int.parse(c) - 1;
          }
        }
      }
    }
    final bool expectPawn = expectedPieceChar.isEmpty;

    // For pawn captures with file disambiguation (e.g. \"exd5\", \"cxd5\"),
    // record the file hint so we can prefer the correct pawn when multiple
    // can capture the same square.
    int? pawnFileHint;
    if (expectPawn && prefixWithoutCapture.length == 1) {
      final c = prefixWithoutCapture[0].toLowerCase();
      if (c.compareTo('a') >= 0 && c.compareTo('h') <= 0) {
        pawnFileHint = c.codeUnitAt(0) - 'a'.codeUnitAt(0);
      }
    }
    
    for (final move in legalMoves) {
      final toAlg = _indexToSquare(move.toSquare);
      if (!workingAlg.endsWith(toAlg)) {
        continue;
      }

      // Enforce piece type consistency with SAN.
      final pieceAtSource = board.getPieceAt(move.fromSquare);
      final actualPieceChar = _getPieceChar(pieceAtSource);
      final actualIsPawn = actualPieceChar.isEmpty;

      if (!expectPawn) {
        // SAN refers to a specific piece type (e.g. 'N' for knight).
        if (actualPieceChar != expectedPieceChar) {
          continue;
        }
      } else {
        // SAN has no piece letter; this should be a pawn move.
        if (!actualIsPawn) {
          continue;
        }
        // If we have a pawn file hint (e.g. \"exd5\" -> 'e'), enforce it.
        if (pawnFileHint != null) {
          final fromFile = move.fromSquare % 8;
          if (fromFile != pawnFileHint) {
            continue;
          }
        }
      }

      // If we have disambiguation hints for a piece, enforce them.
      if (!expectPawn && (fileHint != null || rankHint != null)) {
        final fromFile = move.fromSquare % 8;
        final fromRank = move.fromSquare ~/ 8;
        if (fileHint != null && fileHint != fromFile) {
          continue;
        }
        if (rankHint != null && rankHint != fromRank) {
          continue;
        }
      }
      
      // Basic capture consistency check
      final isCapture = board.getPieceAt(move.toSquare) != PieceType.none ||
          move.isEnPassant;
      if (hasCaptureSymbol && !isCapture) continue;
      
      // Basic promotion consistency check (if present)
      if (promotionChar != null && promotionChar.isNotEmpty) {
        try {
          final expectedPromo =
              _parsePromotionPiece(promotionChar, currentTurn == ChessColor.white);
          if (move.promotedPiece != expectedPromo) {
            continue;
          }
        } catch (_) {
          // If promotion char itself is invalid, skip this fallback entirely
          continue;
        }
      }
      
      if (isPieceMove || isPawnCapture) {
        final piece = board.getPieceAt(move.fromSquare);
        _debugLog(
          'H1',
          'move_parser.dart:parseAlgebraic',
          'fallback match selected move',
          {
            'alg': alg,
            'cleaned': cleaned,
            'workingAlg': workingAlg,
            'uci': move.toUCI(),
            'fromSquare': move.fromSquare,
            'toSquare': move.toSquare,
            'pieceType': piece.toString(),
          },
        );
      }
      
      return move;
    }
    
    if (isPieceMove || isPawnCapture) {
      _debugLog(
        'H1',
        'move_parser.dart:parseAlgebraic',
        'no matching move found',
        {
          'alg': alg,
          'cleaned': cleaned,
          'fen': board.getFen(),
        },
      );
    }
    return null; // No matching move found, even with fallback
  }
  
  /// Find castling move
  static Move? _findCastlingMove(ChessBoard board, ChessEngine engine, bool kingside) {
    final legalMoves = engine.generateLegalMoves(board);
    final currentTurn = board.getSideToMove();
    
    // Find king's starting square
    final kingRank = currentTurn == ChessColor.white ? 0 : 7;
    final kingSquare = kingRank * 8 + 4; // e1 or e8
    
    for (final move in legalMoves) {
      final piece = board.getPieceAt(move.fromSquare);
      // Treat any king move from the starting square that moves two
      // files horizontally as castling. This avoids relying on the
      // native engine correctly setting the isCastling flag.
      if (!piece.isKing || move.fromSquare != kingSquare) {
        continue;
      }

      final toSquare = move.toSquare;
      final fileDiff = (toSquare % 8) - (kingSquare % 8);
      if (kingside && fileDiff == 2) return move;
      if (!kingside && fileDiff == -2) return move;
    }
    
    return null;
  }
  
  /// Check if a move matches algebraic notation
  static bool _matchesAlgebraic(String alg, Move move, ChessBoard board, ChessColor turn) {
    final fromSquare = move.fromSquare;
    final toSquare = move.toSquare;
    final piece = board.getPieceAt(fromSquare);
    
    // Convert destination to algebraic
    final toAlg = _indexToSquare(toSquare);
    
    // Extract destination from algebraic notation
    // Pattern: [piece][disambiguation][capture]destination[promotion]
    // Examples: "e4", "Nf3", "Qxf5", "exd5", "e8=Q"
    
    String workingAlg = alg;
    
    // Check for promotion
    String? promotionChar;
    if (workingAlg.contains('=')) {
      final parts = workingAlg.split('=');
      if (parts.length == 2) {
        workingAlg = parts[0];
        promotionChar = parts[1];
      }
    }
    
    // Check if destination matches (must be last 2 characters)
    if (workingAlg.length < 2 || !workingAlg.endsWith(toAlg)) {
      return false;
    }
    
    // Remove destination to get piece/disambiguation/capture info
    final prefix = workingAlg.length > 2 
        ? workingAlg.substring(0, workingAlg.length - 2)
        : '';
    
    // Check promotion
    if (promotionChar != null) {
      final expectedPromo = _parsePromotionPiece(promotionChar, turn == ChessColor.white);
      if (move.promotedPiece != expectedPromo) {
        return false;
      }
    } else if (move.promotedPiece != 0) {
      return false; // Move has promotion but notation doesn't
    }
    
    // Check piece type
    final pieceChar = _getPieceChar(piece);
    final isPawn = pieceChar == '';
    
    // Remove capture marker from prefix for piece matching
    final prefixWithoutCapture = prefix.replaceAll(RegExp(r'[xX]'), '');

    if (prefixWithoutCapture.isNotEmpty) {
      // For non-pawns, prefix should start with piece character (e.g., "K", "Q", "Nf3" -> "N")
      if (!isPawn) {
        if (!prefixWithoutCapture.startsWith(pieceChar)) {
          return false;
        }
        // Handle disambiguation for pieces, e.g. "Rad1", "R1d1", "Rfd1"
        final disamb = prefixWithoutCapture.substring(1);
        if (disamb.isNotEmpty) {
          int? fileHint;
          int? rankHint;

          for (var i = 0; i < disamb.length; i++) {
            final c = disamb[i].toLowerCase();
            if (c.compareTo('a') >= 0 && c.compareTo('h') <= 0) {
              fileHint = c.codeUnitAt(0) - 'a'.codeUnitAt(0);
            } else if (c.compareTo('1') >= 0 && c.compareTo('8') <= 0) {
              rankHint = int.parse(c) - 1;
            }
          }

          final fromFile = fromSquare % 8;
          final fromRank = fromSquare ~/ 8;

          if (fileHint != null && fileHint != fromFile) {
            return false;
          }
          if (rankHint != null && rankHint != fromRank) {
            return false;
          }
        }
      } else {
        // For pawns, prefix might be file disambiguation (e.g., "exd5" -> "ex")
        if (prefixWithoutCapture.length == 1) {
          final fileChar = prefixWithoutCapture[0].toLowerCase();
          final fromFile = String.fromCharCode('a'.codeUnitAt(0) + (fromSquare % 8));
          if (fileChar != fromFile) {
            return false;
          }
        } else if (prefixWithoutCapture.isNotEmpty) {
          // Pawn moves shouldn't have piece prefix
          return false;
        }
      }
    } else {
      // No prefix - must be a pawn move
      if (!isPawn) {
        return false;
      }
    }
    
    // Check capture
    final hasCapture = prefix.contains('x') || prefix.contains('X');
    final isCapture = board.getPieceAt(toSquare) != PieceType.none || move.isEnPassant;
    if (hasCapture != isCapture) {
      return false;
    }
    
    return true;
  }
  
  /// Get piece character for algebraic notation
  static String _getPieceChar(PieceType piece) {
    if (piece.isKing) return 'K';
    if (piece.isQueen) return 'Q';
    if (piece.isRook) return 'R';
    if (piece.isBishop) return 'B';
    if (piece.isKnight) return 'N';
    return ''; // Pawn
  }
  
  /// Convert square index to algebraic notation
  static String _indexToSquare(int index) {
    final file = String.fromCharCode('a'.codeUnitAt(0) + (index % 8));
    final rank = (index ~/ 8) + 1;
    return '$file$rank';
  }
  
  /// Parse a move string (tries UCI first, then algebraic notation)
  /// 
  /// This is the main entry point that handles both formats
  static Move? parseMove(String moveStr, ChessBoard board, ChessEngine engine) {
    final trimmed = moveStr.trim();
    if (trimmed.isEmpty) return null;

    // Try UCI first (4-5 characters, all lowercase)
    if (trimmed.length >= 4 && trimmed.length <= 5 && 
        trimmed.toLowerCase() == trimmed && 
        !trimmed.contains(RegExp(r'[A-Z]'))) {
      try {
        return parseUci(trimmed, board, engine);
      } catch (e) {
        // Fall through to algebraic
      }
    }
    
    // Try algebraic notation
    return parseAlgebraic(trimmed, board, engine);
  }
  
  /// Parse multiple moves from a list (handles both UCI and algebraic)
  static List<Move> parseMoveList(List<String> moves, ChessBoard board, ChessEngine engine) {
    final parsedMoves = <Move>[];
    
    for (final moveStr in moves) {
      try {
        final move = parseMove(moveStr, board, engine);
        if (move != null) {
          parsedMoves.add(move);
          // Apply move to board for next move parsing
          board.makeMove(move);
        } else {
          print('Could not parse move: $moveStr');
        }
      } catch (e) {
        print('Error parsing move "$moveStr": $e');
      }
    }
    
    return parsedMoves;
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
