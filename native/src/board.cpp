#include "../include/board.h"
#include "../include/utils.h"
#include <regex>
#include <iostream>
#include "board.h"
#include <sstream>

Board::Board() {
    sideToMove = Color::WHITE;
    castlingRightsState = static_cast<u_int8_t>(CastlingRights::ALL);
    enPassantSquare = std::nullopt;
    half_move_clock = 0;
    num_moves_total = 0;
    color_can_en_passant = Color::NONE;
    history_ply = 0;

    bitboard_array = {
        0x000000000000FF00,
        0x0000000000000042,
        0x0000000000000024,
        0x0000000000000081,
        0x0000000000000008,
        0x0000000000000010,
        0x00FF000000000000,
        0x4200000000000000,
        0x2400000000000000,
        0x8100000000000000,
        0x0800000000000000,
        0x1000000000000000
    };

    update_color_bitboard();
}

Bitboard Board::get_piece_bitboard(Piece piece) const
{
    return bitboard_array[piece];
}

Bitboard Board::get_piece_bitboard(PieceType type, Color color) const
{
    return bitboard_array[((int) type) + (color == Color::WHITE ? 0 : 1)];
}

void Board::set_position_fen(const std::string &fen)
{
    const auto parts = splitString(fen, ' ');

    if(parts.size() != 6){
        throw std::invalid_argument("Fen Should have 6 parts");
    }

    const std::string piece_placement = parts[0];
    const std::string side_to_move = parts[1];
    const std::string castling_rights = parts[2];
    const std::string en_passant_target_square = parts[3];
    const std::string half_move_clock = parts[4];
    const std::string num_moves_total = parts[5];

    //Piece placements
    parse_piece_placement(piece_placement);

    //Turn
    this->sideToMove = side_to_move == "w" ? Color::WHITE : Color::BLACK;

    //En Passant
    if(en_passant_target_square == "-") {
      enPassantSquare = std::nullopt;
    } else {
      enPassantSquare = squareIndexFromAlgebraicConst(en_passant_target_square);
    }

    //Castling Ability
    castlingRightsState = static_cast<uint8_t>(CastlingRights::NONE);
    castlingRightsState = castling_rights.find("K") != std::string::npos ? castlingRightsState | static_cast<uint8_t>(CastlingRights::WHITE_KINGSIDE) : castlingRightsState; 
    castlingRightsState = castling_rights.find("Q") != std::string::npos ? castlingRightsState | static_cast<uint8_t>(CastlingRights::WHITE_QUEENSIDE) : castlingRightsState; 
    castlingRightsState = castling_rights.find("k") != std::string::npos ? castlingRightsState | static_cast<uint8_t>(CastlingRights::BLACK_KINGSIDE) : castlingRightsState;  
    castlingRightsState = castling_rights.find("q") != std::string::npos ? castlingRightsState | static_cast<uint8_t>(CastlingRights::BLACK_QUEENSIDE) : castlingRightsState;    

    //Half Move Clock
    this->half_move_clock = std::stoi(half_move_clock);

    //Full Move Count
    this->num_moves_total = std::stoi(num_moves_total);

    update_color_bitboard();
}

