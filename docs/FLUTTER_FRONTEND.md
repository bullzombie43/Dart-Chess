# Flutter Frontend Documentation

## Overview

The Flutter frontend provides a cross-platform user interface for the chess application. It handles user interactions, displays the chess board, manages game state, and communicates with the C++ engine via FFI (Foreign Function Interface).

## Directory Structure

```
lib/
├── main.dart                    # Application entry point
├── game/                       # Game logic and state
│   ├── chess_engine.dart      # High-level engine wrapper
│   ├── chess_ffi.dart          # Low-level FFI bindings
│   ├── game_controller.dart    # Game state controller
│   ├── game_state.dart         # Alternative game state (legacy?)
│   ├── cutechess_manager.dart  # Engine vs. engine matches
│   └── pgn_watcher.dart        # PGN file monitoring
└── ui/                         # User interface components
    ├── board_background.dart   # Chess board rendering
    ├── board_pieces.dart       # Piece rendering and interaction
    ├── board_controls.dart     # Game controls UI
    ├── board_builder.dart      # Board layout builder
    └── engine_dropdown_button.dart # Engine selection
```

## Architecture

### Layer Structure

```
┌─────────────────────────────────────────┐
│         UI Layer (Widgets)              │
│  - Board rendering                      │
│  - User interaction handling            │
│  - Game controls                        │
└─────────────────────────────────────────┘
              │
              ▼
┌─────────────────────────────────────────┐
│      Game Logic Layer (Dart)            │
│  - Move validation                      │
│  - Game state management                │
│  - Timer management                     │
└─────────────────────────────────────────┘
              │
              ▼
┌─────────────────────────────────────────┐
│      FFI Layer (Dart FFI)               │
│  - C function bindings                  │
│  - Type conversions                     │
│  - Memory management                    │
└─────────────────────────────────────────┘
              │
              ▼
┌─────────────────────────────────────────┐
│      Native Layer (C++)                 │
│  - Chess engine                         │
│  - Move generation                      │
│  - Search algorithms                    │
└─────────────────────────────────────────┘
```

## Core Components

### 1. Application Entry (`main.dart`)

**Purpose**: Initializes the application, sets up window management, and creates the widget tree.

#### Key Responsibilities
- Window initialization and configuration
- Provider setup for state management
- Widget tree construction
- User interaction handling (taps, drags)

#### Window Configuration
```dart
WindowOptions windowOptions = const WindowOptions(
  size: Size(1000, 800),
  center: true,
  backgroundColor: Colors.transparent,
  skipTaskbar: false,
  titleBarStyle: TitleBarStyle.hidden,
);
```

#### State Management
Uses `ChangeNotifierProvider` for reactive UI updates:
```dart
ChangeNotifierProvider(
  create: (context) => controller,
  child: MyApp(...),
)
```

#### User Interaction
- **Tap Handling**: `handleSquareTap()` - Selects pieces and makes moves
- **Drag Handling**: `handlePieceDragEnd()` - Drag-and-drop piece movement
- **Move Validation**: Checks legal moves before execution
- **Promotion Dialog**: Shows promotion choices when pawn reaches 8th rank

### 2. Game Logic Layer

#### `chess_ffi.dart` - Low-Level FFI Bindings

**Purpose**: Provides direct bindings to C functions from the native library.

**Key Features**:
- Library loading (platform-specific)
- C type definitions (`CMove`, handles)
- Function signature definitions
- Function lookups from native library

**Library Loading**:
```dart
ffi.DynamicLibrary _loadLibrary() {
  if (Platform.isMacOS) {
    return ffi.DynamicLibrary.open('libchess_bridge.dylib');
  } else if (Platform.isLinux) {
    return ffi.DynamicLibrary.open('libchess_bridge.so');
  }
  // ... other platforms
}
```

**Type Definitions**:
```dart
final class CMove extends ffi.Struct {
  @ffi.Uint8()
  external int piece;
  @ffi.Uint8()
  external int fromSquare;
  // ... other fields
}
```

**Function Bindings**:
```dart
final chessBoardCreate = _nativeLib
    .lookup<ffi.NativeFunction<ChessBoardCreateNative>>('board_create')
    .asFunction<ChessBoardCreateDart>();
```

#### `chess_engine.dart` - High-Level Engine Wrapper

**Purpose**: Provides a Dart-friendly API for chess operations.

