import 'package:chess_ui/game/game_state.dart';
import 'package:chess_ui/ui/board_background.dart';
import 'package:flutter/material.dart';

class BoardTouch extends StatefulWidget {
  final GameState gameState;

  final BoardSize size;

  final int orientation;

  const BoardTouch({
    super.key, 
    required this.gameState,
    this.size = BoardSize.chess,
    this.orientation = 0,
  });



  @override
  State<BoardTouch> createState() => _BoardTouchState();
}

class _BoardTouchState extends State<BoardTouch> {


  @override
  Widget build(BuildContext context) {
    // TODO: implement build
    throw UnimplementedError();
  }

}