//We assume a move passed into here is valid
void Board::make_move(Move& move) {
    Bitboard bitboard = bitboard_array[move.piece];

    move_history[history_ply++] = Move_State{
        move,
        move.captured_piece,
        enPassantSquare,
        castlingRightsState,
        move.promoted_piece != Piece::NONE
    };

    if(move.is_enpassant && move.captured_piece != Piece::NONE){
        int captured_pawn_square = (move.piece == Piece::W_PAWN)
            ? move.to_square - 8
            : move.to_square + 8;
        
        remove_captured_piece(captured_pawn_square, move.captured_piece);
    } else if (move.captured_piece != Piece::NONE) {
        remove_captured_piece(move.to_square, move.captured_piece);
    }

    //En Passant updates
    if(move.piece == Piece::W_PAWN && (move.from_square / 8 == 1) && (move.to_square / 8 == 3)){
        enPassantSquare = move.from_square + 8;
        color_can_en_passant = Color::BLACK;
    } else if (move.piece == Piece::B_PAWN && (move.from_square / 8 == 6) && (move.to_square / 8 == 4)){
        enPassantSquare = move.from_square - 8;
        color_can_en_passant = Color::WHITE;
    } else {
        enPassantSquare = std::nullopt;
        color_can_en_passant = Color::NONE;
    }

    //Castling Rights updates
    if(move.piece == W_KING){
        remove_all_castling_rights_white();
    } else if (move.piece == B_KING) {
        remove_all_castling_rights_black();
    } else if(move.piece == W_ROOK && move.from_square == 0){
        remove_castling_right(CastlingRights::WHITE_QUEENSIDE);
    }
    else if(move.piece == W_ROOK && move.from_square == 7){
        remove_castling_right(CastlingRights::WHITE_KINGSIDE);
    }
    else if(move.piece == B_ROOK && move.from_square == 56){
        remove_castling_right(CastlingRights::BLACK_QUEENSIDE);
    }
    else if(move.piece == B_ROOK && move.from_square == 63){
        remove_castling_right(CastlingRights::BLACK_KINGSIDE);
    }

    // castlingRightsState &= ~CASTLING_RIGHTS_MASK[move.from_square];
    // castlingRightsState &= ~CASTLING_RIGHTS_MASK[move.to_square];  // If rook captured

    //Castling
    if(move.is_castling){
        castle_move(move);
    }

    //Regular Logic & Promotion

    Bitboard startMask = 1ULL << move.from_square;
    Bitboard newBitboard = bitboard & (~startMask);

    Bitboard endMask = 1ULL << move.to_square;

    //Promotion
    if(move.promoted_piece != Piece::NONE){
        // Remove pawn from its bitboard (already done in newBitboard)
      bitboard_array[move.piece] = newBitboard; // update pawn board

      // Add the promoted piece to its bitboard
      Bitboard promoBitboard = bitboard_array[move.promoted_piece]; //We know its not none
      bitboard_array[move.promoted_piece] = promoBitboard | endMask;
    } else { //Normal Logic
        newBitboard = newBitboard | endMask;
        bitboard_array[move.piece] = newBitboard;
    }
    
    //Switch turn
    sideToMove = sideToMove == Color::WHITE ? Color::BLACK : Color::WHITE;

    update_color_bitboard();
}

void Board::undo_move() {
    if(history_ply == 0){
        throw std::invalid_argument("Tried to invoke undo_move when move_history was empty");
    }

    Move_State last = move_history[--history_ply];

    Move move = last.move;

    //Revert side
    sideToMove = sideToMove == Color::WHITE ? Color::BLACK : Color::WHITE;

    //Restore Flags
    enPassantSquare = last.enPassantSquare;
    castlingRightsState = last.castling_rights;

    //Undo Piece Movement
    bitboard_array[move.piece] &= ~(1ULL << move.to_square);
    bitboard_array[move.piece] |= 1ULL << move.from_square;

    //Restore capture
    if(last.captured_piece != Piece::NONE && !move.is_enpassant){
        bitboard_array[last.captured_piece] |= (1ULL << move.to_square);
    }

    //Undo special moves (castle, en passant, etc.)
    if(move.is_castling){
        if (move.to_square - move.from_square == 2) {
        // kingside
        int rookStart = colorOf(static_cast<Piece>(move.piece)) == Color::WHITE ? 7 : 63;
        int rookEnd = colorOf(static_cast<Piece>(move.piece)) == Color::WHITE  ? 5 : 61;
        
        undo_rook_castle(colorOf(static_cast<Piece>(move.piece)), rookStart, rookEnd);
      } else if (move.to_square - move.from_square == -2) {
        // queenside
        int rookStart = colorOf(static_cast<Piece>(move.piece)) == Color::WHITE ? 0 : 56;
        int rookEnd = colorOf(static_cast<Piece>(move.piece)) == Color::WHITE ? 3 : 59;

        undo_rook_castle(colorOf(static_cast<Piece>(move.piece)), rookStart, rookEnd);
      }
    }

    // Undo en passant
    if (move.is_enpassant && move.captured_piece != Piece::NONE) {
        int capturedPawnSquare = colorOf(static_cast<Piece>(move.piece)) == Color::WHITE
            ? move.to_square - 8
            : move.to_square + 8;
        Piece capturedPawn = colorOf(static_cast<Piece>(move.piece)) == Color::WHITE
            ? Piece::B_PAWN
            : Piece::W_PAWN;
        bitboard_array[move.captured_piece] |= (1ULL << capturedPawnSquare);
    }

    //undo promotion
    if (move.promoted_piece != Piece::NONE) {
      // Remove the promoted piece from the board
      bitboard_array[move.promoted_piece] &= ~(1ULL << move.to_square);
    }

    update_color_bitboard();
}

