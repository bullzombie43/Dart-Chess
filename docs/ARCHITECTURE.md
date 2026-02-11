# Chess UI - Overall Architecture and Design

## Overview

Chess UI is a cross-platform chess application built with Flutter for the user interface and a custom C++ chess engine for game logic. The application supports human vs. engine gameplay, engine vs. engine matches, and provides a modern, responsive UI for chess gameplay.

## System Architecture

The application follows a **hybrid architecture** combining:
- **Flutter Frontend**: Cross-platform UI layer (Dart)
- **C++ Engine Backend**: High-performance chess engine
- **FFI Bridge**: Foreign Function Interface connecting Flutter to C++

```
┌─────────────────────────────────────────────────────────┐
│                    Flutter UI Layer                     │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐   │
│  │   UI Widgets │  │ Game Logic   │  │  Controllers │   │
│  │  (board, etc)│  │  (Dart)      │  │  (state mgmt)│   │
│  └──────────────┘  └──────────────┘  └──────────────┘   │
└─────────────────────────────────────────────────────────┘
                          │
                          │ FFI (Foreign Function Interface)
                          ▼
┌─────────────────────────────────────────────────────────┐
│                    FFI Bridge Layer                     │
│  ┌────────────────────────────────────────────────────┐ │
│  │  chess_bridge.cpp / chess_bridge.h (C API)         │ │
│  │  - Opaque handles (ChessBoardHandle, etc.)         │ │
│  │  - C-compatible structs (CMove)                    │ │
│  │  - Memory management functions                     │ │
│  └────────────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────────┘
                          │
                          │ C++ calls
                          ▼
┌─────────────────────────────────────────────────────────┐
│                  C++ Chess Engine Core                  │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐   │
│  │    Board     │  │    Engine    │  │ Transposition│   │
│  │  (position)  │  │  (search)    │  │    Table     │   │
│  └──────────────┘  └──────────────┘  └──────────────┘   │
│  ┌──────────────┐  ┌──────────────┐                     │
│  │     UCI      │  │    Utils     │                     │
│  │  (protocol)  │  │  (helpers)   │                     │
│  └──────────────┘  └──────────────┘                     │
└─────────────────────────────────────────────────────────┘
```

## Key Components

### 1. Flutter Frontend (`lib/`)

**Purpose**: Provides the user interface and orchestrates game flow.

**Key Modules**:
- `main.dart`: Application entry point, window management, widget tree setup
- `game/`: Game logic and state management
  - `chess_engine.dart`: Dart wrapper around FFI bindings
  - `chess_ffi.dart`: Low-level FFI bindings to C++ library
  - `game_controller.dart`: Game state controller with timers
  - `game_state.dart`: Alternative Dart-based game state (legacy)
  - `cutechess_manager.dart`: Engine vs. engine match management
  - `pgn_watcher.dart`: PGN file monitoring for match results
- `ui/`: User interface components
  - `board_background.dart`: Chess board rendering
  - `board_pieces.dart`: Piece rendering and interaction
  - `board_controls.dart`: Game controls (timers, new game, etc.)
  - `board_builder.dart`: Board layout builder
  - `engine_dropdown_button.dart`: Engine selection UI

### 2. FFI Bridge (`native/bridge/`)

**Purpose**: Provides a C-compatible API for Flutter to call C++ functions.

**Key Files**:
- `chess_bridge.h`: C API header with function declarations
- `chess_bridge.cpp`: Implementation converting C calls to C++

**Design Principles**:
- **Opaque Handles**: C++ objects (Board, Engine) are exposed as `void*` handles
- **POD Structs**: Simple C structs (CMove) for data transfer
- **Explicit Memory Management**: Create/destroy pairs for all resources
- **Pre-allocated Buffers**: Move arrays allocated by caller to avoid dynamic allocation in hot paths

### 3. C++ Engine (`native/src/` and `native/include/`)

**Purpose**: High-performance chess engine implementing game rules, move generation, and search.

**Key Modules**:
- `board.h/cpp`: Chess position representation using bitboards
- `engine.h/cpp`: Move generation, search algorithms (negamax, alpha-beta)
- `transposition.h/cpp`: Transposition table for search optimization
- `uci.h/cpp`: Universal Chess Interface protocol support
- `utils.h/cpp`: Utility functions and helpers

## Data Flow

### Making a Move

1. **User Interaction** (`main.dart`):
   - User taps/drags a piece
   - `handleSquareTap()` or `handlePieceDragEnd()` called

2. **Move Validation** (`main.dart`):
   - `ChessEngine.generateLegalMoves()` called (Dart)
   - Calls FFI: `chessEngineGenerateLegalMoves()`

