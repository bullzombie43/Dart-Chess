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

    std::cout << "Updated" << std::endl;
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
