import 'package:chess_ui/game/chess_engine.dart';
import 'package:chess_ui/game/game_mode.dart';
import 'package:chess_ui/game/player_config.dart';
import 'package:chess_ui/ui/engine_dropdown_button.dart';
import 'package:flutter/material.dart';
import 'package:stop_watch_timer/stop_watch_timer.dart';

const List<String> engineOptions = <String>['one', 'two', 'three'];

void _emptyCallback(String value) {}

class BoardControls extends StatelessWidget {
  final StopWatchTimer whiteTimer;
  final StopWatchTimer blackTimer;
  final ChessBoard board;
  final String blackPlayer;
  final String whitePlayer;
  final GameMode? gameMode; // Optional - if null, infer from context
  final PlayerConfig? playerConfig; // Player configuration
  final void Function(PlayerConfig)? onPlayerConfigChanged; // Callback when config changes
  final bool isPlayerConfigEditable; // Whether player config dropdowns are editable

  final StringCallback setWhiteEngine;
  final StringCallback setBlackEngine;

  final VoidCallback onNewGame;
  final VoidCallback? startEngineMatch; // Optional for engine match mode
  final VoidCallback? onStartMatch; // Optional for starting a match

  const BoardControls({
    super.key, 
    required this.whiteTimer, 
    required this.blackTimer, 
    required this.board, 
    required this.onNewGame,
    this.startEngineMatch,
    required this.blackPlayer,
    required this.whitePlayer,
    this.setWhiteEngine = _emptyCallback,
    this.setBlackEngine = _emptyCallback,
    this.gameMode,
    this.playerConfig,
    this.onPlayerConfigChanged,
    this.isPlayerConfigEditable = true,
    this.onStartMatch,
  });

  @override
  Widget build(BuildContext context) {
    // Determine if we're in engine vs engine mode
    final isEngineMatch = gameMode == GameMode.engineVsEngine;
    
    return LayoutBuilder(
      builder: (context, constraints) {
        final defaultPadding = constraints.maxWidth * 0.02; // 2% of parent width

        return Container(
          color: Colors.grey,
          child: Column(
            mainAxisAlignment: MainAxisAlignment.center,
            mainAxisSize: MainAxisSize.max,
            children: [
              // Black player (top)
              isEngineMatch 
                  ? EngineDropdownButton(selectionCallback: setBlackEngine)
                  : _buildPlayerSelector(
                      playerName: blackPlayer,
                      playerType: playerConfig?.blackPlayer,
                      isEditable: isPlayerConfigEditable,
                      onTypeChanged: (type) {
                        if (playerConfig != null && onPlayerConfigChanged != null) {
                          onPlayerConfigChanged!(PlayerConfig(
                            whitePlayer: playerConfig!.whitePlayer,
                            blackPlayer: type,
                          ));
                        }
                      },
                    ),
              const SizedBox(height: 20),
              getTimeDisplay(blackTimer, defaultPadding, Colors.black),
              const SizedBox(height: 100),
              // Center button
              _buildCenterButton(),
              const SizedBox(height: 100),
              getTimeDisplay(whiteTimer, defaultPadding, Colors.white),
              const SizedBox(height: 20),
              // White player (bottom)
              isEngineMatch
                  ? EngineDropdownButton(selectionCallback: setWhiteEngine)
                  : _buildPlayerSelector(
                      playerName: whitePlayer,
                      playerType: playerConfig?.whitePlayer,
                      isEditable: isPlayerConfigEditable,
                      onTypeChanged: (type) {
                        if (playerConfig != null && onPlayerConfigChanged != null) {
                          onPlayerConfigChanged!(PlayerConfig(
                            whitePlayer: type,
                            blackPlayer: playerConfig!.blackPlayer,
                          ));
                        }
                      },
                    ),
            ],
          ),
        );
      }
    );
  }

