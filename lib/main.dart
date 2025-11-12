import 'package:chess_ui/game/game_state.dart';
import 'package:chess_ui/ui/board_background.dart';
import 'package:chess_ui/ui/board_pieces.dart';
import 'package:flutter/material.dart';
import 'package:flutter_svg/svg.dart';
import 'package:window_manager/window_manager.dart';

void main() async {
  WidgetsFlutterBinding.ensureInitialized();
  // Must add this line.
  await windowManager.ensureInitialized();

  WindowOptions windowOptions = const WindowOptions(
    size: Size(800, 800),
    center: true,
    backgroundColor: Colors.transparent,
    skipTaskbar: false,
    titleBarStyle: TitleBarStyle.hidden,
  );

  windowManager.waitUntilReadyToShow(windowOptions, () async {
    await windowManager.show();
    await windowManager.focus();
    await windowManager.setAspectRatio(1.0);
  });

  GameState state = GameState();
  //state.setBoardToFen("r3k2r/p1ppqpb1/bn2p1p1/3PN3/1p2n3/P1N2Q1p/1PPBBPPP/R3K2R w KQkq - 0 1");


  runApp(MyApp(state: state));
}

class MyApp extends StatelessWidget {

  final GameState state;

  const MyApp({
    super.key,
    required this.state
  });

  

  // This widget is the root of your application.
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Flutter Demo',
      theme: ThemeData(
        // This is the theme of your application.
        //
        // This works for code too, not just values: Most code changes can be
        // tested with just a hot reload.
        colorScheme: ColorScheme.fromSeed(seedColor: Colors.deepPurple),
        useMaterial3: true,
      ),
      home: MyHomePage(title: 'Chess UI', state: state),
    );
  }
}

class MyHomePage extends StatefulWidget {
  final GameState state;
  final BoardSize boardSize;
  final int orientation;

  const MyHomePage({
    super.key, 
    required this.title,
    required this.state,
    this.boardSize = BoardSize.chess,
    this.orientation = 0,
  });

  // This widget is the home page of your application. It is stateful, meaning
  // that it has a State object (defined below) that contains fields that affect
  // how it looks.

  // This class is the configuration for the state. It holds the values (in this
  // case the title) provided by the parent (in this case the App widget) and
  // used by the build method of the State. Fields in a Widget subclass are
  // always marked "final".

  final String title;

  @override
  State<MyHomePage> createState() => _MyHomePageState();
}

class _MyHomePageState extends State<MyHomePage> {
  Map<int, Marker> markers = {};
  Map<int, Move> legalMoves = {};
  int? selectedIndex;
  Map<int, HighlightType> highlights = {};

  void fullPlayerMove(Move move){
    final snapshot = widget.state.clone();

    widget.state.safeMakeMove(move);

    if(widget.state.isKingInCheck(snapshot.turn)){
      widget.state.restore(snapshot);
      print("Illegal move – your king would be in check.");
      
    }
  }

  Future<void> handleSquareTap(int index) async {
    // Handle logic before calling setState
    if (selectedIndex == null) {
      final Piece? piece = widget.state.getPieceAt(index);
      if (piece != null) {
        // Just compute markers (no need for async here)
        final newMarkers = _generateMarkers(
          target: index,
          state: widget.state,
          color: piece.color,
        );

        setState(() {
          markers = newMarkers;
          selectedIndex = index;
          highlights = _generateHighlights(state: widget.state);
        });
      }
    } else {
      // Piece already selected
      if (markers.containsKey(index)) {
        // Legal move tapped → perform move
        Move move = legalMoves[index]!;

        // Check promotion before updating UI
        if (isPromotionAttempt(movePiece(move), startingSquare(move), endingSquare(move))) {
          Piece? choice = await showPromotionDialog(context, movePiece(move).color);
          if (choice == null) return; // user cancelled
          print("Entered");
          widget.state.safeMakeMove(
            makeMoveInt(
              from: startingSquare(move), 
              to: endingSquare(move), 
              movingPiece: movePiece(move), 
              color: moveColor(move),
              isCapture: isCapture(move),
              isEP: isEnPassant(move),
              promotionPiece: choice
            )
          );
        } else {
          widget.state.safeMakeMove(move);
        }

        // Now update state synchronously
        setState(() {
          selectedIndex = null;
          markers.clear();
          legalMoves.clear();
          highlights = _generateHighlights(state: widget.state);
        });
      } else if (widget.state.getPieceAt(index) != null && index != selectedIndex) {
        final newMarkers = _generateMarkers(
          target: index,
          state: widget.state,
          color: widget.state.getPieceAt(index)!.color,
        );

        setState(() {
          selectedIndex = index;
          markers = newMarkers;
          highlights = _generateHighlights(state: widget.state);
        });
      } else {
        // Tapped empty/illegal square → deselect
        setState(() {
          selectedIndex = null;
          markers.clear();
          legalMoves.clear();
          highlights = _generateHighlights(state: widget.state);
        });
      }
    }
  }

