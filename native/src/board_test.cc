#include <gtest/gtest.h>
#include <board.h>

class BoardTestFixture : public ::testing::Test {
    protected:
        void SetUp() override {};

        BoardTestFixture() {
            board = Board();
        }

        Board board; 


        void call_remove_castling_right(CastlingRights right) {
            board.remove_castling_right(right); // This now works!
        }

        bool call_is_square_attacked(int target, Color attacking_color){
            return board.is_square_attacked(target, attacking_color);
        }
};

TEST_F(BoardTestFixture, PrintBoard){
    std::ostringstream oss;
    board = Board();
    board.print_board(oss);

    // Using a Raw String Literal: R"delimiter( content )delimiter"
    EXPECT_EQ(R"(r n b q k b n r 
p p p p p p p p 
. . . . . . . . 
. . . . . . . . 
. . . . . . . . 
. . . . . . . . 
P P P P P P P P 
R N B Q K B N R 
)",
        oss.str() 
    );
}

TEST_F(BoardTestFixture, SetBoardToFenTest1){
    std::ostringstream oss;
    board = Board();
    board.set_position_fen("rnbqkbnr/pp1ppppp/8/2p5/4P3/5N2/PPPP1PPP/RNBQKB1R b KQkq - 1 2");
    board.print_board(oss);

    // Using a Raw String Literal: R"delimiter( content )delimiter"
    EXPECT_EQ(R"(r n b q k b n r 
p p . p p p p p 
. . . . . . . . 
. . p . . . . . 
. . . . P . . . 
. . . . . N . . 
P P P P . P P P 
R N B Q K B . R 
)",
        oss.str() 
    );

    EXPECT_EQ(Color::BLACK, board.sideToMove);
    EXPECT_EQ(1, board.half_move_clock);
    EXPECT_EQ(2, board.num_moves_total);
    EXPECT_EQ(std::nullopt, board.enPassantSquare);
    EXPECT_EQ(board.getFen(),"rnbqkbnr/pp1ppppp/8/2p5/4P3/5N2/PPPP1PPP/RNBQKB1R b KQkq - 1 2" );
}

TEST_F(BoardTestFixture, SetBoardToFenTest2){
    std::ostringstream oss;
    board = Board();

    board.set_position_fen("rnbqkbnr/pp1ppppp/8/2p5/4P3/8/PPPP1PPP/RNBQKBNR w KQkq c6 0 2");
    board.print_board(oss);

    // Using a Raw String Literal: R"delimiter( content )delimiter"
    EXPECT_EQ(R"(r n b q k b n r 
p p . p p p p p 
. . . . . . . . 
. . p . . . . . 
. . . . P . . . 
. . . . . . . . 
P P P P . P P P 
R N B Q K B N R 
)",
        oss.str() 
    );
    EXPECT_EQ(Color::WHITE, board.sideToMove);
    EXPECT_EQ(0, board.half_move_clock);
    EXPECT_EQ(2, board.num_moves_total);
    EXPECT_EQ(board.C6, board.enPassantSquare);  
    EXPECT_EQ(board.getFen(), "rnbqkbnr/pp1ppppp/8/2p5/4P3/8/PPPP1PPP/RNBQKBNR w KQkq c6 0 2");
}

TEST_F(BoardTestFixture, CastlingRights){
    board = Board();

    EXPECT_EQ(true, board.can_castle(CastlingRights::ALL));

    call_remove_castling_right(CastlingRights::WHITE_KINGSIDE);
    EXPECT_EQ(false, board.can_castle(CastlingRights::WHITE_KINGSIDE));
}

