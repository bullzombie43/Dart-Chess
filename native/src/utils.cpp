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

int rankOf(int square){ return square / 8;}
int fileOf(int square){ return square % 8;}

const std::array<int, 8> knight_offsets = {17, 15, 10, 6, -17, -15, -10, -6}; 
const std::array<int, 2> pawn_attack_offsets = {-1, 1}; 
const std::array<int, 4> diagonal_offsets = {9, 7, -9, -7};
const std::array<int, 4> rook_offsets = {8, -8, 1, -1}; // up, down, right, left

const std::array<int, 16> direction_offsets = {8,-8,-1,1,7,-7,9,-9, 6, 10, 15, 17, -6, -10, -15, -17}; 


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

Bitboard compute_knight_attacks(int square)
{
    int rank = rankOf(square);
    int file = fileOf(square);
    uint64_t attacks = 0ULL;

    for (int j = 0; j < 8; j++) {
        int attacked_square = square + knight_offsets[j];

        // Quick board boundary check
        if (attacked_square < 0 || attacked_square >= 64) continue;

        int targetRank = rankOf(attacked_square);
        int targetFile = fileOf(attacked_square);

        // Make sure knight move doesn’t wrap around the board
        if (std::abs(targetRank - rank) > 2 || std::abs(targetFile - file) > 2) continue;

        attacks |= (1ULL << attacked_square);
    }

    return static_cast<Bitboard>(attacks);
}

Bitboard compute_king_attacks(int square){
    int rank = rankOf(square);
    int file = fileOf(square);
    uint64_t attacks = 0ULL;

    for (int dr = -1; dr <= 1; dr++) {
        for (int df = -1; df <= 1; df++) {
            if (dr == 0 && df == 0) continue;
            int r = rank + dr;
            int f = file + df;
            if (r >= 0 && r < 8 && f >= 0 && f < 8) {
                attacks |= (1ULL << (r * 8 + f));
            }
        }
    }
    return static_cast<Bitboard>(attacks);
}

Bitboard compute_pawn_attacks(int square, Color color)
{
    int rank = rankOf(square);
    int file = fileOf(square);
    uint64_t attacks = 0ULL;

    int forward = color == Color::WHITE ? 1 : -1;

    for (int df : pawn_attack_offsets) {
        int r = rank + forward;
        int f = file + df;
        if (r >= 0 && r < 8 && f >= 0 && f < 8) {
            attacks |= (1ULL << (r * 8 + f));
        }
    }

    return static_cast<Bitboard>(attacks);
}

const std::array<Bitboard, 64> knight_moves = []{
    std::array<Bitboard, 64> result;
    for(int i = 0; i < 64; i++){
        result[i] = compute_knight_attacks(i);
    }
    return result;
}();

const std::array<Bitboard, 64> king_moves = []{
    std::array<Bitboard, 64> result;
    for(int i = 0; i < 64; i++){
        result[i] = compute_king_attacks(i);
    }
    return result;
}();

const std::array<Bitboard, 128> pawn_attacks = []{
    std::array<Bitboard, 128> result;
    for(int i = 0; i < 64; i++){
        result[i] = compute_pawn_attacks(i, Color::WHITE);
        result[i + 64] = compute_pawn_attacks(i, Color::BLACK);
    }
    return result;
}();

const const std::array<std::array<int,8>, 64> num_squares_to_edge = []{
    std::array<std::array<int,8>, 64> data;

    for (int file = 0; file < 8; file++) {
      for (int rank = 0; rank < 8; rank++) {
        int squareIndex = rank * 8 + file;

        int north = 7 - rank;
        int south = rank;
        int west = file;
        int east = 7 - file;
        int northWest = std::min(north, west);
        int northEast = std::min(north, east);
        int southWest = std::min(south, west);
        int southEast = std::min(south, east);

        data[squareIndex] = {
          north,
          south,
          west,
          east,
          northWest,
          southEast,
          northEast,
          southWest,
        };
      }
    }
    return data;
}();


const std::array<std::array<Bitboard, 4>, 64> rook_ray_masks = []{
    std::array<std::array<Bitboard, 4>, 64> result;
    for(int file = 0; file < 8; file++){
        for(int rank = 0; rank < 8; rank++){
            int squareIndex = rank * 8 + file;

            int north = 7 - rank;
            int south = rank;
            int west = file;
            int east = 7 - file;

            // 0: North Ray Mask
            result[squareIndex][0] = generate_ray_mask(squareIndex, north, 8); 

            // 1: South Ray Mask
            result[squareIndex][1] = generate_ray_mask(squareIndex, south, -8); 

            // 2: East Ray Mask
            result[squareIndex][2] = generate_ray_mask(squareIndex, east, 1);
            
            // 3: West Ray Mask
            //Possible Error: Special handling for the West ray to avoid wrapping from A-file to H-file
            result[squareIndex][3] = generate_ray_mask(squareIndex, west, -1);
        }
    }

    return result;
}();

const std::array<std::array<Bitboard, 4>, 64> bishop_ray_masks = []{
    std::array<std::array<Bitboard, 4>, 64> result;
    for (int rank = 0; rank < 8; rank++) {
        for (int file = 0; file < 8; file++) {
            int squareIndex = rank * 8 + file;

            int north = 7 - rank;
            int south = rank;
            int west = file;
            int east = 7 - file;
            
            // 0: Northeast (NE)
            int dist_ne = std::min(north, east);
            result[squareIndex][0] = generate_ray_mask(squareIndex, 9, dist_ne); 

            // 1: Northwest (NW)
            int dist_nw = std::min(north, west);
            result[squareIndex][1] = generate_ray_mask(squareIndex, 7, dist_nw); 

            // 2: Southeast (SE)
            int dist_se = std::min(south, east);
            result[squareIndex][2] = generate_ray_mask(squareIndex, -7, dist_se);
            
            // 3: Southwest (SW)
            int dist_sw = std::min(south, west);
            result[squareIndex][3] = generate_ray_mask(squareIndex, -9, dist_sw);
        }
    }

    return result;
}();

Bitboard get_bishop_attacks(int square, Bitboard occupied){
    uint64_t attacks = 0ULL;
    

    for (int dir : diagonal_offsets) {
        int s = square;
        while (true) {
            s += dir;
            if (s < 0 || s >= 64) break;
            
            if ((dir == 9 && fileOf(s) == 0) || (dir == -9 && fileOf(s) == 7) ||
                (dir == 7 && fileOf(s) == 7) || (dir == -7 && fileOf(s) == 0)) break;

            attacks |= (1ULL << s);
            if ((occupied & (1ULL << s)) != 0) break;
        }
    }
    return attacks;
}

Bitboard get_rook_attacks(int square, Bitboard occupied){
    Bitboard attacks = 0;

    for (int dir : rook_offsets) {
        int s = square;
        while (true) {
            s += dir;
            if (s < 0 || s >= 64) break;
            // stop wrap-around (left/right edges)
            if ((dir == 1 && fileOf(s) == 0) || (dir == -1 && fileOf(s) == 7)) break;

            attacks |= (1ULL << s);
            if ((occupied & (1ULL << s)) != 0) break; // blocked
        }
    }
    return attacks;
}

Bitboard get_queen_attacks(int square, Bitboard occupied){
    return get_bishop_attacks(square, occupied) | get_rook_attacks(square, occupied);
}

