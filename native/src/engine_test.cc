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
    int result = engine.perft(board, 4);
    EXPECT_EQ(result, 197281);
    std::cout << "Depth: 4, Moves: " << result << std::endl;

};

TEST_F(EngineTestFixture, PerfFive){
    board.set_position_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    int result = engine.perft(board, 5);
    EXPECT_EQ(result, 4865609);
    std::cout << "Depth: 5, Moves: " << result << std::endl;
}

TEST_F(EngineTestFixture, PerfSix){
    board.set_position_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    int result = engine.perft(board, 6);
    EXPECT_EQ(result, 119060324);
    std::cout << "Depth: 6, Moves: " << result << std::endl;
}