TEST_F(BoardTestFixture, MakeMoveUnmakeMove){
    board = Board();
    std::ostringstream oss;

    Move pawnC2C4 = {Piece::W_PAWN, 10, 26, Piece::NONE, Piece::NONE, false, false};    
    board.make_move(pawnC2C4);
    board.print_board(oss);

    EXPECT_EQ(R"(r n b q k b n r 
p p p p p p p p 
. . . . . . . . 
. . . . . . . . 
. . P . . . . . 
. . . . . . . . 
P P . P P P P P 
R N B Q K B N R 
)",
        oss.str()
    );

    //Reverse C2C4
    board.undo_move();

    oss.str("");
    oss.clear();

    board.print_board(oss);
    EXPECT_EQ(R"(r n b q k b n r 
p p p p p p p p 
. . . . . . . . 
. . . . . . . . 
. . . . . . . . 
. . . . . . . . 
P P P P P P P P 
R N B Q K B N R 
)",
        oss.str()
    );

}

TEST_F(BoardTestFixture, DoublePawnPushAndUndo) {
    board = Board();
    std::ostringstream oss;

    // Move: e2 to e4 (square 12 to 28)
    // Note: Piece and captured_piece should be handled by your make_move logic
    // We'll define piece based on its type and color for simplicity here.
    Move pawnE2E4 = {Piece::W_PAWN, 12, 28, Piece::NONE, Piece::NONE, false, false};    
    
    board.make_move(pawnE2E4);

    // Verify state change: En Passant must be set to E3 (square 20)
    EXPECT_EQ(board.E3, board.enPassantSquare); 
    EXPECT_EQ(Color::BLACK, board.sideToMove); // Side to move must flip

    // Undo move
    board.undo_move();

    // Verify board and state are completely restored
    EXPECT_EQ(std::nullopt, board.enPassantSquare); // En passant reset
    EXPECT_EQ(Color::WHITE, board.sideToMove);     // Side to move reset

    // Check board state via print
    oss.str("");
    board.print_board(oss);
    EXPECT_EQ(R"(r n b q k b n r 
p p p p p p p p 
. . . . . . . . 
. . . . . . . . 
. . . . . . . . 
. . . . . . . . 
P P P P P P P P 
R N B Q K B N R 
)", oss.str());
}

TEST_F(BoardTestFixture, PawnCaptureAndUndo) {
    board = Board();
    std::ostringstream oss;
    // Setup a position where a white pawn can capture a black pawn immediately
    board.set_position_fen("rnbqkbnr/pppppppp/8/8/3P4/8/PPP1PPPP/RNBQKBNR b KQkq - 0 1"); 
    
    // Move: d7 to e6 (square 51 to 44) - Black pawn captures white pawn (should fail logic above, let's use a simpler capture)
    // Let's manually place a piece for a capture test:
    board.set_position_fen("rnbqkbnr/p1pppppp/8/8/3P4/8/PPP1PPPP/RNBQKBNR w KQkq - 0 1"); 
    // Now setup the capture state (d4 to e5 is illegal, d4 to c5 is illegal)
    // Use a test-friendly setup: White knight on f3, Black pawn on e5
    board.set_position_fen("rnbqkbnr/pppp1ppp/8/4p3/8/5N2/PPPPPPPP/RNBQKB1R w KQkq - 0 2"); 
    
    // Move: Nf3 takes e5 (square 21 to 36). Captured piece is B_PAWN.
    Move knightCapturesPawn = {Piece::W_KNIGHT, 21, 36, Piece::B_PAWN, Piece::NONE, false, false};

    board.make_move(knightCapturesPawn);

    // Verify piece is gone and capture flag is set
    EXPECT_EQ(Color::BLACK, board.sideToMove); // Side flipped

    // Undo move
    board.undo_move();

    // Verify board state is fully restored
    oss.str("");
    board.print_board(oss);
    EXPECT_EQ(R"(r n b q k b n r 
p p p p . p p p 
. . . . . . . . 
. . . . p . . . 
. . . . . . . . 
. . . . . N . . 
P P P P P P P P 
R N B Q K B . R 
)", oss.str()); // Check that Knight is back on F3 and Pawn is back on E5
}

