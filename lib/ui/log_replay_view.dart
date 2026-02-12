import 'dart:io';

import 'package:chess_ui/game/game_controller.dart';
import 'package:chess_ui/game/pgn_replayer.dart';
import 'package:chess_ui/ui/game_board.dart';
import 'package:file_picker/file_picker.dart';
import 'package:flutter/material.dart';

/// Standalone view for replaying PGN logs on the board.
///
/// - Left: non-interactive board driven by [PgnReplayer].
/// - Right: PGN file selection (logs directory + manual picker) and controls.
class LogReplayView extends StatefulWidget {
  final GameController controller;

  const LogReplayView({
    super.key,
    required this.controller,
  });

  @override
  State<LogReplayView> createState() => _LogReplayViewState();
}

class _LogReplayViewState extends State<LogReplayView> {
  late final PgnReplayer _replayer;

  /// All discovered PGN files under the project logs directory.
  List<FileSystemEntity> _logFiles = [];

  /// Absolute path of the currently selected PGN file.
  String? _selectedPgnPath;

  /// Raw PGN text of the selected file.
  String? _selectedPgnText;

  /// Individual games parsed from the current PGN file (if multi-game).
  List<String> _games = [];

  /// Index (0-based) of the selected game within the current file.
  int _selectedGameIndex = 0;

  /// Whether a replay is currently in progress.
  bool _isReplaying = false;

  /// Parsed move list from the current PGN (raw SAN/strings).
  List<String> _parsedMoves = [];

  /// Index (0-based) of the move currently being applied.
  int _currentMoveIndex = -1;

  /// Details of the first parse error, if any.
  ReplayErrorInfo? _firstError;

  /// Current move delay in milliseconds.
  double _moveDelayMs = 500;

  @override
  void initState() {
    super.initState();
    _replayer = PgnReplayer(widget.controller);
    _replayer.setMoveDelay(Duration(milliseconds: _moveDelayMs.toInt()));
    _replayer.events.listen(_handleReplayEvent);
    _scanLogsDirectory();
  }

  void _handleReplayEvent(ReplayEvent event) {
    if (!mounted) return;
    setState(() {
      switch (event.type) {
        case ReplayEventType.replayStarted:
          _isReplaying = true;
          _currentMoveIndex = -1;
          _firstError = null;
          break;
        case ReplayEventType.moveApplied:
          if (event.moveNumber != null) {
            _currentMoveIndex = event.moveNumber! - 1;
          }
          break;
        case ReplayEventType.replayFinished:
        case ReplayEventType.replayStopped:
          _isReplaying = false;
          if (event.error != null && _firstError == null) {
            _firstError = ReplayErrorInfo(
              moveIndex: (event.moveNumber ?? 0) - 1,
              rawMove: event.rawMove ?? '',
              message: event.error!,
              fenBefore: event.fenBefore,
              fenAfter: event.fenAfter,
            );
          }
          break;
        case ReplayEventType.replayPaused:
        case ReplayEventType.replayResumed:
          // No-op for now; could reflect paused state separately if needed.
          break;
      }
    });
  }

  @override
  void dispose() {
    _replayer.dispose();
    super.dispose();
  }

  /// Scan the default project logs directory for PGN files.
  Future<void> _scanLogsDirectory() async {
    // For now, assume logs/ lives at project root next to lib/.
    // In a future refinement we can pass projectRoot through MatchManager.
    final projectRoot = Directory.current.path;
    final logsDir = Directory('$projectRoot/logs');

    if (!await logsDir.exists()) {
      setState(() {
        _logFiles = [];
      });
      return;
    }

    final files = await logsDir
        .list(recursive: false, followLinks: false)
        .where((e) =>
            e is File &&
            e.path.toLowerCase().endsWith('.pgn'))
        .toList();

    setState(() {
      _logFiles = files;
    });
  }

  Future<void> _pickManualPgn() async {
    final result = await FilePicker.platform.pickFiles(
      type: FileType.custom,
      allowedExtensions: ['pgn'],
    );

    if (result == null || result.files.isEmpty) return;

    final path = result.files.single.path;
    if (path == null) return;

    await _loadPgnFromPath(path);
  }