bool Board::is_in_check(Color color) {
    int kingSquare = get_king_square(color);
    return is_square_attacked(kingSquare, color == Color::WHITE ? Color::BLACK : Color::WHITE);
}

bool Board::can_castle(CastlingRights right) const {
    //if can castle 1 bit = 1, so non-zero = true, else all 0 = false
    return (castlingRightsState & static_cast<uint8_t>(right)) != 0; 
}

Piece Board::get_piece_at(int square) const
{
    if(square < 0 || square > 63){
        throw std::invalid_argument("Square must be between 0 and 63");  
    } 
    

    Bitboard squareMask = 1ULL << square;

    if((bitboard_array[W_PAWN] & squareMask) != 0) return Piece::W_PAWN;
    if((bitboard_array[W_KNIGHT] & squareMask) != 0) return Piece::W_KNIGHT;
    if((bitboard_array[W_BISHOP] & squareMask) != 0) return Piece::W_BISHOP;
    if((bitboard_array[W_ROOK] & squareMask) != 0) return Piece::W_ROOK;
    if((bitboard_array[W_QUEEN] & squareMask) != 0) return Piece::W_QUEEN;
    if((bitboard_array[W_KING] & squareMask) != 0) return Piece::W_KING;
    if((bitboard_array[B_PAWN] & squareMask) != 0) return Piece::B_PAWN;
    if((bitboard_array[B_KNIGHT] & squareMask) != 0) return Piece::B_KNIGHT;
    if((bitboard_array[B_BISHOP] & squareMask) != 0) return Piece::B_BISHOP;
    if((bitboard_array[B_ROOK] & squareMask) != 0) return Piece::B_ROOK;
    if((bitboard_array[B_QUEEN] & squareMask) != 0) return Piece::B_QUEEN;
    if((bitboard_array[B_KING] & squareMask) != 0) return Piece::B_KING;

    return Piece::NONE;
}

Bitboard Board::get_active_color_bb() const
{
    if(sideToMove == Color::WHITE) 
        return white_occupancy;
    else
        return black_occupancy;
}

Bitboard Board::get_empty_squares() const
{
    return ~(white_occupancy | black_occupancy);
}

void Board::update_color_bitboard()
{
    white_occupancy = bitboard_array[W_PAWN] | bitboard_array[W_KNIGHT] | bitboard_array[W_BISHOP] 
    | bitboard_array[W_ROOK] | bitboard_array[W_QUEEN] | bitboard_array[W_KING];

    black_occupancy =  bitboard_array[B_PAWN] | bitboard_array[B_KNIGHT] | bitboard_array[B_BISHOP] 
    | bitboard_array[B_ROOK] | bitboard_array[B_QUEEN] | bitboard_array[B_KING];
}

