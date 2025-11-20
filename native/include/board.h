#pragma once
#include <cstdint>
#include <string>
#include <optional>
#include <array>
#include <stack>

typedef uint64_t Bitboard;

class BoardTestFixture;

enum class PieceType : uint8_t {
    PAWN, //0
    KNIGHT, //1
    BISHOP, //2
    ROOK, //3
    QUEEN, //4
    KING //5
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
};

inline PieceType typeOf(Piece p) {
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
    uint8_t piece;
    uint8_t from_square;
    uint8_t to_square;
    std::optional<Piece> captured_piece;
    std::optional<Piece> promoted_piece;
    bool is_enpassant;
    bool is_castling;
};

struct Move_State {
    Move move;
    std::optional<uint8_t> captured_piece;
    std::optional<int> enPassantSquare;
    uint8_t castling_rights;
    bool wasPromotion; 
};

class Board {
    friend class BoardTestFixture;

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

        Bitboard white_occupancy;
        Bitboard black_occupancy;

        Color color_can_en_passant;

        Board();

        Bitboard get_piece_bitboard(Piece piece) const;
        Bitboard get_piece_bitboard(PieceType type, Color color) const;
        void set_position_fen(const std::string& fen);
        void make_move(Move& move);
        void undo_move();
        bool is_in_check(Color color);
        bool can_castle(CastlingRights right) const;
        std::optional<Piece> get_piece_at(int square) const;
        Bitboard get_active_color_bb() const;
        Bitboard get_empty_squares() const;
        bool is_square_attacked(int square, Color attacking_color) const;
        std::string getFen();

        void print_board(std::ostream& os) const;

    private:
        std::array<uint64_t, 12> bitboard_array;

        Move_State move_history[2048];
        int history_ply;

        //Castling stuff
        void remove_castling_right(CastlingRights right);
        void remove_all_castling_rights_white();
        void remove_all_castling_rights_black();

        void parse_piece_placement(const std::string& positions);
        void set_castling_rights(uint8_t&newCastlingRights);
        void undo_rook_castle(Color color, int start, int end);
        void remove_captured_piece(int square, Piece captured_piece);
        void castle_move(Move& king_move);

        int get_king_square(Color color);

        void update_color_bitboard();

        std::string generate_piece_placement_fen();
        std::string index_to_square(int index);

        
};

constexpr int squareIndexFromAlgebraicConst(const std::string notation) {
    return (notation[1] - '1') * 8 + (notation[0] - 'a');
}

constexpr Bitboard RANK3 = 0xFFULL << 16; // Value: 0x0000000000FF0000ULL
constexpr Bitboard RANK7 = 0xFFULL << 48; // Value: 0x00FF000000000000ULL
