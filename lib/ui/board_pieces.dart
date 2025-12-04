import 'package:chess_ui/game/chess_engine.dart';
import 'package:chess_ui/ui/board_background.dart';
import 'package:chess_ui/ui/board_builder.dart';
import 'package:flutter/material.dart';
import 'package:flutter_svg/flutter_svg.dart';

class BoardPieces extends StatefulWidget{
  final BoardSize size;

  final int orientation;

  final ChessBoard board;

  /// Called when a piece is tapped.
  final void Function(int)? onTap;

  /// Called when a drag is started.
  final void Function(int)? onDragStarted;

  /// Called when a drag is cancelled.
  final void Function(int)? onDragCancelled;

  /// Called when a drag ends, i.e. a piece was dropped on a target.
  final void Function(int, int)? onDragEnd;

  final double piecePadding;

  const BoardPieces({
    super.key,
    this.size = BoardSize.chess,
    this.orientation = 0,
    required this.board,
    this.onTap,
    this.onDragStarted,
    this.onDragCancelled,
    this.onDragEnd,
    this.piecePadding = 0.0,
  });

  @override
  State<StatefulWidget> createState() => _BoardPiecesState();

}

class _BoardPiecesState extends State<BoardPieces> {
  final Map<PieceType, String> assets = {
    PieceType.wPawn: "assets/pieces/wP.svg",
    PieceType.wKnight: "assets/pieces/wN.svg",
    PieceType.wBishop: "assets/pieces/wB.svg",
    PieceType.wRook: "assets/pieces/wR.svg",
    PieceType.wQueen: "assets/pieces/wQ.svg",
    PieceType.wKing: "assets/pieces/wK.svg",
    PieceType.bPawn: "assets/pieces/bP.svg",
    PieceType.bKnight: "assets/pieces/bN.svg",
    PieceType.bBishop: "assets/pieces/bB.svg",
    PieceType.bRook: "assets/pieces/bR.svg",
    PieceType.bQueen: "assets/pieces/bQ.svg",
    PieceType.bKing: "assets/pieces/bK.svg",
  };

  @override
  Widget build(BuildContext context) {
    return Boardbuilder(
      builder: (rank, file, squareSize) => 
        _piece(context, rank, file, squareSize),
      size: widget.size,
    );
  }

  Widget _piece(BuildContext context, int rank, int file, double squareSize){
    int index = widget.size.getSquareIndex(rank, file, widget.orientation);

    bool hasPiece = widget.board.getPieceAt(index) != PieceType.none;

    Widget piece = hasPiece ? 
      PieceWidget(
        assetPath: assets[widget.board.getPieceAt(index)]!,
        size: squareSize,
        draggable: true,
        onTap: () => widget.onTap != null ? widget.onTap!(index) : print("tapped"),
        onDragEnd: (offset) {
          double maxSize = squareSize * 8;

          // Check if drop is outside board bounds by a reasonable margin
          if (offset.dx < 0 || offset.dy < 0 || 
              offset.dx >= maxSize || offset.dy >= maxSize) {
            return;
          }

          int file = (offset.dx / squareSize).floor();
          int rank = (offset.dy / squareSize).floor();

          // Clamp to board bounds (in case of small overshoots)
          file = file.clamp(0, 7);
          rank = 7 - rank.clamp(0, 7);

          int endIndex = widget.size.getSquareIndex(rank, file, widget.orientation);

          widget.onDragEnd != null ? widget.onDragEnd!.call(index, endIndex) : print("Dropped at $offset");
        },
      ) 
      
      : GestureDetector(
          onTap: () => widget.onTap != null ? widget.onTap!(index) : print("tapped"),
          child: Container(
            width: squareSize,
            height: squareSize,
            color: Colors.transparent, // <-- important!
        ),
      );

    return SizedBox(
      width: squareSize,
      height: squareSize,
      child: piece,
    );
  }
}

class PieceWidget extends StatelessWidget {
  final String assetPath;
  final double size;
  final bool draggable;
  final VoidCallback? onTap;
  final void Function(Offset)? onDragEnd;

  const PieceWidget({
    super.key,
    required this.assetPath,
    required this.size,
    this.draggable = false,
    this.onTap,
    this.onDragEnd,
  });

  @override
  Widget build(BuildContext context) {
    final pieceImage = SvgPicture.asset(
      assetPath,
      width: size,
      height: size,
      fit: BoxFit.contain,
    );

    if (draggable) {
      return Draggable<String>(
        data: assetPath, // You can attach move data here if needed
        feedback: pieceImage,
        childWhenDragging: const SizedBox.shrink(),
        onDragEnd: (details) => onDragEnd?.call(details.offset),
        child: GestureDetector(onTap: onTap, child: pieceImage),
      );
    } else {
      return GestureDetector(onTap: onTap, child: pieceImage);
    }
  }
}