  void handlePieceDragEnd(int startIndex, int endIndex){
    setState(() {
      Move? move;
      List<Move> possibleMoves = widget.state.generatePsuedoLegalMoves(startIndex);

      for(Move m in possibleMoves){
        if(startingSquare(m) == startIndex && endingSquare(m) == endIndex){
          move = m;
        }
      }

      if(move != null){
        widget.state.safeMakeMove(move);
      }

      highlights.clear();

      highlights = _generateHighlights(state: widget.state);
    });
  }

  @override
  Widget build(BuildContext context) {
    // This method is rerun every time setState is called, for instance as done
    // by the _incrementCounter method above.
    //
    // The Flutter framework has been optimized to make rerunning build methods
    // fast, so that you can just rebuild anything that needs updating rather
    // than having to individually change instances of widgets.
    return Scaffold(
      backgroundColor: const Color.fromARGB(255, 140, 208, 161),
      body:  Center(
        // Center is a layout widget. It takes a single child and positions it
        // in the middle of the parent.
        child: Padding(
          padding:  const EdgeInsets.all(24.0),
          child: Stack(
            children: [
              Boardbackground(
                markers: markers, 
                boardSize: widget.boardSize, 
                orientation: widget.orientation,
                highlights: highlights,
              ),
              BoardPieces(
                state: widget.state,
                size: widget.boardSize,
                orientation: widget.orientation, 
                onTap: handleSquareTap, 
                onDragEnd: handlePieceDragEnd,
                
              )
            ],
          ),
        )
      ),
    );
  }

  Map<int, Marker> _generateMarkers({required int target, required GameState state, required PieceColor color}){
    Map<int, Marker> markers = {};

    List<Move> legalMoves;

    if(state.turn == color){
      legalMoves = state.generateFullLegalMoves(target);
    } else {
      legalMoves = state.generatePsuedoLegalMoves(target);
    }
    

    for(final Move move in legalMoves){
      this.legalMoves[endingSquare(move)] = move;
      HighlightType highlightType = state.isTurnColor(color) ? HighlightType.selected : HighlightType.premove;
      markers[endingSquare(move)] = isCapture(move) ? Marker.piece(highlightType) : Marker.empty(highlightType);
    }

    return markers;
  }

  Map<int, HighlightType> _generateHighlights({required GameState state}){
    HighlightType selectionColor = HighlightType.selected;

    Map<int, HighlightType> highlights = {};

    if(widget.state.whiteToMove && widget.state.premoveBlack.isNotEmpty){
        for(Move move in widget.state.premoveBlack){
          highlights[startingSquare(move)] = HighlightType.premove;
          highlights[endingSquare(move)] = HighlightType.premove;
        }
      }

      if(!widget.state.whiteToMove && widget.state.premoveWhite.isNotEmpty){
       for(Move move in widget.state.premoveWhite){
          highlights[startingSquare(move)] = HighlightType.premove;
          highlights[endingSquare(move)] = HighlightType.premove;
        }
      }


    return highlights;
  }

  Future<Piece?> showPromotionDialog(BuildContext context, PieceColor color) async {
    List<Piece> promotionChoices = color == PieceColor.WHITE
        ? [Piece.whiteQueen, Piece.whiteRook, Piece.whiteBishop, Piece.whiteKnight]
        : [Piece.blackQueen, Piece.blackRook, Piece.blackBishop, Piece.blackKnight];

    return await showDialog<Piece>(
      context: context,
      barrierDismissible: true,
      builder: (context) {
        return AlertDialog(
          title: const Text('Promote to:'),
          content: Row(
            mainAxisAlignment: MainAxisAlignment.spaceEvenly,
            children: [
              for (Piece piece in promotionChoices)
                GestureDetector(
                  onTap: () => Navigator.pop(context, piece),
                  child: SvgPicture.asset(
                    'assets/pieces/${piece.asset}', // e.g. white_queen.png
                    height: 48,
                    width: 48,
                  ),
                ),
            ],
          ),
        );
      },
    );
  }

  bool isPromotionAttempt(Piece piece, int from, int to) {
    // White promotion rank = rank 8 → index 56–63
    // Black promotion rank = rank 1 → index 0–7
    int rankTo = rankOf(to);
    return (piece == Piece.whitePawn && rankTo == 7) ||
          (piece == Piece.blackPawn && rankTo == 0);
  }
}