std::string Board::getFen()
{
    std::array<std::string,6> parts;

    // ---------- 1. Piece placement ----------
    parts[0] = generate_piece_placement_fen();

    // ---------- 2. Side to move ----------
    parts[1] = sideToMove == Color::WHITE  ? "w" : "b";

    // ---------- 3. Castling rights ----------
    parts[2] = "";
    if (can_castle(CastlingRights::WHITE_KINGSIDE))  parts[2] += "K";
    if (can_castle(CastlingRights::WHITE_QUEENSIDE)) parts[2] += "Q";
    if (can_castle(CastlingRights::BLACK_KINGSIDE))  parts[2] += "k";
    if (can_castle(CastlingRights::BLACK_QUEENSIDE)) parts[2] += "q";
    if (parts[2].empty()) parts[2] = "-";

    // ---------- 4. En passant ----------
    parts[3] = enPassantSquare.has_value() ? index_to_square(enPassantSquare.value()) : "-";

    // ---------- 5. Halfmove clock ----------
    parts[4] = std::to_string(half_move_clock);

    // ---------- 6. Fullmove number ----------
    parts[5] = std::to_string(num_moves_total);

    // Join all parts with spaces
    std::ostringstream fen;
    for (int i = 0; i < 6; ++i) {
        if (i > 0) fen << " ";
        fen << parts[i];
    }

    return fen.str();
}

std::string Board::generate_piece_placement_fen()
{
    std::ostringstream buffer;

    // Loop ranks from 8 down to 1 (FEN goes top → bottom)
    for(int rank = 7; rank >= 0; --rank) {
        int emptyCount = 0;

        for(int file = 0; file < 8; ++file) {
            int square = rank * 8 + file;
            Piece piece = get_piece_at(square);

            if(piece == Piece::NONE) {
                emptyCount++;
            } else {
                if(emptyCount > 0) {
                    buffer << emptyCount;
                    emptyCount = 0;
                }
                buffer << pieceToChar[piece];
            }
        }

        // Write trailing empties if any
        if(emptyCount > 0) buffer << emptyCount;

        if(rank > 0) buffer << '/';
    }

    return buffer.str();
}

std::string Board::index_to_square(int index)
{
    int rank = index / 8;       // integer division
    int file = index % 8;

    char fileChar = 'a' + file; // 'a'..'h'
    char rankChar = '1' + rank; // '1'..'8'

    return std::string() + fileChar + rankChar;
}

void Board::print_board(std::ostream& os) const {
    for(int rank = 7; rank >=0; rank--){
        for(int file = 0; file <= 7; file++){
            int square = rank * 8 + file;
            std::string s = ".";
            for(int i = 0; i < 12; i++){
                Bitboard board = bitboard_array[i];
                Bitboard mask = 1ULL << square; //specify 64 bit int
                bool hasPiece = (board & mask) != 0;

                if(hasPiece){
                    s = pieceToChar[i];
                    break;
                }
            }

            os << s << " ";
        }

        os << std::endl;
    }
}

void Board::remove_castling_right(CastlingRights right) {
    castlingRightsState &= ~static_cast<uint8_t>(right);
}

void Board::remove_all_castling_rights_white() {
    remove_castling_right(CastlingRights::WHITE_ALL);
}

void Board::remove_all_castling_rights_black(){
    remove_castling_right(CastlingRights::BLACK_ALL);
}

void Board::parse_piece_placement(const std::string& positions) {
    //Clear all bitboards
    for(int i = 0; i < 12; i++){
        bitboard_array[i] = 0;
    }

    const auto ranks = splitString(positions, '/');

    if(ranks.size() != 8){
        throw std::invalid_argument("Invalid FEN: should have 8 ranks");
    }

    int squareIndex = 56; // a8 (top-left), descending down to a1
    std::regex wordPattern("r'[1-8]'");


    for(const auto& rank : ranks){
        for(const char& c : rank){
            if(c >= '1' && c<= '8'){
                squareIndex += c - '0';
            } else {
                Piece piece = charToPiece[c];
                bitboard_array[piece] = bitboard_array[piece] | (1ULL << squareIndex);
                squareIndex++; //go to next A1->A2
            }
        }
        squareIndex -= 16; //Go down a rank and back to file 1
    }
}

void Board::set_castling_rights(uint8_t& newCastlingRights){
    this->castlingRightsState = newCastlingRights;
}

