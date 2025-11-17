#pragma once
#include <vector>
#include <board.h>

constexpr int CHAR_MAP_SIZE = 128;

std::vector<std::string> splitString(const std::string& string, char delimiter);

extern const Piece charToPiece[CHAR_MAP_SIZE];
extern const char pieceToChar[CHAR_MAP_SIZE];