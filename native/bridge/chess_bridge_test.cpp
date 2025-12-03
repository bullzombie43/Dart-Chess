#include <gtest/gtest.h>
#include "chess_bridge.h"
#include <cstring>

/*
 * =============================================================================
 * STAGE 1: LIFECYCLE TESTS
 * =============================================================================
 * 
 * Test that we can create and destroy objects without crashes or leaks.
 * These are the most fundamental tests - if these fail, nothing else matters!
 */

// =============================================================================
// ENGINE LIFECYCLE
// =============================================================================

TEST(BridgeEngineTest, CreateDestroy) {
    ChessEngineHandle engine = engine_create();
    ASSERT_NE(engine, nullptr);
    engine_destroy(engine);
    // If we get here without crashing, success!
}

TEST(BridgeEngineTest, DestroyNull) {
    // Should not crash with NULL handle
    EXPECT_NO_THROW(engine_destroy(nullptr));
}

TEST(BridgeEngineTest, CreateMultiple) {
    // Should be able to create multiple engines
    ChessEngineHandle engine1 = engine_create();
    ChessEngineHandle engine2 = engine_create();
    
    ASSERT_NE(engine1, nullptr);
    ASSERT_NE(engine2, nullptr);
    EXPECT_NE(engine1, engine2);  // Should be different objects
    
    engine_destroy(engine1);
    engine_destroy(engine2);
}

// =============================================================================
// BOARD LIFECYCLE
// =============================================================================

TEST(BridgeBoardTest, CreateDestroy) {
    ChessBoardHandle board = board_create();
    ASSERT_NE(board, nullptr);
    board_destroy(board);
}

TEST(BridgeBoardTest, DestroyNull) {
    EXPECT_NO_THROW(board_destroy(nullptr));
}

TEST(BridgeBoardTest, CreateMultiple) {
    ChessBoardHandle board1 = board_create();
    ChessBoardHandle board2 = board_create();
    
    ASSERT_NE(board1, nullptr);
    ASSERT_NE(board2, nullptr);
    EXPECT_NE(board1, board2);
    
    board_destroy(board1);
    board_destroy(board2);
}

TEST(BridgeBoardTest, CreateFromValidFen) {
    const char* fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    ChessBoardHandle board = board_create_from_fen(fen);
    ASSERT_NE(board, nullptr);
    board_destroy(board);
}

TEST(BridgeBoardTest, CreateFromInvalidFen) {
    ChessBoardHandle board = board_create_from_fen("invalid fen string");
    EXPECT_EQ(board, nullptr);  // Should return NULL on failure
}

TEST(BridgeBoardTest, CreateFromNullFen) {
    ChessBoardHandle board = board_create_from_fen(nullptr);
    EXPECT_EQ(board, nullptr);
}

/*
 * =============================================================================
 * STAGE 2: BOARD STATE QUERY TESTS
 * =============================================================================
 * 
 * Test that we can query board state correctly:
 * - Getting pieces at specific squares
 * - Side to move
 * - Check detection
 * - Proper NULL handling
 */

// =============================================================================
// GET PIECE AT SQUARE TESTS
// =============================================================================

TEST(BridgeBoardStateTest, GetPieceAtStartingPosition) {
    ChessBoardHandle board = board_create();
    
    // Test white pieces on rank 1
    EXPECT_EQ(board_get_piece_at(board, 0), PIECE_W_ROOK);    // a1
    EXPECT_EQ(board_get_piece_at(board, 1), PIECE_W_KNIGHT);  // b1
    EXPECT_EQ(board_get_piece_at(board, 2), PIECE_W_BISHOP);  // c1
    EXPECT_EQ(board_get_piece_at(board, 3), PIECE_W_QUEEN);   // d1
    EXPECT_EQ(board_get_piece_at(board, 4), PIECE_W_KING);    // e1
    EXPECT_EQ(board_get_piece_at(board, 5), PIECE_W_BISHOP);  // f1
    EXPECT_EQ(board_get_piece_at(board, 6), PIECE_W_KNIGHT);  // g1
    EXPECT_EQ(board_get_piece_at(board, 7), PIECE_W_ROOK);    // h1
    
    // Test white pawns on rank 2
    for (int i = 8; i < 16; i++) {
        EXPECT_EQ(board_get_piece_at(board, i), PIECE_W_PAWN) 
            << "Square " << i << " should have white pawn";
    }
    
    // Test empty squares on ranks 3-6
    for (int i = 16; i < 48; i++) {
        EXPECT_EQ(board_get_piece_at(board, i), PIECE_NONE)
            << "Square " << i << " should be empty";
    }
    
    // Test black pawns on rank 7
    for (int i = 48; i < 56; i++) {
        EXPECT_EQ(board_get_piece_at(board, i), PIECE_B_PAWN)
            << "Square " << i << " should have black pawn";
    }
    
    // Test black pieces on rank 8
    EXPECT_EQ(board_get_piece_at(board, 56), PIECE_B_ROOK);   // a8
    EXPECT_EQ(board_get_piece_at(board, 57), PIECE_B_KNIGHT); // b8
    EXPECT_EQ(board_get_piece_at(board, 58), PIECE_B_BISHOP); // c8
    EXPECT_EQ(board_get_piece_at(board, 59), PIECE_B_QUEEN);  // d8
    EXPECT_EQ(board_get_piece_at(board, 60), PIECE_B_KING);   // e8
    EXPECT_EQ(board_get_piece_at(board, 61), PIECE_B_BISHOP); // f8
    EXPECT_EQ(board_get_piece_at(board, 62), PIECE_B_KNIGHT); // g8
    EXPECT_EQ(board_get_piece_at(board, 63), PIECE_B_ROOK);   // h8
    
    board_destroy(board);
}

TEST(BridgeBoardStateTest, GetPieceAtCustomPosition) {
    // Position after 1.e4
    const char* fen = "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1";
    ChessBoardHandle board = board_create_from_fen(fen);
    ASSERT_NE(board, nullptr);
    
    // e4 should have white pawn
    EXPECT_EQ(board_get_piece_at(board, 28), PIECE_W_PAWN);  // e4 = 28
    
    // e2 should be empty
    EXPECT_EQ(board_get_piece_at(board, 12), PIECE_NONE);    // e2 = 12
    
    // Other pieces unchanged
    EXPECT_EQ(board_get_piece_at(board, 4), PIECE_W_KING);   // e1
    EXPECT_EQ(board_get_piece_at(board, 60), PIECE_B_KING);  // e8
    
    board_destroy(board);
}

