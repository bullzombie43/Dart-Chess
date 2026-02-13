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
    final isEngineMatch = gameMode == GameMode.engineVsEngine;

    return LayoutBuilder(
      builder: (context, constraints) {
        final defaultPadding = constraints.maxWidth * 0.02;

        return Card(
          child: Padding(
            padding: EdgeInsets.symmetric(
              horizontal: defaultPadding * 1.5,
              vertical: 20,
            ),
            child: Column(
              mainAxisAlignment: MainAxisAlignment.center,
              mainAxisSize: MainAxisSize.max,
              children: [
                isEngineMatch
                    ? EngineDropdownButton(selectionCallback: setBlackEngine)
                    : _buildPlayerSelector(
                        context: context,
                        playerName: blackPlayer,
                        playerType: playerConfig?.blackPlayer,
                        isEditable: isPlayerConfigEditable,
                        onTypeChanged: (type) {
                          if (playerConfig != null &&
                              onPlayerConfigChanged != null) {
                            onPlayerConfigChanged!(PlayerConfig(
                              whitePlayer: playerConfig!.whitePlayer,
                              blackPlayer: type,
                            ));
                          }
                        },
                      ),
                const SizedBox(height: 20),
                getTimeDisplay(
                  context,
                  blackTimer,
                  defaultPadding,
                  isBlack: true,
                ),
                const SizedBox(height: 100),
                _buildCenterButton(),
                const SizedBox(height: 100),
                getTimeDisplay(
                  context,
                  whiteTimer,
                  defaultPadding,
                  isBlack: false,
                ),
                const SizedBox(height: 20),
                isEngineMatch
                    ? EngineDropdownButton(selectionCallback: setWhiteEngine)
                    : _buildPlayerSelector(
                        context: context,
                        playerName: whitePlayer,
                        playerType: playerConfig?.whitePlayer,
                        isEditable: isPlayerConfigEditable,
                        onTypeChanged: (type) {
                          if (playerConfig != null &&
                              onPlayerConfigChanged != null) {
                            onPlayerConfigChanged!(PlayerConfig(
                              whitePlayer: type,
                              blackPlayer: playerConfig!.blackPlayer,
                            ));
                          }
                        },
                      ),
              ],
            ),
          ),
        );
      },
    );
  }

  Widget getTimeDisplay(
    BuildContext context,
    StopWatchTimer timer,
    double padding, {
    required bool isBlack,
  }) {
    final theme = Theme.of(context);
    final colorScheme = theme.colorScheme;
    return Container(
      padding: EdgeInsets.all(padding),
      decoration: BoxDecoration(
        color: colorScheme.surfaceContainerLow,
        borderRadius: BorderRadius.circular(12),
        border: Border.all(
          color: isBlack
              ? colorScheme.onSurface.withValues(alpha: 0.4)
              : colorScheme.outlineVariant,
          width: 1,
        ),
        boxShadow: [
          BoxShadow(
            color: colorScheme.shadow.withValues(alpha: 0.08),
            blurRadius: 4,
            offset: const Offset(0, 2),
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
            mainAxisSize: MainAxisSize.min,
            children: [
              Text(
                isBlack ? 'Black' : 'White',
                style: theme.textTheme.labelMedium?.copyWith(
                  color: colorScheme.onSurfaceVariant,
                ) ?? TextStyle(
                  fontSize: 12,
                  color: colorScheme.onSurfaceVariant,
                ),
              ),
              const SizedBox(height: 4),
              Text(
                displayTime,
                style: theme.textTheme.titleLarge?.copyWith(
                  fontWeight: FontWeight.w600,
                  fontFeatures: const [FontFeature.tabularFigures()],
                  color: colorScheme.onSurface,
                ) ?? TextStyle(
                  fontSize: 20,
                  fontWeight: FontWeight.w600,
                  fontFeatures: const [FontFeature.tabularFigures()],
                  color: colorScheme.onSurface,
                ),
              ),
            ],
          );
        },
      ),
    );
  } 

  Widget getPlayerTitle(BuildContext context, String player) {
    final theme = Theme.of(context);
    return Text(
      player,
      style: theme.textTheme.titleMedium?.copyWith(
        fontWeight: FontWeight.bold,
        letterSpacing: 1.0,
      ) ?? TextStyle(
        fontSize: 16,
        fontWeight: FontWeight.bold,
        letterSpacing: 1.0,
        color: theme.colorScheme.onSurface,
      ),
    );
  }

  /// Build the center button (Start Match / New Game)
  Widget _buildCenterButton() {
    if (gameMode == GameMode.engineVsEngine && startEngineMatch != null) {
      return ElevatedButton(
        onPressed: startEngineMatch,
        child: const Text('Start Engine Match'),
      );
    }
    if (onStartMatch != null) {
      return ElevatedButton(
        onPressed: onStartMatch,
        child: const Text('Start Match'),
      );
    }
    return ElevatedButton(
      onPressed: onNewGame,
      child: const Text('New Game'),
    );
  }

  /// Build a player selector dropdown
  Widget _buildPlayerSelector({
    required BuildContext context,
    required String playerName,
    PlayerType? playerType,
    required void Function(PlayerType) onTypeChanged,
    bool isEditable = true,
  }) {
    final colorScheme = Theme.of(context).colorScheme;
    if (playerType == null) {
      return getPlayerTitle(context, playerName);
    }

    final playerDisplay = Row(
      mainAxisSize: MainAxisSize.min,
      children: [
        getPlayerTitle(context, playerName),
        const SizedBox(width: 8),
        Icon(
          playerType == PlayerType.human ? Icons.person : Icons.computer,
          size: 20,
          color: colorScheme.onSurface,
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



