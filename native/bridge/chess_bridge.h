#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * =============================================================================
 * CHESS ENGINE FFI BRIDGE
 * =============================================================================
 * 
 * This C API provides a Foreign Function Interface (FFI) for accessing
 * the C++ chess engine from Dart/Flutter.
 * 
 * DESIGN PRINCIPLES:
 * - Opaque handles for C++ objects (Board, Engine)
 * - Pre-allocated buffers for move arrays (no dynamic allocation in hot path)
 * - Explicit memory management (create/destroy pairs)
 * - Simple C types only (POD structs, integers, pointers)
 * 
 * MEMORY OWNERSHIP RULES:
 * - Handles: Caller creates and must destroy
 * - Strings: Library allocates, caller must free with chess_free_string()
 * - Move arrays: Caller allocates buffer, library fills it
 */

/*
 * =============================================================================
 * OPAQUE HANDLES
 * =============================================================================
 */

/**
 * Opaque handle to a Board object.
 * 
 * Represents the chess position, move history, and game state.
 * Thread-safety: Each handle should be used by only one thread.
 */
typedef void* ChessBoardHandle;

/**
 * Opaque handle to an Engine object.
 * 
 * Handles move generation, search, and evaluation.
 * Currently stateless but designed to support future features like
 * transposition tables and search configuration.
 * 
 * Thread-safety: Each handle should be used by only one thread.
 */
typedef void* ChessEngineHandle;

/*
 * =============================================================================
 * DATA STRUCTURES
 * =============================================================================
 */

/**
 * C-compatible move structure.
 * 
 * This is a Plain Old Data (POD) type that Dart can directly map.
 * Layout matches the C++ Move struct for efficient conversion.
 */
typedef struct {
    uint8_t piece;           // Piece type (use PIECE_* constants)
    uint8_t from_square;     // Source square (0-63, where 0=A1, 63=H8)
    uint8_t to_square;       // Destination square (0-63)
    uint8_t captured_piece;  // Captured piece type or PIECE_NONE
    uint8_t promoted_piece;  // Promotion piece type or PIECE_NONE
    uint8_t is_enpassant;    // 1 if en passant capture, 0 otherwise
    uint8_t is_castling;     // 1 if castling move, 0 otherwise
} CMove;

/*
 * =============================================================================
 * CONSTANTS
 * =============================================================================
 */

/* Color constants (matching C++ Color enum) */
#define COLOR_WHITE 0
#define COLOR_BLACK 1
#define COLOR_NONE  2


/* Piece constants (matching C++ Piece enum) */
#define PIECE_W_PAWN   0
#define PIECE_W_KNIGHT 1
#define PIECE_W_BISHOP 2
#define PIECE_W_ROOK   3
#define PIECE_W_QUEEN  4
#define PIECE_W_KING   5
#define PIECE_B_PAWN   6
#define PIECE_B_KNIGHT 7
#define PIECE_B_BISHOP 8
#define PIECE_B_ROOK   9
#define PIECE_B_QUEEN  10
#define PIECE_B_KING   11
#define PIECE_NONE     12

/* Maximum number of legal moves in any chess position */
#define MAX_LEGAL_MOVES 256

/*
 * =============================================================================
 * ENGINE FUNCTIONS
 * =============================================================================
 */

/**
 * Creates a new chess engine.
 * 
 * @return Engine handle (never returns NULL)
 * 
 * OWNERSHIP: Caller must call engine_destroy() when done.
 * 
 * NOTE: Currently the engine is stateless, but this API is designed
 * to support future features like transposition tables and configuration.
 */
ChessEngineHandle engine_create(void);

/**
 * Destroys an engine and frees associated memory.
 * 
 * @param handle Engine handle from engine_create()
 * 
 * SAFETY: Safe to call with NULL (no-op).
 * After calling, the handle is invalid and must not be used.
 */
void engine_destroy(ChessEngineHandle handle);