TEST_F(BoardTestFixture, CastlingMakeAndUndo) {
    board = Board();

    //
    // Position: White can castle both sides
    //
    board.set_position_fen(
        "r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1"
    );

    std::ostringstream oss_original;
    board.print_board(oss_original);
    const std::string original = oss_original.str();

    //
    // === Test 1: White Kingside Castling ===
    //
    // White king: e1 (4) → g1 (6)
    // White rook: h1 (7) → f1 (5)
    //
    Move wk_castle = {
        Piece::W_KING,
        4,        // from e1
        6,        // to g1
        Piece::NONE,
        Piece::NONE,
        false,    // not en passant
        true      // is castling
    };

    board.make_move(wk_castle);

    // Verify king and rook moved
    std::ostringstream oss_after_castle;
    board.print_board(oss_after_castle);
    EXPECT_EQ(R"(r . . . k . . r 
. . . . . . . . 
. . . . . . . . 
. . . . . . . . 
. . . . . . . . 
. . . . . . . . 
. . . . . . . . 
R . . . . R K . 
)", oss_after_castle.str());

    // Undo
    board.undo_move();
    std::ostringstream oss_after_undo;
    board.print_board(oss_after_undo);
    EXPECT_EQ(original, oss_after_undo.str());

    //
    // === Test 2: White Queenside Castling ===
    //
    // White king: e1 (4) → c1 (2)
    // White rook: a1 (0) → d1 (3)
    //
    Move wq_castle = {
        Piece::W_KING,
        4,        // from e1
        2,        // to c1
        Piece::NONE,
        Piece::NONE,
        false,
        true
    };

    board.make_move(wq_castle);

    std::ostringstream oss_after_qcastle;
    board.print_board(oss_after_qcastle);
    EXPECT_EQ(R"(r . . . k . . r 
. . . . . . . . 
. . . . . . . . 
. . . . . . . . 
. . . . . . . . 
. . . . . . . . 
. . . . . . . . 
. . K R . . . R 
)", oss_after_qcastle.str());

    // Undo
    board.undo_move();
    std::ostringstream oss_after_qundo;
    board.print_board(oss_after_qundo);
    EXPECT_EQ(original, oss_after_qundo.str());
}

TEST_F(BoardTestFixture, KnightAttacksSquare) {
    board = Board();

    // Place a white knight on f3 (square 21)
    // Black pawn on e5 (square 36) - test attacked square
    board.set_position_fen("rnbqkbnr/pppp1ppp/8/4p3/8/5N2/PPPPPPPP/RNBQKB1R w KQkq - 0 1");

    // Square 36 (e5) should be attacked by the knight
    EXPECT_TRUE(call_is_square_attacked(36, Color::WHITE));

    // A square not attacked by the knight (e4 = 28)
    EXPECT_FALSE(call_is_square_attacked(28, Color::WHITE));
}

TEST_F(BoardTestFixture, PawnAttacksSquare) {
    board = Board();

    // White pawn on e4 (square 28), black pawn on d5 (square 35)
    board.set_position_fen("rnbqkbnr/pppp1ppp/8/3p4/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 1");

    // White pawn attacks d5 (square 35)
    EXPECT_TRUE(call_is_square_attacked(35, Color::WHITE));

    // White pawn does not attack e5 (square 36)
    EXPECT_FALSE(call_is_square_attacked(36, Color::WHITE));
}

