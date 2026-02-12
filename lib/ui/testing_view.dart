import 'package:chess_ui/game/game_controller.dart';
import 'package:chess_ui/game/game_mode.dart';
import 'package:chess_ui/game/match_manager.dart';
import 'package:chess_ui/ui/engine_dropdown_button.dart';
import 'package:chess_ui/ui/game_board.dart';
import 'package:flutter/material.dart';

/// View for engine vs engine testing with statistics
class TestingView extends StatefulWidget {
  final GameController controller;
  final MatchManager matchManager;
  
  const TestingView({
    super.key,
    required this.controller,
    required this.matchManager,
  });

  @override
  State<TestingView> createState() => _TestingViewState();
}

class _TestingViewState extends State<TestingView> {
  String? _whiteEngine;
  String? _blackEngine;
  bool _isTestActive = false;
  int _currentGameNumber = 0;
  int _whiteWins = 0;
  int _blackWins = 0;
  int _draws = 0;
  int _totalGames = 0;
  String? _lastResult;
  
  @override
  void initState() {
    super.initState();
    widget.controller.setGameMode(GameMode.engineVsEngine);
    
    // Listen to match events
    widget.matchManager.events.listen((event) {
      setState(() {
        switch (event.type) {
          case MatchEventType.matchStarted:
            _isTestActive = true;
            _resetStatistics();
            break;
          case MatchEventType.gameStarted:
            _currentGameNumber = event.gameNumber ?? 0;
            break;
          case MatchEventType.gameFinished:
            _lastResult = event.result;
            _updateStatistics(event.result);
            _totalGames++;
            break;
          case MatchEventType.matchFinished:
            _isTestActive = false;
            break;
          default:
            break;
        }
      });
    });
  }

  /// Reset statistics when starting a new test
  void _resetStatistics() {
    _whiteWins = 0;
    _blackWins = 0;
    _draws = 0;
    _totalGames = 0;
    _currentGameNumber = 0;
    _lastResult = null;
  }

