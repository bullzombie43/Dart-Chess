/// Game mode enum representing different types of gameplay
enum GameMode {
  /// Human player vs chess engine
  humanVsEngine,
  
  /// Engine vs engine match (visualization or testing)
  engineVsEngine,
  
  /// Analysis mode (for future use)
  analysis,
}

/// Match mode for engine vs engine matches
enum MatchMode {
  /// Real-time visualization using direct UCI communication
  visualization,
  
  /// ELO testing using cutechess-cli with PGN replay
  testing,
}

/// Configuration for a game mode
class GameModeConfig {
  final GameMode mode;
  final bool isInteractive;
  final bool showTimers;
  final bool allowUndo;
  
  const GameModeConfig({
    required this.mode,
    required this.isInteractive,
    required this.showTimers,
    this.allowUndo = false,
  });
  
  /// Get configuration for a game mode
  static GameModeConfig forMode(GameMode mode) {
    switch (mode) {
      case GameMode.humanVsEngine:
        return const GameModeConfig(
          mode: GameMode.humanVsEngine,
          isInteractive: true,
          showTimers: true,
          allowUndo: false,
        );
      case GameMode.engineVsEngine:
        return const GameModeConfig(
          mode: GameMode.engineVsEngine,
          isInteractive: false,
          showTimers: false,
          allowUndo: false,
        );
      case GameMode.analysis:
        return const GameModeConfig(
          mode: GameMode.analysis,
          isInteractive: true,
          showTimers: false,
          allowUndo: true,
        );
    }
  }
}
