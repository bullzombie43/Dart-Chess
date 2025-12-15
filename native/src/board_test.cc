#include <gtest/gtest.h>
#include <board.h>
#include <utils.h>

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
    EXPECT_EQ(board.get_fen(),"rnbqkbnr/pp1ppppp/8/2p5/4P3/5N2/PPPP1PPP/RNBQKB1R b KQkq - 1 2" );
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
    EXPECT_EQ(board.get_fen(), "rnbqkbnr/pp1ppppp/8/2p5/4P3/8/PPPP1PPP/RNBQKBNR w KQkq c6 0 2");
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

    int preMoveWhite = board.get_pst_color(Color::WHITE);
    int preMoveBlack = board.get_pst_color(Color::BLACK);

    Move pawnC2C4 = {Piece::W_PAWN, 10, 26, Piece::NONE, Piece::NONE, false, false};    
    board.make_move(pawnC2C4);

    std::cout << "PRE: " << preMoveWhite << " Post: " << board.get_pst_color(Color::WHITE) << std::endl;

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

    EXPECT_EQ(board.get_pst_color(Color::WHITE), preMoveWhite);
    EXPECT_EQ(board.get_pst_color(Color::BLACK), preMoveBlack);

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

    int preMoveWhite = board.get_pst_color(Color::WHITE);
    int preMoveBlack = board.get_pst_color(Color::BLACK);
    
    board.make_move(pawnE2E4);

    // Verify state change: En Passant must be set to E3 (square 20)
    EXPECT_EQ(board.E3, board.enPassantSquare); 
    EXPECT_EQ(Color::BLACK, board.sideToMove); // Side to move must flip

    // Undo move
    board.undo_move();


    
    EXPECT_EQ(board.get_pst_color(Color::WHITE), preMoveWhite);
    EXPECT_EQ(board.get_pst_color(Color::BLACK), preMoveBlack);

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

    int preMoveWhite = board.get_pst_color(Color::WHITE);
    int preMoveBlack = board.get_pst_color(Color::BLACK);

    board.make_move(knightCapturesPawn);

    // Verify piece is gone and capture flag is set
    EXPECT_EQ(Color::BLACK, board.sideToMove); // Side flipped

    // Undo move
    board.undo_move();

    EXPECT_EQ(board.get_pst_color(Color::WHITE), preMoveWhite);
    EXPECT_EQ(board.get_pst_color(Color::BLACK), preMoveBlack);

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