**Key Classes**:

**`ChessBoard`**:
```dart
class ChessBoard {
  late ffi_bindings.ChessBoardHandle _handle;
  
  ChessBoard() {
    _handle = ffi_bindings.chessBoardCreate();
  }
  
  PieceType getPieceAt(int square) { ... }
  void makeMove(Move move) { ... }
  void undoMove() { ... }
  String getFen() { ... }
  void dispose() { ... }
}
```

**`ChessEngine`**:
```dart
class ChessEngine {
  late ffi_bindings.ChessEngineHandle _handle;
  
  List<Move> generateLegalMoves(ChessBoard board) { ... }
  Move? getBestMove(ChessBoard board) { ... }
  Move? getRandomMove(ChessBoard board) { ... }
  bool isCheckmate(ChessBoard board) { ... }
  bool isStalemate(ChessBoard board) { ... }
}
```

**`Move`**:
```dart
class Move {
  final int piece;
  final int fromSquare;
  final int toSquare;
  final int capturedPiece;
  final int promotedPiece;
  final bool isEnPassant;
  final bool isCastling;
  
  factory Move.fromCMove(ffi_bindings.CMove cmove) { ... }
  ffi.Pointer<ffi_bindings.CMove> _toCMove() { ... }
  String toUCI() { ... }
}
```

**Memory Management**:
- Handles are created/destroyed explicitly
- `dispose()` methods clean up native resources
- FFI memory allocated with `calloc`, freed with `calloc.free()`

#### `game_controller.dart` - Game State Controller

**Purpose**: Manages game state, timers, and notifies UI of changes.

**Key Features**:
- Timer management (white/black timers)
- Move execution coordination
- State change notifications via `ChangeNotifier`

```dart
class GameController extends ChangeNotifier {
  final ChessBoard board;
  final ChessEngine engine;
  final StopWatchTimer whiteTimer;
  final StopWatchTimer blackTimer;
  
  void makeMove(Move move) {
    board.makeMove(move);
    // Update timers
    // Notify listeners
    notifyListeners();
  }
  
  void resetTimers() { ... }
}
```

**Timer Management**:
- Countdown timers for each player
- Time increment support
- Automatic start/stop on move

### 3. UI Components

#### `board_background.dart` - Chess Board Rendering

**Purpose**: Renders the chess board squares, highlights, and markers.

**Features**:
- Square colors (light/dark)
- Move markers (legal move indicators)
- Highlights (selected square, check, etc.)
- Responsive sizing

**Key Widgets**:
- `Boardbackground`: Main board widget
- `Marker`: Visual indicators for legal moves
- `HighlightType`: Types of square highlights

#### `board_pieces.dart` - Piece Rendering

**Purpose**: Renders chess pieces and handles piece interactions.

**Features**:
- SVG piece rendering
- Tap and drag interactions
- Piece selection visualization
- Drag preview

**Key Widgets**:
- `BoardPieces`: Main piece rendering widget
- Handles tap and drag callbacks
- Piece asset loading from `assets/pieces/`

#### `board_controls.dart` - Game Controls

**Purpose**: Provides UI controls for game management.

**Features**:
- Timer displays
- New game button
- Engine selection dropdowns
- Player names
- Engine match controls

#### `board_builder.dart` - Board Layout

**Purpose**: Builds the board grid layout.

**Features**:
- Responsive grid layout
- Square size calculation
- Board orientation support
- Custom square builders

## Data Models

### Piece Types

```dart
enum PieceType {
  wPawn(0, 'P', "wP.svg"),
  wKnight(1, 'N', "wN.svg"),
  // ... other pieces
  none(12, '.', "");
  
  final int value;
  final String symbol;
  final String asset;
}
```

### Colors

```dart
enum ChessColor {
  white(0),
  black(1),
  none(2);
  
  final int value;
}
```

### Moves

```dart
class Move {
  final int piece;
  final int fromSquare;
  final int toSquare;
  final int capturedPiece;
  final int promotedPiece;
  final bool isEnPassant;
  final bool isCastling;
}
```

## User Interaction Flow

### Making a Move (Tap)

1. User taps a square
2. `handleSquareTap()` called
3. If no piece selected:
   - Check if square has piece
   - Generate legal moves for that piece
   - Show move markers
   - Set `selectedIndex`
