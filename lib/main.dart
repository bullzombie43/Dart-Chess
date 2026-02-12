import 'package:chess_ui/game/chess_engine.dart';
import 'package:chess_ui/game/game_controller.dart';
import 'package:chess_ui/game/match_manager.dart';
import 'package:chess_ui/ui/log_replay_view.dart';
import 'package:chess_ui/ui/single_match_view.dart';
import 'package:chess_ui/ui/testing_view.dart';
import 'package:flutter/material.dart';
import 'package:provider/provider.dart';
import 'package:window_manager/window_manager.dart';

void main() async {
  WidgetsFlutterBinding.ensureInitialized();
  await windowManager.ensureInitialized();

  WindowOptions windowOptions = const WindowOptions(
    size: Size(1000, 800),
    center: true,
    backgroundColor: Colors.transparent,
    skipTaskbar: false,
    titleBarStyle: TitleBarStyle.hidden,
  );

  windowManager.waitUntilReadyToShow(windowOptions, () async {
    await windowManager.show();
    await windowManager.focus();
    await windowManager.setAspectRatio(10.0 / 8.0);
  });

  // Initialize game components
  final board = ChessBoard();
  final engine = ChessEngine();
  final controller = GameController(board: board, engine: engine);
  final matchManager = MatchManager(controller);

  runApp(
    ChangeNotifierProvider(
      create: (context) => controller,
      child: MyApp(
        controller: controller,
        engine: engine,
        matchManager: matchManager,
      ),
    ),
  );
}

class MyApp extends StatelessWidget {
  final GameController controller;
  final ChessEngine engine;
  final MatchManager matchManager;

  const MyApp({
    super.key,
    required this.controller,
    required this.engine,
    required this.matchManager,
  });

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Chess UI',
      theme: ThemeData(
        colorScheme: ColorScheme.fromSeed(seedColor: Colors.deepPurple),
        useMaterial3: true,
      ),
      home: HomePage(
        controller: controller,
        engine: engine,
        matchManager: matchManager,
      ),
    );
  }
}

/// Home page that routes to appropriate view based on game mode
class HomePage extends StatefulWidget {
  final GameController controller;
  final ChessEngine engine;
  final MatchManager matchManager;

  const HomePage({
    super.key,
    required this.controller,
    required this.engine,
    required this.matchManager,
  });

  @override
  State<HomePage> createState() => _HomePageState();
}

enum AppScreen {
  singleMatch,
  testing,
  replayLogs,
}

class _HomePageState extends State<HomePage> {
  AppScreen _currentScreen = AppScreen.singleMatch;

  void _switchScreen(AppScreen screen) {
    setState(() {
      _currentScreen = screen;
    });
  }

  @override
  Widget build(BuildContext context) {
    // Route to appropriate view based on screen
    Widget currentView;
    switch (_currentScreen) {
      case AppScreen.singleMatch:
        currentView = SingleMatchView(
          controller: widget.controller,
          engine: widget.engine,
        );
        break;
      case AppScreen.testing:
        currentView = TestingView(
          controller: widget.controller,
          matchManager: widget.matchManager,
        );
        break;
      case AppScreen.replayLogs:
        currentView = LogReplayView(
          controller: widget.controller,
        );
        break;
    }

    return Scaffold(
      appBar: AppBar(
        title: const Text('Chess UI'),
        actions: [
          PopupMenuButton<String>(
            icon: const Icon(Icons.settings),
            onSelected: (value) {
              if (value == 'single_match') {
                _switchScreen(AppScreen.singleMatch);
              } else if (value == 'testing') {
                _switchScreen(AppScreen.testing);
              } else if (value == 'replay_logs') {
                _switchScreen(AppScreen.replayLogs);
              }
            },
            itemBuilder: (BuildContext context) => [
              PopupMenuItem<String>(
                value: 'single_match',
                child: Row(
                  children: [
                    Icon(
                      Icons.sports_esports,
                      size: 20,
                      color: _currentScreen == AppScreen.singleMatch 
                          ? Theme.of(context).colorScheme.primary 
                          : null,
                    ),
                    const SizedBox(width: 8),
                    Text(
                      'Single Match',
                      style: _currentScreen == AppScreen.singleMatch
                          ? TextStyle(
                              fontWeight: FontWeight.bold,
                              color: Theme.of(context).colorScheme.primary,
                            )
                          : null,
                    ),
                  ],
                ),
              ),
              PopupMenuItem<String>(
                value: 'testing',
                child: Row(
                  children: [
                    Icon(
                      Icons.science,
                      size: 20,
                      color: _currentScreen == AppScreen.testing 
                          ? Theme.of(context).colorScheme.primary 
                          : null,
                    ),
                    const SizedBox(width: 8),
                    Text(
                      'Testing',
                      style: _currentScreen == AppScreen.testing
                          ? TextStyle(
                              fontWeight: FontWeight.bold,
                              color: Theme.of(context).colorScheme.primary,
                            )
                          : null,
                    ),
                  ],
                ),
              ),
              PopupMenuItem<String>(
                value: 'replay_logs',
                child: Row(
                  children: [
                    Icon(
                      Icons.movie,
                      size: 20,
                      color: _currentScreen == AppScreen.replayLogs
                          ? Theme.of(context).colorScheme.primary
                          : null,
                    ),
                    const SizedBox(width: 8),
                    Text(
                      'Replay Logs',
                      style: _currentScreen == AppScreen.replayLogs
                          ? TextStyle(
                              fontWeight: FontWeight.bold,
                              color: Theme.of(context).colorScheme.primary,
                            )
                          : null,
                    ),
                  ],
                ),
              ),
            ],
          ),
        ],
      ),
      body: currentView,
    );
  }
}
