import 'dart:async';
import 'dart:io';

import 'package:chess_ui/game/chess_engine.dart';
import 'package:chess_ui/game/game_controller.dart';
import 'package:chess_ui/game/pgn_replayer.dart';
import 'package:flutter_test/flutter_test.dart';

void main() {
  group('PGN parser sanity suite', () {
    test('replays all games in parser_sanity_tests.pgn without errors', () async {
      // Load the curated PGN test file from the logs directory.
      final file = File('logs/parser_sanity_tests.pgn');
      expect(file.existsSync(), isTrue,
          reason: 'Expected logs/parser_sanity_tests.pgn to exist for sanity tests.');

      final pgnText = file.readAsStringSync();

      // Split into individual game PGNs using the same approach as PGNWatcher /
      // LogReplayView: each game starts with an [Event ...] header.
      final rawGames = <String>[];
      final buffer = StringBuffer();
      final lines = pgnText.split('\n');

      for (final line in lines) {
        if (line.startsWith('[Event')) {
          if (buffer.isNotEmpty) {
            rawGames.add(buffer.toString());
            buffer.clear();
          }
        }
        buffer.writeln(line);
      }
      if (buffer.isNotEmpty) {
        rawGames.add(buffer.toString());
      }

      // Sanity: we expect at least one game in the test file.
      expect(rawGames, isNotEmpty,
          reason: 'parser_sanity_tests.pgn should contain at least one game.');

      for (var i = 0; i < rawGames.length; i++) {
        final gamePgn = rawGames[i];

        // Fresh board/engine/controller per game to avoid cross-game state.
        final board = ChessBoard();
        final engine = ChessEngine();
        final controller = GameController(board: board, engine: engine);
        final replayer = PgnReplayer(controller);

        final errors = <ReplayEvent>[];
        final movesApplied = <ReplayEvent>[];

        final sub = replayer.events.listen((event) {
          switch (event.type) {
            case ReplayEventType.moveApplied:
              movesApplied.add(event);
              break;
            case ReplayEventType.replayStopped:
              if (event.error != null && event.error!.isNotEmpty) {
                errors.add(event);
              }
              break;
            default:
              break;
          }
        });

        // Use a timeout to avoid hanging tests if something goes wrong.
        try {
          await replayer
              .replayGameFromPgn(gamePgn)
              .timeout(const Duration(seconds: 20));
        } on TimeoutException {
          fail('Replay timed out for sanity test game index $i.');
        } finally {
          await sub.cancel();
          replayer.dispose();
          board.dispose();
          engine.dispose();
        }

        // Assert that no replayStopped events with errors were emitted.
        if (errors.isNotEmpty) {
          final firstError = errors.first;
          final idx = firstError.moveNumber ?? -1;
          final rawMove = firstError.rawMove ?? '<unknown>';
          final msg = firstError.error ?? '<no message>';
          fail(
            'Sanity test game index $i failed at move $idx ($rawMove): $msg',
          );
        }

        // We also expect at least one move to have been applied in each game.
        expect(
          movesApplied,
          isNotEmpty,
          reason:
              'Sanity test game index $i had no moves applied; PGN may be empty or malformed.',
        );
      }
    });
  });
}

