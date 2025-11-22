#include "../include/engine.h"
#include <ranges> 
#include "utils.h"
#include "engine.h"
#include <iostream>
#include "engine.h"

int Engine::generate_psuedo_legal_moves(const Board &board, Move* moves)
{
    int move_count = 0;
    Color color = board.sideToMove;
    
    // Determine the range of pieces to check (W_PAWN to W_KING or B_PAWN to B_KING)
    int start_piece = (color == Color::WHITE) ? Piece::W_PAWN : Piece::B_PAWN;
    int end_piece = (color == Color::WHITE) ? Piece::W_KING : Piece::B_KING;

    generate_pawn_moves(board, moves, move_count);

    for (int p_idx = start_piece; p_idx <= end_piece; ++p_idx) {
        Piece piece = static_cast<Piece>(p_idx);

        if(piece == B_PAWN || piece == W_PAWN) continue;
        
        // 1. Get the Bitboard for the current piece type
        Bitboard pieces_bb = board.get_piece_bitboard(piece);
        
        // 2. Iterate over every square (bit) set in the Bitboard
        // This loop replaces your old "for square = 0 to 63"
        while (pieces_bb) {
            // Extracts the least significant set bit's index (the piece's square)
            int from_square = __builtin_ctzll(pieces_bb); 

            // 3. Call the specialized helper to generate moves from this square
            generate_moves_from_square(board, piece, from_square, moves, move_count);

            // 4. Clear the bit we just processed
            pieces_bb &= (pieces_bb - 1);
        }
    }
    
    return move_count;
}

int Engine::generate_legal_moves(Board &board, Move* moves)
{
    int psuedo_count = generate_psuedo_legal_moves(board, moves);

    Move* out = moves;         // write pointer for legal moves
    Move* in  = moves;         // read pointer for pseudo-legal moves

    for(int i = 0; i < psuedo_count; i++, in++){
        board.make_move(*in); // read and make the move in psuedo-legal

        //increment until we find a legal move basically
        if(!board.is_in_check(board.sideToMove == Color::WHITE ? Color::BLACK : Color::WHITE)){
            *out = *in; //overwrites with the next legal move
            out++; //by the end the front n moves are all the legal moves, and we return n
                    //that way we when we loop over we only look at the front n moves, aka the legal moves
        }

        board.undo_move();
    }

    return static_cast<int>(out - moves); //out starts at start of moves and goes to last move, so the difference is # of legal moves
}

uint64_t Engine::perft(Board &board, int depth) {
    if (depth == 0) return 1;

    Move move_list[MAX_NUMBER_OF_MOVES];
    int n_moves = generate_legal_moves(board, move_list);

    if (depth == 1) return n_moves;

    uint64_t nodes = 0;
    for (int i = 0; i < n_moves; i++) {
        board.make_move(move_list[i]);
        nodes += perft(board, depth - 1);
        board.undo_move();
    }

    return nodes;
}

uint64_t Engine::perft_divide(Board &board, int depth) {
    Move move_list[MAX_NUMBER_OF_MOVES];
    int n_moves = generate_legal_moves(board, move_list);

    uint64_t total = 0;
    for (int i = 0; i < n_moves; i++) {
        board.make_move(move_list[i]);
        uint64_t nodes = perft(board, depth - 1);
        if(move_to_string(move_list[i]) == "f3c3"){
            std::cout << "Move From: " << (int) move_list[i].from_square << " Move To: " << (int) move_list[i].to_square << std::endl;
            board.print_board(std::cout);
        }
        board.undo_move();

        std::cout << move_to_string(move_list[i]) << ": " << nodes << std::endl;
        total += nodes;
    }
    
    std::cout << "\nTotal: " << total << std::endl;
    return total;
}

void Engine::generate_moves_from_square(const Board &board, Piece piece, uint8_t index, Move* moves, int& move_count)
{
    if(piece == Piece::W_KNIGHT || piece == Piece::B_KNIGHT){
        generate_knight_moves(board, piece, index, moves, move_count);
    } else if(piece == Piece::W_KING || piece == Piece::B_KING){
        generate_king_moves(board, piece, index, moves, move_count);
    } else {
        generate_sliding_moves(board, piece, index, moves, move_count);
    }
}

