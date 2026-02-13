import 'package:chess_ui/game/game_controller.dart';
import 'package:chess_ui/game/game_mode.dart';
import 'package:chess_ui/game/match_manager.dart';
import 'package:chess_ui/ui/board_controls.dart';
import 'package:chess_ui/ui/game_board.dart';
import 'package:flutter/material.dart';

/// View for engine vs engine matches
/// 
/// Supports both visualization mode (real-time) and testing mode (PGN replay)
class MatchView extends StatefulWidget {
  final GameController controller;
  final MatchManager matchManager;
  final void Function(GameMode)? onModeChange;
  
  const MatchView({
    super.key,
    required this.controller,
    required this.matchManager,
    this.onModeChange,
  });

  @override
  State<MatchView> createState() => _MatchViewState();
}

class _MatchViewState extends State<MatchView> {
  MatchMode _currentMode = MatchMode.visualization;
  bool _isMatchActive = false;
  int _currentGameNumber = 0;
  String? _lastResult;
  String? _whiteEngine;
  String? _blackEngine;
  
  @override
  void initState() {
    super.initState();
    widget.controller.setGameMode(GameMode.engineVsEngine);
    
    // Listen to match events
    widget.matchManager.events.listen((event) {
      setState(() {
        switch (event.type) {
          case MatchEventType.matchStarted:
            _isMatchActive = true;
            break;
          case MatchEventType.gameStarted:
            _currentGameNumber = event.gameNumber ?? 0;
            break;
          case MatchEventType.gameFinished:
            _lastResult = event.result;
            break;
          case MatchEventType.matchFinished:
            _isMatchActive = false;
            break;
          default:
            break;
        }
      });
    });
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      body: Row(
        children: [
          // Chess board - Expanded so it shares space and shrinks with window
          Expanded(
            child: Padding(
              padding: EdgeInsets.all(MediaQuery.of(context).size.width * 0.024),
              child: GameBoard(
                controller: widget.controller,
                interactive: false, // Non-interactive for engine matches
              ),
            ),
          ),
          // Controls panel - min width so it stays usable
          Flexible(
            child: ConstrainedBox(
              constraints: const BoxConstraints(minWidth: 280),
              child: SingleChildScrollView(
                child: Padding(
                  padding: EdgeInsets.fromLTRB(
                    0,
                    MediaQuery.of(context).size.width * 0.024,
                    MediaQuery.of(context).size.width * 0.024,
                    MediaQuery.of(context).size.width * 0.024,
                  ),
                  child: Column(
                    children: [
                  // Mode selector
                  _buildModeSelector(),
                  const SizedBox(height: 20),
                  // Match controls
                  _buildMatchControls(),
                  const SizedBox(height: 20),
                  // Match status
                  _buildMatchStatus(),
                  const SizedBox(height: 20),
                  // Board controls (timers, etc.)
                  BoardControls(
                    whiteTimer: widget.controller.whiteTimer,
                    blackTimer: widget.controller.blackTimer,
                    board: widget.controller.board,
                    blackPlayer: "Engine 2",
                    whitePlayer: "Engine 1",
                    gameMode: GameMode.engineVsEngine,
                    onNewGame: () {},
                    setWhiteEngine: (value) {
                      setState(() {
                        _whiteEngine = value;
                      });
                    },
                    setBlackEngine: (value) {
                      setState(() {
                        _blackEngine = value;
                      });
                    },
                  ),
                    ],
                  ),
                ),
              ),
            ),
          ),
        ],
      ),
    );
  }

  /// Build mode selector widget
  Widget _buildModeSelector() {
    final textTheme = Theme.of(context).textTheme;
    return Card(
      child: Padding(
        padding: const EdgeInsets.all(16.0),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Text(
              'Match Mode',
              style: textTheme.titleLarge?.copyWith(
                fontWeight: FontWeight.bold,
              ),
            ),
            const SizedBox(height: 8),
            SegmentedButton<MatchMode>(
              segments: const [
                ButtonSegment(
                  value: MatchMode.visualization,
                  label: Text('Real-time'),
                ),
                ButtonSegment(
                  value: MatchMode.testing,
                  label: Text('Testing'),
                ),
              ],
              selected: {_currentMode},
              onSelectionChanged: (Set<MatchMode> newSelection) {
                if (!_isMatchActive && newSelection.isNotEmpty) {
                  setState(() {
                    _currentMode = newSelection.first;
                  });
                }
              },
            ),
          ],
        ),
      ),
    );
  }

  /// Build match controls
  Widget _buildMatchControls() {
    return Card(
      child: Padding(
        padding: const EdgeInsets.all(16.0),
        child: Column(
          children: [
            if (!_isMatchActive)
              ElevatedButton.icon(
                onPressed: _startMatch,
                icon: const Icon(Icons.play_arrow),
                label: const Text('Start Match'),
              )
            else
              Row(
                mainAxisAlignment: MainAxisAlignment.center,
                children: [
                  ElevatedButton.icon(
                    onPressed: _stopMatch,
                    icon: const Icon(Icons.stop),
                    label: const Text('Stop Match'),
                  ),
                  if (_currentMode == MatchMode.testing) ...[
                    const SizedBox(width: 8),
                    ElevatedButton.icon(
                      onPressed: widget.matchManager.isActive 
                          ? _pauseReplay 
                          : _resumeReplay,
                      icon: Icon(
                        widget.matchManager.isActive ? Icons.pause : Icons.play_arrow,
                      ),
                      label: Text(
                        widget.matchManager.isActive ? 'Pause' : 'Resume',
                      ),
                    ),
                  ],
                ],
              ),
          ],
        ),
      ),
    );
  }

  /// Build match status display
  Widget _buildMatchStatus() {
    final textTheme = Theme.of(context).textTheme;
    return Card(
      child: Padding(
        padding: const EdgeInsets.all(16.0),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Text(
              'Match Status',
              style: textTheme.titleLarge?.copyWith(
                fontWeight: FontWeight.bold,
              ),
            ),
            const SizedBox(height: 8),
            Text(
              'Status: ${_isMatchActive ? "Active" : "Idle"}',
              style: textTheme.bodyMedium,
            ),
            if (_currentGameNumber > 0)
              Text('Game: $_currentGameNumber', style: textTheme.bodyMedium),
            if (_lastResult != null)
              Text(
                'Last Result: $_lastResult',
                style: textTheme.bodyMedium,
              ),
          ],
        ),
      ),
    );
  }

  /// Start a match
  void _startMatch() async {
    try {
      await widget.matchManager.startMatch(
        mode: _currentMode,
        engine1: "MyEngine_v1",
        engine2: "Stockfish",
        stockfishSkill: 1,
        rounds: 100,
      );
    } catch (e) {
      if (mounted) {
        ScaffoldMessenger.of(context).showSnackBar(
          SnackBar(content: Text('Failed to start match: $e')),
        );
      }
    }
  }

  /// Stop the current match
  void _stopMatch() async {
    await widget.matchManager.stopMatch();
  }

  /// Pause replay (testing mode)
  void _pauseReplay() {
    widget.matchManager.pauseReplay();
  }

  /// Resume replay (testing mode)
  void _resumeReplay() {
    widget.matchManager.resumeReplay();
  }
}