TEST(BridgeBoardStateTest, GetPieceAtInvalidSquares) {
    ChessBoardHandle board = board_create();
    
    // Negative squares
    EXPECT_ANY_THROW(board_get_piece_at(board, -1));
    EXPECT_ANY_THROW(board_get_piece_at(board, -100));
    
    // Out of bounds (> 63)
    EXPECT_ANY_THROW(board_get_piece_at(board, 64));
    EXPECT_ANY_THROW(board_get_piece_at(board, 100));
    
    board_destroy(board);
}

TEST(BridgeBoardStateTest, GetPieceAtNullBoard) {
    EXPECT_ANY_THROW(board_get_piece_at(nullptr, 0));
    EXPECT_ANY_THROW(board_get_piece_at(nullptr, 28));
}

// =============================================================================
// SIDE TO MOVE TESTS
// =============================================================================

TEST(BridgeBoardStateTest, SideToMoveStartingPosition) {
    ChessBoardHandle board = board_create();
    EXPECT_EQ(board_get_side_to_move(board), COLOR_WHITE);
    board_destroy(board);
}

TEST(BridgeBoardStateTest, SideToMoveAfterBlackMove) {
    // Position with black to move
    const char* fen = "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1";
    ChessBoardHandle board = board_create_from_fen(fen);
    ASSERT_NE(board, nullptr);
    
    EXPECT_EQ(board_get_side_to_move(board), COLOR_BLACK);
    
    board_destroy(board);
}

TEST(BridgeBoardStateTest, SideToMoveNullBoard) {
    EXPECT_ANY_THROW(board_get_side_to_move(nullptr));
}

// =============================================================================
// CHECK DETECTION TESTS
// =============================================================================

TEST(BridgeBoardStateTest, IsInCheckStartingPosition) {
    ChessBoardHandle board = board_create();
    
    // No check in starting position
    EXPECT_EQ(board_is_in_check(board), 0);
    
    board_destroy(board);
}

TEST(BridgeBoardStateTest, IsInCheckWhiteInCheck) {
    // Position where white king is in check from black queen
    // This is a bit tricky to set up, so we'll use a known check position
    // Scholar's mate position: white king in check
    const char* fen = "r1bqkb1r/pppp1ppp/2n2n2/4p2Q/2B1P3/8/PPPP1PPP/RNB1K1NR b KQkq - 0 1";
    ChessBoardHandle board = board_create_from_fen(fen);
    
    if (board != nullptr) {
        // Black to move, but we're checking if white is in check
        // Actually this position white is NOT in check, let me use a better example
        board_destroy(board);
    }
    
    // Let's use a simpler check position: white king on e1, black rook on e8
    const char* check_fen = "4r3/8/8/8/8/8/8/4K3 w - - 0 1";
    board = board_create_from_fen(check_fen);
    ASSERT_NE(board, nullptr);
    
    // White king is in check from rook
    EXPECT_EQ(board_is_in_check(board), 1);
    
    board_destroy(board);
}

TEST(BridgeBoardStateTest, IsInCheckBlackInCheck) {
    // Black king on e8, white rook on e1
    const char* fen = "4k3/8/8/8/8/8/8/4R3 b - - 0 1";
    ChessBoardHandle board = board_create_from_fen(fen);
    ASSERT_NE(board, nullptr);
    
    // Black king is in check from rook
    EXPECT_EQ(board_is_in_check(board), 1);
    
    board_destroy(board);
}

TEST(BridgeBoardStateTest, IsInCheckNullBoard) {
    EXPECT_ANY_THROW(board_is_in_check(nullptr));
}

// =============================================================================
// INTEGRATION TEST: State After FEN
// =============================================================================

TEST(BridgeBoardStateTest, CompleteStateFromFEN) {
    // Test that all state queries work together correctly
    const char* fen = "r1bqkbnr/pppp1ppp/2n5/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R w KQkq e6 0 3";
    ChessBoardHandle board = board_create_from_fen(fen);
    ASSERT_NE(board, nullptr);
    
    // Verify side to move
    EXPECT_EQ(board_get_side_to_move(board), COLOR_WHITE);
    
    // Verify specific pieces
    EXPECT_EQ(board_get_piece_at(board, 21), PIECE_W_KNIGHT);  // f3
    EXPECT_EQ(board_get_piece_at(board, 28), PIECE_W_PAWN);    // e4
    EXPECT_EQ(board_get_piece_at(board, 36), PIECE_B_PAWN);    // e5
    EXPECT_EQ(board_get_piece_at(board, 42), PIECE_B_KNIGHT);  // c6
    
    // Verify not in check
    EXPECT_EQ(board_is_in_check(board), 0);
    
    board_destroy(board);
}

/*
 * =============================================================================
 * STAGE 3: FEN OPERATIONS AND STRING MEMORY MANAGEMENT
 * =============================================================================
 * 
 * Critical tests for string handling:
 * - Getting FEN strings (allocation)
 * - Setting FEN strings (parsing)
 * - Proper memory cleanup (no leaks!)
 * - Round-trip FEN consistency
 */

// =============================================================================
// GET FEN TESTS
// =============================================================================

TEST(BridgeFENTest, GetFenStartingPosition) {
    ChessBoardHandle board = board_create();
    char* fen = board_get_fen(board);
    
    ASSERT_NE(fen, nullptr);
    
    // Should contain starting position components
    EXPECT_NE(strstr(fen, "rnbqkbnr"), nullptr) << "Missing black pieces";
    EXPECT_NE(strstr(fen, "RNBQKBNR"), nullptr) << "Missing white pieces";
    EXPECT_NE(strstr(fen, " w "), nullptr) << "Missing white to move";
    EXPECT_NE(strstr(fen, "KQkq"), nullptr) << "Missing castling rights";
    
    // Full starting FEN should match exactly
    const char* expected = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 0"; //We dont use total_moves yet so leaver it at 0
    EXPECT_STREQ(fen, expected);
    
    chess_free_string(fen);
    board_destroy(board);
}

TEST(BridgeFENTest, GetFenCustomPosition) {
    // Create from FEN, get it back, should match
    const char* original_fen = "r1bqkbnr/pppp1ppp/2n5/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R w KQkq e6 0 3";
    ChessBoardHandle board = board_create_from_fen(original_fen);
    ASSERT_NE(board, nullptr);
    
    char* retrieved_fen = board_get_fen(board);
    ASSERT_NE(retrieved_fen, nullptr);
    
    // Should match exactly
    EXPECT_STREQ(retrieved_fen, original_fen);
    
    chess_free_string(retrieved_fen);
    board_destroy(board);
}