TEST_F(BoardTestFixture, EnPassantCaptureAndUndoPST) {
    board = Board();
    std::ostringstream oss;
    
    // Setup position where en passant is available
    // White pawn on e5, black pawn just moved d7-d5 (en passant target on d6)
    board.set_position_fen("rnbqkbnr/ppp1pppp/8/3pP3/8/8/PPPP1PPP/RNBQKBNR w KQkq d6 0 2");
    
    // Save PST values before move
    int preMoveWhite = board.get_pst_color(Color::WHITE);
    int preMoveBlack = board.get_pst_color(Color::BLACK);
    
    // Save FEN for comparison
    std::string originalFen = board.get_fen();
    
    // En passant capture: e5 takes d6 (white pawn captures black pawn on d5)
    // from_square = 36 (e5), to_square = 43 (d6)
    // captured_piece = B_PAWN (on d5, square 35)
    Move enPassantMove = {
        Piece::W_PAWN,      // piece
        36,                 // from_square (e5)
        43,                 // to_square (d6)
        Piece::B_PAWN,      // captured_piece
        Piece::NONE,        // promoted_piece
        true,               // is_enpassant
        false               // is_castling
    };
    
    // Verify initial state
    EXPECT_EQ(board.get_piece_at(36), Piece::W_PAWN);  // White pawn on e5
    EXPECT_EQ(board.get_piece_at(35), Piece::B_PAWN);  // Black pawn on d5
    EXPECT_EQ(board.get_piece_at(43), Piece::NONE);    // d6 is empty
    
    // Make en passant move
    board.make_move(enPassantMove);
    
    // Verify move was made correctly
    EXPECT_EQ(board.sideToMove, Color::BLACK);         // Side flipped
    EXPECT_EQ(board.get_piece_at(36), Piece::NONE);    // e5 is now empty
    EXPECT_EQ(board.get_piece_at(43), Piece::W_PAWN);  // White pawn moved to d6
    EXPECT_EQ(board.get_piece_at(35), Piece::NONE);    // Black pawn on d5 was captured!
    
    // PST should have changed
    int afterMoveWhite = board.get_pst_color(Color::WHITE);
    int afterMoveBlack = board.get_pst_color(Color::BLACK);
    
    EXPECT_NE(afterMoveWhite, preMoveWhite) << "White PST should change after en passant";
    EXPECT_NE(afterMoveBlack, preMoveBlack) << "Black PST should change (pawn captured)";
    
    // Undo the en passant move
    board.undo_move();
    
    // Verify PST is fully restored
    EXPECT_EQ(board.get_pst_color(Color::WHITE), preMoveWhite) 
        << "White PST should be restored after undo";
    EXPECT_EQ(board.get_pst_color(Color::BLACK), preMoveBlack)
        << "Black PST should be restored after undo";
    
    // Verify board state is fully restored
    EXPECT_EQ(board.sideToMove, Color::WHITE);
    EXPECT_EQ(board.get_piece_at(36), Piece::W_PAWN);  // White pawn back on e5
    EXPECT_EQ(board.get_piece_at(35), Piece::B_PAWN);  // Black pawn restored on d5
    EXPECT_EQ(board.get_piece_at(43), Piece::NONE);    // d6 is empty again
    
    // Verify FEN is restored
    std::string restoredFen = board.get_fen();
    EXPECT_EQ(restoredFen, originalFen) << "FEN should be fully restored after undo";
    
    // Verify visual board state
    oss.str("");
    board.print_board(oss);
    EXPECT_EQ(R"(r n b q k b n r 
p p p . p p p p 
. . . . . . . . 
. . . p P . . . 
. . . . . . . . 
. . . . . . . . 
P P P P . P P P 
R N B Q K B N R 
)", oss.str());
}

TEST_F(BoardTestFixture, PromotionAndUndoPST) {
    board = Board();
    std::ostringstream oss;
    
    // Setup position where white pawn on e7 can promote
    // Black king on e8, white pawn on e7 ready to promote
    board.set_position_fen("2k5/4P3/8/8/8/8/8/2K5 w - - 0 1");
    
    // Save PST values before move
    int preMoveWhite = board.get_pst_color(Color::WHITE);
    int preMoveBlack = board.get_pst_color(Color::BLACK);
    
    // Save FEN for comparison
    std::string originalFen = board.get_fen();
    
    // Promotion move: e7 to e8, promote to Queen
    // from_square = 52 (e7), to_square = 60 (e8)
    Move promotionMove = {
        Piece::W_PAWN,      // piece
        52,                 // from_square (e7)
        60,                 // to_square (e8)
        Piece::NONE,        // captured_piece (no capture)
        Piece::W_QUEEN,     // promoted_piece
        false,              // is_enpassant
        false               // is_castling
    };
    
    // Verify initial state
    EXPECT_EQ(board.get_piece_at(52), Piece::W_PAWN);  // White pawn on e7
    EXPECT_EQ(board.get_piece_at(60), Piece::NONE);    // e8 is empty
    
    // Make promotion move
    board.make_move(promotionMove);
    
    // Verify move was made correctly
    EXPECT_EQ(board.sideToMove, Color::BLACK);         // Side flipped
    EXPECT_EQ(board.get_piece_at(52), Piece::NONE);    // e7 is now empty
    EXPECT_EQ(board.get_piece_at(60), Piece::W_QUEEN); // White queen on e8
    
    // PST should have changed (pawn removed, queen added)
    int afterMoveWhite = board.get_pst_color(Color::WHITE);
    int afterMoveBlack = board.get_pst_color(Color::BLACK);
    
    EXPECT_NE(afterMoveWhite, preMoveWhite) << "White PST should change after promotion";
    EXPECT_EQ(afterMoveBlack, preMoveBlack) << "Black PST should remain unchanged";
    
    // Undo the promotion move
    board.undo_move();
    
    // Verify PST is fully restored
    EXPECT_EQ(board.get_pst_color(Color::WHITE), preMoveWhite) 
        << "White PST should be restored after undo";
    EXPECT_EQ(board.get_pst_color(Color::BLACK), preMoveBlack)
        << "Black PST should remain unchanged after undo";
    
    // Verify board state is fully restored
    EXPECT_EQ(board.sideToMove, Color::WHITE);
    EXPECT_EQ(board.get_piece_at(52), Piece::W_PAWN);  // White pawn back on e7
    EXPECT_EQ(board.get_piece_at(60), Piece::NONE);    // e8 is empty again
    
    // Verify FEN is restored
    std::string restoredFen = board.get_fen();
    EXPECT_EQ(restoredFen, originalFen) << "FEN should be fully restored after undo";
    
    // Verify visual board state
    oss.str("");
    board.print_board(oss);
    EXPECT_EQ(R"(. . k . . . . . 
. . . . P . . . 
. . . . . . . . 
. . . . . . . . 
. . . . . . . . 
. . . . . . . . 
. . . . . . . . 
. . K . . . . . 
)", oss.str());
}

