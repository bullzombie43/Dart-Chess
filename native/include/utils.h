#pragma once
#include <vector>
#include <board.h>

constexpr int CHAR_MAP_SIZE = 128;

std::vector<std::string> splitString(const std::string& string, char delimiter);
int rankOf(int square);
int fileOf(int square);

extern const Piece charToPiece[CHAR_MAP_SIZE];
extern const char pieceToChar[CHAR_MAP_SIZE];

extern const std::array<int, 8> knight_offsets;
extern const std::array<int, 2> pawn_attack_offsets;
extern const std::array<int, 4> diagonal_offsets;
extern const std::array<int, 4> rook_offsets;

extern const std::array<std::array<Bitboard, 4>, 64> rook_ray_masks; // North South East West Rays for each of the 64 squares
extern const std::array<std::array<Bitboard, 4>, 64> bishop_ray_masks;// Diagonal Rays for each of the 64 squares

extern const std::array<Bitboard, 64> knight_moves; //We call these moves instead of attacks 
extern const std::array<Bitboard, 64> king_moves; //because they are used in move generation as well
extern const std::array<Bitboard, 128> pawn_attacks;
extern const std::array<std::array<int,8>, 64> num_squares_to_edge;
extern const std::array<int, 16> direction_offsets;

const std::array<int, 64> flip_array = {
    56, 57, 58, 59, 60, 61, 62, 63,
    48, 49, 50, 51, 52, 53, 54, 55,
    40, 41, 42, 43, 44, 45, 46, 47,
    32, 33, 34, 35, 36, 37, 38, 39,
    24, 25, 26, 27, 28, 29, 30, 31,
    16, 17, 18, 19, 20, 21, 22, 23,
    8,  9, 10, 11, 12, 13, 14, 15,
    0,  1,  2,  3,  4,  5,  6,  7
};

extern const std::array<std::array<int, 64>, 6> piece_square_table;

Bitboard compute_knight_attacks(int square);
Bitboard compute_king_attacks(int square);
Bitboard compute_pawn_attacks(int square, Color color);

Bitboard get_bishop_attacks(int square, Bitboard occupied);
Bitboard get_rook_attacks(int square, Bitboard occupied);
Bitboard get_queen_attacks(int square, Bitboard occupied);

constexpr uint64_t A_FILE_MASK = 0x0101010101010101ULL; //(A1, A2, ..., A8)
constexpr uint64_t H_FILE_MASK = 0x8080808080808080ULL; //(H1, H2, ..., H8)

// Helper function to generate a ray mask (Can be defined inside _precompute_ray_data)
constexpr Bitboard generate_ray_mask(int squareIndex, int distance, int shift) {

    Bitboard mask = 0ULL;
    Bitboard current_bit = 1ULL << squareIndex;

    // Loop for the distance (C++14/17 allows simple for loops in constexpr)
    for (int i = 0; i < distance; ++i) {
        if (shift > 0) {
            current_bit <<= shift; // North or East
        } else {
            current_bit >>= (-shift); // South or West
        }
        
        // Accumulate the new bit position to the mask
        mask |= current_bit;
    }
    return mask;
}

int pop_lsb(Bitboard& board);