/**
 * Generates all legal moves for the current position.
 * 
 * @param engine Engine handle
 * @param board Board handle
 * @param moves Pre-allocated array to receive moves (caller owns buffer)
 * @param max_moves Size of the moves array (use MAX_LEGAL_MOVES to be safe)
 * @return Number of legal moves generated
 *         Returns 0 if checkmate or stalemate
 * 
 * USAGE:
 *   CMove moves[MAX_LEGAL_MOVES];
 *   int count = engine_generate_legal_moves(engine, board, 
 *                                                  moves, MAX_LEGAL_MOVES);
 *   for (int i = 0; i < count; i++) {
 *       // Process moves[i]
 *   }
 * 
 * SAFETY: If max_moves is less than the actual number of legal moves,
 * only the first max_moves are returned (but this should never happen
 * with MAX_LEGAL_MOVES = 256).
 */
int32_t engine_generate_legal_moves(ChessEngineHandle engine,
                                           ChessBoardHandle board,
                                           CMove* moves,
                                           int32_t max_moves);

uint8_t engine_get_random_move(ChessEngineHandle engine, ChessBoardHandle board, CMove* move);

/*
 * =============================================================================
 * BOARD LIFECYCLE
 * =============================================================================
 */

/**
 * Creates a new board in the standard starting position.
 * 
 * @return Board handle (never returns NULL)
 * 
 * OWNERSHIP: Caller must call board_destroy() when done.
 */
ChessBoardHandle board_create(void);

/**
 * Creates a board from a FEN (Forsyth-Edwards Notation) string.
 * 
 * @param fen Null-terminated FEN string (caller retains ownership)
 * @return Board handle, or NULL if FEN parsing failed
 * 
 * OWNERSHIP: On success, caller must call board_destroy() when done.
 * 
 * MEMORY: The FEN string is copied internally; caller can free their
 * copy immediately after this call returns.
 * 
 * EXAMPLE:
 *   ChessBoard board = board_create_from_fen(
 *       "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
 *   if (board == NULL) {
 *       // Handle invalid FEN
 *   }
 */
ChessBoardHandle board_create_from_fen(const char* fen);

/**
 * Destroys a board and frees all associated memory.
 * 
 * @param handle Board handle from board_create()
 * 
 * SAFETY: Safe to call with NULL (no-op).
 * After calling, the handle is invalid and must not be used.
 */
void board_destroy(ChessBoardHandle handle);

/*
 * =============================================================================
 * BOARD MANIPULATION
 * =============================================================================
 */

/**
 * Makes a move on the board.
 * 
 * @param handle Board handle
 * @param move Pointer to move structure (typically from generate_legal_moves)
 * 
 * ASSUMPTIONS: The move is assumed to be legal. Making an illegal move
 * results in undefined behavior (corrupted board state).
 * 
 * SIDE EFFECTS:
 * - Updates board position
 * - Switches side to move
 * - Updates castling rights, en passant, clocks
 * - Pushes move to history stack (for undo)
 * 
 * PERFORMANCE: This is a hot path function - no validation overhead.
 */
void board_make_move(ChessBoardHandle handle, const CMove* move);

/**
 * Undoes the last move made.
 * 
 * @param handle Board handle
 * 
 * SIDE EFFECTS: Restores the previous board position by popping from
 * the move history stack.
 * 
 * SAFETY: Safe to call when no moves have been made (no-op).
 */
void board_undo_move(ChessBoardHandle handle);

/**
 * Sets the board position from a FEN string.
 * 
 * @param handle Board handle
 * @param fen Null-terminated FEN string
 * @return 1 on success, 0 on parse error
 * 
 * SIDE EFFECTS: Clears move history, resets all board state.
 * 
 * USAGE: Useful for loading saved games, setting up puzzles, etc.
 */
uint8_t board_set_fen(ChessBoardHandle handle, const char* fen);

/*
 * =============================================================================
 * BOARD STATE QUERIES
 * =============================================================================
 */

/**
 * Gets the piece at a specific square.
 * 
 * @param handle Board handle
 * @param square Square index (0-63, where 0=A1, 7=H1, 56=A8, 63=H8)
 * @return Piece constant (PIECE_W_PAWN, etc.) or PIECE_NONE if empty
 * 
 * USAGE: Primary method for rendering the board in the UI.
 */
uint8_t board_get_piece_at(ChessBoardHandle handle, int square);

/**
 * Gets which side is to move.
 * 
 * @param handle Board handle
 * @return COLOR_WHITE or COLOR_BLACK
 */
uint8_t board_get_side_to_move(ChessBoardHandle handle);

