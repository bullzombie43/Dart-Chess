#pragma once
#include <vector>
#include "board.h"

class Engine{
    public:
        /*  *   *   *   *  *  */
        /*   MOVE GENERATION  */
        /*  *   *   *   *  *  */

        // Generates all pseudo-legal moves for the current side-to-move.
        std::vector<Move> generate_psuedo_legal_moves(const Board& board);

        // Filters the pseudo-legal moves to only include those that don't
        // leave the king in check (i.e., making them legal).
        std::vector<Move> generate_legal_moves(Board& board);

        // Performs the Perft search (counting legal move positions) recursively.
        uint64_t perft(Board& board, int depth);

        // Utility for the top level, prints results clearly.
        void perft_divide(Board& board, int depth);

        /*  *   *   *   *   *  *  */
        /*  SEARCH AND EVALUATION */
        /*  *   *   *   *   *  *  */




    private:
        // Helper function to generate moves for a single piece type/color
        void generate_moves_from_square(const Board& board, Piece piece, uint8_t index, std::vector<Move>& moves);

        void generate_sliding_moves(const Board& board, Piece piece, uint8_t index, std::vector<Move>& moves);
        void generate_pawn_moves(const Board& board, Piece piece, uint8_t index, std::vector<Move>& moves);
        void generate_knight_moves(const Board& board, Piece piece, uint8_t index, std::vector<Move>& moves);
        void generate_king_moves(const Board& board, Piece piece, uint8_t index, std::vector<Move>& moves);
        void generate_castle_moves(const Board& board, Piece piece, uint8_t index, std::vector<Move>& moves);

        enum Direction{
            North = 8,
            South = -8,
            East = 1,
            West = -1
        };

        Bitboard shift(Bitboard board, Direction direction);
        void extract_pawn_push(Bitboard board, int shift, std::vector<Move>& moves);
};

inline std::array<Piece,4> promotion_pieces(Color c) {
    if (c == Color::WHITE) {
        return { Piece::W_QUEEN, Piece::W_ROOK, Piece::W_BISHOP, Piece::W_KNIGHT };
    } else {
        return { Piece::B_QUEEN, Piece::B_ROOK, Piece::B_BISHOP, Piece::B_KNIGHT };
    }
}