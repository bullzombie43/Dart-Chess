// This is a basic Flutter widget test.
//
// To perform an interaction with a widget in your test, use the WidgetTester
// utility in the flutter_test package. For example, you can send tap and scroll
// gestures. You can also use WidgetTester to find child widgets in the widget
// tree, read text, and verify that the values of widget properties are correct.

import 'package:flutter/material.dart';
import 'package:flutter_test/flutter_test.dart';

import 'package:chess_ui/game/chess_engine.dart';
import 'package:chess_ui/game/game_controller.dart';
import 'package:chess_ui/game/match_manager.dart';
import 'package:chess_ui/main.dart';

void main() {
  testWidgets('Counter increments smoke test', (WidgetTester tester) async {
    // Build our app and trigger a frame with required dependencies.
    final board = ChessBoard();
    final engine = ChessEngine();
    final controller = GameController(board: board, engine: engine);
    final matchManager = MatchManager(controller);

    await tester.pumpWidget(
      MyApp(
        controller: controller,
        engine: engine,
        matchManager: matchManager,
      ),
    );

    // Verify that our counter starts at 0.
    expect(find.text('0'), findsOneWidget);
    expect(find.text('1'), findsNothing);

    // Tap the '+' icon and trigger a frame.
    await tester.tap(find.byIcon(Icons.add));
    await tester.pump();

    // Verify that our counter has incremented.
    expect(find.text('0'), findsNothing);
    expect(find.text('1'), findsOneWidget);
  });
}