TEST(BridgeFENTest, GetFenAfterMove) {
    ChessEngineHandle engine = engine_create();
    ChessBoardHandle board = board_create();
    
    // Generate and make a move
    CMove moves[MAX_LEGAL_MOVES];
    int32_t count = engine_generate_legal_moves(engine, board, moves, MAX_LEGAL_MOVES);
    ASSERT_GT(count, 0);
    
    board_make_move(board, &moves[0]);
    
    // Get FEN after move
    char* fen = board_get_fen(board);
    ASSERT_NE(fen, nullptr);
    
    // Should show black to move now
    EXPECT_NE(strstr(fen, " b "), nullptr) << "Should be black's turn";
    
    // Should NOT match starting position
    const char* starting_fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    EXPECT_STRNE(fen, starting_fen);
    
    chess_free_string(fen);
    engine_destroy(engine);
    board_destroy(board);
}

TEST(BridgeFENTest, GetFenNullBoard) {
    EXPECT_ANY_THROW(board_get_fen(nullptr));
}

TEST(BridgeFENTest, GetFenMultipleTimes) {
    // Test that we can get FEN multiple times without issues
    ChessBoardHandle board = board_create();
    
    char* fen1 = board_get_fen(board);
    char* fen2 = board_get_fen(board);
    char* fen3 = board_get_fen(board);
    
    ASSERT_NE(fen1, nullptr);
    ASSERT_NE(fen2, nullptr);
    ASSERT_NE(fen3, nullptr);
    
    // All should be equal
    EXPECT_STREQ(fen1, fen2);
    EXPECT_STREQ(fen2, fen3);
    
    // But should be different memory addresses (separately allocated)
    EXPECT_NE(fen1, fen2);
    EXPECT_NE(fen2, fen3);
    
    chess_free_string(fen1);
    chess_free_string(fen2);
    chess_free_string(fen3);
    board_destroy(board);
}

// =============================================================================
// SET FEN TESTS
// =============================================================================

TEST(BridgeFENTest, SetFenValid) {
    ChessBoardHandle board = board_create();
    
    const char* new_fen = "r1bqkbnr/pppp1ppp/2n5/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R w KQkq e6 0 3";
    uint8_t success = board_set_fen(board, new_fen);
    
    EXPECT_EQ(success, 1);
    
    // Verify it was actually set
    char* retrieved = board_get_fen(board);
    ASSERT_NE(retrieved, nullptr);
    EXPECT_STREQ(retrieved, new_fen);
    
    chess_free_string(retrieved);
    board_destroy(board);
}

TEST(BridgeFENTest, SetFenInvalid) {
    ChessBoardHandle board = board_create();
    
    // Try to set invalid FEN
    EXPECT_ANY_THROW(board_set_fen(board, "this is not a valid fen"));  // Should fail
    
    // Board should still be in some valid state (likely unchanged or default)
    uint8_t side = board_get_side_to_move(board);
    EXPECT_TRUE(side == COLOR_WHITE || side == COLOR_BLACK);
    
    board_destroy(board);
}

TEST(BridgeFENTest, SetFenNull) {
    ChessBoardHandle board = board_create();
    
    EXPECT_ANY_THROW(board_set_fen(board, nullptr));
    
    board_destroy(board);
}

TEST(BridgeFENTest, SetFenNullBoard) {
    const char* fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

    EXPECT_ANY_THROW(board_set_fen(nullptr, fen););
}

TEST(BridgeFENTest, SetFenMultipleTimes) {
    ChessBoardHandle board = board_create();
    
    const char* fen1 = "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1";
    const char* fen2 = "rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq e6 0 2";
    const char* fen3 = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    
    EXPECT_EQ(board_set_fen(board, fen1), 1);
    EXPECT_EQ(board_get_side_to_move(board), COLOR_BLACK);
    
    EXPECT_EQ(board_set_fen(board, fen2), 1);
    EXPECT_EQ(board_get_side_to_move(board), COLOR_WHITE);
    
    EXPECT_EQ(board_set_fen(board, fen3), 1);
    EXPECT_EQ(board_get_side_to_move(board), COLOR_WHITE);
    
    board_destroy(board);
}

// =============================================================================
// FEN ROUND-TRIP TESTS
// =============================================================================

TEST(BridgeFENTest, RoundTripStartingPosition) {
    const char* original = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    
    ChessBoardHandle board = board_create_from_fen(original);
    ASSERT_NE(board, nullptr);
    
    char* retrieved = board_get_fen(board);
    ASSERT_NE(retrieved, nullptr);
    
    EXPECT_STREQ(original, retrieved);
    
    chess_free_string(retrieved);
    board_destroy(board);
}

TEST(BridgeFENTest, RoundTripComplexPosition) {
    const char* original = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    
    ChessBoardHandle board = board_create_from_fen(original);
    ASSERT_NE(board, nullptr);
    
    char* retrieved = board_get_fen(board);
    ASSERT_NE(retrieved, nullptr);
    
    EXPECT_STREQ(original, retrieved);
    
    chess_free_string(retrieved);
    board_destroy(board);
}

TEST(BridgeFENTest, RoundTripAfterSetFen) {
    ChessBoardHandle board = board_create();
    
    const char* fen = "r1bqkb1r/pppp1ppp/2n2n2/4p3/2B1P3/5N2/PPPP1PPP/RNBQK2R w KQkq e6 0 4";
    
    ASSERT_EQ(board_set_fen(board, fen), 1);
    
    char* retrieved = board_get_fen(board);
    ASSERT_NE(retrieved, nullptr);
    
    EXPECT_STREQ(fen, retrieved);
    
    chess_free_string(retrieved);
    board_destroy(board);
}

// =============================================================================
// MEMORY MANAGEMENT TESTS
// =============================================================================

TEST(BridgeFENTest, FreeStringNull) {
    // Should not crash
    EXPECT_NO_THROW(chess_free_string(nullptr));
}

TEST(BridgeFENTest, FreeStringMultipleTimes) {
    // Note: This test is intentionally dangerous - freeing twice is UB
    // But we want to document expected behavior
    // In practice, DON'T do this - it's for testing our implementation
    
    ChessBoardHandle board = board_create();
    char* fen = board_get_fen(board);
    ASSERT_NE(fen, nullptr);
    
    chess_free_string(fen);
    // Do NOT free again - that would be a double-free bug
    // chess_free_string(fen);  // NEVER DO THIS
    
    board_destroy(board);
}

