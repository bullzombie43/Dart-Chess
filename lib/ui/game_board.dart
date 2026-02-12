import 'package:chess_ui/game/chess_engine.dart';
import 'package:chess_ui/game/game_controller.dart';
import 'package:chess_ui/game/move_handler.dart';
import 'package:chess_ui/ui/board_background.dart';
import 'package:chess_ui/ui/board_pieces.dart';
import 'package:flutter/material.dart';

/// Combined board widget that handles both background and pieces
/// 
/// This widget combines BoardBackground and BoardPieces into a single
/// reusable component that handles user interactions and delegates
/// move logic to MoveHandler.
class GameBoard extends StatelessWidget {
  final GameController controller;
  final BoardSize boardSize;
  final int orientation;
  final bool interactive;
  final VoidCallback? onMoveExecuted;
  final VoidCallback? onGameOver;

  const GameBoard({
    super.key,
    required this.controller,
    this.boardSize = BoardSize.chess,
    this.orientation = 0,
    this.interactive = true,
    this.onMoveExecuted,
    this.onGameOver,
  });

  @override
  Widget build(BuildContext context) {
    return ListenableBuilder(
      listenable: controller,
      builder: (context, _) {
        // Generate markers and highlights from controller state
        final markers = MoveHandler.generateMarkers(controller);
        final highlights = MoveHandler.generateHighlights(controller);

        return Stack(
          children: [
            // Board background with markers and highlights
            Boardbackground(
              markers: markers,
              boardSize: boardSize,
              orientation: orientation,
              highlights: highlights,
            ),
            // Board pieces with interaction handlers
            BoardPieces(
              board: controller.board,
              size: boardSize,
              orientation: orientation,
              onTap: interactive ? (index) => _handleTap(context, index) : null,
              onDragEnd: interactive 
                  ? (from, to) => _handleDrag(context, from, to) 
                  : null,
              canDrag: interactive,
              canTap: interactive,
            ),
          ],
        );
      },
    );
  }

  /// Handle square tap interaction
  void _handleTap(BuildContext context, int index) async {
    final result = MoveHandler.handleSquareTap(controller, index);

    switch (result) {
      case MoveResult.success:
        // A legal move was tapped, execute it
        final move = controller.legalMoves[index]!;
        await _executeMoveWithDelay(context, move);
        break;
      
      case MoveResult.promotionRequired:
        // Promotion needed, show dialog
        final move = controller.legalMoves[index]!;
        final promotionResult = await MoveHandler.handlePromotion(
          context,
          controller,
          move,
        );
        await _handleMoveResult(context, promotionResult);
        break;
      
      case MoveResult.invalid:
      case MoveResult.gameOver:
        // Invalid move or game over - already handled by selectSquare
        break;
    }
  }

  /// Handle piece drag interaction
  void _handleDrag(BuildContext context, int fromIndex, int toIndex) async {
    final move = MoveHandler.handlePieceDrag(controller, fromIndex, toIndex);
    
    if (move != null) {
      if (MoveHandler.isPromotionAttempt(move)) {
        // Need promotion
        final result = await MoveHandler.handlePromotion(
          context,
          controller,
          move,
        );
        await _handleMoveResult(context, result);
      } else {
        // Execute move directly
        await _executeMoveWithDelay(context, move);
      }
    }
  }

  /// Execute a move with a small delay for UI update
  Future<void> _executeMoveWithDelay(BuildContext context, Move move) async {
    final result = MoveHandler.executeMove(controller, move);
    await Future.delayed(const Duration(milliseconds: 100));
    await _handleMoveResult(context, result);
  }

  /// Handle the result of a move execution
  Future<void> _handleMoveResult(BuildContext context, MoveResult result) async {
    switch (result) {
      case MoveResult.success:
        onMoveExecuted?.call();
        break;
      
      case MoveResult.gameOver:
        onGameOver?.call();
        break;
      
      case MoveResult.invalid:
      case MoveResult.promotionRequired:
        // Already handled or user cancelled
        break;
    }
  }
}