  /// Update statistics based on game result
  void _updateStatistics(String? result) {
    if (result == null) return;
    
    // Parse result: "1-0" (white wins), "0-1" (black wins), "1/2-1/2" (draw)
    if (result == "1-0") {
      _whiteWins++;
    } else if (result == "0-1") {
      _blackWins++;
    } else if (result == "1/2-1/2") {
      _draws++;
    }
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      backgroundColor: const Color.fromARGB(255, 140, 208, 161),
      body: Row(
        children: [
          // Chess board (left side)
          Padding(
            padding: EdgeInsets.all(MediaQuery.of(context).size.width * 0.024),
            child: GameBoard(
              controller: widget.controller,
              interactive: false, // Non-interactive for testing
            ),
          ),
          // Testing control panel (right side)
          Expanded(
            child: Padding(
              padding: EdgeInsetsGeometry.fromLTRB(
                0,
                MediaQuery.of(context).size.width * 0.024,
                MediaQuery.of(context).size.width * 0.024,
                MediaQuery.of(context).size.width * 0.024,
              ),
              child: _buildTestingPanel(),
            ),
          ),
        ],
      ),
    );
  }

  /// Build the testing control panel
  Widget _buildTestingPanel() {
    return Column(
      children: [
        // Engine Selection Section
        Card(
          child: Padding(
            padding: const EdgeInsets.all(16.0),
            child: Column(
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                const Text(
                  'Engine Selection',
                  style: TextStyle(fontSize: 18, fontWeight: FontWeight.bold),
                ),
                const SizedBox(height: 16),
                Row(
                  children: [
                    const Expanded(
                      child: Text('White Engine:', style: TextStyle(fontSize: 14)),
                    ),
                    Expanded(
                      flex: 2,
                      child: IgnorePointer(
                        ignoring: _isTestActive,
                        child: Opacity(
                          opacity: _isTestActive ? 0.5 : 1.0,
                          child: SizedBox(
                            width: double.infinity,
                            child: EngineDropdownButton(
                              selectionCallback: (value) {
                                setState(() {
                                  _whiteEngine = value;
                                });
                              },
                            ),
                          ),
                        ),
                      ),
                    ),
                  ],
                ),
                const SizedBox(height: 12),
                Row(
                  children: [
                    const Expanded(
                      child: Text('Black Engine:', style: TextStyle(fontSize: 14)),
                    ),
                    Expanded(
                      flex: 2,
                      child: IgnorePointer(
                        ignoring: _isTestActive,
                        child: Opacity(
                          opacity: _isTestActive ? 0.5 : 1.0,
                          child: SizedBox(
                            width: double.infinity,
                            child: EngineDropdownButton(
                              selectionCallback: (value) {
                                setState(() {
                                  _blackEngine = value;
                                });
                              },
                            ),
                          ),
                        ),
                      ),
                    ),
                  ],
                ),
              ],
            ),
          ),
        ),
        const SizedBox(height: 20),
        // Controls Section
        Card(
          child: Padding(
            padding: const EdgeInsets.all(16.0),
            child: Column(
              children: [
                if (!_isTestActive)
                  ElevatedButton.icon(
                    onPressed: _startTest,
                    icon: const Icon(Icons.play_arrow),
                    label: const Text('Start Test'),
                    style: ElevatedButton.styleFrom(
                      minimumSize: const Size(double.infinity, 48),
                    ),
                  )
                else
                  Row(
                    children: [
                      Expanded(
                        child: ElevatedButton.icon(
                          onPressed: _stopTest,
                          icon: const Icon(Icons.stop),
                          label: const Text('Stop Test'),
                          style: ElevatedButton.styleFrom(
                            minimumSize: const Size(0, 48),
                          ),
                        ),
                      ),
                      const SizedBox(width: 8),
                      Expanded(
                        child: ElevatedButton.icon(
                          onPressed: widget.matchManager.isActive 
                              ? _pauseReplay 
                              : _resumeReplay,
                          icon: Icon(
                            widget.matchManager.isActive ? Icons.pause : Icons.play_arrow,
                          ),
                          label: Text(
                            widget.matchManager.isActive ? 'Pause' : 'Resume',
                          ),
                          style: ElevatedButton.styleFrom(
                            minimumSize: const Size(0, 48),
                          ),
                        ),
                      ),
                    ],
                  ),
              ],
            ),
          ),
        ),
        const SizedBox(height: 20),
        // Statistics Section
        Expanded(
          child: Card(
            child: Padding(
              padding: const EdgeInsets.all(16.0),
              child: Column(
                crossAxisAlignment: CrossAxisAlignment.start,
                children: [
                  const Text(
                    'Statistics',
                    style: TextStyle(fontSize: 18, fontWeight: FontWeight.bold),
                  ),
                  const SizedBox(height: 16),
                  _buildStatRow('Status', _isTestActive ? 'Active' : 'Idle'),
                  if (_currentGameNumber > 0)
                    _buildStatRow('Current Game', '$_currentGameNumber'),
                  const Divider(),
                  _buildStatRow('Total Games', '$_totalGames'),
                  _buildStatRow('White Wins', '$_whiteWins'),
                  _buildStatRow('Black Wins', '$_blackWins'),
                  _buildStatRow('Draws', '$_draws'),
                  if (_lastResult != null) ...[
                    const Divider(),
                    _buildStatRow('Last Result', _lastResult!),
                  ],
                ],
              ),
            ),
          ),
        ),
      ],
    );
  }

  /// Build a statistics row
  Widget _buildStatRow(String label, String value) {
    return Padding(
      padding: const EdgeInsets.symmetric(vertical: 8.0),
      child: Row(
        mainAxisAlignment: MainAxisAlignment.spaceBetween,
        children: [
          Text(
            label,
            style: const TextStyle(fontSize: 14, fontWeight: FontWeight.w500),
          ),
          Text(
            value,
            style: const TextStyle(fontSize: 14),
          ),
        ],
      ),
    );
  }

  /// Start a test
  void _startTest() async {
    if (_whiteEngine == null || _blackEngine == null) {
      ScaffoldMessenger.of(context).showSnackBar(
        const SnackBar(content: Text('Please select both engines')),
      );
      return;
    }

    try {
      await widget.matchManager.startMatch(
        mode: MatchMode.testing,
        engine1: _whiteEngine!,
        engine2: _blackEngine!,
        stockfishSkill: 1, // Default, could be made configurable
        rounds: 100, // Default, could be made configurable
      );
    } catch (e) {
      if (mounted) {
        ScaffoldMessenger.of(context).showSnackBar(
          SnackBar(content: Text('Failed to start test: $e')),
        );
      }
    }
  }

  /// Stop the current test
  void _stopTest() async {
    await widget.matchManager.stopMatch();
  }

  /// Pause replay
  void _pauseReplay() {
    widget.matchManager.pauseReplay();
  }

  /// Resume replay
  void _resumeReplay() {
    widget.matchManager.resumeReplay();
  }
}