TEST(BridgeFENTest, NoLeaksMultipleGetFen) {
    // This test will be caught by ASan if there are leaks
    ChessBoardHandle board = board_create();
    
    for (int i = 0; i < 100; i++) {
        char* fen = board_get_fen(board);
        ASSERT_NE(fen, nullptr);
        chess_free_string(fen);
    }
    
    board_destroy(board);
}

TEST(BridgeFENTest, NoLeaksMultipleSetFen) {
    // ASan will catch leaks here too
    ChessBoardHandle board = board_create();
    
    const char* fens[] = {
        "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1",
        "rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq e6 0 2",
        "rnbqkbnr/pppp1ppp/8/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R b KQkq - 1 2",
    };
    
    for (int i = 0; i < 50; i++) {
        for (int j = 0; j < 3; j++) {
            ASSERT_EQ(board_set_fen(board, fens[j]), 1);
        }
    }
    
    board_destroy(board);
}

// =============================================================================
// EDGE CASES
// =============================================================================

TEST(BridgeFENTest, EmptyFENString) {
    ChessBoardHandle board = board_create();
    EXPECT_ANY_THROW(board_set_fen(board, ""););  // Empty string should fail
    board_destroy(board);
}

TEST(BridgeFENTest, VeryLongInvalidFEN) {
    ChessBoardHandle board = board_create();
    
    // Create a very long invalid string
    std::string long_fen(1000, 'x');
    
    EXPECT_ANY_THROW(board_set_fen(board, long_fen.c_str()));  // Should fail
    
    board_destroy(board);
}

/*
 * =============================================================================
 * STAGE 4: MOVE GENERATION TESTS
 * =============================================================================
 * 
 * Essential move generation tests:
 * - Basic move counts (starting position)
 * - Move structure validity
 * - Special moves (promotions, en passant, castling)
 * - NULL safety
 */

// =============================================================================
// BASIC MOVE GENERATION
// =============================================================================

TEST(BridgeMoveGenTest, StartingPositionMoveCount) {
    ChessEngineHandle engine = engine_create();
    ChessBoardHandle board = board_create();
    
    CMove moves[MAX_LEGAL_MOVES];
    int32_t count = engine_generate_legal_moves(engine, board, moves, MAX_LEGAL_MOVES);
    
    EXPECT_EQ(count, 20);  // Starting position has exactly 20 legal moves
    
    engine_destroy(engine);
    board_destroy(board);
}

TEST(BridgeMoveGenTest, MoveStructureValid) {
    ChessEngineHandle engine = engine_create();
    ChessBoardHandle board = board_create();
    
    CMove moves[MAX_LEGAL_MOVES];
    int32_t count = engine_generate_legal_moves(engine, board, moves, MAX_LEGAL_MOVES);
    
    ASSERT_EQ(count, 20);
    
    for (int i = 0; i < count; i++) {
        // Squares must be 0-63
        EXPECT_GE(moves[i].from_square, 0);
        EXPECT_LE(moves[i].from_square, 63);
        EXPECT_GE(moves[i].to_square, 0);
        EXPECT_LE(moves[i].to_square, 63);
        
        // From and to must be different
        EXPECT_NE(moves[i].from_square, moves[i].to_square);
        
        // Starting position: no special moves
        EXPECT_EQ(moves[i].captured_piece, PIECE_NONE);
        EXPECT_EQ(moves[i].promoted_piece, PIECE_NONE);
        EXPECT_EQ(moves[i].is_enpassant, 0);
        EXPECT_EQ(moves[i].is_castling, 0);
    }
    
    engine_destroy(engine);
    board_destroy(board);
}

TEST(BridgeMoveGenTest, CheckmateHasNoMoves) {
    ChessEngineHandle engine = engine_create();
    // Fool's mate - white is checkmated
    const char* fen = "rnb1kbnr/pppp1ppp/8/4p3/6Pq/5P2/PPPPP2P/RNBQKBNR w KQkq - 1 3";
    ChessBoardHandle board = board_create_from_fen(fen);
    ASSERT_NE(board, nullptr);
    
    CMove moves[MAX_LEGAL_MOVES];
    int32_t count = engine_generate_legal_moves(engine, board, moves, MAX_LEGAL_MOVES);
    
    EXPECT_EQ(count, 0);  // Checkmated side has no legal moves
    
    engine_destroy(engine);
    board_destroy(board);
}

// =============================================================================
// SPECIAL MOVES
// =============================================================================

TEST(BridgeMoveGenTest, PromotionMovesGenerated) {
    ChessEngineHandle engine = engine_create();
    // White pawn on 7th rank can promote
    const char* fen = "8/2P5/8/8/8/8/8/4K2k w - - 0 1";
    ChessBoardHandle board = board_create_from_fen(fen);
    ASSERT_NE(board, nullptr);
    
    CMove moves[MAX_LEGAL_MOVES];
    int32_t count = engine_generate_legal_moves(engine, board, moves, MAX_LEGAL_MOVES);
    
    // Count promotions
    int promotions = 0;
    for (int i = 0; i < count; i++) {
        if (moves[i].promoted_piece != PIECE_NONE) {
            promotions++;
        }
    }
    
    EXPECT_EQ(promotions, 4);  // Q, R, B, N
    
    engine_destroy(engine);
    board_destroy(board);
}

TEST(BridgeMoveGenTest, EnPassantMoveGenerated) {
    ChessEngineHandle engine = engine_create();
    // En passant available on f6
    const char* fen = "rnbqkbnr/ppp1p1pp/8/3pPp2/8/8/PPPP1PPP/RNBQKBNR w KQkq f6 0 3";
    ChessBoardHandle board = board_create_from_fen(fen);
    ASSERT_NE(board, nullptr);
    
    CMove moves[MAX_LEGAL_MOVES];
    int32_t count = engine_generate_legal_moves(engine, board, moves, MAX_LEGAL_MOVES);
    
    // Find en passant move
    bool found_ep = false;
    for (int i = 0; i < count; i++) {
        if (moves[i].is_enpassant != 0) {
            found_ep = true;
            EXPECT_EQ(moves[i].piece, PIECE_W_PAWN);
            EXPECT_EQ(moves[i].captured_piece, PIECE_B_PAWN);
        }
    }
    
    EXPECT_TRUE(found_ep);
    
    engine_destroy(engine);
    board_destroy(board);
}

