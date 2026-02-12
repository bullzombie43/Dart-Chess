import 'package:chess_ui/game/chess_engine.dart';
import 'package:chess_ui/game/game_controller.dart';
import 'package:chess_ui/game/game_mode.dart';
import 'package:chess_ui/game/player_config.dart';
import 'package:chess_ui/ui/board_controls.dart';
import 'package:chess_ui/ui/game_board.dart';
import 'package:flutter/material.dart';

/// Single match view for playing individual games
/// 
/// Supports all player combinations: Human vs Human, Human vs Engine, Engine vs Engine
class SingleMatchView extends StatefulWidget {
  final GameController controller;
  final ChessEngine engine;
  
  const SingleMatchView({
    super.key,
    required this.controller,
    required this.engine,
  });

  @override
  State<SingleMatchView> createState() => _SingleMatchViewState();
}

class _SingleMatchViewState extends State<SingleMatchView> {
  bool _isProcessingMove = false;
  PlayerConfig _playerConfig = PlayerConfig.defaultHumanVsEngine;

  @override
  void initState() {
    super.initState();
    widget.controller.setGameMode(GameMode.humanVsEngine);
  }

  /// Handle start match button click
  void _handleStartMatch() {
    widget.controller.setMatchStatus(MatchStatus.active);
    setState(() {}); // Update UI to show New Game button
    
    // If white is engine, start it playing immediately
    if (_playerConfig.whitePlayer == PlayerType.engine) {
      WidgetsBinding.instance.addPostFrameCallback((_) {
        _handleMoveExecuted();
      });
    }
  }

  @override
  Widget build(BuildContext context) {
    // Check if player config is editable (only when game is idle)
    final isEditable = widget.controller.matchStatus == MatchStatus.idle;
    
    return Scaffold(
      backgroundColor: const Color.fromARGB(255, 140, 208, 161),
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
              padding: EdgeInsetsGeometry.fromLTRB(
                0,
                MediaQuery.of(context).size.width * 0.024,
                MediaQuery.of(context).size.width * 0.024,
                MediaQuery.of(context).size.width * 0.024,
              ),
              child: ListenableBuilder(
                listenable: widget.controller,
                builder: (context, _) {
                  final currentIsGameActive = widget.controller.matchStatus == MatchStatus.active;
                  return BoardControls(
                    whiteTimer: widget.controller.whiteTimer,
                    blackTimer: widget.controller.blackTimer,
                    board: widget.controller.board,
                    blackPlayer: _playerConfig.blackPlayer == PlayerType.human ? "Human" : "Engine",
                    whitePlayer: _playerConfig.whitePlayer == PlayerType.human ? "Human" : "Engine",
                    gameMode: GameMode.humanVsEngine,
                    onNewGame: _handleNewGame,
                    onStartMatch: currentIsGameActive ? null : _handleStartMatch,
                    onPlayerConfigChanged: (config) {
                      setState(() {
                        _playerConfig = config;
                      });
                    },
                    playerConfig: _playerConfig,
                    isPlayerConfigEditable: isEditable,
                  );
                },
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
    
    // Set status to active on first move
    if (widget.controller.matchStatus == MatchStatus.idle) {
      widget.controller.setMatchStatus(MatchStatus.active);
      setState(() {}); // Update UI to show New Game button
    }
    
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
          
          // Check for game over after engine move
          if (!_isGameOver()) {
            // If both players are engines, continue the game
            if (_playerConfig.whitePlayer == PlayerType.engine && 
                _playerConfig.blackPlayer == PlayerType.engine &&
                widget.controller.matchStatus == MatchStatus.active) {
              // Reset processing flag before recursive call so black engine can play
              _isProcessingMove = false;
              // Recursively call to continue engine vs engine
              _handleMoveExecuted();
              return; // Exit early since recursive call will handle continuation
            }
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
      builder: (context) {
        final isDraw = result == 'Draw';
        return AlertDialog(
          title: Row(
            children: [
              Icon(
                isDraw ? Icons.handshake : Icons.emoji_events,
                color: isDraw ? Colors.blue : Colors.amber,
                size: 32,
              ),
              const SizedBox(width: 12),
              Text(reason ?? 'Game Over'),
            ],
          ),
          content: Text(
            result ?? 'Game Over',
            style: const TextStyle(fontSize: 20, fontWeight: FontWeight.bold),
          ),
          actions: [
            TextButton(
              onPressed: () => Navigator.of(context).pop(),
              child: const Text('View Board'),
            ),
            ElevatedButton(
              onPressed: () {
                Navigator.of(context).pop();
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
