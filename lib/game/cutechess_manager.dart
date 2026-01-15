import 'dart:async';
import 'dart:convert';
import 'dart:io';

import 'package:chess_ui/game/pgn_watcher.dart';

class CutechessManager {
  Process? _process;
  IOSink? _logFile;
  final StreamController<GameEvent> _eventController = StreamController.broadcast();
  
  Stream<GameEvent> get events => _eventController.stream;
  bool get isActive => _process != null;
  Process? get process => _process;
  
  Future<void> startMatch({
    required String engineName,
    required int stockfishSkill,
    String? projectRoot,
  }) async {
    final root = projectRoot ?? '/Users/justin/VSCODE PROJECTS/chess_ui';
    final logPath = '$root/logs/${engineName}_vs_sf$stockfishSkill.log';
    
    try {
      _process = await startEngineMatch(
        engineName: engineName,
        skillLevel: stockfishSkill,
        projectRoot: projectRoot,
      );
      
      // Open log file
      _logFile = File(logPath).openWrite();
      
      // Listen to stdout - parse AND log
      _process!.stdout
          .transform(utf8.decoder)
          .transform(const LineSplitter())
          .listen((line) {
            _parseOutput(line);
            _logFile?.writeln(line); // Write to log
          });
      
      // Listen to stderr - log only
      _process!.stderr
          .transform(utf8.decoder)
          .transform(const LineSplitter())
          .listen((line) {
            print('stderr: $line');
            _logFile?.writeln('STDERR: $line'); // Write to log
            if (line.toLowerCase().contains('error')) {
              _eventController.add(BuildErrorEvent(line));
            }
          });
      
      // Listen for process exit
      _process!.exitCode.then((code) {
        _eventController.add(MatchCompleteEvent(code));
        _logFile?.close();
        _logFile = null;
        _process = null;
      });
      
    } catch (e) {
      _eventController.add(BuildErrorEvent('Failed to start match: $e'));
      _logFile?.close();
      _logFile = null;
      _process = null;
      rethrow;
    }
  }
  
  void _parseOutput(String line) {
    print('cutechess: $line');
    
    if (line.contains('Started game')) {
      final match = RegExp(r'Started game (\d+)').firstMatch(line);
      if (match != null) {
        _eventController.add(GameStartedEvent(int.parse(match.group(1)!)));
      }
    } else if (line.contains('Finished game')) {
      final match = RegExp(r'Finished game (\d+).*?: (.+)').firstMatch(line);
      if (match != null) {
        _eventController.add(GameFinishedEvent(
          int.parse(match.group(1)!),
          match.group(2)!.trim(),
        ));
      }
    } else if (line.contains('Score of')) {
      _eventController.add(ScoreUpdateEvent(line));
    }
  }
  
  Future<void> stop() async {
    if (_process != null) {
      _process!.kill(ProcessSignal.sigterm);
      
      try {
        await _process!.exitCode.timeout(
          const Duration(seconds: 5),
          onTimeout: () {
            _process!.kill(ProcessSignal.sigkill);
            return -1;
          },
        );
      } catch (e) {
        print('Error stopping process: $e');
      }
      
      await _logFile?.close();
      _logFile = null;
      _process = null;
    }
    
    await _eventController.close();
  }

  Future<Process> startEngineMatch({
    required String engineName,
    required int skillLevel,
    String? projectRoot,
  }) async {
    // Use provided project root or default
    final root = projectRoot ?? '/Users/justin/VSCODE PROJECTS/chess_ui';
    
    // Step 1: Build the engine
    print('Building engine...');
    final buildDir = '$root/native/build';
    
    final buildResult = await Process.run(
      'cmake',
      ['--build', '.', '--target', 'chess_engine_uci'],
      workingDirectory: buildDir,
    );
    
    if (buildResult.exitCode != 0) {
      throw Exception('Build failed:\n${buildResult.stderr}');
    }
    print('Engine built successfully');
    
    // Step 2: Copy engine to test_versions
    final engineSource = '$root/native/build/chess_engine_uci';
    final testVersionsDir = '$root/native/test_versions';
    final engineDest = '$testVersionsDir/$engineName';
    
    // Ensure test_versions directory exists
    await Directory(testVersionsDir).create(recursive: true);
    
    // Copy the file
    await File(engineSource).copy(engineDest);
    print('Engine copied to $engineDest');
    
    // Step 3: Ensure logs directory exists
    final logsDir = '$root/logs';
    await Directory(logsDir).create(recursive: true);
    
    // Step 4: Start cutechess-cli
    final pgnPath = '$logsDir/${engineName}_vs_sf$skillLevel.pgn';
    final logPath = '$logsDir/${engineName}_vs_sf$skillLevel.log';
    
    print('Starting match...');
    
    final process = await Process.start(
      'cutechess-cli',
      [
        '-engine',
        'name=$engineName',
        'cmd=$engineSource',
        'proto=uci',
        '-engine',
        'name=StockfishL$skillLevel',
        'cmd=/opt/homebrew/bin/stockfish',
        'proto=uci',
        'option.Skill Level=$skillLevel',
        '-each',
        'tc=40/60+0.6',
        '-rounds',
        '100',
        '-repeat',
        '-recover',
        '-pgnout',
        pgnPath,
      ],
      workingDirectory: root,
      runInShell: true,
    );
    
    return process;
  }
}

// Event classes
abstract class GameEvent {}

class GameStartedEvent extends GameEvent {
  final int gameNumber;
  GameStartedEvent(this.gameNumber);
}

class GameFinishedEvent extends GameEvent {
  final int gameNumber;
  final String result;
  GameFinishedEvent(this.gameNumber, this.result);
}

class ScoreUpdateEvent extends GameEvent {
  final String scoreLine;
  ScoreUpdateEvent(this.scoreLine);
}

class BuildErrorEvent extends GameEvent {
  final String error;
  BuildErrorEvent(this.error);
}

class MatchCompleteEvent extends GameEvent {
  final int exitCode;
  MatchCompleteEvent(this.exitCode);
}

void main() async{
  String engine1Name = "MyEngine_v1";
  String engine2Name = "MyEngine_v1";
  int skillLevel = 1;

  final pgnPath = '/Users/justin/VSCODE PROJECTS/chess_ui/native/logs/${engine1Name}_vs_sf$skillLevel.pgn';

  CutechessManager manager = CutechessManager();
  PGNWatcher watcher = PGNWatcher(pgnPath);

  manager.events.listen((event) {
    if (event is GameStartedEvent) {
      print('Game ${event.gameNumber} started');
    } else if (event is GameFinishedEvent) {
      print('Game ${event.gameNumber}: ${event.result}');
    } else if (event is MatchCompleteEvent) {
      print('Match complete with exit code ${event.exitCode}');
    }
  });

  // Listen to moves from watcher
  watcher.moves.listen((gameMoves) {
    print('\n📝 Game ${gameMoves.gameNumber} completed:');
    print('   Moves: ${gameMoves.moves.join(" ")}');
    print('   Result: ${gameMoves.result}');
    print('   Total moves: ${gameMoves.moves.length}\n');
  });

  await manager.startMatch(
    engineName: engine1Name,
    stockfishSkill: skillLevel,
  );

  watcher.start();

  await manager.process?.exitCode;

  watcher.stop();
  await manager.stop();
}