TEST(BridgeMoveGenTest, CastlingMovesGenerated) {
    ChessEngineHandle engine = engine_create();
    // Both kings can castle both directions
    const char* fen = "r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1";
    ChessBoardHandle board = board_create_from_fen(fen);
    ASSERT_NE(board, nullptr);
    
    CMove moves[MAX_LEGAL_MOVES];
    int32_t count = engine_generate_legal_moves(engine, board, moves, MAX_LEGAL_MOVES);
    
    // Count castling moves
    int castling_count = 0;
    for (int i = 0; i < count; i++) {
        if (moves[i].is_castling != 0) {
            castling_count++;
            EXPECT_EQ(moves[i].piece, PIECE_W_KING);
        }
    }
    
    EXPECT_EQ(castling_count, 2);  // Kingside and queenside
    
    engine_destroy(engine);
    board_destroy(board);
}

// =============================================================================
// NULL SAFETY & EDGE CASES
// =============================================================================

TEST(BridgeMoveGenTest, NullHandles) {
    ChessEngineHandle engine = engine_create();
    ChessBoardHandle board = board_create();
    CMove moves[MAX_LEGAL_MOVES];
    
    // NULL engine
    EXPECT_ANY_THROW(engine_generate_legal_moves(nullptr, board, moves, MAX_LEGAL_MOVES));
    
    // NULL board
    EXPECT_ANY_THROW(engine_generate_legal_moves(engine, nullptr, moves, MAX_LEGAL_MOVES));
    
    // NULL moves array
    EXPECT_ANY_THROW(engine_generate_legal_moves(engine, board, nullptr, MAX_LEGAL_MOVES));
    
    engine_destroy(engine);
    board_destroy(board);
}

TEST(BridgeMoveGenTest, SmallBuffer) {
    ChessEngineHandle engine = engine_create();
    ChessBoardHandle board = board_create();
    
    // Buffer smaller than actual move count
    CMove moves[10];
    int32_t count = engine_generate_legal_moves(engine, board, moves, 10);
    
    // Should clamp to buffer size
    EXPECT_LE(count, 10);
    
    engine_destroy(engine);
    board_destroy(board);
}

TEST(BridgeMoveGenTest, BoardStateUnchangedAfterGeneration) {
    ChessEngineHandle engine = engine_create();
    ChessBoardHandle board = board_create();
    
    // Get FEN before
    char* fen_before = board_get_fen(board);
    ASSERT_NE(fen_before, nullptr);
    
    // Generate moves
    CMove moves[MAX_LEGAL_MOVES];
    engine_generate_legal_moves(engine, board, moves, MAX_LEGAL_MOVES);
    
    // Get FEN after
    char* fen_after = board_get_fen(board);
    ASSERT_NE(fen_after, nullptr);
    
    // Should be unchanged
    EXPECT_STREQ(fen_before, fen_after);
    
    chess_free_string(fen_before);
    chess_free_string(fen_after);
    engine_destroy(engine);
    board_destroy(board);
}

/*
 * =============================================================================
 * STAGE 5: MAKE/UNDO MOVE TESTS
 * =============================================================================
 * 
 * THE MOST CRITICAL TESTS!
 * - Make move changes board state
 * - Undo restores previous state
 * - Multiple move sequences work
 * - No memory leaks with make/undo cycles
 */

// =============================================================================
// BASIC MAKE/UNDO
// =============================================================================

TEST(BridgeMakeMoveTest, MakeMoveSwitchesSide) {
    ChessEngineHandle engine = engine_create();
    ChessBoardHandle board = board_create();
    
    EXPECT_EQ(board_get_side_to_move(board), COLOR_WHITE);
    
    // Generate and make first move
    CMove moves[MAX_LEGAL_MOVES];
    int32_t count = engine_generate_legal_moves(engine, board, moves, MAX_LEGAL_MOVES);
    ASSERT_GT(count, 0);
    
    board_make_move(board, &moves[0]);
    
    EXPECT_EQ(board_get_side_to_move(board), COLOR_BLACK);
    
    engine_destroy(engine);
    board_destroy(board);
}

TEST(BridgeMakeMoveTest, MakeMoveChangesBoardState) {
    ChessEngineHandle engine = engine_create();
    ChessBoardHandle board = board_create();
    
    char* fen_before = board_get_fen(board);
    ASSERT_NE(fen_before, nullptr);
    
    // Make a move
    CMove moves[MAX_LEGAL_MOVES];
    engine_generate_legal_moves(engine, board, moves, MAX_LEGAL_MOVES);
    board_make_move(board, &moves[0]);
    
    char* fen_after = board_get_fen(board);
    ASSERT_NE(fen_after, nullptr);
    
    // Should be different
    EXPECT_STRNE(fen_before, fen_after);
    
    chess_free_string(fen_before);
    chess_free_string(fen_after);
    engine_destroy(engine);
    board_destroy(board);
}

TEST(BridgeMakeMoveTest, UndoRestoresState) {
    ChessEngineHandle engine = engine_create();
    ChessBoardHandle board = board_create();
    
    char* original_fen = board_get_fen(board);
    ASSERT_NE(original_fen, nullptr);
    
    // Make a move
    CMove moves[MAX_LEGAL_MOVES];
    engine_generate_legal_moves(engine, board, moves, MAX_LEGAL_MOVES);
    board_make_move(board, &moves[0]);
    
    // Undo the move
    board_undo_move(board);
    
    // Should match original
    char* restored_fen = board_get_fen(board);
    ASSERT_NE(restored_fen, nullptr);
    EXPECT_STREQ(original_fen, restored_fen);
    
    chess_free_string(original_fen);
    chess_free_string(restored_fen);
    engine_destroy(engine);
    board_destroy(board);
}

TEST(BridgeMakeMoveTest, UndoWithNoMoves) {
    ChessBoardHandle board = board_create();
    
    char* fen_before = board_get_fen(board);
    ASSERT_NE(fen_before, nullptr);
    
    // Undo when no moves made - should be safe (no-op)
    board_undo_move(board);
    
    char* fen_after = board_get_fen(board);
    ASSERT_NE(fen_after, nullptr);
    
    EXPECT_STREQ(fen_before, fen_after);
    
    chess_free_string(fen_before);
    chess_free_string(fen_after);
    board_destroy(board);
}

