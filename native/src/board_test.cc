#include <gtest/gtest.h>
#include <board.h>

TEST(BoardTest, PrintBoard){
    std::ostringstream oss;
    Board board = Board();
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

TEST(BoardTest, SetBoardToFenTest1){
    std::ostringstream oss;
    Board board = Board();

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

TEST(BoardTest, SetBoardToFenTest2){
    std::ostringstream oss;
    Board board = Board();

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

