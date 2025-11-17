#include "../include/board.h"
#include "../include/utils.h"
#include <regex>
#include <iostream>

Board::Board() {
    sideToMove = Color::WHITE;
    castlingRightsState = static_cast<u_int8_t>(CastlingRights::ALL);

    bitboard_array = new uint64_t[12];

    bitboard_array[W_PAWN] = 0x000000000000FF00;
    bitboard_array[W_KNIGHT] = 0x0000000000000042;
    bitboard_array[W_BISHOP] = 0x0000000000000024;
    bitboard_array[W_ROOK] = 0x0000000000000081;
    bitboard_array[W_QUEEN] = 0x0000000000000008;
    bitboard_array[W_KING] = 0x0000000000000010;
    bitboard_array[B_PAWN] = 0x00FF000000000000;
    bitboard_array[B_KNIGHT] = 0x4200000000000000;
    bitboard_array[B_BISHOP] = 0x2400000000000000;
    bitboard_array[B_ROOK] = 0x8100000000000000;
    bitboard_array[B_QUEEN] = 0x0800000000000000;
    bitboard_array[B_KING] = 0x1000000000000000;
}

void Board::set_position_fen(const std::string &fen) {
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
    castlingRightsState = castling_rights.find("K") ? castlingRightsState | static_cast<uint8_t>(CastlingRights::WHITE_KINGSIDE) : castlingRightsState; 
    castlingRightsState = castling_rights.find("Q") ? castlingRightsState | static_cast<uint8_t>(CastlingRights::WHITE_QUEENSIDE) : castlingRightsState; 
    castlingRightsState = castling_rights.find("k") ? castlingRightsState | static_cast<uint8_t>(CastlingRights::BLACK_KINGSIDE) : castlingRightsState;  
    castlingRightsState = castling_rights.find("q") ? castlingRightsState | static_cast<uint8_t>(CastlingRights::BLACK_QUEENSIDE) : castlingRightsState;    

    //Half Move Clock
    this->half_move_clock = std::stoi(half_move_clock);

    //Full Move Count
    this->num_moves_total = std::stoi(num_moves_total);
}

void Board::make_move(Move &move)
{

}

void Board::undo_move(Move &move)
{
}

bool Board::is_in_check() {
    return false;
}

bool Board::can_castle(CastlingRights right) {
    //if can castle 1 bit = 1, so non-zero = true, else all 0 = false
    return castlingRightsState & static_cast<uint8_t>(right); 
}

void Board::print_board(std::ostream& os) const {
    for(int rank = 7; rank >=0; rank--){
        for(int file = 0; file <= 7; file++){
            int square = rank * 8 + file;
            std::string s = ".";
            for(int i = 0; i < 12; i++){
                Bitboard& board = bitboard_array[i];
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

}

void Board::remove_all_castling_rights_white() {

}

void Board::remove_all_castling_rights_black()
{
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
                if(piece != Piece::NONE){
                    bitboard_array[piece] = bitboard_array[piece] | (1ULL << squareIndex);
                }
                squareIndex++; //go to next A1->A2
            }
        }
        squareIndex -= 16; //Go down a rank and back to file 1
    }
}