// =============================================================================
// MOVE SEQUENCES
// =============================================================================

TEST(BridgeMakeMoveTest, MultipleMovesAndUndos) {
    ChessEngineHandle engine = engine_create();
    ChessBoardHandle board = board_create();
    
    char* original_fen = board_get_fen(board);
    
    CMove moves[MAX_LEGAL_MOVES];
    
    // Make 5 moves
    for (int i = 0; i < 5; i++) {
        int32_t count = engine_generate_legal_moves(engine, board, moves, MAX_LEGAL_MOVES);
        ASSERT_GT(count, 0) << "Should have moves at ply " << i;
        board_make_move(board, &moves[0]);
    }
    
    // Undo all 5 moves
    for (int i = 0; i < 5; i++) {
        board_undo_move(board);
    }
    
    // Should be back to original
    char* final_fen = board_get_fen(board);
    EXPECT_STREQ(original_fen, final_fen);
    
    chess_free_string(original_fen);
    chess_free_string(final_fen);
    engine_destroy(engine);
    board_destroy(board);
}

TEST(BridgeMakeMoveTest, MakeUndoMakeSequence) {
    ChessEngineHandle engine = engine_create();
    ChessBoardHandle board = board_create();
    
    CMove moves[MAX_LEGAL_MOVES];
    int32_t count = engine_generate_legal_moves(engine, board, moves, MAX_LEGAL_MOVES);
    ASSERT_GE(count, 2);
    
    char* start_fen = board_get_fen(board);
    
    // Make move 0
    board_make_move(board, &moves[0]);
    char* after_move0 = board_get_fen(board);
    
    // Undo
    board_undo_move(board);
    
    // Make move 1 (different move)
    board_make_move(board, &moves[1]);
    char* after_move1 = board_get_fen(board);
    
    // Should be different from each other
    EXPECT_STRNE(after_move0, after_move1);
    
    // Undo again
    board_undo_move(board);
    char* final_fen = board_get_fen(board);
    
    // Should be back to start
    EXPECT_STREQ(start_fen, final_fen);
    
    chess_free_string(start_fen);
    chess_free_string(after_move0);
    chess_free_string(after_move1);
    chess_free_string(final_fen);
    engine_destroy(engine);
    board_destroy(board);
}

// =============================================================================
// SPECIAL MOVE UNDO
// =============================================================================

TEST(BridgeMakeMoveTest, UndoCastling) {
    ChessEngineHandle engine = engine_create();
    const char* fen = "r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1";
    ChessBoardHandle board = board_create_from_fen(fen);
    ASSERT_NE(board, nullptr);
    
    char* original_fen = board_get_fen(board);
    
    // Find and make castling move
    CMove moves[MAX_LEGAL_MOVES];
    int32_t count = engine_generate_legal_moves(engine, board, moves, MAX_LEGAL_MOVES);
    
    int castle_idx = -1;
    for (int i = 0; i < count; i++) {
        if (moves[i].is_castling != 0) {
            castle_idx = i;
            break;
        }
    }
    ASSERT_NE(castle_idx, -1);
    
    board_make_move(board, &moves[castle_idx]);
    board_undo_move(board);
    
    char* restored_fen = board_get_fen(board);
    EXPECT_STREQ(original_fen, restored_fen);
    
    chess_free_string(original_fen);
    chess_free_string(restored_fen);
    engine_destroy(engine);
    board_destroy(board);
}

TEST(BridgeMakeMoveTest, UndoEnPassant) {
    ChessEngineHandle engine = engine_create();
    const char* fen = "rnbqkbnr/ppp1p1pp/8/3pPp2/8/8/PPPP1PPP/RNBQKBNR w KQkq f6 0 3";
    ChessBoardHandle board = board_create_from_fen(fen);
    ASSERT_NE(board, nullptr);
    
    char* original_fen = board_get_fen(board);
    
    // Find and make en passant move
    CMove moves[MAX_LEGAL_MOVES];
    int32_t count = engine_generate_legal_moves(engine, board, moves, MAX_LEGAL_MOVES);
    
    int ep_idx = -1;
    for (int i = 0; i < count; i++) {
        if (moves[i].is_enpassant != 0) {
            ep_idx = i;
            break;
        }
    }
    ASSERT_NE(ep_idx, -1);
    
    board_make_move(board, &moves[ep_idx]);
    board_undo_move(board);
    
    char* restored_fen = board_get_fen(board);
    EXPECT_STREQ(original_fen, restored_fen);
    
    chess_free_string(original_fen);
    chess_free_string(restored_fen);
    engine_destroy(engine);
    board_destroy(board);
}

TEST(BridgeMakeMoveTest, UndoPromotion) {
    ChessEngineHandle engine = engine_create();
    const char* fen = "8/2P5/8/8/8/8/8/4K2k w - - 0 1";
    ChessBoardHandle board = board_create_from_fen(fen);
    ASSERT_NE(board, nullptr);
    
    char* original_fen = board_get_fen(board);
    
    // Find and make promotion move
    CMove moves[MAX_LEGAL_MOVES];
    int32_t count = engine_generate_legal_moves(engine, board, moves, MAX_LEGAL_MOVES);
    
    int promo_idx = -1;
    for (int i = 0; i < count; i++) {
        if (moves[i].promoted_piece != PIECE_NONE) {
            promo_idx = i;
            break;
        }
    }
    ASSERT_NE(promo_idx, -1);
    
    board_make_move(board, &moves[promo_idx]);
    board_undo_move(board);
    
    char* restored_fen = board_get_fen(board);
    EXPECT_STREQ(original_fen, restored_fen);
    
    chess_free_string(original_fen);
    chess_free_string(restored_fen);
    engine_destroy(engine);
    board_destroy(board);
}

// =============================================================================
// STRESS TESTS (Memory Leak Detection)
// =============================================================================

TEST(BridgeMakeMoveTest, ManyMakeUndoCycles) {
    // ASan will catch leaks here
    ChessEngineHandle engine = engine_create();
    ChessBoardHandle board = board_create();
    
    CMove moves[MAX_LEGAL_MOVES];
    
    for (int cycle = 0; cycle < 100; cycle++) {
        int32_t count = engine_generate_legal_moves(engine, board, moves, MAX_LEGAL_MOVES);
        if (count > 0) {
            board_make_move(board, &moves[0]);
            board_undo_move(board);
        }
    }
    
    engine_destroy(engine);
    board_destroy(board);
}

