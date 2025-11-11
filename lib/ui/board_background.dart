import 'package:chess_ui/ui/board_builder.dart';
import 'package:flutter/material.dart';

class Boardbackground extends StatelessWidget{
  final BackgroundConfig config; 

  /// Determines which way around the board is facing.
  /// 0 (white) will place the white pieces at the bottom,
  /// and 1 will place the black pieces there.
  final int orientation;

  final BoardSize boardSize;

  final BoardTheme theme;

  //key int is the square index
  final Map<int, HighlightType> highlights;

  //key int is the square index
  final Map<int, Marker> markers;

  const Boardbackground({
    super.key, 
    this.config = BackgroundConfig.standard,
    this.orientation = 0,
    this.boardSize = BoardSize.chess,
    this.theme = BoardTheme.brown,
    this.highlights = const {},
    this.markers = const {}
  });


  @override
  Widget build(BuildContext context) {
    return Boardbuilder(
      builder: (rank, file, squareSize) => 
        _square(context, rank, file, squareSize),
      size: boardSize
    );
  }

  Widget _square(BuildContext context, int rank, int file, double squareSize){
    int index = boardSize.getSquareIndex(rank, file, orientation);

    Color squareColor = ((rank + file) % 2 == 0) ? theme.darkSquare : theme.lightSquare;
    
    Widget? child;

    if(config.drawHighlightedSquares && highlights.containsKey(index)){
      squareColor = Color.alphaBlend(theme._getHighlight(highlights[index]!), squareColor);
    }

    if(config.drawMarkers && markers.containsKey(index)){
      child = markers[index]!.hasPiece ? _getPieceMarker(index, squareSize) : _getEmptyMarker(index, squareSize);
    }

    return AnimatedContainer(
      duration: const Duration(milliseconds: 100),
      width: squareSize,
      height: squareSize,
      color: squareColor,
      child: child,
    );
  }

  Widget _getEmptyMarker(int index, double squareSize){
    return SizedBox(
        width: squareSize,
        height: squareSize,
        child: Padding(
          padding: EdgeInsets.all(squareSize / 3),
          child: Container(
            decoration: BoxDecoration(
              color: theme._getHighlight(markers[index]!.colour),
              shape: BoxShape.circle,
            ),
          ),
        ),
      );
  }

  Widget _getPieceMarker(int index, double squareSize){
    return Container(
      width: squareSize,
      height: squareSize,
      decoration: BoxDecoration(
        borderRadius: BorderRadius.circular(squareSize / 6),
        border: Border.all(
          color: theme._getHighlight(markers[index]!.colour),
          width: squareSize / 16,
        ),
      ),
    );
  }
  
}


class BackgroundConfig {
  final bool drawHighlightedSquares;
  final bool drawMarkers;
  final double? opacity;

  const BackgroundConfig({
    this.drawHighlightedSquares = true,
    this.drawMarkers = true,
    this.opacity
  });

  static const standard = BackgroundConfig();
}


class BoardSize{
  final int rows;
  final int cols;

  double get aspectRatio => rows / cols;

  int get numSquares => rows * cols;

  int get maxRank => cols - 1;

  int get maxFile => rows - 1;

  const BoardSize(this.rows, this.cols);

  static const chess = BoardSize(8, 8);

  /// Gets a square id for [rank], [file] and [orientation].
  /// Orientation can be 0 (white) or 1 (black);
  int getSquareIndex(int rank, int file, int orientation){
    if (orientation == 0) {
      return rank * 8 + file;
    } else {
      return (7 - rank) * 8 + (7 - file);
    }
  }
}

class BoardTheme{
  /// Base colour of the light squares.
  final Color lightSquare;

  /// Base colour of the dark squares.
  final Color darkSquare;

  /// Colour for squares in check.
  final Color check;

  /// Colour for squares in checkmate.
  final Color checkmate;

  /// Colour for previous move highlights.
  final Color previous;

  /// Colour for selected pieces, and possible move decorations.
  final Color selected;

  /// Colour for committed premoves.
  final Color premove;


  const BoardTheme({
    required this.lightSquare,
    required this.darkSquare,
    required this.check,
    required this.checkmate,
    required this.previous,
    required this.selected,
    required this.premove
  });

  Color _getHighlight(HighlightType highlight){
    switch(highlight){
      case HighlightType.check: return check;
      case HighlightType.checkmate: return checkmate;
      case HighlightType.premove: return premove;
      case HighlightType.previous: return previous;
      case HighlightType.selected: return selected; 
    }
  }

  /// Brown. Classic. Looks like chess.
  static const brown = BoardTheme(
    lightSquare: Color(0xfff0d9b6),
    darkSquare: Color(0xffb58863),
    check: Color(0xffeb5160),
    checkmate: Colors.orange,
    previous: Color(0x809cc700),
    selected: Color(0x8014551e),
    premove: Color(0x80141e55),
  );

  /// A more modern blueish greyish theme.
  static const blueGrey = BoardTheme(
    lightSquare: Color(0xffdee3e6),
    darkSquare: Color(0xff788a94),
    check: Color(0xffeb5160),
    checkmate: Colors.orange,
    previous: Color(0x809bc700),
    selected: Color(0x8014551e),
    premove: Color(0x807b56b3),
  );

  /// Eye pain theme.
  static const pink = BoardTheme(
    lightSquare: Color(0xffeef0c7),
    darkSquare: Color(0xffe27c78),
    check: Color(0xffcb3927),
    checkmate: Colors.blue,
    previous: Color(0xff6ad1eb),
    selected: Color(0x8014551e),
    premove: Color(0x807b56b3),
  );

  /// A tribute.
  static const dart = BoardTheme(
    lightSquare: Color(0xff41c4ff),
    darkSquare: Color(0xff0f659f),
    check: Color(0xffeb5160),
    checkmate: Color(0xff56351e),
    previous: Color(0x80a9fbd7),
    selected: Color(0x80f6f1d1),
    premove: Color(0x80e3d8f1),
  );

}

/// Various types of highlights that might be present on the board.
enum HighlightType {
  check,
  checkmate,
  previous,
  selected,
  premove,
}

/// Data representation of a marker on the board, to simplify building them.
class Marker {
  /// The colour of the marker.
  final HighlightType colour;

  /// Whether the marker is for a square that has a piece on it or not.
  final bool hasPiece;

  const Marker({
    required this.colour,
    required this.hasPiece,
  });
  factory Marker.empty(HighlightType colour) =>
      Marker(colour: colour, hasPiece: false);
  factory Marker.piece(HighlightType colour) =>
      Marker(colour: colour, hasPiece: true);
}