  Future<void> _loadPgnFromPath(String path) async {
    try {
      final text = await File(path).readAsString();
      final games = _parseAllGames(text);
      final gameIndex = games.isNotEmpty ? 0 : 0;
      final selectedGameText =
          games.isNotEmpty ? games[gameIndex] : text;
      final moves = _extractMovesFromPgn(selectedGameText);
      setState(() {
        _selectedPgnPath = path;
        _selectedPgnText = text;
        _games = games;
        _selectedGameIndex = gameIndex;
        _parsedMoves = moves;
        _currentMoveIndex = -1;
        _firstError = null;
      });
    } catch (e) {
      if (!mounted) return;
      ScaffoldMessenger.of(context).showSnackBar(
        SnackBar(content: Text('Failed to read PGN: $e')),
      );
    }
  }

  Future<void> _startReplay() async {
    final gameText = _currentGamePgn;
    if (gameText == null || gameText.trim().isEmpty) {
      ScaffoldMessenger.of(context).showSnackBar(
        const SnackBar(content: Text('Please select a PGN file to replay.')),
      );
      return;
    }

    setState(() {
      _isReplaying = true;
      _currentMoveIndex = -1;
      _firstError = null;
    });

    try {
      await _replayer.replayGameFromPgn(gameText);
    } catch (e) {
      if (mounted) {
        ScaffoldMessenger.of(context).showSnackBar(
          SnackBar(content: Text('Replay failed: $e')),
        );
      }
    } finally {
      if (mounted) {
        setState(() {
          _isReplaying = false;
        });
      }
    }
  }

  Future<void> _restartReplay() async {
    // Simply restart from the beginning with current PGN.
    await _startReplay();
  }

  void _updateSpeed(double value) {
    setState(() {
      _moveDelayMs = value;
    });
    _replayer.setMoveDelay(Duration(milliseconds: value.toInt()));
  }

  /// Extract flat move list from a PGN text (mirrors PgnReplayer logic).
  List<String> _extractMovesFromPgn(String pgnText) {
    final lines = pgnText.split('\n');
    final movetext = lines
        .where((line) => !line.startsWith('[') && line.trim().isNotEmpty)
        .join(' ');

    final cleaned = movetext
        .replaceAll(RegExp(r'\d+\.+'), ' ')
        .replaceAll(RegExp(r'\{[^}]*\}'), ' ')
        .replaceAll(RegExp(r'1-0|0-1|1/2-1/2|\*'), ' ')
        .trim();

    return cleaned
        .split(RegExp(r'\s+'))
        .where((m) => m.isNotEmpty)
        .toList();
  }

  /// Split a PGN file into individual game blocks (same as PGNWatcher).
  List<String> _parseAllGames(String content) {
    final games = <String>[];
    final lines = content.split('\n');
    StringBuffer currentGame = StringBuffer();

    for (final line in lines) {
      if (line.startsWith('[Event ') && currentGame.isNotEmpty) {
        games.add(currentGame.toString());
        currentGame = StringBuffer();
      }
      currentGame.writeln(line);
    }

    if (currentGame.isNotEmpty) {
      games.add(currentGame.toString());
    }

    return games.where((g) => g.trim().isNotEmpty).toList();
  }

  /// Get the PGN text of the currently selected game in the file.
  String? get _currentGamePgn {
    if (_selectedPgnText == null) return null;
    if (_games.isEmpty) return _selectedPgnText;
    if (_selectedGameIndex < 0 || _selectedGameIndex >= _games.length) {
      return _games.first;
    }
    return _games[_selectedGameIndex];
  }

