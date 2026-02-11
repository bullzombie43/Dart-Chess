# C++ Chess Engine Backend Documentation

## Overview

The C++ chess engine is a high-performance chess engine implementing complete chess rules, move generation, and search algorithms. It is designed to be called from Flutter via a C-compatible FFI bridge.

## Directory Structure

```
native/
├── include/           # Header files
│   ├── board.h       # Board representation
│   ├── engine.h      # Engine and search
│   ├── transposition.h # Transposition table
│   ├── uci.h         # UCI protocol
│   └── utils.h        # Utility functions
├── src/              # Implementation files
│   ├── board.cpp
│   ├── engine.cpp
│   ├── transposition.cpp
│   ├── uci.cpp
│   └── utils.cpp
├── bridge/           # FFI bridge layer
│   ├── chess_bridge.h
│   └── chess_bridge.cpp
└── CMakeLists.txt    # Build configuration
```

## Core Components

### 1. Board (`board.h` / `board.cpp`)

**Purpose**: Represents the chess position and manages game state.

#### Key Features
- **Bitboard Representation**: Uses 64-bit integers to represent piece positions
- **Move History**: Stack-based undo system for move/undo operations
- **FEN Support**: Load/save positions in Forsyth-Edwards Notation
- **Zobrist Hashing**: Position hashing for transposition tables

#### Data Structures

```cpp
class Board {
    // Position representation
    std::array<uint64_t, 12> bitboard_array;  // One bitboard per piece type
    Bitboard white_occupancy;                 // All white pieces
    Bitboard black_occupancy;                 // All black pieces
    
    // Game state
    Color sideToMove;                        // WHITE or BLACK
    uint8_t castlingRightsState;             // Castling availability
    std::optional<int> enPassantSquare;     // En passant target square
    int half_move_clock;                     // 50-move rule counter
    int num_moves_total;                     // Full move counter
    
    // Evaluation
    std::array<int, 2> pst_colors;          // Piece-square table scores
    
    // Move history
    Move_State move_history[2048];           // Stack for undo
    int history_ply;                         // Current depth in history
};
```

#### Key Methods

- `Board()`: Initialize starting position
- `set_position_fen(const std::string& fen)`: Load position from FEN
- `make_move(Move& move)`: Execute a move
- `undo_move()`: Undo last move
- `get_piece_at(int square)`: Get piece at square (0-63)
- `is_in_check(Color color)`: Check if king is in check
- `get_fen()`: Export position as FEN string
- `get_pst_color(Color color)`: Get material + positional score

#### Bitboard Operations

The engine uses bitboards for efficient piece manipulation:
- Each piece type has its own bitboard (12 total: 6 white + 6 black)
- Bitwise operations for move generation
- `__builtin_ctzll()` for bit scanning (GCC/Clang intrinsic)

### 2. Engine (`engine.h` / `engine.cpp`)

**Purpose**: Move generation, search algorithms, and position evaluation.

#### Key Features
- **Legal Move Generation**: Generates all legal moves for a position
- **Search Algorithms**: Negamax with alpha-beta pruning
- **Iterative Deepening**: Time-limited search with increasing depth
- **Move Ordering**: Prioritizes captures and good moves
- **Quiescence Search**: Extends search in tactical positions

#### Search Algorithms

**Negamax with Alpha-Beta Pruning**:
```cpp
int negamaxAB(Board& board, int depth, int ply, int alpha, int beta);
```