TEST_F(BoardTestFixture, PromotionWithCaptureAndUndoPST) {
    board = Board();
    std::ostringstream oss;
    
    // Setup position where white pawn on d7 can capture and promote
    // Black rook on e8, white pawn on d7 ready to capture-promote
    board.set_position_fen("4r3/3P4/8/8/8/8/8/4K3 w - - 0 1");
    
    // Save PST values before move
    int preMoveWhite = board.get_pst_color(Color::WHITE);
    int preMoveBlack = board.get_pst_color(Color::BLACK);
    
    // Save FEN for comparison
    std::string originalFen = board.get_fen();
    
    // Promotion with capture: d7 takes e8, promote to Queen
    // from_square = 51 (d7), to_square = 60 (e8)
    Move promotionCaptureMove = {
        Piece::W_PAWN,      // piece
        51,                 // from_square (d7)
        60,                 // to_square (e8)
        Piece::B_ROOK,      // captured_piece
        Piece::W_QUEEN,     // promoted_piece
        false,              // is_enpassant
        false               // is_castling
    };
    
    // Verify initial state
    EXPECT_EQ(board.get_piece_at(51), Piece::W_PAWN);  // White pawn on d7
    EXPECT_EQ(board.get_piece_at(60), Piece::B_ROOK);  // Black rook on e8
    
    // Make promotion with capture move
    board.make_move(promotionCaptureMove);
    
    // Verify move was made correctly
    EXPECT_EQ(board.sideToMove, Color::BLACK);         // Side flipped
    EXPECT_EQ(board.get_piece_at(51), Piece::NONE);    // d7 is now empty
    EXPECT_EQ(board.get_piece_at(60), Piece::W_QUEEN); // White queen on e8
    
    // PST should have changed (pawn removed, queen added, black rook captured)
    int afterMoveWhite = board.get_pst_color(Color::WHITE);
    int afterMoveBlack = board.get_pst_color(Color::BLACK);
    
    EXPECT_NE(afterMoveWhite, preMoveWhite) << "White PST should change after promotion";
    EXPECT_NE(afterMoveBlack, preMoveBlack) << "Black PST should change (rook captured)";
    
    // Undo the promotion with capture move
    board.undo_move();
    
    // Verify PST is fully restored
    EXPECT_EQ(board.get_pst_color(Color::WHITE), preMoveWhite) 
        << "White PST should be restored after undo";
    EXPECT_EQ(board.get_pst_color(Color::BLACK), preMoveBlack)
        << "Black PST should be restored after undo";
    
    // Verify board state is fully restored
    EXPECT_EQ(board.sideToMove, Color::WHITE);
    EXPECT_EQ(board.get_piece_at(51), Piece::W_PAWN);  // White pawn back on d7
    EXPECT_EQ(board.get_piece_at(60), Piece::B_ROOK);  // Black rook restored on e8
    
    // Verify FEN is restored
    std::string restoredFen = board.get_fen();
    EXPECT_EQ(restoredFen, originalFen) << "FEN should be fully restored after undo";
    
    // Verify visual board state
    oss.str("");
    board.print_board(oss);
    EXPECT_EQ(R"(. . . . r . . . 
. . . P . . . . 
. . . . . . . . 
. . . . . . . . 
. . . . . . . . 
. . . . . . . . 
. . . . . . . . 
. . . . K . . . 
)", oss.str());
}

