import 'package:chess_ui/game/chess_engine.dart';

/// Player type - human or engine
enum PlayerType {
  human,
  engine,
}

/// Configuration for game players
class PlayerConfig {
  final PlayerType whitePlayer;
  final PlayerType blackPlayer;
  
  const PlayerConfig({
    required this.whitePlayer,
    required this.blackPlayer,
  });
  
  /// Check if the given color should be played by engine
  bool isEngineColor(ChessColor color) {
    return color == ChessColor.white 
        ? whitePlayer == PlayerType.engine
        : blackPlayer == PlayerType.engine;
  }
  
  /// Check if the given color should be played by human
  bool isHumanColor(ChessColor color) {
    return !isEngineColor(color);
  }
  
  /// Get player type for a color
  PlayerType getPlayerType(ChessColor color) {
    return color == ChessColor.white ? whitePlayer : blackPlayer;
  }
  
  /// Default: human vs engine (human white, engine black)
  static const PlayerConfig defaultHumanVsEngine = PlayerConfig(
    whitePlayer: PlayerType.human,
    blackPlayer: PlayerType.engine,
  );
  
  /// Human vs human
  static const PlayerConfig humanVsHuman = PlayerConfig(
    whitePlayer: PlayerType.human,
    blackPlayer: PlayerType.human,
  );
  
  /// Engine vs engine
  static const PlayerConfig engineVsEngine = PlayerConfig(
    whitePlayer: PlayerType.engine,
    blackPlayer: PlayerType.engine,
  );
}
