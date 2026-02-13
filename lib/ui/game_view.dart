import 'package:chess_ui/game/chess_engine.dart';
import 'package:chess_ui/game/game_controller.dart';
import 'package:chess_ui/game/game_mode.dart';
import 'package:chess_ui/game/player_config.dart';
import 'package:chess_ui/ui/board_controls.dart';
import 'package:chess_ui/ui/game_board.dart';
import 'package:flutter/material.dart';

/// Main game view for human vs engine gameplay
class GameView extends StatefulWidget {
  final GameController controller;
  final ChessEngine engine;
  final void Function(GameMode)? onModeChange;
  
  const GameView({
    super.key,
    required this.controller,
    required this.engine,
    this.onModeChange,
  });

  @override
  State<GameView> createState() => _GameViewState();
}

class _GameViewState extends State<GameView> {
  bool _isProcessingMove = false;
  PlayerConfig _playerConfig = PlayerConfig.defaultHumanVsEngine;

  @override
  void initState() {
    super.initState();
    widget.controller.setGameMode(GameMode.humanVsEngine);
    
    // Start engine vs engine game if both are engines
    if (_playerConfig.whitePlayer == PlayerType.engine && 
        _playerConfig.blackPlayer == PlayerType.engine) {
      WidgetsBinding.instance.addPostFrameCallback((_) {
        _handleMoveExecuted();
      });
    }
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      body: Row(
        children: [
          // Chess board
          Padding(
            padding: EdgeInsets.all(MediaQuery.of(context).size.width * 0.024),
              child: ListenableBuilder(
                listenable: widget.controller,
                builder: (context, _) => GameBoard(
                  controller: widget.controller,
                  interactive: _playerConfig.isHumanColor(widget.controller.turn),
                  onMoveExecuted: _handleMoveExecuted,
                  onGameOver: _handleGameOver,
                ),
              ),
          ),
          // Controls panel
          Expanded(
            child: Padding(
              padding: EdgeInsets.fromLTRB(
                0,
                MediaQuery.of(context).size.width * 0.024,
                MediaQuery.of(context).size.width * 0.024,
                MediaQuery.of(context).size.width * 0.024,
              ),
              child: BoardControls(
                whiteTimer: widget.controller.whiteTimer,
                blackTimer: widget.controller.blackTimer,
                board: widget.controller.board,
                blackPlayer: _playerConfig.blackPlayer == PlayerType.human ? "Human" : "Engine",
                whitePlayer: _playerConfig.whitePlayer == PlayerType.human ? "Human" : "Engine",
                gameMode: GameMode.humanVsEngine,
                onNewGame: _handleNewGame,
                onPlayerConfigChanged: (config) {
                  setState(() {
                    _playerConfig = config;
                  });
                  // If both are engines, start the game
                  if (config.whitePlayer == PlayerType.engine && 
                      config.blackPlayer == PlayerType.engine) {
                    WidgetsBinding.instance.addPostFrameCallback((_) {
                      _handleMoveExecuted();
                    });
                  }
                },
                playerConfig: _playerConfig,
              ),
            ),
          ),
        ],
      ),
    );
  }

  /// Handle when a move is executed
  void _handleMoveExecuted() async {
    if (_isProcessingMove) return;
    
    _isProcessingMove = true;
    
    // Give UI time to update
    await Future.delayed(const Duration(milliseconds: 100));
    
    // Check for game over
    if (!_isGameOver()) {
      // Check if it's engine's turn
      final currentTurn = widget.controller.turn;
      if (_playerConfig.isEngineColor(currentTurn)) {
        // Engine's turn - get best move
        final engineMove = widget.engine.getBestMove(widget.controller.board);
        
        if (engineMove != null) {
          widget.controller.makeMove(engineMove);
          await Future.delayed(const Duration(milliseconds: 100));
          _isGameOver();
          
          // If both players are engines, continue the game
          if (_playerConfig.whitePlayer == PlayerType.engine && 
              _playerConfig.blackPlayer == PlayerType.engine) {
            // Recursively call to continue engine vs engine
            _handleMoveExecuted();
          }
        }
      }
    }
    
    _isProcessingMove = false;
  }

  /// Handle game over
  void _handleGameOver() {
    _showGameOverDialog();
  }

  /// Check if game is over and show dialog if so
  bool _isGameOver() {
    if (widget.engine.isCheckmate(widget.controller.board)) {
      widget.controller.stopWhiteTimer();
      widget.controller.stopBlackTimer();
      widget.controller.setMatchStatus(MatchStatus.finished);
      
      final winner = widget.controller.board.getSideToMove() == ChessColor.black 
          ? "White Wins" 
          : "Black Wins";
      
      _showGameOverDialog(
        result: winner,
        reason: "Checkmate",
      );
      return true;
    } else if (widget.engine.isStalemate(widget.controller.board)) {
      widget.controller.stopWhiteTimer();
      widget.controller.stopBlackTimer();
      widget.controller.setMatchStatus(MatchStatus.finished);
      
      _showGameOverDialog(
        result: "Draw",
        reason: "Stalemate",
      );
      return true;
    }
    
    return false;
  }

  /// Show game over dialog
  void _showGameOverDialog({
    String? result,
    String? reason,
  }) {
    showDialog(
      context: context,
      barrierDismissible: false,
      builder: (dialogContext) {
        final theme = Theme.of(dialogContext);
        final colorScheme = theme.colorScheme;
        final textTheme = theme.textTheme;
        final isDraw = result == 'Draw';
        return AlertDialog(
          title: Row(
            children: [
              Icon(
                isDraw ? Icons.handshake : Icons.emoji_events,
                color: isDraw
                    ? colorScheme.primary
                    : colorScheme.tertiary,
                size: 32,
              ),
              const SizedBox(width: 12),
              Text(
                reason ?? 'Game Over',
                style: textTheme.titleLarge,
              ),
            ],
          ),
          content: Text(
            result ?? 'Game Over',
            style: textTheme.headlineSmall?.copyWith(
              fontWeight: FontWeight.bold,
            ),
          ),
          actions: [
            TextButton(
              onPressed: () => Navigator.of(dialogContext).pop(),
              child: const Text('View Board'),
            ),
            ElevatedButton(
              onPressed: () {
                Navigator.of(dialogContext).pop();
                _handleNewGame();
              },
              child: const Text('New Game'),
            ),
          ],
        );
      },
    );
  }

  /// Handle new game
  void _handleNewGame() {
    widget.controller.resetGame();
    setState(() {});
  }
}
