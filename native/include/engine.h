#pragma once
#include <vector>
#include "board.h"

class Engine{
    public:
        Engine() = default;

        /*  *   *   *   *  *  */
        /*   MOVE GENERATION  */
        /*  *   *   *   *  *  */

        // Generates all pseudo-legal moves for the current side-to-move.
        int generate_psuedo_legal_moves(const Board& board, Move* moves);

        // Filters the pseudo-legal moves to only include those that don't
        // leave the king in check (i.e., making them legal).
        int generate_legal_moves(Board& board, Move* moves);

        // Performs the Perft search (counting legal move positions) recursively.
        uint64_t perft(Board& board, int depth);

        // Utility for the top level, prints results clearly.
        uint64_t perft_divide(Board& board, int depth);

        /*  *   *   *   *   *  *  */
        /*  SEARCH AND EVALUATION */
        /*  *   *   *   *   *  *  */


        /// @brief Evaluate the position returning a numerical value. 
        /// Higher score = Better For White, Lower Score = better for black
        /// @param board the current position as a board object
        /// @return The score of the position from whites perspective. 
        int evaluate_position(Board& board);


    private:
        // Helper function to generate moves for a single piece type/color
        void generate_moves_from_square(const Board& board, Piece piece, uint8_t index, Move* moves, int& move_count);

        void generate_sliding_moves(const Board& board, Piece piece, uint8_t index, Move* moves, int& move_count);
        void generate_pawn_moves(const Board& board, Move* moves, int& move_count);
        void generate_knight_moves(const Board& board, Piece piece, uint8_t index, Move* moves, int& move_count);
        void generate_king_moves(const Board& board, Piece piece, uint8_t index, Move* moves, int& move_count);
        void generate_castle_moves(const Board& board, Piece piece, uint8_t index, Move* moves, int& move_count);

        enum Direction{
            North = 8,
            South = -8,
            East = 1,
            West = -1
        };

        Bitboard shift(Bitboard board, int direction);
        void extract_pawn_push(Bitboard bb, Piece piece, int shift, Move* moves, int& move_count);
        void extract_pawn_capture(Bitboard bb, Piece piece, int shift, Move* moves, int& move_count, const Board& board);
        void extract_promotion_push(Bitboard bb, Piece piece, int shift, Move* moves, int& move_count);
        void extract_promotion_capture(Bitboard bb, Piece piece, int shift, Move* moves, int& move_count, const Board& board);
        
};

inline std::array<Piece,4> promotion_pieces(Color c) {
    if (c == Color::WHITE) {
        return { Piece::W_QUEEN, Piece::W_ROOK, Piece::W_BISHOP, Piece::W_KNIGHT };
    } else {
        return { Piece::B_QUEEN, Piece::B_ROOK, Piece::B_BISHOP, Piece::B_KNIGHT };
    }
}

inline std::string move_to_string(Move m) {
    auto sq = [](int s) {
        char file = 'a' + (s % 8);
        char rank = '1' + (s / 8);
        return std::string{file, rank};
    };

    std::string str;

    // From → To coordinates
    str += sq(m.from_square);
    str += sq(m.to_square);

    // Promotion (e7e8q)
    if (m.promoted_piece != Piece::NONE) {
        switch (m.promoted_piece) {
            case Piece::W_QUEEN:
            case Piece::B_QUEEN:  str += "q"; break;
            case Piece::W_ROOK:
            case Piece::B_ROOK:   str += "r"; break;
            case Piece::W_BISHOP:
            case Piece::B_BISHOP: str += "b"; break;
            case Piece::W_KNIGHT:
            case Piece::B_KNIGHT: str += "n"; break;
            default: break;
        }
    }

    return str;
}

constexpr int MAX_NUMBER_OF_MOVES = 256;
constexpr int MAX_DEPTH = 6;               // or whatever max perft depth you need
