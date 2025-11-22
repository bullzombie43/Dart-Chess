#include <gtest/gtest.h>
#include <engine.h>
#include <board.h>

class EngineTestFixture : public ::testing::Test {
    protected:
        void SetUp() override {};

        EngineTestFixture() {
            board = Board();
            engine = Engine();
        }

        Board board;
        Engine engine; 
};

TEST_F(EngineTestFixture, PerftOne){
    board.set_position_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    std::cout << "starting 1" << std::endl;
    int result = engine.perft(board, 1);
    EXPECT_EQ(result, 20);
    std::cout << "Depth: 1, Moves: " << result << std::endl;
 };

TEST_F(EngineTestFixture, PerfTwo){
    board.set_position_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    int result = engine.perft(board, 2);
    EXPECT_EQ(result, 400);
    std::cout << "Depth: 2, Moves: " << result << std::endl;
};

TEST_F(EngineTestFixture, PerfThree){
    board.set_position_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    int result = engine.perft(board, 3);
    EXPECT_EQ(result, 8902);
    std::cout << "Depth: 3, Moves: " << result << std::endl;
    
};

TEST_F(EngineTestFixture, PerfFour){
    board.set_position_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    std::cout << board.getFen() << std::endl;
    int result = engine.perft(board, 4);
    EXPECT_EQ(result, 197281);
    std::cout << "Depth: 4, Moves: " << result << std::endl;
    std::cout << board.getFen() << std::endl;


};

TEST_F(EngineTestFixture, PerfFive){
    //board.set_position_fen("rnbqkbnr/1ppppppp/8/p7/3P4/8/PPP1PPPP/RNBQKBNR w KQkq a6 0 1");
    board.set_position_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    std::cout << board.getFen() << std::endl;
    //int result = engine.perft_divide(board, 3);
    int result = engine.perft(board, 5);
    EXPECT_EQ(result, 4865609);
    std::cout << "Depth: 5, Moves: " << result << std::endl;
    std::cout << board.getFen() << std::endl;
};

TEST_F(EngineTestFixture, PerfSix){
    board.set_position_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    int result = engine.perft(board, 6);
    EXPECT_EQ(result, 119060324);
    std::cout << "Depth: 6, Moves: " << result << std::endl;
}

TEST_F(EngineTestFixture, PerftPosition2){
    board.set_position_fen("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
    board.print_board(std::cout);
    int result = engine.perft(board, 5);
    EXPECT_EQ(result, 193690690);
}

TEST_F(EngineTestFixture, PerftPosition3){
    board.set_position_fen("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1");
    int result = engine.perft(board, 6);
    EXPECT_EQ(result, 11030083);
}

TEST_F(EngineTestFixture, PerftPosition4){
    board.set_position_fen("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1");
    int result = engine.perft(board, 5);
    EXPECT_EQ(result, 15833292);
}

TEST_F(EngineTestFixture, PerftPosition5){
    board.set_position_fen("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8");
    int result = engine.perft(board, 5);
    EXPECT_EQ(result, 89941194);
}
TEST_F(EngineTestFixture, PerftPosition6){
    board.set_position_fen("r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10 ");
    int result = engine.perft(board, 5);
    EXPECT_EQ(result, 164075551);
}





