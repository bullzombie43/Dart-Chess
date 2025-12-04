import 'package:chess_ui/game/chess_engine.dart';
import 'package:chess_ui/ui/board_background.dart';
import 'package:chess_ui/ui/board_pieces.dart';
import 'package:flutter/material.dart';
import 'package:flutter_svg/svg.dart';
import 'package:window_manager/window_manager.dart';
import 'package:flutter/material.dart' as material;

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

  ChessBoard board = ChessBoard();
  ChessEngine engine = ChessEngine();


  runApp(MyApp(board: board, engine: engine,));
}

class MyApp extends StatelessWidget {

  final ChessBoard board;
  final ChessEngine engine;

  const MyApp({
    super.key,
    required this.board,
    required this.engine
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
      home: MyHomePage(title: 'Chess UI', board: board, engine: engine,),
    );
  }
}

class MyHomePage extends StatefulWidget {
  final ChessBoard board;
  final ChessEngine engine;
  final BoardSize boardSize;
  final int orientation;

  const MyHomePage({
    super.key, 
    required this.title,
    required this.board,
    required this.engine,
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
  Map<int, Move> legalMoves = {}; //The legal moves for the currently selected piece
  int? selectedIndex;
  Map<int, HighlightType> highlights = {};

  void fullPlayerMove(Move move){
    widget.board.makeMove(move);

    //Check for checkmate after each move
   
  }

  Future<void> handleSquareTap(int index) async {
    // Handle logic before calling setState
    if (selectedIndex == null) {
      final PieceType piece = widget.board.getPieceAt(index);

      //Update legal moves first
      legalMoves.clear();
      List<Move> allPossibleMoves = widget.engine.generateLegalMoves(widget.board);

      for(Move move in allPossibleMoves){
        if(move.fromSquare == index){
          legalMoves[move.toSquare] = move;
        }
      }

      if (piece != PieceType.none) {
        // Just compute markers (no need for async here)
        final newMarkers = _generateMarkers(
          target: index,
          board: widget.board,
          engine: widget.engine,
          color: piece.isWhite ? Color.white : Color.black,
          legalMoves: legalMoves
        );

        setState(() {
          markers = newMarkers;
          selectedIndex = index;
          highlights = _generateHighlights(board: widget.board, engine: widget.engine);
        });
      }
    } else {
      // Piece already selected
      if (markers.containsKey(index)) { //index is the ending square which is also the int value in our map
        print("Index Tapped: $index");
        // Legal move tapped → perform move
        Move move = legalMoves[index]!;

        // Check promotion before updating UI
        if (isPromotionAttempt(PieceType.fromValue(move.piece), move.fromSquare, move.toSquare)) {
          PieceType? choice = await showPromotionDialog(context, PieceType.fromValue(move.piece).isWhite ? Color.white : Color.black);
          if (choice == null) return; // user cancelled
          print("Entered");
          widget.board.makeMove(
            Move(
              piece: move.piece,
              fromSquare: move.fromSquare,
              toSquare: move.toSquare,
              capturedPiece: move.capturedPiece,
              promotedPiece: choice.value,
              isCastling: false,
              isEnPassant: false
            )
          );
        } else {
          widget.board.makeMove(move);
        }

        // Now update state synchronously
        setState(() {
          selectedIndex = null;
          markers.clear();
          legalMoves.clear();
          highlights = _generateHighlights(board: widget.board, engine: widget.engine);
        });
      } else if (widget.board.getPieceAt(index) != PieceType.none && index != selectedIndex) {
          //Regenerate legal moves for the new square
          legalMoves.clear();
          List<Move> allPossibleMoves = widget.engine.generateLegalMoves(widget.board);

          for(Move move in allPossibleMoves){
            if(move.fromSquare == index){
              legalMoves[move.toSquare] = move;
            }
          }

          final newMarkers = _generateMarkers(
            target: index,
            board: widget.board,
            engine: widget.engine,
            color: widget.board.getPieceAt(index).isWhite ? Color.white : Color.black,
            legalMoves: legalMoves
          );

          setState(() {
            selectedIndex = index;
            markers = newMarkers;
            highlights = _generateHighlights(board: widget.board, engine: widget.engine);
          });
      } else {
        // Tapped empty/illegal square → deselect
        setState(() {
          selectedIndex = null;
          markers.clear();
          legalMoves.clear();
          highlights = _generateHighlights(board: widget.board, engine: widget.engine);
        });
      }
    }
  }

  void handlePieceDragEnd(int startIndex, int endIndex){
    setState(() {
      Move? move;
      List<Move> allPossibleMoves = widget.engine.generateLegalMoves(widget.board);

      for(Move m in allPossibleMoves){
        if(m.fromSquare == startIndex && m.toSquare == endIndex){
          move = m;
        }
      }

      if(move != null){
        widget.board.makeMove(move);
      }

      highlights.clear();

      highlights = _generateHighlights(board: widget.board, engine: widget.engine);
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
      backgroundColor: const material.Color.fromARGB(255, 140, 208, 161),
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
                board: widget.board,
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

  Map<int, Marker> _generateMarkers({required int target, required ChessBoard board, required ChessEngine engine, required Color color, required Map<int, Move> legalMoves}){
    Map<int, Marker> markers = {};

    if(board.getSideToMove() != color){
      return markers;
    }
    
    for(final Move move in legalMoves.values){
      HighlightType highlightType = HighlightType.selected;
      markers[move.toSquare] = PieceType.fromValue(move.capturedPiece) != PieceType.none ? Marker.piece(highlightType) : Marker.empty(highlightType);
    }

    return markers;
  }

  Map<int, HighlightType> _generateHighlights({required ChessBoard board, required ChessEngine engine}){
    HighlightType selectionColor = HighlightType.selected;

    Map<int, HighlightType> highlights = {};

    return highlights;
  }

  Future<PieceType?> showPromotionDialog(BuildContext context, Color color) async {
    List<PieceType> promotionChoices = color == Color.white
        ? [PieceType.wQueen, PieceType.wRook, PieceType.wBishop, PieceType.wKnight]
        : [PieceType.bQueen, PieceType.bRook, PieceType.bBishop, PieceType.bKnight];

    return await showDialog<PieceType>(
      context: context,
      barrierDismissible: true,
      builder: (context) {
        return AlertDialog(
          title: const Text('Promote to:'),
          content: Row(
            mainAxisAlignment: MainAxisAlignment.spaceEvenly,
            children: [
              for (PieceType piece in promotionChoices)
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

  bool isPromotionAttempt(PieceType piece, int from, int to) {
    // White promotion rank = rank 8 → index 56–63
    // Black promotion rank = rank 1 → index 0–7
    int rankTo = rankOf(to);
    return (piece == PieceType.wPawn && rankTo == 7) ||
          (piece == PieceType.bPawn && rankTo == 0);
  }

  void showCheckmateDialog(
    BuildContext context, {
    required String winner,
    required VoidCallback onNewGame,
  }) {
    showDialog(
      context: context,
      barrierDismissible: false,
      builder: (BuildContext context) {
        return AlertDialog(
          shape: RoundedRectangleBorder(
            borderRadius: BorderRadius.circular(20),
          ),
          title: const Column(
            children: [
              Icon(
                Icons.emoji_events,
                color: Colors.amber,
                size: 48,
              ),
              SizedBox(height: 8),
              Text(
                'Checkmate!',
                style: TextStyle(
                  fontSize: 28,
                  fontWeight: FontWeight.bold,
                ),
              ),
            ],
          ),
          content: Container(
            padding: const EdgeInsets.all(16),
            decoration: BoxDecoration(
              color: winner == 'White' ? Colors.grey[100] : Colors.grey[800],
              borderRadius: BorderRadius.circular(12),
            ),
            child: Column(
              mainAxisSize: MainAxisSize.min,
              children: [
                Text(
                  '$winner wins!',
                  style: TextStyle(
                    fontSize: 24,
                    fontWeight: FontWeight.bold,
                    color: winner == 'White' ? Colors.black : Colors.white,
                  ),
                ),
                const SizedBox(height: 8),
                Text(
                  'Game Over',
                  style: TextStyle(
                    fontSize: 16,
                    color: winner == 'White' ? Colors.grey[700] : Colors.grey[300],
                  ),
                ),
              ],
            ),
          ),
          actions: [
            OutlinedButton(
              onPressed: () => Navigator.of(context).pop(),
              child: const Text('View Board'),
            ),
            ElevatedButton(
              onPressed: () {
                Navigator.of(context).pop();
                onNewGame();
              },
              style: ElevatedButton.styleFrom(
                backgroundColor: Colors.green,
                padding: const EdgeInsets.symmetric(horizontal: 24, vertical: 12),
              ),
              child: const Text('New Game'),
            ),
          ],
          actionsAlignment: MainAxisAlignment.spaceEvenly,
        );
      },
    );
  }

  void showGameOverDialog(
    BuildContext context, {
    required String result,  // "White wins", "Black wins", or "Draw"
    String? reason,          // "Checkmate", "Stalemate", etc.
    required VoidCallback onNewGame,
  }) {
    final isDraw = result == 'Draw';
    
    showDialog(
      context: context,
      barrierDismissible: false,
      builder: (BuildContext context) {
        return AlertDialog(
          title: Row(
            children: [
              Icon(
                isDraw ? Icons.handshake : Icons.emoji_events,
                color: isDraw ? Colors.blue : Colors.amber,
                size: 32,
              ),
              const SizedBox(width: 12),
              Text(reason ?? 'Game Over'),
            ],
          ),
          content: Text(
            result,
            style: const TextStyle(fontSize: 20, fontWeight: FontWeight.bold),
          ),
          actions: [
            TextButton(
              onPressed: () => Navigator.of(context).pop(),
              child: const Text('View Board'),
            ),
            ElevatedButton(
              onPressed: () {
                Navigator.of(context).pop();
                onNewGame();
              },
              child: const Text('New Game'),
            ),
          ],
        );
      },
    );
  }
}


