import 'package:chess_ui/game/chess_engine.dart';
import 'package:chess_ui/game/game_mode.dart';
import 'package:flutter/material.dart';
import 'package:stop_watch_timer/stop_watch_timer.dart';

/// Match status for tracking game/match state
enum MatchStatus {
  idle,
  active,
  paused,
  finished,
}

class GameController extends ChangeNotifier{
  final ChessBoard board;
  final ChessEngine engine;
  final int totalTimeSeconds;
  final int timeIncrementSeconds; 

  final StopWatchTimer whiteTimer;
  final StopWatchTimer blackTimer;

  // Game mode state
  GameMode _gameMode = GameMode.humanVsEngine;
  GameMode get gameMode => _gameMode;
  GameModeConfig get modeConfig => GameModeConfig.forMode(_gameMode);

  // Match status
  MatchStatus _matchStatus = MatchStatus.idle;
  MatchStatus get matchStatus => _matchStatus;

  // Board selection state
  int? _selectedIndex;
  int? get selectedIndex => _selectedIndex;
  
  final Map<int, Move> _legalMoves = {};
  Map<int, Move> get legalMoves => Map.unmodifiable(_legalMoves);

  ChessColor get turn => board.getSideToMove();

  GameController({
    required this.board, 
    required this.engine, 
    this.timeIncrementSeconds = 1, 
    this.totalTimeSeconds = 120}
  ) : whiteTimer = StopWatchTimer(
    mode: StopWatchMode.countDown,
    presetMillisecond: StopWatchTimer.getMilliSecFromSecond(totalTimeSeconds), // millisecond => minute.
  ), blackTimer = StopWatchTimer(
    mode: StopWatchMode.countDown,
    presetMillisecond: StopWatchTimer.getMilliSecFromSecond(totalTimeSeconds), // millisecond => minute.
  );

  void makeMove(Move move){
    board.makeMove(move);

    if(turn == ChessColor.black){
      startBlackTimer();
      stopWhiteTimer();
      incrementWhiteTimer();
    } else {
      startWhiteTimer();
      stopBlackTimer();
      incrementBlackTimer();
    }

    notifyListeners();
  }

  void startWhiteTimer(){
    whiteTimer.onStartTimer();
  }

  void stopWhiteTimer(){
    whiteTimer.onStopTimer();
  }

  void incrementWhiteTimer(){
    whiteTimer.setPresetTime(mSec: (timeIncrementSeconds*1000));
  }

  void startBlackTimer(){
    blackTimer.onStartTimer();
  }

  void stopBlackTimer(){
    blackTimer.onStopTimer();
  }

  void incrementBlackTimer(){
    blackTimer.setPresetTime(mSec: (timeIncrementSeconds*1000));
  }

  void resetTimers(){
    blackTimer.onStopTimer();
    whiteTimer.onStopTimer();
    blackTimer.setPresetSecondTime(totalTimeSeconds, add: false);
    whiteTimer.setPresetSecondTime(totalTimeSeconds, add: false);
    blackTimer.onResetTimer();
    whiteTimer.onResetTimer();

    notifyListeners();
  }

  /// Set the game mode
  void setGameMode(GameMode mode) {
    if (_gameMode != mode) {
      _gameMode = mode;
      clearSelection(); // Clear selection when switching modes
      notifyListeners();
    }
  }

  /// Select a square and update legal moves
  void selectSquare(int index) {
    if (index < 0 || index > 63) {
      throw RangeError('Square index must be 0-63, got: $index');
    }

    final piece = board.getPieceAt(index);
    
    // If clicking the same square, deselect
    if (_selectedIndex == index) {
      clearSelection();
      return;
    }

    // If clicking a different piece of the same color, select that instead
    if (_selectedIndex != null) {
      final currentPiece = board.getPieceAt(_selectedIndex!);
      if (piece != PieceType.none && 
          piece.isWhite == currentPiece.isWhite) {
        _selectedIndex = index;
        _updateLegalMoves(index);
        notifyListeners();
        return;
      }
    }

    // Select new square if it has a piece
    if (piece != PieceType.none) {
      _selectedIndex = index;
      _updateLegalMoves(index);
      notifyListeners();
    } else {
      // Clicking empty square - check if it's a legal move destination first
      // (This check is done in MoveHandler before calling selectSquare)
      // If not a legal move, deselect
      clearSelection();
    }
  }

  /// Clear the current selection
  void clearSelection() {
    if (_selectedIndex != null || _legalMoves.isNotEmpty) {
      _selectedIndex = null;
      _legalMoves.clear();
      notifyListeners();
    }
  }

  /// Get legal moves for a specific square
  List<Move> getLegalMovesForSquare(int square) {
    if (square < 0 || square > 63) {
      return [];
    }

    final allMoves = engine.generateLegalMoves(board);
    return allMoves.where((move) => move.fromSquare == square).toList();
  }

  /// Update legal moves for the currently selected square
  void _updateLegalMoves(int square) {
    _legalMoves.clear();
    final moves = getLegalMovesForSquare(square);
    for (final move in moves) {
      _legalMoves[move.toSquare] = move;
    }
  }

  /// Set match status
  void setMatchStatus(MatchStatus status) {
    if (_matchStatus != status) {
      _matchStatus = status;
      notifyListeners();
    }
  }

  /// Reset the game to starting position
  void resetGame() {
    board.setFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    resetTimers();
    clearSelection();
    setMatchStatus(MatchStatus.idle);
  }
}

