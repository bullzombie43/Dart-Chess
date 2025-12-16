#include "transposition.h"

TranspositionTable::TranspositionTable(size_t mb)
{
    size_t num_entries = (mb * 1024 * 1024) / sizeof(TTEntry);

    //Round down to power of 2 for fast modulo
    num_entries = 1ULL << static_cast<int>(std::log2(num_entries));

    table.resize(num_entries);
    size_mask = num_entries - 1;
}

size_t TranspositionTable::get_index(uint64_t zobrist_key) const
{
    return zobrist_key & size_mask; //Only applicable for power of 2, faster than key % num_entries
}

std::optional<TTEntry> TranspositionTable::get_entry(uint64_t zobrist_key)
{
    size_t index = get_index(zobrist_key);
    TTEntry entry = table[index];

    if(entry.zobrist_key == zobrist_key){
        return entry;
    }

    return std::nullopt;
}

void TranspositionTable::store(uint64_t zobrist_key, int score, int depth, uint8_t flag, Move best_move)
{
    size_t index = get_index(zobrist_key);
    TTEntry entry = table[index];

    //Replace if deeper scheme
    if(entry.zobrist_key == 0 || entry.depth <= depth){
        entry.zobrist_key = zobrist_key;
        entry.score = score;
        entry.depth = depth;
        entry.flag = flag;
        entry.best_move = best_move;
    }
}

void TranspositionTable::clear()
{
    std::fill(table.begin(), table.end(), TTEntry{});
}
