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

#include <gtest/gtest.h>
#include "engine.h"
#include "board.h"

/*
 * =============================================================================
 * POSITION EVALUATION TESTS
 * =============================================================================
 */

TEST(EngineEvaluationTest, StartingPositionIsEqual) {
    Board board;
    Engine engine;
    
    int score = engine.evaluate_position(board);
    
    // Starting position: equal material, white to move
    // Score should be 0 (or very close to 0 if PST has positional factors)
    EXPECT_EQ(score, 0) << "Starting position should be roughly equal";
}

TEST(EngineEvaluationTest, WhiteUpMaterial) {
    Board board;
    Engine engine;
    
    // White has extra queen (massive advantage)
    board.set_position_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    // Actually, let's use a position where white has captured black's queen
    board.set_position_fen("rnb1kbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    
    int score = engine.evaluate_position(board);
    
    // White is up a queen (~900 centipawns), should be very positive
    EXPECT_GT(score, 800) << "White up a queen should have large positive score";
}

TEST(EngineEvaluationTest, BlackUpMaterial) {
    Board board;
    Engine engine;
    
    // Black has extra queen, white to move
    board.set_position_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNB1KBNR w KQkq - 0 1");
    
    int score = engine.evaluate_position(board);
    
    // Black is up a queen, white to move, should be very negative
    EXPECT_LT(score, -800) << "Black up a queen (white to move) should be very negative";
}

TEST(EngineEvaluationTest, NegamaxFlipsScore) {
    Board board;
    Engine engine;
    
    // Position with white up a pawn, white to move
    board.set_position_fen("rnbqkbnr/ppppppp1/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    int score_white_move = engine.evaluate_position(board);
    
    // Same position, black to move
    board.set_position_fen("rnbqkbnr/ppppppp1/8/8/8/8/PPPPPPPP/RNBQKBNR b KQkq - 0 1");
    int score_black_move = engine.evaluate_position(board);
    
    // Negamax property: scores should be negatives of each other
    EXPECT_EQ(score_white_move, -score_black_move) 
        << "Negamax should flip score based on side to move";
    
    // White is up material, so white to move should be positive
    EXPECT_GT(score_white_move, 0);
    // Black to move should be negative (from black's perspective, losing)
    EXPECT_LT(score_black_move, 0);
}

TEST(EngineEvaluationTest, SymmetricPosition) {
    Board board;
    Engine engine;
    
    // After 1.e4 e5 - symmetric pawn structure
    board.set_position_fen("rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq e6 0 2");
    
    int score = engine.evaluate_position(board);
    
    // Should be very close to 0 (symmetric position)
    EXPECT_NEAR(score, 0, 50) << "Symmetric position should be near 0";
}

TEST(EngineEvaluationTest, OnlyKingsLeft) {
    Board board;
    Engine engine;
    
    // Just two kings - draw
    board.set_position_fen("8/4k3/8/8/8/8/4K3/8 w - - 0 1");
    
    int score = engine.evaluate_position(board);
    
    // No material, should be exactly 0
    EXPECT_EQ(score, 0) << "King vs King should be 0";
}

TEST(EngineEvaluationTest, MassiveMaterialImbalance) {
    Board board;
    Engine engine;
    
    // White has everything, black has just king
    board.set_position_fen("4k3/8/8/8/8/8/PPPPPPPP/RNBQKBNR w - - 0 1");
    
    int score = engine.evaluate_position(board);
    
    // White has massive material advantage
    EXPECT_GT(score, 3000) << "White with all pieces vs lone king should be huge";
}

TEST(EngineEvaluationTest, CheckmatePositionEvaluation) {
    Board board;
    Engine engine;
    
    // Checkmate position (fool's mate)
    board.set_position_fen("rnb1kbnr/pppp1ppp/8/4p3/6Pq/5P2/PPPPP2P/RNBQKBNR w KQkq - 1 3");
    
    int score = engine.evaluate_position(board);
    
    // White is checkmated, so should be very negative
    // (Note: evaluation doesn't detect checkmate, just material)
    EXPECT_LT(score, 0) << "Checkmated side should have negative score";
}

TEST(EngineEvaluationTest, QueenVsRookEndgame) {
    Board board;
    Engine engine;
    
    // White has queen, black has rook (white to move)
    board.set_position_fen("4k3/8/8/8/8/8/4K3/Q6r w - - 0 1");
    
    int score = engine.evaluate_position(board);
    
    // Queen (900) vs Rook (500) = ~400 centipawn advantage
    EXPECT_GT(score, 300);
    EXPECT_LT(score, 500);
}

TEST(EngineEvaluationTest, MinorPieceImbalance) {
    Board board;
    Engine engine;
    
    // White has bishop, black has knight (equal value, ~300 each)
    board.set_position_fen("4k3/8/8/8/8/8/4K3/B6n w - - 0 1");
    
    int score = engine.evaluate_position(board);
    
    // Should be very close to 0 (minor pieces roughly equal)
    EXPECT_NEAR(score, 0, 50);
}

TEST(EngineEvaluationTest, PawnAdvantage) {
    Board board;
    Engine engine;
    
    // White up two pawns
    board.set_position_fen("rnbqkbnr/pppppp2/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    
    int score = engine.evaluate_position(board);
    
    // Two pawns = ~200 centipawns
    EXPECT_GT(score, 150);
    EXPECT_LT(score, 250);
}

TEST(EngineEvaluationTest, AllPawnsGone) {
    Board board;
    Engine engine;
    
    // No pawns, just pieces
    board.set_position_fen("rnbqkbnr/8/8/8/8/8/8/RNBQKBNR w KQkq - 0 1");
    
    int score = engine.evaluate_position(board);
    
    // Should be 0 (equal material)
    EXPECT_EQ(score, 0);
}

TEST(EngineEvaluationTest, EvaluationConsistency) {
    Board board;
    Engine engine;
    
    // Evaluate same position multiple times
    board.set_position_fen("rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1");
    
    int score1 = engine.evaluate_position(board);
    int score2 = engine.evaluate_position(board);
    int score3 = engine.evaluate_position(board);
    
    // Should return same score every time
    EXPECT_EQ(score1, score2);
    EXPECT_EQ(score2, score3);
}

TEST(EngineEvaluationTest, NoPanicOnEmptySquares) {
    Board board;
    Engine engine;
    
    // Sparse position
    board.set_position_fen("8/8/3k4/8/8/3K4/8/8 w - - 0 1");
    
    // Should not crash
    int score = engine.evaluate_position(board);
    EXPECT_EQ(score, 0);
}



