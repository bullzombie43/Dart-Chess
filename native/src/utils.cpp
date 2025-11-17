#include "utils.h"
#include <sstream>

std::vector<std::string> splitString(const std::string& s, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::stringstream ss(s); // Create a stringstream from the input string

    while (std::getline(ss, token, delimiter)) { // Read tokens until delimiter is found
        tokens.push_back(token);
    }
    return tokens;
}

const Piece charToPiece[CHAR_MAP_SIZE] {
    ['P'] = W_PAWN,
    ['N'] = W_KNIGHT,
    ['B'] = W_BISHOP,
    ['R'] = W_ROOK,
    ['Q'] = W_QUEEN,
    ['K'] = W_KING,
    ['p'] = B_PAWN,
    ['n'] = B_KNIGHT,
    ['b'] = B_BISHOP,
    ['r'] = B_ROOK,
    ['q'] = B_QUEEN,
    ['k'] = B_KING
};

const char pieceToChar[CHAR_MAP_SIZE] {
    [W_PAWN] = 'P',
    [W_KNIGHT] = 'N',
    [W_BISHOP] = 'B',
    [W_ROOK] = 'R',
    [W_QUEEN] = 'Q',
    [W_KING] = 'K',
    [B_PAWN] = 'p',
    [B_KNIGHT] = 'n',
    [B_BISHOP] = 'b',
    [B_ROOK] = 'r',
    [B_QUEEN] = 'q',
    [B_KING] = 'k'
};