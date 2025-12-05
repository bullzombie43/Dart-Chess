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

const std::array<std::array<int,8>, 64> num_squares_to_edge = []{
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

const std::array<std::array<int, 64>, 6> piece_square_table = []{
    std::array<std::array<int, 64>, 6> pst;
    
    std::array<int, 64> pawn_array = {
        // Pawns on 1st/8th rank are impossible (would be promoted)
        // Value increases as pawns advance, bonus for central pawns
        0,   0,   0,   0,   0,   0,   0,   0,
        105, 105, 105, 105, 105, 105, 105, 105,
        100, 100, 100, 110, 110, 100, 100, 100,
        100, 100, 105, 115, 115, 105, 100, 100,
        100, 100, 110, 120, 120, 110, 100, 100,
        105, 105, 115, 125, 125, 115, 105, 105,
        110, 110, 120, 130, 130, 120, 110, 110,
        0,   0,   0,   0,   0,   0,   0,   0
    };

    std::array<int, 64> knight_array = {
        //This is is the same we'd view the board as whitw with A1 in bot left, we will flip later
        290, 300, 300, 300, 300, 300, 300, 290,
        300, 305, 305, 305, 305, 305, 305, 300,
        300, 305, 325, 325, 325, 325, 305, 300,
        300, 305, 325, 325, 325, 325, 305, 300,
        300, 305, 325, 325, 325, 325, 305, 300,
        300, 305, 320, 325, 325, 325, 305, 300,
        300, 305, 305, 305, 305, 305, 305, 300,
        290, 310, 300, 300, 300, 300, 310, 290
    };

    std::array<int, 64> rook_array = {
        //This is is the same we'd view the board as whitw with A1 in bot left, we will flip later
        500,   500,   500,   500,   500,   500,   500,   500,
        520,   520,   520,   520,   520,   520,   520,   520,
        500,   500,   500,   500,   500,   500,   500,   500,
        500,   500,   500,   500,   500,   500,   500,   500,
        500,   500,   500,   500,   500,   500,   500,   500,
        500,   500,   500,   500,   500,   500,   500,   500,
        500,   500,   500,   500,   500,   500,   500,   500,
        500,   500,   500,   510,   510,   505,   500,   500
    };

    std::array<int, 64> bishop_array = {
        // Bishops slightly prefer center, avoid being trapped in corners
        310, 315, 315, 315, 315, 315, 315, 310,
        315, 325, 320, 320, 320, 320, 325, 315,
        315, 320, 325, 325, 325, 325, 320, 315,
        315, 320, 325, 330, 330, 325, 320, 315,
        315, 320, 325, 330, 330, 325, 320, 315,
        315, 325, 325, 325, 325, 325, 325, 315,
        315, 330, 320, 320, 320, 320, 330, 315,
        310, 315, 310, 315, 315, 310, 315, 310
    };

    std::array<int, 64> queen_array = {
        // Queen prefers center, avoid early development to edges
        880, 890, 890, 895, 895, 890, 890, 880,
        890, 900, 900, 900, 900, 900, 900, 890,
        890, 900, 905, 905, 905, 905, 900, 890,
        895, 900, 905, 910, 910, 905, 900, 895,
        895, 900, 905, 910, 910, 905, 900, 895,
        890, 900, 905, 905, 905, 905, 900, 890,
        890, 900, 900, 900, 900, 900, 900, 890,
        880, 890, 890, 895, 895, 890, 890, 880
    };

    std::array<int, 64> king_middlegame_array = {
        // King should castle and stay safe (corners/edges)
        // Heavy penalty for center exposure
        20000, 20050, 20030, 20000, 20000, 20030, 20050, 20000,
        20000, 20000, 20000, 20000, 20000, 20000, 20000, 20000,
        19980, 19980, 19980, 19980, 19980, 19980, 19980, 19980,
        19970, 19970, 19970, 19970, 19970, 19970, 19970, 19970,
        19960, 19960, 19960, 19960, 19960, 19960, 19960, 19960,
        19950, 19950, 19950, 19950, 19950, 19950, 19950, 19950,
        19950, 19950, 19950, 19950, 19950, 19950, 19950, 19950,
        19950, 19970, 19960, 19940, 19940, 19940, 19970, 19950
    };

    std::array<int, 64> king_endgame_array = {
        // In endgame, king should be active and centralized
        19970, 19980, 19985, 19990, 19990, 19985, 19980, 19970,
        19980, 19990, 19995, 20000, 20000, 19995, 19990, 19980,
        19985, 19995, 20005, 20010, 20010, 20005, 19995, 19985,
        19990, 20000, 20010, 20020, 20020, 20010, 20000, 19990,
        19990, 20000, 20010, 20020, 20020, 20010, 20000, 19990,
        19985, 19995, 20005, 20010, 20010, 20005, 19995, 19985,
        19980, 19990, 19995, 20000, 20000, 19995, 19990, 19980,
        19970, 19980, 19985, 19990, 19990, 19985, 19980, 19970
    };

    pst[static_cast<int>(PieceType::PAWN)] = pawn_array;
    pst[static_cast<int>(PieceType::KNIGHT)] = knight_array;
    pst[static_cast<int>(PieceType::BISHOP)] = bishop_array;
    pst[static_cast<int>(PieceType::ROOK)] = rook_array;
    pst[static_cast<int>(PieceType::QUEEN)] = queen_array;
    pst[static_cast<int>(PieceType::KING)] = king_middlegame_array;
    
    return pst;

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

int pop_lsb(Bitboard &bitboard)
{
    int square = std::countr_zero(bitboard);
    bitboard &= bitboard - 1;
    return square;
}
