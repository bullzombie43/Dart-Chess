#pragma once
#include <cstdint>
#include <string>
#include <optional>

typedef uint64_t Bitboard;

enum class PieceType : uint8_t {
    NONE, //0
    PAWN, //1
    KNIGHT, //2
    BISHOP, //3
    ROOK, //4
    QUEEN, //5
    KING //6
};

enum Piece : uint8_t{
    W_PAWN = 0,
    W_KNIGHT,
    W_BISHOP,
    W_ROOK,
    W_QUEEN,
    W_KING,
    B_PAWN,
    B_KNIGHT,
    B_BISHOP,
    B_ROOK,
    B_QUEEN,
    B_KING,
    NONE , //12
};

inline PieceType typeOf(Piece p) {
    if (p == Piece::NONE) return PieceType::NONE;
    return PieceType((p - 1) % 6 + 1);
}

enum class Color : uint8_t {
    NONE, //0
    WHITE, //1
    BLACK //2
};

inline Color colorOf(Piece p) {
    if (p >= W_PAWN && p <= W_KING) return Color::WHITE;
    if (p >= B_PAWN && p <= B_KING) return Color::BLACK;
    return Color::NONE; // default for NO_PIECE
};

enum class CastlingRights : uint8_t {
    NONE = 0,
    WHITE_KINGSIDE = 1 << 0, //00000001
    WHITE_QUEENSIDE = 1 << 1, //00000010
    BLACK_KINGSIDE = 1 << 2, //00000100
    BLACK_QUEENSIDE = 1 << 3, //00001000
    WHITE_ALL = WHITE_KINGSIDE | WHITE_QUEENSIDE,
    BLACK_ALL = BLACK_KINGSIDE | BLACK_QUEENSIDE,
    ALL = WHITE_ALL | BLACK_ALL
};

//These constexpr as so we can use bit operations on Castling rights, just convenient
constexpr CastlingRights operator|(CastlingRights a, CastlingRights b){
    return static_cast<CastlingRights>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b)); 
}

constexpr CastlingRights operator&(CastlingRights a, CastlingRights b){
    return static_cast<CastlingRights>(static_cast<uint8_t>(a) & static_cast<uint8_t>(b)); 
}

//TODO: Consider switching to a uint16_t like in the dart implementation, but get this working first
struct Move {
    uint8_t from_square;
    uint8_t to_square;
    PieceType captured_piece_type;
    PieceType promoted_piece_type;
    bool is_enpassant;
    bool is_castling;
};

class Board {
    public:
        //Constants for squares for readability
        enum Squares {
            A1 = 0, B1, C1, D1, E1, F1, G1, H1,
            A2, B2, C2, D2, E2, F2, G2, H2,
            A3, B3, C3, D3, E3, F3, G3, H3,
            A4, B4, C4, D4, E4, F4, G4, H4,
            A5, B5, C5, D5, E5, F5, G5, H5,
            A6, B6, C6, D6, E6, F6, G6, H6,
            A7, B7, C7, D7, E7, F7, G7, H7,
            A8, B8, C8, D8, E8, F8, G8, H8 //63
        };

        Color sideToMove;
        uint8_t castlingRightsState; //only really need 4 bits but whatever
        std::optional<int> enPassantSquare;
        int half_move_clock;
        int num_moves_total;

        Board();

        Bitboard get_piece_bitboard(Piece piece) const;
        void set_position_fen(const std::string& fen);
        void make_move(Move& move);
        void undo_move(Move& move);
        bool is_in_check();
        bool can_castle(CastlingRights right);

        void print_board(std::ostream& os) const;

    private:
        uint64_t* bitboard_array;

        Bitboard white_pieces;
        Bitboard black_pieces;
        Bitboard all_pieces;

        //Castling stuff
        void remove_castling_right(CastlingRights right);
        void remove_all_castling_rights_white();
        void remove_all_castling_rights_black();

        void parse_piece_placement(const std::string& positions);


};

constexpr int squareIndexFromAlgebraicConst(const std::string notation) {
    return (notation[1] - '1') * 8 + (notation[0] - 'a');
}