void Board::undo_rook_castle(Color color, int start, int end) {
    if(color == Color::WHITE){
        bitboard_array[W_ROOK] &= ~(1ULL << end);
        bitboard_array[W_ROOK] |= (1ULL << start);
    } else {
        bitboard_array[B_ROOK] &= ~(1ULL << end);
        bitboard_array[B_ROOK] |= (1ULL << start);
    }
}

void Board::remove_captured_piece(int square, Piece capturedPiece)
{
    bitboard_array[capturedPiece] &= ~(1ULL << square);
    if(capturedPiece == Piece::W_ROOK && square == 7) remove_castling_right(CastlingRights::WHITE_KINGSIDE);
    if(capturedPiece == Piece::W_ROOK && square == 0) remove_castling_right(CastlingRights::WHITE_QUEENSIDE);
    if(capturedPiece == Piece::B_ROOK && square == 56) remove_castling_right(CastlingRights::BLACK_QUEENSIDE);
    if(capturedPiece == Piece::B_ROOK && square == 63) remove_castling_right(CastlingRights::BLACK_KINGSIDE);
}

void Board::castle_move(Move &king_move)
{
    int rookStart, rookEnd;
    if(king_move.to_square - king_move.from_square == 2){
        //Kingside
        rookStart = king_move.from_square + 3; // rook originally on h-file
        rookEnd = king_move.from_square + 1;   // rook moves next to king
    } else if (king_move.to_square - king_move.from_square == -2) {
      // Queen-side castling
      rookStart = king_move.from_square - 4; // rook originally on a-file
      rookEnd = king_move.from_square - 1;   // rook moves next to king
    } else {
        std::cout << "FEN: " << getFen() << std::endl;
        throw std::invalid_argument("Invalid castling move");
    }

    // Determine which rook piece
    Piece rookPiece = colorOf(static_cast<Piece>(king_move.piece)) == Color::WHITE ? Piece::W_ROOK : Piece::B_ROOK;

    // Move the rook on the bitboard
    bitboard_array[rookPiece] &= ~(1ULL << rookStart); //remove from start
    bitboard_array[rookPiece] |= (1ULL << rookEnd); //add to end
}

bool Board::is_square_attacked(int target, Color attacking_color) const
{
    if(target < 0 || target > 63) std::cerr << "is_square_atatcked: Target must be between 0 and 63" << std::endl;

    Bitboard occupied = 0;
    for(int i = 0; i < 12; i++){
        occupied |= bitboard_array[i];
    }

    //Pawns
    if(attacking_color == Color::WHITE){
        if((pawn_attacks[target+64] & bitboard_array[W_PAWN]) != 0) return true;
    } else {
        if((pawn_attacks[target] & bitboard_array[B_PAWN]) != 0) return true;
    }

    // Knights
    if ((knight_moves[target] &
        (attacking_color == Color::WHITE
            ? bitboard_array[W_KNIGHT]
            : bitboard_array[B_KNIGHT])) != 0) {
      return true;
    }

    // Kings
    if ((king_moves[target] &
        (attacking_color == Color::WHITE
            ? bitboard_array[W_KING]
            : bitboard_array[B_KING])) != 0){
      return true;
    }

    // Bishops / Queens (diagonals)
    Bitboard bishopLike = get_bishop_attacks(target, occupied) &
        ((attacking_color == Color::WHITE
            ? (bitboard_array[W_BISHOP] | bitboard_array[W_QUEEN])
            : (bitboard_array[B_BISHOP] | bitboard_array[B_QUEEN])));

    if (bishopLike != 0ULL){
      return true;
    } 

    // Rooks / Queens (files + ranks)
    Bitboard rookLike = get_rook_attacks(target, occupied) &
        ((attacking_color == Color::WHITE
            ? (bitboard_array[W_ROOK] | bitboard_array[W_QUEEN])
            : (bitboard_array[B_ROOK] | bitboard_array[B_QUEEN])));

    if (rookLike != 0ULL){
       return true;
    }

    return false;
}

int Board::get_king_square(Color color)
{
    if(color == Color::WHITE){
      return std::countr_zero(bitboard_array[W_KING]);
    } else {
      return std::countr_zero(bitboard_array[B_KING]);
    }
}
