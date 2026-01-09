import 'package:chess_ui/game/chess_engine.dart';
import 'package:flutter/material.dart';
import 'package:stop_watch_timer/stop_watch_timer.dart';

class BoardControls extends StatelessWidget {
  final StopWatchTimer whiteTimer;
  final StopWatchTimer blackTimer;
  final ChessBoard board;
  final VoidCallback onNewGame;
  final String blackPlayer;
  final String whitePlayer;
  

  const BoardControls({
    super.key, 
    required this.whiteTimer, 
    required this.blackTimer, 
    required this.board, 
    required this.onNewGame,
    required this.blackPlayer,
    required this.whitePlayer,
  });

  @override
  Widget build(BuildContext context) {
    
    return LayoutBuilder(
      builder: (context, constraints) {
        final defaultPadding = constraints.maxWidth * 0.02; // 2% of parent width

        return Container(
              color: Colors.grey,
              child: Column(
                mainAxisAlignment: MainAxisAlignment.center,
                mainAxisSize: MainAxisSize.max,
                children: [
                  getPlayerTitle(blackPlayer),
                  const SizedBox(height: 20),
                  getTimeDisplay(blackTimer, defaultPadding, Colors.black),
                  const SizedBox(height: 100),
                   ElevatedButton(
                    onPressed: () {
                      onNewGame();
                    },
                    child: const Text('New Game'),
                  ),
                  const SizedBox(height: 100,),
                  getTimeDisplay(whiteTimer, defaultPadding, Colors.white),
                  const SizedBox(height: 20),
                  getPlayerTitle(whitePlayer),                 
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
}



