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

    for (int p_idx = start_piece; p_idx <= end_piece; ++p_idx) {
        Piece piece = static_cast<Piece>(p_idx);
        
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

uint64_t Engine::perft(Board &board, int depth, int current_depth, Move* moves) {
    if(depth == 0) return 1;

    // Use a small, fixed-size array allocated on the stack.
    // This is incredibly fast and avoids pointer arithmetic errors.
    Move move_list[MAX_NUMBER_OF_MOVES]; 

    // Generate legal moves into the stack array
    // (You will need to modify your generate_legal_moves signature)
    int n_moves = generate_legal_moves(board, move_list); 

    if(depth == 1) return n_moves;

    uint64_t nodes = 0;

    for(int i = 0; i < n_moves; i++) {
        board.make_move(move_list[i]);
        // Recursive call without passing a move array
        nodes += perft(board, depth - 1); 
        board.undo_move(); 
    }

    return nodes;
}

uint64_t Engine::perft(Board &board, int depth) {
    Move moves[MAX_DEPTH * MAX_NUMBER_OF_MOVES];
    return perft(board, depth, 0, moves);
}

uint64_t Engine::perft_divide(Board &board, int depth, Move* moves, int current_depth = 0)
{
    // Generate legal moves into the slice for this depth
    int n_moves = generate_legal_moves(board, moves);
    uint64_t total_nodes = 0;

    // Slice for the next depth
    Move* next_moves = moves + MAX_NUMBER_OF_MOVES;  // assuming MAX_MOVES_PER_NODE is defined

    for (int i = 0; i < n_moves; i++) {
        board.make_move(moves[i]);

        uint64_t nodes;
        if(depth - 1 == 0){
            nodes = 1;
        } else {
            nodes = perft(board, depth - 1, current_depth + 1, next_moves);
        }

        board.undo_move();
        total_nodes += nodes;

        std::cout << move_to_string(moves[i]) << ": " << nodes << "\n";
    }

    return total_nodes;
}

uint64_t Engine::perft_divide(Board &board, int depth) {
    Move moves[MAX_DEPTH * MAX_NUMBER_OF_MOVES];
    return perft_divide(board, depth, moves, 0);
}

void Engine::generate_moves_from_square(const Board &board, Piece piece, uint8_t index, Move* moves, int& move_count)
{
    if(piece == Piece::W_KNIGHT || piece == Piece::B_KNIGHT){
        generate_knight_moves(board, piece, index, moves, move_count);
    } else if(piece == Piece::W_KING || piece == Piece::B_KING){
        generate_king_moves(board, piece, index, moves, move_count);
    } else if (piece == Piece::W_PAWN || piece == Piece::B_PAWN){
        generate_pawn_moves(board, piece, index, moves, move_count);
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

        if(targetSquare < 0 || targetSquare > 63) break;


        const Bitboard target_bit = 1ULL << targetSquare;
        
        if((same_color & target_bit) != 0){
          break;
        }

        if((opp_color & target_bit) != 0){
            auto captured_piece = board.get_piece_at(targetSquare);
            // Construct the Move struct and add it to the list
            moves[move_count++] = Move{
                (uint8_t)piece,          
                (uint8_t)index,     
                (uint8_t)targetSquare,    
                captured_piece,                 
                std::nullopt,             
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
            std::nullopt,                 
            std::nullopt,             
            false,                    
            false                    
        };
      }
    }
}

void Engine::generate_pawn_moves(const Board &board, Piece piece, uint8_t index, Move* moves, int& move_count)
{
    Color our_color = colorOf(piece);

    int direction = our_color == Color::WHITE ? 8 : -8;
    int leftCap = (our_color == Color::WHITE ? +7 : -9);
    int rightCap = (our_color == Color::WHITE ? +9 : -7);

    int start_rank = (our_color == Color::WHITE ? 1 : 6);     // rank 2 or rank 7
    int promo_rank = (our_color == Color::WHITE ? 7 : 0);     // rank 8 or rank 1
    int ep_direction = direction;

    int single_push = index + direction;

    // ---------- 1. FORWARD MOVES ----------
    if(single_push >= 0 && single_push < 64 && !board.get_piece_at(single_push).has_value()){
        if(rankOf(single_push) == promo_rank){
            for (Piece promo : promotion_pieces(our_color)) {
                moves[move_count++] = Move{piece, (uint8_t)index, (uint8_t)single_push, std::nullopt, promo, false, true};
            }
        } else {
            moves[move_count++]= Move{piece, index, static_cast<uint8_t>(single_push), std::nullopt, std::nullopt, false, false};

            if(rankOf(index) == start_rank){
                int double_push = index + 2*direction;
                if(double_push >= 0 && double_push < 64 && !board.get_piece_at(double_push).has_value()){
                    moves[move_count++] = Move{piece, index, static_cast<uint8_t>(double_push), std::nullopt, std::nullopt, false, false};
                }
            }
        }
    }

    // ---------- 2. CAPTURE MOVES ----------
    for(int diag_offset : {leftCap,rightCap}){
        int target = index + diag_offset;
        if (target < 0 || target >= 64) continue;
        std::optional<Piece> target_piece = board.get_piece_at(target);

        // Check file wrapping
        int fileDiff = ((index+diag_offset) % 8) - (index % 8);

        if (std::abs(fileDiff) != 1) continue;

        //Normal
        if(target_piece != std::nullopt && colorOf(target_piece.value()) != our_color){
            if(rankOf(target) == promo_rank){
                for (Piece promo : promotion_pieces(our_color)) {
                    moves[move_count++] = Move{piece, (uint8_t)index, (uint8_t)target, target_piece, promo, false, true};
                }
            } else {
                moves[move_count++] = Move{piece, index, (uint8_t) target, target_piece, std::nullopt, false, false};
            }
        }

        // ---------- 3. EN PASSANT ----------
        if(board.enPassantSquare.has_value() && target == board.enPassantSquare.value() && board.color_can_en_passant == our_color){
            int ep_index = target + (our_color == Color::WHITE ? -8 : +8);
            std::optional<Piece> ep_piece = (ep_index >= 0 && ep_index < 64) ? board.get_piece_at(ep_index) : std::nullopt;
            moves[move_count++] = Move{piece, index, (uint8_t) target, ep_piece, std::nullopt, true, false};
        }
    }

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

      std::optional<Piece> target_piece = board.get_piece_at(targetSquare);

      if(target_piece.has_value() && colorOf(target_piece.value()) == colorOf(piece)) continue;

      moves[move_count++] = Move{piece, index, (uint8_t) targetSquare, target_piece, std::nullopt, false, false};
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
      
      std::optional<Piece> target_piece = board.get_piece_at(targetSquare);

      if(target_piece.has_value() && colorOf(target_piece.value()) == colorOf(piece)) continue;

      moves[move_count++] = Move{piece, index, (uint8_t)targetSquare, target_piece, std::nullopt, false, false};
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

        // Only evaluate rights for the correct color
        if ((color == Color::WHITE && cs.right >= CastlingRights::BLACK_KINGSIDE) ||
            (color == Color::BLACK && cs.right <= CastlingRights::WHITE_QUEENSIDE))
            continue;

        // 1. Check castling right flag
        if (!board.can_castle(cs.right)) continue;

        // 2. Rook must still be there
        auto rook = board.get_piece_at(cs.rookSquare);
        if (!rook.has_value()) continue;

        // 3. Squares must be empty
        bool empty_ok = true;
        for (int sq : cs.emptySquares) {
            if (sq != -1 && board.get_piece_at(sq).has_value()) {
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
            board.get_piece_at(cs.kingFrom).value(),
            (uint8_t)cs.kingFrom,
            (uint8_t)cs.kingTo,
            std::nullopt,
            std::nullopt,
            false,
            true  // is_castle
        };
    }
}

Bitboard Engine::shift(Bitboard board, Direction direction)
{
    return (board << static_cast<int>(direction));
}