TEST(BridgeMakeMoveTest, DeepMoveStack) {
    // Test making many moves without undo
    ChessEngineHandle engine = engine_create();
    ChessBoardHandle board = board_create();
    
    CMove moves[MAX_LEGAL_MOVES];
    
    // Make 50 moves (should not overflow move history)
    for (int i = 0; i < 50; i++) {
        int32_t count = engine_generate_legal_moves(engine, board, moves, MAX_LEGAL_MOVES);
        if (count == 0) break;  // Checkmate or stalemate
        board_make_move(board, &moves[0]);
    }
    
    // Undo all
    for (int i = 0; i < 50; i++) {
        board_undo_move(board);
    }
    
    // Should be back to starting position
    EXPECT_EQ(board_get_side_to_move(board), COLOR_WHITE);
    
    engine_destroy(engine);
    board_destroy(board);
}

// =============================================================================
// NULL SAFETY
// =============================================================================

TEST(BridgeMakeMoveTest, MakeMoveNull) {
    ChessBoardHandle board = board_create();
    
    // NULL board
    EXPECT_ANY_THROW(board_make_move(nullptr, nullptr));
    
    // NULL move
    EXPECT_ANY_THROW(board_make_move(board, nullptr));
    
    board_destroy(board);
}

TEST(BridgeMakeMoveTest, UndoMoveNull) {
    EXPECT_ANY_THROW(board_undo_move(nullptr));
}

/*
 * =============================================================================
 * STAGE 6: UTILITIES & PERFT (FINAL STAGE!)
 * =============================================================================
 * 
 * Test utility functions and perft validation:
 * - Move to string conversion
 * - Perft correctness (known values)
 * - Final integration tests
 */

// =============================================================================
// MOVE TO STRING
// =============================================================================

TEST(BridgeUtilsTest, MoveToStringNormalMove) {
    ChessEngineHandle engine = engine_create();
    ChessBoardHandle board = board_create();
    
    CMove moves[MAX_LEGAL_MOVES];
    int32_t count = engine_generate_legal_moves(engine, board, moves, MAX_LEGAL_MOVES);
    ASSERT_GT(count, 0);
    
    // Convert first move to string
    char* move_str = chess_move_to_string(&moves[0]);
    ASSERT_NE(move_str, nullptr);
    
    // Should be 4 or 5 characters (e.g., "e2e4" or "e7e8q")
    size_t len = strlen(move_str);
    std::cout << "STRING: " << move_str << std::endl;
    EXPECT_GE(len, 4);
    EXPECT_LE(len, 5);
    
    chess_free_string(move_str);
    engine_destroy(engine);
    board_destroy(board);
}

TEST(BridgeUtilsTest, MoveToStringPromotion) {
    ChessEngineHandle engine = engine_create();
    const char* fen = "8/2P5/8/8/8/8/8/4K2k w - - 0 1";
    ChessBoardHandle board = board_create_from_fen(fen);
    ASSERT_NE(board, nullptr);
    
    CMove moves[MAX_LEGAL_MOVES];
    int32_t count = engine_generate_legal_moves(engine, board, moves, MAX_LEGAL_MOVES);
    
    // Find a queen promotion
    int queen_promo_idx = -1;
    for (int i = 0; i < count; i++) {
        if (moves[i].promoted_piece == PIECE_W_QUEEN) {
            queen_promo_idx = i;
            break;
        }
    }
    ASSERT_NE(queen_promo_idx, -1);
    
    char* move_str = chess_move_to_string(&moves[queen_promo_idx]);
    ASSERT_NE(move_str, nullptr);
    
    // Should end with 'q' (e.g., "c7c8q")
    size_t len = strlen(move_str);
    EXPECT_EQ(len, 5);
    EXPECT_EQ(move_str[4], 'q');
    
    chess_free_string(move_str);
    engine_destroy(engine);
    board_destroy(board);
}

TEST(BridgeUtilsTest, MoveToStringNull) {
    char* str = chess_move_to_string(nullptr);
    EXPECT_EQ(str, nullptr);
}

TEST(BridgeUtilsTest, MoveToStringMultiple) {
    // Test that multiple conversions work and are freed correctly
    ChessEngineHandle engine = engine_create();
    ChessBoardHandle board = board_create();
    
    CMove moves[MAX_LEGAL_MOVES];
    int32_t count = engine_generate_legal_moves(engine, board, moves, MAX_LEGAL_MOVES);
    ASSERT_GE(count, 3);
    
    char* str1 = chess_move_to_string(&moves[0]);
    char* str2 = chess_move_to_string(&moves[1]);
    char* str3 = chess_move_to_string(&moves[2]);
    
    ASSERT_NE(str1, nullptr);
    ASSERT_NE(str2, nullptr);
    ASSERT_NE(str3, nullptr);
    
    // All should be different strings (different memory)
    EXPECT_NE(str1, str2);
    EXPECT_NE(str2, str3);
    
    chess_free_string(str1);
    chess_free_string(str2);
    chess_free_string(str3);
    engine_destroy(engine);
    board_destroy(board);
}

// =============================================================================
// PERFT TESTS (Known Correct Values)
// =============================================================================

TEST(BridgePerftTest, PerftStartingPositionDepth1) {
    ChessBoardHandle board = board_create();
    ChessEngineHandle engine = engine_create();
    
    uint64_t nodes = chess_perft(engine, board, 1);
    EXPECT_EQ(nodes, 20);
    
    board_destroy(board);
    engine_destroy(engine);
}

TEST(BridgePerftTest, PerftStartingPositionDepth2) {
    ChessBoardHandle board = board_create();
    ChessEngineHandle engine = engine_create();
    
    uint64_t nodes = chess_perft(engine, board, 2);
    EXPECT_EQ(nodes, 400);
    
    board_destroy(board);
    engine_destroy(engine);
    
}

TEST(BridgePerftTest, PerftStartingPositionDepth3) {
    ChessBoardHandle board = board_create();
    ChessEngineHandle engine = engine_create();
    
    uint64_t nodes = chess_perft(engine, board, 3);
    EXPECT_EQ(nodes, 8902);
    
    board_destroy(board);
    engine_destroy(engine);
}

TEST(BridgePerftTest, PerftStartingPositionDepth4) {
    // This one is slower but important validation
    ChessBoardHandle board = board_create();
    ChessEngineHandle engine = engine_create();
    
    uint64_t nodes = chess_perft(engine, board, 4);
    EXPECT_EQ(nodes, 197281);
    
    board_destroy(board);
    engine_destroy(engine);
}