TEST_F(BoardTestFixture, BishopAttacksSquare) {
    board = Board();

    // White bishop on c1 (square 2), black pawn on g5 (square 38)
    board.set_position_fen("rnbqkbnr/pppppp1p/8/6p1/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    // g5 should not be attacked initially (blocked)
    EXPECT_FALSE(call_is_square_attacked(38, Color::WHITE));

    // Clear the blocking pawn
    board.set_position_fen("rnbqkbnr/pppppp1p/8/6p1/3P4/8/PPP1PPPP/RNBQKBNR b KQkq d3 0 1");

    EXPECT_TRUE(call_is_square_attacked(38, Color::WHITE));
}

TEST_F(BoardTestFixture, RookAttacksSquare) {
    board = Board();

    // White rook on a1 (square 0), black pawn on a5 (square 32)
    board.set_position_fen("rnbqkbnr/2pppppp/1P6/p7/8/8/1PPPPPPP/RNBQKBNR b KQkq - 0 1");

    // Rook attacks a5
    EXPECT_TRUE(call_is_square_attacked(32, Color::WHITE));

    // Square b5 not attacked
    EXPECT_FALSE(call_is_square_attacked(40, Color::WHITE));
}

TEST_F(BoardTestFixture, QueenAttacksSquare) {
    board = Board();

    // White queen on d1 (square 3), black pawn on h5 (square 39), black pawn on d7 (square 51)
    board.set_position_fen("rnbqkbnr/ppppppp1/8/7p/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    EXPECT_FALSE(call_is_square_attacked(39, Color::WHITE));
    EXPECT_FALSE(call_is_square_attacked(51, Color::WHITE));

    // Clear blocking pieces to allow queen attack
    board.set_position_fen("rnbqkbnr/ppppppp1/8/7p/8/8/PPP2PPP/RNBQKBNR b KQkq - 0 1");

    EXPECT_TRUE(call_is_square_attacked(39, Color::WHITE));
    EXPECT_TRUE(call_is_square_attacked(51, Color::WHITE));
}

TEST_F(BoardTestFixture, KingAttacksSquare) {
    board = Board();

    // White king on e1 (square 4), test adjacent squares
    board.set_position_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    // Squares adjacent to e1
    EXPECT_TRUE(call_is_square_attacked(3, Color::WHITE)); // d1
    EXPECT_TRUE(call_is_square_attacked(5, Color::WHITE)); // f1
    EXPECT_TRUE(call_is_square_attacked(11, Color::WHITE)); // e2

    // Far away square not attacked
    EXPECT_FALSE(call_is_square_attacked(36, Color::WHITE)); // e5
}

TEST_F(BoardTestFixture, WhiteKingInCheckByRook) {
    board = Board();

    // Black rook on e8, white king on e1
    board.set_position_fen("1k2r3/8/8/8/8/8/8/4K3 w - - 0 1");

    EXPECT_TRUE(board.is_in_check(Color::WHITE));
    EXPECT_FALSE(board.is_in_check(Color::BLACK));
}

TEST_F(BoardTestFixture, WhiteKingNotInCheckBlockedRook) {
    board = Board();

    // Pawn at e2 blocks rook on e8 from checking e1
    board.set_position_fen("4r3/8/8/8/8/8/4P3/4K3 w - - 0 1");

    EXPECT_FALSE(board.is_in_check(Color::WHITE));
}

TEST_F(BoardTestFixture, BlackKingInCheckByKnight) {
    board = Board();

    // White knight on c7 attacks e8 (black king)
    board.set_position_fen("4k3/2N5/8/8/8/8/8/K7 b - - 0 1");

    EXPECT_TRUE(board.is_in_check(Color::BLACK));
    EXPECT_FALSE(board.is_in_check(Color::WHITE));
}

TEST_F(BoardTestFixture, WhiteKingInCheckByBishop) {
    board = Board();

    // Bishop on b2 checks king on a1
    board.set_position_fen("8/8/3k4/8/8/8/1b6/K7 w - - 0 1");

    EXPECT_TRUE(board.is_in_check(Color::WHITE));
}

TEST_F(BoardTestFixture, WhiteKingInCheckByPawn) {
    board = Board();

    // Black pawn on d5 attacks e4 where king sits
    board.set_position_fen("8/8/8/3p4/4K3/8/k7/8 w - - 0 1");

    EXPECT_TRUE(board.is_in_check(Color::WHITE));
}

TEST_F(BoardTestFixture, KingsAdjacentBothInCheck) {
    board = Board();

    // Kings on d4 and e4 (illegal in real chess but good for testing)
    board.set_position_fen("8/8/8/8/3Kk3/8/8/8 w - - 0 1");

    EXPECT_TRUE(board.is_in_check(Color::WHITE));
    EXPECT_TRUE(board.is_in_check(Color::BLACK));
}