  /// Human-friendly description of a game, derived from PGN headers.
  String _describeGame(String gamePgn, int index) {
    String white = 'White';
    String black = 'Black';
    String result = '';

    final whiteMatch = RegExp(r'\[White "([^"]+)"\]').firstMatch(gamePgn);
    if (whiteMatch != null) {
      white = whiteMatch.group(1)!;
    }

    final blackMatch = RegExp(r'\[Black "([^"]+)"\]').firstMatch(gamePgn);
    if (blackMatch != null) {
      black = blackMatch.group(1)!;
    }

    final resultMatch = RegExp(r'\[Result "([^"]+)"\]').firstMatch(gamePgn);
    if (resultMatch != null) {
      result = resultMatch.group(1)!;
    }

    final base = 'Game ${index + 1}: $white vs $black';
    return result.isNotEmpty ? '$base ($result)' : base;
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      backgroundColor: const Color.fromARGB(255, 140, 208, 161),
      body: Row(
        children: [
          // Chess board (left side)
          Padding(
            padding:
                EdgeInsets.all(MediaQuery.of(context).size.width * 0.024),
            child: GameBoard(
              controller: widget.controller,
              interactive: false,
            ),
          ),
          // Control panel (right side)
          Expanded(
            child: Padding(
              padding: EdgeInsets.fromLTRB(
                0,
                MediaQuery.of(context).size.width * 0.024,
                MediaQuery.of(context).size.width * 0.024,
                MediaQuery.of(context).size.width * 0.024,
              ),
              child: Column(
                children: [
                  _buildFileSelectionCard(),
                  const SizedBox(height: 16),
                  _buildControlsCard(),
                  const SizedBox(height: 16),
                  Expanded(child: _buildInfoCard()),
                ],
              ),
            ),
          ),
        ],
      ),
    );
  }

  Widget _buildFileSelectionCard() {
    return Card(
      child: Padding(
        padding: const EdgeInsets.all(16.0),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            const Text(
              'PGN Log Selection',
              style: TextStyle(fontSize: 18, fontWeight: FontWeight.bold),
            ),
            const SizedBox(height: 12),
            Row(
              children: [
                Expanded(
                  child: DropdownButtonFormField<String>(
                    value: _selectedPgnPath != null &&
                            _logFiles.any((f) => f.path == _selectedPgnPath)
                        ? _selectedPgnPath
                        : null,
                    decoration: const InputDecoration(
                      labelText: 'Logs directory PGNs',
                      border: OutlineInputBorder(),
                    ),
                    items: _logFiles
                        .map(
                          (f) => DropdownMenuItem<String>(
                            value: f.path,
                            child: Text(
                              f.uri.pathSegments.isNotEmpty
                                  ? f.uri.pathSegments.last
                                  : f.path,
                              overflow: TextOverflow.ellipsis,
                            ),
                          ),
                        )
                        .toList(),
                    onChanged: (value) {
                      if (value != null) {
                        _loadPgnFromPath(value);
                      }
                    },
                  ),
                ),
                const SizedBox(width: 8),
                ElevatedButton.icon(
                  onPressed: _pickManualPgn,
                  icon: const Icon(Icons.folder_open),
                  label: const Text('Browse…'),
                ),
              ],
            ),
            const SizedBox(height: 8),
            Text(
              _selectedPgnPath != null
                  ? 'Selected: ${_selectedPgnPath}'
                  : 'No PGN selected',
              style: const TextStyle(fontSize: 12),
            ),
            if (_games.isNotEmpty) ...[
              const SizedBox(height: 12),
              const Text(
                'Game in file',
                style: TextStyle(fontSize: 14, fontWeight: FontWeight.w600),
              ),
              const SizedBox(height: 4),
              DropdownButton<int>(
                value: _selectedGameIndex.clamp(0,
                    _games.isEmpty ? 0 : _games.length - 1),
                items: List.generate(_games.length, (index) {
                  final label = _describeGame(_games[index], index);
                  return DropdownMenuItem<int>(
                    value: index,
                    child: Text(
                      label,
                      overflow: TextOverflow.ellipsis,
                    ),
                  );
                }),
                onChanged: (value) {
                  if (value == null) return;
                  final gameText = _games[value];
                  final moves = _extractMovesFromPgn(gameText);
                  setState(() {
                    _selectedGameIndex = value;
                    _parsedMoves = moves;
                    _currentMoveIndex = -1;
                    _firstError = null;
                  });
                },
              ),
            ],
          ],
        ),
      ),
    );
  }

  Widget _buildControlsCard() {
    return Card(
      child: Padding(
        padding: const EdgeInsets.all(16.0),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            const Text(
              'Replay Controls',
              style: TextStyle(fontSize: 18, fontWeight: FontWeight.bold),
            ),
            const SizedBox(height: 12),
            Row(
              children: [
                Expanded(
                  child: ElevatedButton.icon(
                    onPressed: _isReplaying ? null : _startReplay,
                    icon: const Icon(Icons.play_arrow),
                    label: const Text('Play'),
                  ),
                ),
                const SizedBox(width: 8),
                Expanded(
                  child: ElevatedButton.icon(
                    onPressed: _selectedPgnText == null ? null : _restartReplay,
                    icon: const Icon(Icons.refresh),
                    label: const Text('Restart'),
                  ),
                ),
              ],
            ),
            const SizedBox(height: 16),
            const Text('Speed (delay between moves)'),
            Slider(
              min: 50,
              max: 2000,
              divisions: 39,
              value: _moveDelayMs,
              label: '${_moveDelayMs.toInt()} ms',
              onChanged: _updateSpeed,
            ),
          ],
        ),
      ),
    );
  }

  Widget _buildInfoCard() {
    return Card(
      child: Padding(
        padding: const EdgeInsets.all(16.0),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            const Text(
              'Replay Info',
              style: TextStyle(fontSize: 18, fontWeight: FontWeight.bold),
            ),
            const SizedBox(height: 8),
            Text(
              _isReplaying
                  ? 'Replaying game from PGN…'
                  : 'Idle. Select a PGN and press Play to start replay.',
            ),
            const SizedBox(height: 12),
            if (_parsedMoves.isNotEmpty)
              Expanded(
                child: Column(
                  crossAxisAlignment: CrossAxisAlignment.start,
                  children: [
                    const Text(
                      'Moves',
                      style: TextStyle(
                        fontSize: 16,
                        fontWeight: FontWeight.bold,
                      ),
                    ),
                    const SizedBox(height: 8),
                    Expanded(
                      child: ListView.builder(
                        itemCount: _parsedMoves.length,
                        itemBuilder: (context, index) {
                          final move = _parsedMoves[index];
                          final isCurrent = index == _currentMoveIndex;
                          final isError =
                              _firstError != null && index == _firstError!.moveIndex;
                          Color? color;
                          if (isError) {
                            color = Colors.red.shade100;
                          } else if (isCurrent) {
                            color = Colors.blue.shade100;
                          }
                          return Container(
                            color: color,
                            padding: const EdgeInsets.symmetric(
                              vertical: 4,
                              horizontal: 8,
                            ),
                            child: Row(
                              children: [
                                Text('${index + 1}. ',
                                    style:
                                        const TextStyle(fontWeight: FontWeight.bold)),
                                Expanded(
                                  child: Text(move),
                                ),
                              ],
                            ),
                          );
                        },
                      ),
                    ),
                    if (_firstError != null) ...[
                      const SizedBox(height: 8),
                      const Divider(),
                      const Text(
                        'First parse error',
                        style: TextStyle(
                          fontSize: 16,
                          fontWeight: FontWeight.bold,
                        ),
                      ),
                      const SizedBox(height: 4),
                      Text(
                        'Move ${_firstError!.moveIndex + 1}: ${_firstError!.rawMove}',
                        style: const TextStyle(color: Colors.red),
                      ),
                      Text(
                        _firstError!.message,
                        style: const TextStyle(fontSize: 12),
                      ),
                    ],
                  ],
                ),
              ),
          ],
        ),
      ),
    );
  }
}

class ReplayErrorInfo {
  final int moveIndex;
  final String rawMove;
  final String message;
  final String? fenBefore;
  final String? fenAfter;

  ReplayErrorInfo({
    required this.moveIndex,
    required this.rawMove,
    required this.message,
    this.fenBefore,
    this.fenAfter,
  });
}


