#include <cstdint>
#include <string>

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

enum class Color : uint8_t {
    NONE, //0
    WHITE, //1
    BLACK //2
};

struct Piece{
    PieceType type;
    Color color;
};

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
        Color sideToMove;
        uint8_t castlingRights; //We only need 4 bits but I don't want to make a custom nybble type
        int enPassantSquare;

        Bitboard white_pawns;
        Bitboard white_knights;
        Bitboard white_bishops;
        Bitboard white_rooks;
        Bitboard white_queens;
        Bitboard white_kings;
        Bitboard black_pawns;
        Bitboard black_knights;
        Bitboard black_bishops;
        Bitboard black_rooks;
        Bitboard black_queens;
        Bitboard black_kings;

        Bitboard white_pieces;
        Bitboard black_pieces;
        Bitboard all_pieces;

        void set_positions(const std::string& fen);
        void make_move(Move& move);
        void undo_move(Move& move);
        bool is_in_check();

        Board();
    private:
        int x;
};