4. If piece already selected:
   - Check if tap is legal move
   - If promotion: show dialog
   - Execute move via `fullPlayerMove()`
   - Update UI
   - Engine makes move (if playing vs. engine)

### Making a Move (Drag)

1. User drags piece
2. `handlePieceDragEnd()` called
3. Find matching legal move
4. Execute move if found
5. Update UI

### Promotion Flow

1. Detect promotion attempt
2. Show `showPromotionDialog()`
3. User selects piece (Queen, Rook, Bishop, Knight)
4. Create move with `promotedPiece`
5. Execute move

## Game Modes

### Human vs. Engine

- Player makes moves via UI
- Engine responds automatically
- Timers track each side's time

### Engine vs. Engine

- `CutechessManager` orchestrates matches
- `PGNWatcher` monitors game files
- Results logged to PGN files

## State Management

### Provider Pattern

Uses `provider` package for state management:

```dart
ChangeNotifierProvider(
  create: (context) => controller,
  child: MyApp(...),
)
```

### State Updates

- `GameController` extends `ChangeNotifier`
- `notifyListeners()` called on state changes
- UI widgets rebuild automatically

## Assets

### Piece Images

Located in `assets/pieces/`:
- `wP.svg`, `wN.svg`, `wB.svg`, `wR.svg`, `wQ.svg`, `wK.svg`
- `bP.svg`, `bN.svg`, `bB.svg`, `bR.svg`, `bQ.svg`, `bK.svg`

Loaded via `flutter_svg` package:
```dart
SvgPicture.asset('assets/pieces/${piece.asset}')
```

## Dependencies

### Core Dependencies
- `flutter`: SDK
- `ffi`: Foreign Function Interface
- `provider`: State management
- `window_manager`: Desktop window management
- `stop_watch_timer`: Game timers
- `flutter_svg`: SVG rendering

### Dev Dependencies
- `flutter_test`: Testing framework
- `flutter_lints`: Linting rules
- `ffigen`: FFI binding generation (optional)

## Platform Support

### Desktop
- ✅ macOS (primary)
- ✅ Linux
- ✅ Windows

### Mobile
- ⚠️ iOS (planned)
- ⚠️ Android (planned)

### Web
- ❌ Not supported (requires native library)

## Performance Considerations

### UI Responsiveness
- Move generation can block UI (synchronous FFI calls)
- Future: Move engine to isolate for non-blocking UI

### Memory Management
- Explicit disposal of native resources
- FFI memory properly freed
- No memory leaks from handles

### Rendering
- SVG pieces for scalable graphics
- Efficient widget rebuilds via Provider
- Minimal setState calls

## Error Handling

### FFI Errors
- Try-catch around FFI calls
- Null checks for handles
- Validation of square indices

### User Input
- Move validation before execution
- Promotion dialog cancellation handling
- Illegal move prevention

## Testing

### Widget Tests
- Basic widget tests in `test/widget_test.dart`
- Future: More comprehensive UI tests

### Integration Tests
- Future: End-to-end game flow tests

## Known Issues

1. **Blocking UI**: Engine search blocks UI thread
2. **No move history**: Cannot undo/redo moves
3. **Limited game modes**: Only human vs. engine and engine vs. engine
4. **No analysis**: No move evaluation display
5. **No game saving**: Cannot save/load games

## Future Enhancements

1. **Move History**: Undo/redo functionality
2. **Game Analysis**: Show move evaluations
3. **Game Saving**: Save/load games (PGN)
4. **Move Hints**: Show suggested moves
5. **Difficulty Levels**: Adjustable engine strength
6. **Network Play**: Online multiplayer
7. **Game Database**: Store game history
8. **Opening Book**: Show opening names
9. **Endgame Tablebases**: Perfect endgame play indicator
10. **Non-blocking UI**: Move engine to isolate

## Code Style

- **Dart Style Guide**: Follows official Dart style guide
- **Naming**: `camelCase` for variables, `PascalCase` for classes
- **Widgets**: Stateless/Stateful widgets as appropriate
- **State Management**: Provider pattern for reactive updates

## Build Configuration

### `pubspec.yaml`
- SDK: `>=3.4.4 <4.0.0`
- Assets: `assets/pieces/`
- Dependencies: Listed in dependencies section

### Platform-Specific
- **macOS**: Window configuration in `main.dart`
- **Linux/Windows**: Similar window setup
- **Mobile**: Platform-specific configurations in respective directories