/**
 * Gets the piece-square table (PST) score for a color.
 * 
 * @param handle Board handle
 * @param color Color to query (COLOR_WHITE=1 or COLOR_BLACK=2)
 * @return PST score (material + position evaluation)
 * 
 * USAGE: Useful for evaluating material advantage.
 *   int32_t white_score = board_get_pst_of_color(board, COLOR_WHITE);
 *   int32_t black_score = board_get_pst_of_color(board, COLOR_BLACK);
 *   int32_t advantage = white_score - black_score;
 */
int32_t board_get_pst_of_color(ChessBoardHandle handle, int color);

/**
 * Gets the current position as a FEN string.
 * 
 * @param handle Board handle
 * @return Allocated FEN string, or NULL if handle is invalid
 * 
 * MEMORY: Caller MUST call chess_free_string() on the returned string.
 * 
 * USAGE: For saving games, debugging, position sharing.
 * 
 * EXAMPLE:
 *   char* fen = board_get_fen(board);
 *   printf("Position: %s\n", fen);
 *   chess_free_string(fen);  // Don't forget!
 */
char* board_get_fen(ChessBoardHandle handle);

/*
 * =============================================================================
 * GAME STATE DETECTION
 * =============================================================================
 */

/**
 * Checks if the current side to move is in check.
 * 
 * @param handle Board handle
 * @return 1 if in check, 0 otherwise
 * 
 * USAGE: For UI indicators (highlighting king, etc.)
 */
uint8_t board_is_in_check(ChessBoardHandle handle);

/**
 * Checks if the current position is checkmate.
 * 
 * @param handle Board handle
 * @return 1 if checkmate, 0 otherwise
 * 
 * DEFINITION: Checkmate = in check AND no legal moves.
 * 
 * NOTE: This is more efficient than generating moves and checking
 * the count, as it can short-circuit.
 */
uint8_t board_is_checkmate(ChessEngineHandle engine_handle, ChessBoardHandle board_handle);

/**
 * Checks if the current position is stalemate.
 * 
 * @param handle Board handle
 * @return 1 if stalemate, 0 otherwise
 * 
 * DEFINITION: Stalemate = NOT in check AND no legal moves.
 */
uint8_t board_is_stalemate(ChessEngineHandle engine_handle, ChessBoardHandle board_handle);

/*
 * =============================================================================
 * UTILITY FUNCTIONS
 * =============================================================================
 */

/**
 * Converts a move to UCI (Universal Chess Interface) notation.
 * 
 * @param move Pointer to move structure
 * @return Allocated string in UCI format (e.g., "e2e4", "e7e8q")
 * 
 * MEMORY: Caller MUST call chess_free_string() on the returned string.
 * 
 * FORMAT:
 * - Normal moves: "e2e4"
 * - Promotions: "e7e8q" (last char = q/r/b/n)
 * - Castling: "e1g1" (king's move, same as any other move)
 */
char* chess_move_to_string(const CMove* cmove);

/**
 * Frees a string allocated by this library.
 * 
 * @param str String returned by board_get_fen() or chess_move_to_string()
 * 
 * CRITICAL: Always use this function to free library-allocated strings.
 * Do NOT use free() directly, as the library may use a different allocator.
 * 
 * SAFETY: Safe to call with NULL (no-op).
 */
void chess_free_string(char* str);

/*
 * =============================================================================
 * TESTING & DEBUGGING
 * =============================================================================
 */

/**
 * Performs a perft (performance test) search.
 * 
 * @param handle Board handle
 * @param depth Search depth (typically 1-6)
 * @return Number of leaf nodes at the given depth
 * 
 * USAGE: For validating move generation correctness against known values.
 * Standard positions have well-known perft values.
 * 
 * SIDE EFFECTS: Board state is unchanged (all moves are undone).
 * 
 * PERFORMANCE: Can be slow at depth > 5. This is for testing only,
 * not for use in production gameplay.
 * 
 * EXAMPLE:
 *   ChessBoard board = board_create();
 *   uint64_t nodes = chess_perft(board, 5);
 *   // Starting position perft(5) should be 4,865,609
 */
uint64_t chess_perft(ChessEngineHandle engine_handle, ChessBoardHandle board_handle, int32_t depth);

#ifdef __cplusplus
}
#endif