void Engine::generate_sliding_moves(const Board &board, Piece piece, uint8_t index, Move* moves, int& move_count)
{
    //Implemet Shift and Mask Approach later for faster generation
    int startDirIndex = piece == Piece::B_BISHOP || piece == Piece::W_BISHOP ? 4 : 0;
    int endDirIndex = piece == Piece::B_ROOK || piece == Piece::W_ROOK ? 4 : 8;

    const Bitboard same_color = colorOf(piece) == Color::WHITE ? board.white_occupancy : board.black_occupancy;
    const Bitboard opp_color = colorOf(piece) == Color::WHITE ? board.black_occupancy : board.white_occupancy;

    for(int directionIndex = startDirIndex; directionIndex < endDirIndex; directionIndex++){
      for(int n = 0; n < num_squares_to_edge[index][directionIndex]; n++){
        int targetSquare = index + direction_offsets[directionIndex] * (n+1);

        //This prolly unnecessary caus num_squares_to_edge
        if(targetSquare < 0 || targetSquare > 63) break;

        const Bitboard target_bit = 1ULL << targetSquare;
        
        if((same_color & target_bit) != 0ULL){
            break;
        }

        if((opp_color & target_bit) != 0ULL){
            auto captured_piece = board.get_piece_at(targetSquare);
            // Construct the Move struct and add it to the list
            moves[move_count++] = Move{
                (uint8_t)piece,          
                (uint8_t)index,     
                (uint8_t)targetSquare,    
                captured_piece,                 
                Piece::NONE,             
                false,                    
                false                    
            };
            break;
        }

        // Construct the Move struct and add it to the list
        moves[move_count++] = Move{
            (uint8_t)piece,          
            (uint8_t)index,     
            (uint8_t)targetSquare,    
            Piece::NONE,                 
            Piece::NONE,             
            false,                    
            false                    
        };
      }
    }
}

void Engine::generate_pawn_moves(const Board &board, Move* moves, int& move_count)
{
    Color us = board.sideToMove;
    Color them = us == Color::WHITE ? Color::BLACK : Color::WHITE;

    Piece our_piece = us == Color::WHITE ? Piece::W_PAWN : Piece::B_PAWN;

    // Direction constants
    const int UP = (us == Color::WHITE) ? 8 : -8;
    const int UP_RIGHT = (us == Color::WHITE) ? 9 : -9;
    const int UP_LEFT = (us == Color::WHITE) ? 7 : -7;

    //Bitboards
    Bitboard our_pawns = board.get_piece_bitboard(our_piece);
    Bitboard empty = ~(board.white_occupancy | board.black_occupancy);
    Bitboard enemies = us == Color::WHITE ? board.black_occupancy : board.white_occupancy;

    Bitboard pawns_not_on_7th = our_pawns & ~(us == Color::WHITE ? RANK7 : RANK2);
    Bitboard pawns_on_7th = our_pawns & (us == Color::WHITE ? RANK7 : RANK2);

    //Single and double push
    Bitboard single_push = shift(pawns_not_on_7th, UP) & empty;
    extract_pawn_push(single_push, our_piece, UP, moves, move_count);

    Bitboard double_pawns = single_push & (us == Color::WHITE ? RANK3 : RANK6);
    Bitboard double_push = shift(double_pawns, UP) & empty;
    extract_pawn_push(double_push, our_piece, UP*2, moves, move_count);

    //Captures right (excluding H file for white, A file for black)
    Bitboard capture_right_exclude = (us == Color::WHITE) ? ~H_FILE : ~A_FILE;
    Bitboard capture_right = shift(pawns_not_on_7th & capture_right_exclude, UP_RIGHT) & enemies;
    extract_pawn_capture(capture_right, our_piece, UP_RIGHT, moves, move_count, board);

    // Captures left (excluding A file for white, H file for black)
    Bitboard capture_left_exclude = (us == Color::WHITE) ? ~A_FILE : ~H_FILE;
    Bitboard capture_left = shift(pawns_not_on_7th & capture_left_exclude, UP_LEFT) & enemies;
    extract_pawn_capture(capture_left, our_piece, UP_LEFT, moves, move_count, board);

    if(board.enPassantSquare.has_value()){
        Bitboard ep_target = 1ULL << board.enPassantSquare.value();

        Bitboard ep_right = shift(pawns_not_on_7th & capture_right_exclude, UP_RIGHT) & ep_target;

        if (ep_right) {
            int to = board.enPassantSquare.value();
            int from = to - UP_RIGHT;
            Piece captured = (us == Color::WHITE) ? Piece::B_PAWN : Piece::W_PAWN;
            moves[move_count++] = Move{our_piece, (uint8_t) from, (uint8_t) to, captured, Piece::NONE, true, false};
        }

        Bitboard ep_left = shift(pawns_not_on_7th & capture_left_exclude, UP_LEFT) & ep_target;

        if (ep_left) {
            int to = board.enPassantSquare.value();
            int from = to - UP_LEFT;
            Piece captured = (us == Color::WHITE) ? Piece::B_PAWN : Piece::W_PAWN;
            moves[move_count++] = Move{our_piece, (uint8_t) from, (uint8_t) to, captured, Piece::NONE, true, false};
        }
    }

    //PROMOTION

    //Pushes
    Bitboard promo_push = shift(pawns_on_7th, UP) & empty;
    extract_promotion_push(promo_push, our_piece, UP, moves, move_count);

     // Promotion captures right
    Bitboard promo_cap_right = shift(pawns_on_7th & capture_right_exclude, UP_RIGHT) & enemies;
    extract_promotion_capture(promo_cap_right, our_piece , UP_RIGHT, moves, move_count, board);
    
    // Promotion captures left
    Bitboard promo_cap_left = shift(pawns_on_7th & capture_left_exclude, UP_LEFT) & enemies;
    extract_promotion_capture(promo_cap_left, our_piece, UP_LEFT, moves, move_count, board);

}

