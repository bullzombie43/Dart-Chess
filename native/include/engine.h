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
        std::vector<Move> generate_legal_moves(const Board& board);

        // Performs the Perft search (counting legal move positions) recursively.
        uint64_t perft(Board& board, int depth);

        // Utility for the top level, prints results clearly.
        void perft_divide(Board& board, int depth);

        /*  *   *   *   *   *  *  */
        /*  SEARCH AND EVALUATION */
        /*  *   *   *   *   *  *  */




    private:
        // Helper function to generate moves for a single piece type/color
        void generate_sliding_moves(const Board& board, std::vector<Move>& moves);
        void generate_pawn_moves(const Board& board, std::vector<Move>& moves);
        void generate_knight_moves(const Board& board, std::vector<Move>& moves);
        void generate_king_moves(const Board& board, std::vector<Move>& moves);

};