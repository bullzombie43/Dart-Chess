import 'package:flutter/material.dart';
import 'package:stop_watch_timer/stop_watch_timer.dart';

class BoardControls extends StatefulWidget {
  final double totalTime;
  final double timeIncrement;
  

  const BoardControls({super.key, required this.totalTime, required this.timeIncrement});

  @override
  BoardControlsState createState() => BoardControlsState();
}

class BoardControlsState extends State<BoardControls> {

  final _whiteTimer = StopWatchTimer(
    mode: StopWatchMode.countDown,
    presetMillisecond: StopWatchTimer.getMilliSecFromMinute(1), // millisecond => minute.
  );

  final _blackTimer = StopWatchTimer(
    mode: StopWatchMode.countDown,
    presetMillisecond: StopWatchTimer.getMilliSecFromMinute(1), // millisecond => minute.
  );

  @override
  void initState() {
    super.initState();
  }

  @override
  void dispose() async {
    super.dispose();
    await _whiteTimer.dispose();  // Need to call dispose function.
    await _blackTimer.dispose();
  }

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
                  getTimeDisplay(_whiteTimer, defaultPadding, Colors.black),
                  getTimeDisplay(_blackTimer, defaultPadding, Colors.white),
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
}