TEST(BridgePerftTest, PerftKiwiPete) {
    // Famous perft position
    const char* fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    ChessBoardHandle board = board_create_from_fen(fen);
    ChessEngineHandle engine = engine_create();
    ASSERT_NE(board, nullptr);
    
    uint64_t nodes = chess_perft(engine, board, 1);
    EXPECT_EQ(nodes, 48);
    
    nodes = chess_perft(engine, board, 2);
    EXPECT_EQ(nodes, 2039);
    
    board_destroy(board);
    engine_destroy(engine);
}

TEST(BridgePerftTest, PerftPosition3) {
    // Another standard perft position
    const char* fen = "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1";
    ChessBoardHandle board = board_create_from_fen(fen);
    ChessEngineHandle engine = engine_create();
    ASSERT_NE(board, nullptr);
    
    uint64_t nodes = chess_perft(engine, board, 1);
    EXPECT_EQ(nodes, 14);
    
    nodes = chess_perft(engine, board, 2);
    EXPECT_EQ(nodes, 191);
    
    board_destroy(board);
    engine_destroy(engine);
}

TEST(BridgePerftTest, PerftDepthZero) {
    ChessBoardHandle board = board_create();
    ChessEngineHandle engine = engine_create();
    
    uint64_t nodes = chess_perft(engine, board, 0);
    EXPECT_EQ(nodes, 1);  // Depth 0 = count current position only
    
    board_destroy(board);
}

TEST(BridgePerftTest, PerftNullBoard) {
    ChessEngineHandle engine = engine_create();
    EXPECT_ANY_THROW(chess_perft(engine, nullptr, 3));
    engine_destroy(engine);
}

TEST(BridgePerftTest, PerftBoardStateUnchanged) {
    ChessBoardHandle board = board_create();
    ChessEngineHandle engine = engine_create();
    
    char* fen_before = board_get_fen(board);
    ASSERT_NE(fen_before, nullptr);
    
    // Run perft
    chess_perft(engine, board, 4);
    
    // Board should be unchanged
    char* fen_after = board_get_fen(board);
    ASSERT_NE(fen_after, nullptr);
    EXPECT_STREQ(fen_before, fen_after);
    
    chess_free_string(fen_before);
    chess_free_string(fen_after);
    board_destroy(board);
    engine_destroy(engine);
}

// =============================================================================
// INTEGRATION TESTS (Everything Together)
// =============================================================================

TEST(BridgeIntegrationTest, CompleteGameFlow) {
    // Simulate a short game flow
    ChessEngineHandle engine = engine_create();
    ChessBoardHandle board = board_create();
    
    CMove moves[MAX_LEGAL_MOVES];
    
    // Play 10 moves
    for (int i = 0; i < 10; i++) {
        // Check side to move
        uint8_t side = board_get_side_to_move(board);
        EXPECT_TRUE(side == COLOR_WHITE || side == COLOR_BLACK);
        
        // Generate moves
        int32_t count = engine_generate_legal_moves(engine, board, moves, MAX_LEGAL_MOVES);
        if (count == 0) break;  // Game over
        
        // Convert move to string
        char* move_str = chess_move_to_string(&moves[0]);
        ASSERT_NE(move_str, nullptr);
        chess_free_string(move_str);
        
        // Make move
        board_make_move(board, &moves[0]);
        
        // Get FEN
        char* fen = board_get_fen(board);
        ASSERT_NE(fen, nullptr);
        chess_free_string(fen);
    }
    
    // Undo all moves
    for (int i = 0; i < 10; i++) {
        board_undo_move(board);
    }
    
    // Should be back at start
    EXPECT_EQ(board_get_side_to_move(board), COLOR_WHITE);
    
    engine_destroy(engine);
    board_destroy(board);
}

TEST(BridgeIntegrationTest, LoadSavePosition) {
    ChessEngineHandle engine = engine_create();
    const char* original_fen = "r1bqkb1r/pppp1ppp/2n2n2/4p3/2B1P3/5N2/PPPP1PPP/RNBQK2R w KQkq - 4 4";
    
    // Load position
    ChessBoardHandle board = board_create_from_fen(original_fen);
    ASSERT_NE(board, nullptr);
    
    // Make some moves
    CMove moves[MAX_LEGAL_MOVES];
    for (int i = 0; i < 3; i++) {
        int32_t count = engine_generate_legal_moves(engine, board, moves, MAX_LEGAL_MOVES);
        ASSERT_GT(count, 0);
        board_make_move(board, &moves[0]);
    }
    
    // Get current FEN
    char* current_fen = board_get_fen(board);
    ASSERT_NE(current_fen, nullptr);
    
    // Create new board from that FEN
    ChessBoardHandle board2 = board_create_from_fen(current_fen);
    ASSERT_NE(board2, nullptr);
    
    // Should have same state
    char* board2_fen = board_get_fen(board2);
    ASSERT_NE(board2_fen, nullptr);
    EXPECT_STREQ(current_fen, board2_fen);
    
    chess_free_string(current_fen);
    chess_free_string(board2_fen);
    engine_destroy(engine);
    board_destroy(board);
    board_destroy(board2);
}

TEST(BridgeIntegrationTest, MultipleEnginesAndBoards) {
    // Test that multiple engines and boards can coexist
    ChessEngineHandle engine1 = engine_create();
    ChessEngineHandle engine2 = engine_create();
    ChessBoardHandle board1 = board_create();
    ChessBoardHandle board2 = board_create_from_fen("8/8/8/4k3/8/8/8/4K3 w - - 0 1");
    
    ASSERT_NE(board2, nullptr);
    
    CMove moves[MAX_LEGAL_MOVES];
    
    // Use different combinations
    int32_t count1 = engine_generate_legal_moves(engine1, board1, moves, MAX_LEGAL_MOVES);
    int32_t count2 = engine_generate_legal_moves(engine2, board2, moves, MAX_LEGAL_MOVES);
    
    EXPECT_EQ(count1, 20);  // Starting position
    EXPECT_GT(count2, 0);   // King endgame
    
    // Cross-use should also work
    count1 = engine_generate_legal_moves(engine2, board1, moves, MAX_LEGAL_MOVES);
    EXPECT_EQ(count1, 20);
    
    engine_destroy(engine1);
    engine_destroy(engine2);
    board_destroy(board1);
    board_destroy(board2);
}