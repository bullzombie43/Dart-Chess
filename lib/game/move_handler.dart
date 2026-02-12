import 'package:chess_ui/game/chess_engine.dart';
import 'package:chess_ui/game/game_controller.dart';
import 'package:chess_ui/ui/board_background.dart';
import 'package:flutter/material.dart';
import 'package:flutter_svg/flutter_svg.dart';

/// Result of a move attempt
enum MoveResult {
  success,
  invalid,
  promotionRequired,
  gameOver,
}

/// Handles move processing logic extracted from UI
class MoveHandler {
  /// Handle a square tap interaction
  /// 
  /// Returns the result of the interaction:
  /// - If piece selected: updates selection in controller, returns invalid (selection only)
  /// - If legal move tapped: returns MoveResult indicating next action needed
  /// - If invalid: clears selection
  static MoveResult handleSquareTap(
    GameController controller,
    int index,
  ) {
    // Don't handle taps in non-interactive modes
    if (!controller.modeConfig.isInteractive) {
      return MoveResult.invalid;
    }

    // If we have a selected piece and tap a legal move destination, execute the move
    if (controller.selectedIndex != null && 
        controller.selectedIndex != index &&
        controller.legalMoves.containsKey(index)) {
      final move = controller.legalMoves[index]!;
      
      // Check if promotion is needed
      if (isPromotionAttempt(move)) {
        return MoveResult.promotionRequired;
      }
      
      // Move is ready to execute
      return MoveResult.success;
    }
    
    // Otherwise, handle selection/deselection
    controller.selectSquare(index);
    
    // Return invalid to indicate this was just a selection, not a move execution
    return MoveResult.invalid;
  }

  /// Handle a piece drag interaction
  /// 
  /// Returns the move if valid, null otherwise
  static Move? handlePieceDrag(
    GameController controller,
    int fromIndex,
    int toIndex,
  ) {
    // Don't handle drags in non-interactive modes
    if (!controller.modeConfig.isInteractive) {
      return null;
    }

    // Find matching legal move
    final legalMoves = controller.getLegalMovesForSquare(fromIndex);
    
    for (final move in legalMoves) {
      if (move.fromSquare == fromIndex && move.toSquare == toIndex) {
        return move;
      }
    }
    
    return null;
  }

  /// Execute a move on the board
  /// 
  /// Validates the move and applies it to the board via GameController
  /// Returns the result of the move execution
  static MoveResult executeMove(
    GameController controller,
    Move move,
  ) {
    // Validate move is legal
    final legalMoves = controller.getLegalMovesForSquare(move.fromSquare);
    final isValid = legalMoves.any((m) => 
      m.fromSquare == move.fromSquare &&
      m.toSquare == move.toSquare &&
      m.promotedPiece == move.promotedPiece
    );
    
    if (!isValid) {
      return MoveResult.invalid;
    }

    // Check for promotion requirement
    if (isPromotionAttempt(move) && move.promotedPiece == 0) {
      return MoveResult.promotionRequired;
    }

    // Execute the move
    controller.makeMove(move);
    controller.clearSelection();

    // Check for game over
    if (_isGameOver(controller)) {
      return MoveResult.gameOver;
    }

    return MoveResult.success;
  }

  /// Show promotion dialog and return selected piece
  /// 
  /// Returns null if user cancels
  static Future<PieceType?> showPromotionDialog(
    BuildContext context,
    ChessColor color,
  ) async {
    final promotionChoices = color == ChessColor.white
        ? [PieceType.wQueen, PieceType.wRook, PieceType.wBishop, PieceType.wKnight]
        : [PieceType.bQueen, PieceType.bRook, PieceType.bBishop, PieceType.bKnight];

    return await showDialog<PieceType>(
      context: context,
      barrierDismissible: true,
      builder: (context) {
        return AlertDialog(
          title: const Text('Promote to:'),
          content: Row(
            mainAxisAlignment: MainAxisAlignment.spaceEvenly,
            children: [
              for (final piece in promotionChoices)
                GestureDetector(
                  onTap: () => Navigator.pop(context, piece),
                  child: SvgPicture.asset(
                    'assets/pieces/${piece.asset}',
                    height: 48,
                    width: 48,
                  ),
                ),
            ],
          ),
        );
      },
    );
  }

  /// Handle a move that requires promotion
  /// 
  /// Shows promotion dialog and executes move with selected piece
  static Future<MoveResult> handlePromotion(
    BuildContext context,
    GameController controller,
    Move move,
  ) async {
    final pieceColor = PieceType.fromValue(move.piece).isWhite 
        ? ChessColor.white 
        : ChessColor.black;
    
    final promotionChoice = await showPromotionDialog(context, pieceColor);
    
    if (promotionChoice == null) {
      // User cancelled
      return MoveResult.invalid;
    }

    // Create new move with promotion piece
    final promotedMove = Move(
      piece: move.piece,
      fromSquare: move.fromSquare,
      toSquare: move.toSquare,
      capturedPiece: move.capturedPiece,
      promotedPiece: promotionChoice.value,
      isEnPassant: move.isEnPassant,
      isCastling: move.isCastling,
    );

    return executeMove(controller, promotedMove);
  }

  /// Generate markers for legal moves
  /// 
  /// Creates visual markers for squares that are legal move destinations
  static Map<int, Marker> generateMarkers(GameController controller) {
    final markers = <int, Marker>{};
    
    if (controller.selectedIndex == null) {
      return markers;
    }

    final legalMoves = controller.legalMoves;
    
    for (final move in legalMoves.values) {
      final highlightType = HighlightType.selected;
      final hasPiece = PieceType.fromValue(move.capturedPiece) != PieceType.none;
      markers[move.toSquare] = hasPiece 
          ? Marker.piece(highlightType) 
          : Marker.empty(highlightType);
    }

    return markers;
  }

  /// Generate highlights for the board
  /// 
  /// Currently returns empty map, can be extended for check/checkmate highlighting
  static Map<int, HighlightType> generateHighlights(GameController controller) {
    final highlights = <int, HighlightType>{};
    
    // TODO: Add check/checkmate highlighting
    // if (controller.engine.isCheckmate(controller.board)) {
    //   // Highlight king in checkmate
    // }
    
    return highlights;
  }

  /// Check if a move is a promotion attempt
  static bool isPromotionAttempt(Move move) {
    final piece = PieceType.fromValue(move.piece);
    if (piece != PieceType.wPawn && piece != PieceType.bPawn) {
      return false;
    }
    
    final rankTo = move.toSquare ~/ 8;
    return (piece == PieceType.wPawn && rankTo == 7) ||
           (piece == PieceType.bPawn && rankTo == 0);
  }

  /// Check if the game is over (checkmate or stalemate)
  static bool _isGameOver(GameController controller) {
    return controller.engine.isCheckmate(controller.board) ||
           controller.engine.isStalemate(controller.board);
  }
}
