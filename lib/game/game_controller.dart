import 'package:chess_ui/game/chess_engine.dart';
import 'package:flutter/material.dart';
import 'package:stop_watch_timer/stop_watch_timer.dart';

class GameController extends ChangeNotifier{
  final ChessBoard board;
  final ChessEngine engine;
  final int totalTimeSeconds;
  final int timeIncrementSeconds; 

  final StopWatchTimer whiteTimer;
  final StopWatchTimer blackTimer;

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
}

