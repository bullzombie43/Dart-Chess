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

extern const std::array<Bitboard, 64> knight_attacks;
extern const std::array<Bitboard, 64> king_attacks;
extern const std::array<Bitboard, 128> pawn_attacks;

Bitboard compute_knight_attacks(int square);
Bitboard compute_king_attacks(int square);
Bitboard compute_pawn_attacks(int square, Color color);

Bitboard get_bishop_attacks(int square, Bitboard occupied);
Bitboard get_rook_attacks(int square, Bitboard occupied);
Bitboard get_queen_attacks(int square, Bitboard occupied);



