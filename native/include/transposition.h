#pragma once
#include <cstdint>
#include "board.h"

/*
 * =============================================================================
 *                          Transposition Table
 * =============================================================================
 */

struct TTEntry{
    uint64_t zobrist_key;
    uint16_t score; 
    uint8_t depth;
    uint8_t flag; // EXACT, LOWER_BOUND, UPPER_BOUND
    Move best_move;
};

constexpr uint8_t HASH_EXACT = 0;
constexpr uint8_t HASH_BETA = 1;
constexpr uint8_t HASH_ALPHA = 2;


class TranspositionTable{
    private: 
        std::vector<TTEntry> table;
        size_t size_mask;  // For fast modulo using bitwise AND (only applicable for powers of 2)

    public:
        TranspositionTable(size_t mb);
        size_t get_index(uint64_t zobrist_key) const;
        std::optional<TTEntry> get_entry(uint64_t zobrist_key);
        void store(uint64_t zobrist_key, int score, int depth, 
               uint8_t flag, Move best_move);
        void clear();


    
};