void Engine::generate_knight_moves(const Board &board, Piece piece, uint8_t index, Move* moves, int& move_count)
{
    int startDirIndex = 8;
    int endDirIndex = 16;

    for(int directionIndex = startDirIndex; directionIndex < endDirIndex; directionIndex++){
      int targetSquare = index + direction_offsets[directionIndex];

      if(targetSquare < 0 || targetSquare > 63) continue;

      // Check file wrapping
      int fileDiff = (targetSquare % 8) - (index % 8);
      if (std::abs(fileDiff) > 2) continue; // illegal wrap

      Piece target_piece = board.get_piece_at(targetSquare);

      if(target_piece != Piece::NONE && colorOf(target_piece) == colorOf(piece)) continue;

      moves[move_count++] = Move{piece, index, (uint8_t) targetSquare, target_piece, Piece::NONE, false, false};
     }
}

void Engine::generate_king_moves(const Board &board, Piece piece, uint8_t index, Move* moves, int& move_count)
{    for(int directionIndex = 0; directionIndex < 8; directionIndex++){
      int targetSquare = index + direction_offsets[directionIndex];

      if(targetSquare < 0 || targetSquare > 63){
        continue;
      }

      // Check file wrapping
      int fileDiff = (targetSquare % 8) - (index % 8);
      if (std::abs(fileDiff) > 1) continue; // illegal wrap
      
      Piece target_piece = board.get_piece_at(targetSquare);

      if(target_piece != Piece::NONE && colorOf(target_piece) == colorOf(piece)) continue;

      moves[move_count++] = Move{piece, index, (uint8_t)targetSquare, target_piece, Piece::NONE, false, false};
    }

    //Add in castle moves
    generate_castle_moves(board, piece, index, moves,move_count);
}

void Engine::generate_castle_moves(const Board &board, Piece piece, uint8_t index, Move* moves, int& move_count)
{    struct CastleInfo {
        CastlingRights right;
        int rookSquare;
        int kingFrom;
        int kingTo;
        std::array<int,3> emptySquares;  // squares that must be empty
        std::array<int,3> safeSquares;   // squares that must not be attacked
    };

    static const CastleInfo castleData[] = {
        // ---- WHITE KING SIDE ----
        { CastlingRights::WHITE_KINGSIDE, 7, 4, 6, {5,6,-1}, {4,5,6} },

        // ---- WHITE QUEEN SIDE ----
        { CastlingRights::WHITE_QUEENSIDE, 0, 4, 2, {1,2,3}, {4,3,2} },

        // ---- BLACK KING SIDE ----
        { CastlingRights::BLACK_KINGSIDE, 63, 60, 62, {61,62,-1}, {60,61,62} },

        // ---- BLACK QUEEN SIDE ----
        { CastlingRights::BLACK_QUEENSIDE, 56, 60, 58, {57,58,59}, {60,59,58} }
    };

    Color color = colorOf(piece);
    Color opponent = (color == Color::WHITE ? Color::BLACK : Color::WHITE);

    for (const auto& cs : castleData) {

        // Determine the color of this castling right
        Color castleColor = (cs.right == CastlingRights::WHITE_KINGSIDE || 
                            cs.right == CastlingRights::WHITE_QUEENSIDE) 
                            ? Color::WHITE : Color::BLACK;

        // Skip if colors don't match
        if (castleColor != color) continue;


        // 1. Check castling right flag
        if (!board.can_castle(cs.right)) {
            continue;
        }

        // 2. Rook must still be there
        auto rook = board.get_piece_at(cs.rookSquare);
        if (rook == Piece::NONE) {
            continue;
        }

        // 3. Squares must be empty
        bool empty_ok = true;
        for (int sq : cs.emptySquares) {
            if (sq != -1 && board.get_piece_at(sq) != Piece::NONE) {
                empty_ok = false;
                break;
            }
        }
        if (!empty_ok) continue;

        // 4. King cannot pass through attacked squares
        bool safe_ok = true;
        for (int sq : cs.safeSquares) {
            if (sq != -1 && board.is_square_attacked(sq, opponent)) {
                safe_ok = false;
                break;
            }
        }
        if (!safe_ok) continue;

        // 5. Add castling move
        moves[move_count++] = Move{
            board.get_piece_at(cs.kingFrom),
            (uint8_t)cs.kingFrom,
            (uint8_t)cs.kingTo,
            Piece::NONE,
            Piece::NONE,
            false,
            true  // is_castle
        };
    }
}