3. **FFI Bridge** (`chess_bridge.cpp`):
   - Converts Dart `CMove` structs to C++ `Move` structs
   - Calls `Engine::generate_legal_moves()`

4. **Engine Processing** (`engine.cpp`):
   - Generates pseudo-legal moves
   - Filters to legal moves (check validation)
   - Returns move array

5. **Move Execution**:
   - If move is legal, `board.makeMove()` called
   - Updates board state via FFI
   - UI updates via `setState()`

### Engine Move Generation

1. **Request** (`main.dart`):
   - `engine.getBestMove(board)` called

2. **FFI** (`chess_bridge.cpp`):
   - `engine_get_best_move()` converts to C++

3. **Search** (`engine.cpp`):
   - `Engine::search_iterative_deepening()` performs minimax/alpha-beta search
   - Uses transposition table for optimization
   - Returns best move

4. **Execution**:
   - Move applied to board
   - UI updated

## Build System

### C++ Build (`native/CMakeLists.txt`)
- **Static Library**: `chess_core` (board, engine, utils, transposition)
- **Shared Library**: `chess_bridge` (FFI bridge, links to chess_core)
- **Executables**: 
  - `chess_engine`: Standalone engine
  - `chess_engine_uci`: UCI protocol engine
  - Test executables: `board_test`, `engine_test`, `bridge_test`

### Flutter Build (`pubspec.yaml`)
- **Dependencies**:
  - `ffi`: Foreign Function Interface support
  - `window_manager`: Desktop window management
  - `stop_watch_timer`: Game timers
  - `flutter_svg`: Piece rendering
  - `provider`: State management

### Platform Integration
- **macOS**: Library loaded as `libchess_bridge.dylib`
- **Linux**: Library loaded as `libchess_bridge.so`
- **Windows**: Library loaded as `chess_bridge.dll`
- **iOS/Android**: Library embedded in app bundle

## Design Patterns

### 1. Opaque Handle Pattern
C++ objects are hidden behind void pointers to maintain ABI stability and hide implementation details.

```cpp
typedef void* ChessBoardHandle;
ChessBoardHandle board_create(void);
```

### 2. Resource Management
Explicit create/destroy pairs ensure proper memory management across language boundaries.

```dart
final board = ChessBoard();  // Creates handle
// ... use board ...
board.dispose();  // Destroys handle
```

### 3. State Management
Flutter's `ChangeNotifier` pattern used in `GameController` for reactive UI updates.

### 4. Separation of Concerns
- **UI Layer**: Presentation and user interaction
- **Game Logic Layer**: Move validation, game state
- **Engine Layer**: Chess rules, search algorithms
- **Bridge Layer**: Language interoperability

## Threading Model

- **Single-threaded**: All operations run on Flutter's main thread
- **Synchronous FFI calls**: Engine operations block UI (acceptable for chess)
- **Future considerations**: Engine search could be moved to isolate for non-blocking UI

## Memory Management

### C++ Side
- RAII (Resource Acquisition Is Initialization)
- Smart pointers not used (C API compatibility)
- Manual `new`/`delete` in bridge layer

### Dart Side
- Automatic garbage collection
- Explicit `dispose()` methods for native resources
- FFI memory allocated with `calloc`/`malloc`, freed with `free`

## Error Handling

- **C++**: Exceptions thrown, caught at bridge boundary
- **Dart**: Try-catch blocks around FFI calls
- **Validation**: Input validation on both sides (square bounds, null checks)

## Testing

### C++ Tests
- Google Test framework
- Unit tests for board, engine, bridge
- Perft tests for move generation validation

### Flutter Tests
- Widget tests in `test/widget_test.dart`
- Integration tests (future)

## Performance Considerations

1. **Bitboard Representation**: Efficient piece position storage
2. **Pre-allocated Buffers**: Avoid dynamic allocation in hot paths
3. **Transposition Table**: Cache search results
4. **Move Ordering**: Prioritize good moves for alpha-beta pruning
5. **Quiescence Search**: Extend search in tactical positions

## Future Enhancements

- Multi-threaded search
- Opening book
- Endgame tablebases
- Network play
- Game history/analysis
- Move hints
- Difficulty levels

## Dependencies

### C++
- C++20 standard
- Google Test (via FetchContent)
- CMake 3.10+

### Dart/Flutter
- Flutter SDK >=3.4.4
- FFI package for native interop
- Provider for state management

## Platform Support

- ✅ macOS (primary development platform)
- ✅ Linux
- ✅ Windows
- ✅ iOS (planned)
- ✅ Android (planned)
- ✅ Web (not supported - requires native library)