- Implements minimax with alpha-beta pruning
- Negamax formulation (score from current side's perspective)
- Transposition table integration
- Move ordering for better pruning

**Iterative Deepening**:
```cpp
Move search_iterative_deepening(Board& board, int time_limit_ms);
```

- Searches with increasing depth until time limit
- Returns best move found so far
- Allows time management

**Quiescence Search**:
```cpp
int quiesce_search(Board& board, int alpha, int beta, int depth);
```

- Extends search in tactical positions (captures, checks)
- Prevents horizon effect
- Only searches captures and promotions

#### Move Generation

**Pseudo-Legal Moves**:
```cpp
int generate_psuedo_legal_moves(const Board& board, Move* moves);
```

- Generates all moves that follow piece movement rules
- Does not check for legality (king safety)
- Includes: pawn moves, knight moves, sliding pieces, castling

**Legal Moves**:
```cpp
int generate_legal_moves(Board& board, Move* moves);
```

- Filters pseudo-legal moves to only legal ones
- Makes each move, checks if king is safe, undoes move
- Returns count of legal moves

**Move Ordering**:
```cpp
void order_moves(Move* moves, int num_moves);
```

- Prioritizes captures (MVV-LVA: Most Valuable Victim, Least Valuable Attacker)
- Prioritizes promotions
- Helps alpha-beta pruning efficiency

#### Position Evaluation

```cpp
int evaluate_position(Board& board);
```

**Evaluation Components**:
- **Material**: Piece values (Pawn=100, Knight=320, Bishop=330, Rook=500, Queen=900, King=20000)
- **Piece-Square Tables (PST)**: Positional bonuses/penalties
- **Score from current side's perspective** (negamax convention)

### 3. Transposition Table (`transposition.h` / `transposition.cpp`)

**Purpose**: Cache search results to avoid re-searching identical positions.

#### Structure

```cpp
struct TTEntry {
    uint64_t zobrist_key;  // Position hash
    uint16_t score;        // Cached score
    uint8_t depth;         // Search depth
    uint8_t flag;          // EXACT, LOWER_BOUND, UPPER_BOUND
    Move best_move;        // Best move found
};
```

#### Operations

- `get_entry(uint64_t zobrist_key)`: Retrieve cached entry
- `store(...)`: Store search result
- `clear()`: Clear table

#### Hash Flags
- `HASH_EXACT`: Exact score (alpha <= score <= beta)
- `HASH_BETA`: Lower bound (score >= beta, cutoff occurred)
- `HASH_ALPHA`: Upper bound (score <= alpha, all moves failed)

### 4. FFI Bridge (`bridge/chess_bridge.h` / `bridge/chess_bridge.cpp`)

**Purpose**: Provides C-compatible API for Flutter interop.

#### Design Principles

1. **Opaque Handles**: C++ objects exposed as `void*`
   ```cpp
   typedef void* ChessBoardHandle;
   typedef void* ChessEngineHandle;
   ```

2. **C-Compatible Structs**: POD types for data transfer
   ```cpp
   typedef struct {
       uint8_t piece;
       uint8_t from_square;
       uint8_t to_square;
       // ...
   } CMove;
   ```

3. **Explicit Memory Management**: Create/destroy pairs
   ```cpp
   ChessBoardHandle board_create(void);
   void board_destroy(ChessBoardHandle handle);
   ```

#### Key Functions

**Board Operations**:
- `board_create()`: Create board in starting position
- `board_create_from_fen(const char* fen)`: Create from FEN
- `board_destroy(handle)`: Free board
- `board_make_move(handle, move)`: Execute move
- `board_undo_move(handle)`: Undo move
- `board_get_piece_at(handle, square)`: Query piece
- `board_get_fen(handle)`: Get FEN string

**Engine Operations**:
- `engine_create()`: Create engine instance
- `engine_destroy(handle)`: Free engine
- `engine_generate_legal_moves(...)`: Generate moves
- `engine_get_best_move(...)`: Get best move via search
- `engine_get_random_move(...)`: Get random legal move

**Utility**:
- `chess_move_to_string(move)`: Convert to UCI notation
- `chess_free_string(str)`: Free library-allocated strings
- `chess_perft(...)`: Performance test function

### 5. UCI Protocol (`uci.h` / `uci.cpp`)

**Purpose**: Universal Chess Interface support for engine vs. engine matches.

**Status**: Implemented but not primary interface (FFI is main interface).

## Move Representation

### Move Structure

```cpp
struct Move {
    uint8_t piece;           // Moving piece type
    uint8_t from_square;     // Source square (0-63)
    uint8_t to_square;       // Destination square (0-63)
    Piece captured_piece;    // Captured piece or NONE
    Piece promoted_piece;    // Promotion piece or NONE
    bool is_enpassant;       // En passant flag
    bool is_castling;        // Castling flag
};
```

### Square Indexing

Squares are indexed 0-63:
- 0 = a1, 7 = h1
- 8 = a2, 15 = h2
- ...
- 56 = a8, 63 = h8

## Piece Representation

### Piece Enum

```cpp
enum Piece : uint8_t {
    W_PAWN = 0, W_KNIGHT, W_BISHOP, W_ROOK, W_QUEEN, W_KING,
    B_PAWN, B_KNIGHT, B_BISHOP, B_ROOK, B_QUEEN, B_KING,
    NONE
};
```

### Piece Values

```cpp
constexpr int PIECE_VALUES[13] = {
    100,    // W_PAWN
    320,    // W_KNIGHT
    330,    // W_BISHOP
    500,    // W_ROOK
    900,    // W_QUEEN
    20000,  // W_KING
    // ... (same for black)
};
```

## Build System

### CMake Configuration

**Libraries**:
- `chess_core`: Static library (board, engine, utils, transposition)
- `chess_bridge`: Shared library (FFI bridge, links to chess_core)

**Executables**:
- `chess_engine`: Standalone engine
- `chess_engine_uci`: UCI protocol engine
- Test executables: `board_test`, `engine_test`, `bridge_test`

**Build Modes**:
- **Debug**: ASan enabled, debug symbols, no optimizations
- **Release**: `-O3 -march=native`, no debug info

### Compilation Flags

**Debug**:
```cmake
-ggdb -fsanitize=address,undefined -fno-omit-frame-pointer
```

**Release**:
```cmake
-O3 -march=native -DNDEBUG
```

## Testing

### Google Test Framework

**Test Executables**:
- `board_test`: Board functionality tests
- `engine_test`: Engine move generation and search tests
- `bridge_test`: FFI bridge tests

**Perft Tests**: Performance test for move generation correctness
- Validates against known perft values for standard positions
- Depth 1-6 typically tested

## Performance Characteristics

### Move Generation
- **Pseudo-legal**: ~1-2 million moves/second (depends on position)
- **Legal**: ~500k-1M moves/second (includes legality checks)

### Search Performance
- **Nodes/second**: Varies with position complexity
- **Depth**: Typically searches 4-8 plies in reasonable time
- **Transposition table**: Significant speedup (2-5x in some positions)

### Memory Usage
- **Board**: ~200 bytes per board instance
- **Move history**: 2048 moves max (~16KB)
- **Transposition table**: Configurable (default 16MB)

## Algorithm Details

### Alpha-Beta Pruning

The engine uses negamax with alpha-beta pruning:

```cpp
int negamaxAB(Board& board, int depth, int ply, int alpha, int beta) {
    if (depth == 0) {
        return quiesce_search(board, alpha, beta, 0);
    }
    
    // Check transposition table
    // Generate moves
    // Order moves
    // Search each move
    // Update alpha
    // Beta cutoff if applicable
}
```

### Move Ordering Heuristics

1. **Captures**: MVV-LVA (Most Valuable Victim, Least Valuable Attacker)
2. **Promotions**: High priority
3. **Killer moves**: Moves that caused cutoffs at same depth
4. **History heuristic**: Moves that were good in similar positions

### Quiescence Search

Extends search in tactical positions:
- Only searches captures
- Only searches checks (optional)
- Prevents horizon effect
- Typically 2-4 plies deep

## Known Limitations

1. **Single-threaded**: No parallel search
2. **No opening book**: Always starts from scratch
3. **No endgame tablebases**: Relies on search only
4. **Limited time management**: Basic iterative deepening
5. **No pondering**: Doesn't think during opponent's time

## Future Improvements

1. **Multi-threading**: Parallel search with Lazy SMP
2. **Opening book**: Pre-computed opening moves
3. **Endgame tablebases**: Perfect endgame play
4. **Better evaluation**: More sophisticated positional evaluation
5. **Time management**: Better time allocation
6. **Pondering**: Think during opponent's turn
7. **Null-move pruning**: Additional pruning technique
8. **Late move reduction**: Reduce search depth for unlikely moves

## Code Style

- **C++20**: Uses modern C++ features
- **Naming**: `snake_case` for functions, `PascalCase` for classes
- **Comments**: Doxygen-style documentation
- **Error Handling**: Exceptions for errors, assertions for invariants

## Dependencies

- **C++ Standard**: C++20
- **Build System**: CMake 3.10+
- **Testing**: Google Test (via FetchContent)
- **Compiler**: GCC/Clang with C++20 support