Bitboard Engine::shift(Bitboard board, int shift)
{
    return (shift > 0) ? (board << shift) : (board >> -shift);
}

void Engine::extract_pawn_push(Bitboard bb, Piece piece, int shift, Move* moves, int& move_count)
{
    while(bb != 0ULL){
        int to = std::countr_zero(bb);
        //Clear the bit we just processed
        bb &= (bb - 1);

        moves[move_count++] = Move{
            piece,
            (uint8_t) (to-shift),
            (uint8_t) to,
            Piece::NONE,
            Piece::NONE,
            false,
            false
        };
    }
}

void Engine::extract_pawn_capture(Bitboard bb, Piece piece, int shift, Move *moves, int &move_count, const Board &board)
{
    while(bb != 0ULL){
        int to = std::countr_zero(bb);
        //Clear the bit we just processed
        bb &= (bb - 1);

        moves[move_count++] = Move{
            piece,
            (uint8_t) (to - shift),
            (uint8_t) (to),
            board.get_piece_at(to),
            Piece::NONE,
            false,
            false
        };
    }
}

void Engine::extract_promotion_push(Bitboard bb, Piece piece, int shift, Move *moves, int &move_count)
{
    Color us = colorOf(piece);
    Piece queen = (us == Color::WHITE) ? Piece::W_QUEEN : Piece::B_QUEEN;
    Piece rook = (us == Color::WHITE) ? Piece::W_ROOK : Piece::B_ROOK;
    Piece bishop = (us == Color::WHITE) ? Piece::W_BISHOP : Piece::B_BISHOP;
    Piece knight = (us == Color::WHITE) ? Piece::W_KNIGHT : Piece::B_KNIGHT;

    while (bb != 0ULL) {
        int to = std::countr_zero(bb);
        //Clear the bit we just processed
        bb &= (bb - 1);
        int from = to - shift;
        moves[move_count++] = Move{piece, (uint8_t)from, (uint8_t) to, Piece::NONE, queen, false, false};
        moves[move_count++] = Move{piece, (uint8_t)from, (uint8_t) to, Piece::NONE, rook, false, false};
        moves[move_count++] = Move{piece, (uint8_t)from, (uint8_t) to, Piece::NONE, bishop, false, false};
        moves[move_count++] = Move{piece, (uint8_t)from, (uint8_t) to, Piece::NONE, knight, false, false};
    }
}

void Engine::extract_promotion_capture(Bitboard bb, Piece piece, int shift, Move *moves, int &move_count, const Board &board)
{
    Color us = colorOf(piece);
    Piece queen = (us == Color::WHITE) ? Piece::W_QUEEN : Piece::B_QUEEN;
    Piece rook = (us == Color::WHITE) ? Piece::W_ROOK : Piece::B_ROOK;
    Piece bishop = (us == Color::WHITE) ? Piece::W_BISHOP : Piece::B_BISHOP;
    Piece knight = (us == Color::WHITE) ? Piece::W_KNIGHT : Piece::B_KNIGHT;

    while (bb != 0ULL) {
        int to = std::countr_zero(bb);
        //Clear the bit we just processed
        bb &= (bb - 1);
        int from = to - shift;
        Piece capture = board.get_piece_at(to);
        moves[move_count++] = Move{piece, (uint8_t)from, (uint8_t) to, capture, queen, false, false};
        moves[move_count++] = Move{piece, (uint8_t)from, (uint8_t) to, capture, rook, false, false};
        moves[move_count++] = Move{piece, (uint8_t)from, (uint8_t) to, capture, bishop, false, false};
        moves[move_count++] = Move{piece, (uint8_t)from, (uint8_t) to, capture, knight, false, false};
    }
}