  Widget getTimeDisplay(StopWatchTimer timer, double padding, Color color){
    return 
      Container(
        padding: EdgeInsets.all(padding),
        decoration: BoxDecoration(
          color: color,             // button background
          borderRadius: BorderRadius.circular(12), // rounded corners
          boxShadow: const [
            BoxShadow(
              color: Colors.black26,
              blurRadius: 4,
              offset: Offset(2, 2),
            ),
          ],
        ),
        child: StreamBuilder<int>(
          stream: timer.rawTime,
          initialData: 0,
          builder: (context, snap) {
            final value = snap.data;
            final displayTime = StopWatchTimer.getDisplayTime(value!);
            return Column(
              children: <Widget>[
                Padding(
                  padding: const EdgeInsets.all(8),
                  child: Text(
                    displayTime,
                    style: TextStyle(
                      fontSize: 20,
                      fontFamily: 'Helvetica',
                      fontWeight: FontWeight.bold,
                      color: color == Colors.black ? Colors.white : Colors.black,
                    ),
                  ),
                ),
              ],
            );
          },
        ),
      );
  } 

  Widget getPlayerTitle(String player){
    return Text(
      player,
      style: const TextStyle(
        fontSize: 24.0, // Larger size
        fontWeight: FontWeight.bold, // Bolder weight
        color: Colors.black, // A nice title color
        fontFamily: 'Roboto', // Or 'GoogleFonts.lato()', etc.
        letterSpacing: 1.2, // Slightly spread out
        shadows: [ // Optional glow/depth
          Shadow(
            blurRadius: 1.0,
            color: Colors.black54,
            offset: Offset(0.5, 0.5),
          ),
        ],
      ),
    );
  }

  /// Build the center button (Start Match / New Game)
  Widget _buildCenterButton() {
    // Engine match mode
    if (gameMode == GameMode.engineVsEngine && startEngineMatch != null) {
      return ElevatedButton(
        onPressed: startEngineMatch,
        child: const Text(
          'Start Engine Match',
          style: TextStyle(fontSize: 13.0),
        ),
      );
    }
    
    // Single match mode - show Start Match if provided, otherwise New Game
    // If onStartMatch is null, it means game is active, so show New Game
    if (onStartMatch != null) {
      return ElevatedButton(
        onPressed: onStartMatch,
        child: const Text(
          'Start Match',
          style: TextStyle(fontSize: 16.0),
        ),
      );
    }
    
    // Default: New Game button (shown when game is active)
    return ElevatedButton(
      onPressed: onNewGame,
      child: const Text(
        'New Game',
        style: TextStyle(fontSize: 16.0),
      ),
    );
  }

  /// Build a player selector dropdown
  Widget _buildPlayerSelector({
    required String playerName,
    PlayerType? playerType,
    required void Function(PlayerType) onTypeChanged,
    bool isEditable = true,
  }) {
    // If no player config, just show the name
    if (playerType == null) {
      return getPlayerTitle(playerName);
    }

    final playerDisplay = Row(
      mainAxisSize: MainAxisSize.min,
      children: [
        getPlayerTitle(playerName),
        const SizedBox(width: 8),
        Icon(
          playerType == PlayerType.human ? Icons.person : Icons.computer,
          size: 20,
        ),
      ],
    );

    // If not editable, just show the display without popup
    if (!isEditable) {
      return playerDisplay;
    }

    return PopupMenuButton<PlayerType>(
      onSelected: onTypeChanged,
      itemBuilder: (BuildContext context) => [
        const PopupMenuItem<PlayerType>(
          value: PlayerType.human,
          child: Row(
            children: [
              Icon(Icons.person, size: 20),
              SizedBox(width: 8),
              Text('Human'),
            ],
          ),
        ),
        const PopupMenuItem<PlayerType>(
          value: PlayerType.engine,
          child: Row(
            children: [
              Icon(Icons.computer, size: 20),
              SizedBox(width: 8),
              Text('Engine'),
            ],
          ),
        ),
      ],
      child: playerDisplay,
    );
  }

}



