import 'package:chess_ui/ui/board_background.dart';
import 'package:flutter/material.dart';

typedef SquareBuilder = Widget ? Function(int rank, int file, double squareSize);

class Boardbuilder extends StatelessWidget{
  final SquareBuilder builder;

  final BoardSize size;

  const Boardbuilder({
    super.key,
    required this.builder,
    this.size = BoardSize.chess
  });
  
  @override
  Widget build(BuildContext context) {    
    return AspectRatio(
            aspectRatio: size.aspectRatio,
            child: LayoutBuilder(
              builder: (context, constraints) {
                // Pick the smaller dimension to keep it fully visible
                double windowSize = constraints.maxWidth < constraints.maxHeight
                    ? constraints.maxWidth
                    : constraints.maxHeight;

                double squareSize = windowSize / size.rows;

                return GridView.builder(
                  gridDelegate: SliverGridDelegateWithFixedCrossAxisCount(crossAxisCount: size.cols), 
                  itemCount: 64,
                  itemBuilder: (context, index) {
                    int file = index % 8;
                    int rank = 7 - index ~/ 8;
                    
                    return SizedBox(
                      width: squareSize,
                      height: squareSize,
                      child: builder(rank, file, squareSize)
                    );
                  },
                  physics: const NeverScrollableScrollPhysics(),
                  reverse: false,
                );
              }
            )
          );
  }
}