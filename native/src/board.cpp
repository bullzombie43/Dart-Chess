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

//We assume a move passed into here is valid
void Board::make_move(Move &move) {
    Bitboard bitboard = bitboard_array[move.piece];
    
    move_history.emplace(Move_State{
        move,
        move.captured_piece,
        enPassantSquare,
        castlingRightsState,
        move.promoted_piece != Piece::NONE
    });

    if(move.is_enpassant && move.captured_piece != Piece::NONE){
        int captured_pawn_square = (move.piece == Piece::W_PAWN)
            ? move.to_square - 8
            : move.to_square + 8;
        
        //remove captured piece
    } else if (move.captured_piece) {
        //remove captured piece
    }

    //En Passant updates
    if(move.piece == Piece::W_PAWN && (move.from_square / 8 == 1) && (move.to_square / 8 == 3)){
        enPassantSquare = move.to_square + 8;
    } else if (move.piece == Piece::B_PAWN && (move.from_square / 8 == 6) && (move.to_square / 8 == 4)){
        enPassantSquare = move.to_square - 8;
    } else {
        enPassantSquare = std::nullopt;
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

    //Castling
    if(move.is_castling){
        //Castle Helper Function
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
}

void Board::undo_move(Move &move) {
    Move_State last = move_history.top();
    move_history.pop();

    Move& move = last.move;

    //Revert side
    sideToMove = sideToMove == Color::WHITE ? Color::BLACK : Color::WHITE;

    //Restore Flags
    enPassantSquare = last.enPassantSquare;
    castlingRightsState = last.castling_rights;

    //Undo Piece Movement
    bitboard_array[move.piece] &= ~(1ULL << move.to_square);
    bitboard_array[move.piece] |= 1ULL << move.from_square;

    //Restore capture
    if(last.captured_piece.has_value()){
        bitboard_array[last.captured_piece.value()] |= (1ULL << move.to_square);
    }

    //Undo special moves (castle, en passant, etc.)
    if(move.is_castling){
        if (move.to_square - move.from_square == 2) {
        // kingside
        int rookStart = colorOf(static_cast<Piece>(move.piece)) == Color::WHITE ? 7 : 63;
        int rookEnd = colorOf(static_cast<Piece>(move.piece)) == Color::WHITE  ? 5 : 61;
        
        //undoRookForCastle(movePiece(move).color, rookStart, rookEnd);
      } else if (move.to_square - move.from_square == -2) {
        // queenside
        int rookStart = colorOf(static_cast<Piece>(move.piece)) == Color::WHITE ? 0 : 56;
        int rookEnd = colorOf(static_cast<Piece>(move.piece)) == Color::WHITE ? 3 : 59;

        //undoRookForCastle(movePiece(move).color, rookStart, rookEnd);
      }
    }

    // Undo en passant
    if (move.is_enpassant && move.captured_piece != Piece::NONE) {
        int capturedPawnSquare = colorOf(static_cast<Piece>(move.piece)) == Color::WHITE
            ? move.from_square - 8
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

      // Restore the pawn on its starting square
      bitboard_array[move.piece] |= (1ULL << move.from_square);
    }
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

void Board::set_castling_rights(u_int8_t& newCastlingRights){
    this->castlingRightsState = newCastlingRights;
}