TEST_F(BoardTestFixture, CastlingMakeAndUndo) {
    board = Board();

    //
    // Position: White can castle both sides
    //
    board.set_position_fen(
        "r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1"
    );

    int preMoveWhite = board.get_pst_color(Color::WHITE);
    int preMoveBlack = board.get_pst_color(Color::BLACK);

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

    EXPECT_NE(board.get_pst_color(Color::WHITE), preMoveWhite);
    EXPECT_EQ(board.get_pst_color(Color::BLACK), preMoveBlack);

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
    EXPECT_EQ(board.get_pst_color(Color::WHITE), preMoveWhite);
    EXPECT_EQ(board.get_pst_color(Color::BLACK), preMoveBlack);

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

TEST_F(BoardTestFixture, ZobristHash){
    board = Board();

    //Starting Position
    board.set_position_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    EXPECT_EQ(calculate_zobrist_hash(board), 0x463b96181691fc9c) << "Starting Position";

    std::cout << "Done" << std::endl;
    
    //Position after e2e4
    board.set_position_fen("rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1");
    EXPECT_EQ(calculate_zobrist_hash(board), 0x823c9b50fd114196) << "Position after e2e4";

    std::cout << "Done" << std::endl;

    //position after e2e4 d75
    board.set_position_fen("rnbqkbnr/ppp1pppp/8/3p4/4P3/8/PPPP1PPP/RNBQKBNR w KQkq d6 0 2");
    EXPECT_EQ(calculate_zobrist_hash(board), 0x0756b94461c50fb0) << "position after e2e4 d75";

    std::cout << "Done" << std::endl;

    //position after e2e4 d7d5 e4e5
    board.set_position_fen("rnbqkbnr/ppp1pppp/8/3pP3/8/8/PPPP1PPP/RNBQKBNR b KQkq - 0 2");
    EXPECT_EQ(calculate_zobrist_hash(board), 0x662fafb965db29d4) << "position after e2e4 d7d5 e4e5";

    std::cout << "Done" << std::endl;

    //position after e2e4 d7d5 e4e5 f7f5
    board.set_position_fen("rnbqkbnr/ppp1p1pp/8/3pPp2/8/8/PPPP1PPP/RNBQKBNR w KQkq f6 0 3");
    EXPECT_EQ(calculate_zobrist_hash(board), 0x22a48b5a8e47ff78) << "position after e2e4 d7d5 e4e5 f7f5";

    std::cout << "Done" << std::endl;

    //position after e2e4 d7d5 e4e5 f7f5 e1e2
    board.set_position_fen("rnbqkbnr/ppp1p1pp/8/3pPp2/8/8/PPPPKPPP/RNBQ1BNR b kq - 0 3");
    EXPECT_EQ(calculate_zobrist_hash(board), 0x652a607ca3f242c1) << "position after e2e4 d7d5 e4e5 f7f5 e1e2";

    std::cout << "Done" << std::endl;

    //position after e2e4 d7d5 e4e5 f7f5 e1e2 e8f7
    board.set_position_fen("rnbq1bnr/ppp1pkpp/8/3pPp2/8/8/PPPPKPPP/RNBQ1BNR w - - 0 4");
    EXPECT_EQ(calculate_zobrist_hash(board), 0x00fdd303c946bdd9) << "position after e2e4 d7d5 e4e5 f7f5 e1e2 e8f7";

    std::cout << "Done" << std::endl;

    //position after a2a4 b7b5 h2h4 b5b4 c2c4
    board.set_position_fen("rnbqkbnr/p1pppppp/8/8/PpP4P/8/1P1PPPP1/RNBQKBNR b KQkq c3 0 3");
    EXPECT_EQ(calculate_zobrist_hash(board), 0x3c8123ea7b067637) << "position after a2a4 b7b5 h2h4 b5b4 c2c4";

    std::cout << "Done" << std::endl;

    //position after a2a4 b7b5 h2h4 b5b4 c2c4 b4c3 a1a3
    board.set_position_fen("rnbqkbnr/p1pppppp/8/8/P6P/R1p5/1P1PPPP1/1NBQKBNR b Kkq - 0 4");
    EXPECT_EQ(calculate_zobrist_hash(board), 0x5c3f9b829b279560) << "position after a2a4 b7b5 h2h4 b5b4 c2c4 b4c3 a1a3";

    std::cout << "Done" << std::endl;

}

TEST_F(BoardTestFixture, ZobristHashIncremental){
    board = Board();
    board.set_position_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    EXPECT_EQ(board.zobrist_key, 0x463b96181691fc9c) << "Starting Position";

    // Move 1: e2e4
    Move e2e4 = {
        Piece::W_PAWN,      // piece
        12,                 // from_square (e2)
        28,                 // to_square (e4)
        Piece::NONE,        // captured_piece
        Piece::NONE,        // promoted_piece
        false,              // is_enpassant
        false               // is_castling
    };
    board.make_move(e2e4);
    EXPECT_EQ(board.zobrist_key, 0x823c9b50fd114196) << "Position after e2e4";

    // Move 2: d7d5
    Move d7d5 = {
        Piece::B_PAWN,      // piece
        51,                 // from_square (d7)
        35,                 // to_square (d5)
        Piece::NONE,        // captured_piece
        Piece::NONE,        // promoted_piece
        false,              // is_enpassant
        false               // is_castling
    };
    board.make_move(d7d5);
    EXPECT_EQ(board.zobrist_key, 0x0756b94461c50fb0) << "Position after e2e4 d7d5";

    // Move 3: e4e5
    Move e4e5 = {
        Piece::W_PAWN,      // piece
        28,                 // from_square (e4)
        36,                 // to_square (e5)
        Piece::NONE,        // captured_piece
        Piece::NONE,        // promoted_piece
        false,              // is_enpassant
        false               // is_castling
    };
    board.make_move(e4e5);
    EXPECT_EQ(board.zobrist_key, 0x662fafb965db29d4) << "Position after e2e4 d7d5 e4e5";

    // Move 4: f7f5
    Move f7f5 = {
        Piece::B_PAWN,      // piece
        53,                 // from_square (f7)
        37,                 // to_square (f5)
        Piece::NONE,        // captured_piece
        Piece::NONE,        // promoted_piece
        false,              // is_enpassant
        false               // is_castling
    };
    board.make_move(f7f5);
    EXPECT_EQ(board.zobrist_key, 0x22a48b5a8e47ff78) << "Position after e2e4 d7d5 e4e5 f7f5";

    // Move 5: e1e2 (White king moves, loses castling rights)
    Move e1e2 = {
        Piece::W_KING,      // piece
        4,                  // from_square (e1)
        12,                 // to_square (e2)
        Piece::NONE,        // captured_piece
        Piece::NONE,        // promoted_piece
        false,              // is_enpassant
        false               // is_castling
    };
    board.make_move(e1e2);
    EXPECT_EQ(board.zobrist_key, 0x652a607ca3f242c1) << "Position after e2e4 d7d5 e4e5 f7f5 e1e2";

    // Move 6: e8f7 (Black king moves, loses castling rights)
    Move e8f7 = {
        Piece::B_KING,      // piece
        60,                 // from_square (e8)
        53,                 // to_square (f7)
        Piece::NONE,        // captured_piece
        Piece::NONE,        // promoted_piece
        false,              // is_enpassant
        false               // is_castling
    };
    board.make_move(e8f7);
    EXPECT_EQ(board.zobrist_key, 0x00fdd303c946bdd9) << "Position after e2e4 d7d5 e4e5 f7f5 e1e2 e8f7";

    // New position: a2a4 b7b5 h2h4 b5b4 c2c4
    board.set_position_fen("rnbqkbnr/p1pppppp/8/8/PpP4P/8/1P1PPPP1/RNBQKBNR b KQkq c3 0 3");
    EXPECT_EQ(board.zobrist_key, 0x3c8123ea7b067637) << "Position after a2a4 b7b5 h2h4 b5b4 c2c4";

    // Move: b4c3 (pawn capture)
    Move b4c3 = {
        Piece::B_PAWN,      // piece
        25,                 // from_square (b4)
        18,                 // to_square (c3)
        Piece::W_PAWN,        // captured_piece
        Piece::NONE,        // promoted_piece
        true,              // is_enpassant
        false               // is_castling
    };

    board.make_move(b4c3);
    EXPECT_EQ(board.zobrist_key, calculate_zobrist_hash(board)) << "Position after a2a4 b7b5 h2h4 b5b4 c2c4 b4c3" ;

    // Move: a1a3 (rook moves, loses queenside castling)
    Move a1a3 = {
        Piece::W_ROOK,      // piece
        0,                  // from_square (a1)
        16,                 // to_square (a3)
        Piece::NONE,        // captured_piece
        Piece::NONE,        // promoted_piece
        false,              // is_enpassant
        false               // is_castling
    };
    board.make_move(a1a3);
    EXPECT_EQ(board.zobrist_key, 0x5c3f9b829b279560) << "Position after a2a4 b7b5 h2h4 b5b4 c2c4 b4c3 a1a3";
}

TEST_F(BoardTestFixture, ZobristHashUndoRestoration){
    board = Board();
    board.set_position_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    uint64_t originalHash = board.zobrist_key;
    EXPECT_EQ(originalHash, 0x463b96181691fc9c) << "Starting Position";

    // Test 1: Regular pawn move
    Move e2e4 = {
        Piece::W_PAWN, 12, 28, Piece::NONE, Piece::NONE, false, false
    };
    board.make_move(e2e4);
    EXPECT_NE(board.zobrist_key, originalHash) << "Hash should change after move";
    
    board.undo_move();
    EXPECT_EQ(board.zobrist_key, originalHash) << "Hash should restore after undo (pawn move)";

    // Test 2: Sequence with en passant target
    board.make_move(e2e4);
    uint64_t afterE2E4 = board.zobrist_key;
    
    Move d7d5 = {
        Piece::B_PAWN, 51, 35, Piece::NONE, Piece::NONE, false, false
    };
    board.make_move(d7d5);
    uint64_t afterD7D5 = board.zobrist_key;
    
    board.undo_move();
    EXPECT_EQ(board.zobrist_key, afterE2E4) << "Hash should restore after undo (en passant target set)";
    
    board.undo_move();
    EXPECT_EQ(board.zobrist_key, originalHash) << "Hash should restore to original after double undo";

    // Test 3: King move (castling rights change)
    board.set_position_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    originalHash = board.zobrist_key;
    
    Move e1e2 = {
        Piece::W_KING, 4, 12, Piece::NONE, Piece::NONE, false, false
    };
    board.make_move(e1e2);
    
    board.undo_move();
    EXPECT_EQ(board.zobrist_key, originalHash) << "Hash should restore after undo (king move, castling rights)";

    // Test 4: Rook move (partial castling rights loss)
    board.set_position_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    originalHash = board.zobrist_key;
    
    Move a1a3 = {
        Piece::W_ROOK, 0, 16, Piece::NONE, Piece::NONE, false, false
    };
    board.make_move(a1a3);
    
    board.undo_move();
    EXPECT_EQ(board.zobrist_key, originalHash) << "Hash should restore after undo (rook move, queenside castling)";

    // Test 5: Capture
    board.set_position_fen("rnbqkbnr/ppp1pppp/8/3p4/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2");
    originalHash = board.zobrist_key;
    
    Move e4d5 = {
        Piece::W_PAWN, 28, 35, Piece::B_PAWN, Piece::NONE, false, false
    };
    board.make_move(e4d5);
    
    board.undo_move();
    EXPECT_EQ(board.zobrist_key, originalHash) << "Hash should restore after undo (capture)";

    // Test 6: En passant capture
    board.set_position_fen("rnbqkbnr/p1pppppp/8/8/PpP4P/8/1P1PPPP1/RNBQKBNR b KQkq c3 0 3");
    originalHash = board.zobrist_key;
    
    Move b4c3 = {
        Piece::B_PAWN, 25, 18, Piece::W_PAWN, Piece::NONE, true, false
    };
    board.make_move(b4c3);
    
    board.undo_move();
    EXPECT_EQ(board.zobrist_key, originalHash) << "Hash should restore after undo (en passant)";

    // Test 7: Promotion
    board.set_position_fen("4k3/4P3/8/8/8/8/8/4K3 w - - 0 1");
    originalHash = board.zobrist_key;
    
    Move e7e8q = {
        Piece::W_PAWN, 52, 60, Piece::NONE, Piece::W_QUEEN, false, false
    };
    board.make_move(e7e8q);
    
    board.undo_move();
    EXPECT_EQ(board.zobrist_key, originalHash) << "Hash should restore after undo (promotion)";

    // Test 8: Promotion with capture
    board.set_position_fen("4r3/3P4/8/8/8/8/8/4K3 w - - 0 1");
    originalHash = board.zobrist_key;
    
    Move d7e8q = {
        Piece::W_PAWN, 51, 60, Piece::B_ROOK, Piece::W_QUEEN, false, false
    };
    board.make_move(d7e8q);
    
    board.undo_move();
    EXPECT_EQ(board.zobrist_key, originalHash) << "Hash should restore after undo (promotion with capture)";

    // Test 9: Castling (kingside)
    board.set_position_fen("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1");
    originalHash = board.zobrist_key;
    
    Move e1g1 = {
        Piece::W_KING, 4, 6, Piece::NONE, Piece::NONE, false, true
    };
    board.make_move(e1g1);
    
    board.undo_move();
    EXPECT_EQ(board.zobrist_key, originalHash) << "Hash should restore after undo (kingside castling)";

    // Test 10: Castling (queenside)
    Move e1c1 = {
        Piece::W_KING, 4, 2, Piece::NONE, Piece::NONE, false, true
    };
    board.make_move(e1c1);
    
    board.undo_move();
    EXPECT_EQ(board.zobrist_key, originalHash) << "Hash should restore after undo (queenside castling)";
}

TEST_F(BoardTestFixture, ZobristHashMultipleUndos){
    board = Board();
    board.set_position_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    uint64_t originalHash = board.zobrist_key;

    // Make a sequence of moves
    std::vector<Move> moves = {
        {Piece::W_PAWN, 12, 28, Piece::NONE, Piece::NONE, false, false},  // e2e4
        {Piece::B_PAWN, 52, 44, Piece::NONE, Piece::NONE, false, false},  // e7e5
        {Piece::W_KNIGHT, 6, 21, Piece::NONE, Piece::NONE, false, false}, // g1f3
        {Piece::B_KNIGHT, 62, 45, Piece::NONE, Piece::NONE, false, false},// g8f6
        {Piece::W_BISHOP, 5, 26, Piece::NONE, Piece::NONE, false, false}, // f1c4
    };

    std::vector<uint64_t> hashHistory;
    hashHistory.push_back(originalHash);

    // Make all moves and save hashes
    for (const auto& move : moves) {
        board.make_move(const_cast<Move&>(move));
        hashHistory.push_back(board.zobrist_key);
    }

    // Undo all moves and verify each hash
    for (int i = moves.size() - 1; i >= 0; --i) {
        board.undo_move();
        EXPECT_EQ(board.zobrist_key, hashHistory[i]) 
            << "Hash should match after undoing move " << i;
    }

    EXPECT_EQ(board.zobrist_key, originalHash) 
        << "Hash should return to original after undoing all moves";
}

TEST_F(BoardTestFixture, ZobristHashRedoAfterUndo){
    board = Board();
    board.set_position_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    Move e2e4 = {
        Piece::W_PAWN, 12, 28, Piece::NONE, Piece::NONE, false, false
    };

    // Make move, save hash, undo, make again
    board.make_move(e2e4);
    uint64_t hashAfterMove = board.zobrist_key;
    
    board.undo_move();
    board.make_move(e2e4);
    
    EXPECT_EQ(board.zobrist_key, hashAfterMove) 
        << "Hash should be identical when making the same move again after undo";
}