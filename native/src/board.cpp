#include "../include/board.h"
#include "../include/utils.h"
#include <regex>
#include <iostream>
#include "board.h"

Board::Board() {
    sideToMove = Color::WHITE;
    castlingRightsState = static_cast<u_int8_t>(CastlingRights::ALL);
    enPassantSquare = std::nullopt;
    half_move_clock = 0;
    num_moves_total = 0;

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
}

Board::~Board(){}

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
                if(piece != Piece::NONE){
                    bitboard_array[piece] = bitboard_array[piece] | (1ULL << squareIndex);
                }
                squareIndex++; //go to next A1->A2
            }
        }
        squareIndex -= 16; //Go down a rank and back to file 1
    }
}

