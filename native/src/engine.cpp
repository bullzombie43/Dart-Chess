#include "../include/engine.h"
#include <ranges> 
#include "utils.h"
#include "Engine.h"

std::vector<Move> Engine::generate_psuedo_legal_moves(const Board &board)
{
    std::vector<Move> pseudo_moves;
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
            generate_moves_from_square(board, piece, from_square, pseudo_moves);

            // 4. Clear the bit we just processed
            pieces_bb &= (pieces_bb - 1);
        }
    }
    
    return pseudo_moves;
}

std::vector<Move> Engine::generate_legal_moves(Board &board)
{
    std::vector<Move> psuedo_legal_moves = generate_psuedo_legal_moves(board);

    std::vector<Move> legal_moves;

    for(Move& m : psuedo_legal_moves){
        board.make_move(m);

        if(!board.is_in_check(board.sideToMove == Color::WHITE ? Color::BLACK : Color::WHITE)){
            legal_moves.push_back(m);
        }

        board.undo_move();
    }

    return legal_moves;
}

uint64_t Engine::perft(Board &board, int depth)
{
    std::vector<Move> move_list;
    uint64_t n_moves;

    uint64_t nodes = 0;

    move_list = generate_psuedo_legal_moves(board);
    n_moves = move_list.size();

    if(depth == 1){
        return n_moves;
    }

    for(int i = 0; i< n_moves; i++){
        board.make_move(move_list[i]);
        nodes += perft(board, depth-1);
        board.undo_move();
    }

    return nodes;
}

void Engine::perft_divide(Board &board, int depth)
{
}

void Engine::generate_moves_from_square(const Board &board, Piece piece, uint8_t index, std::vector<Move> &moves)
{
    if(piece == Piece::W_KNIGHT || piece == Piece::B_KNIGHT){
        generate_knight_moves(board, piece, index, moves);
    } else if(piece == Piece::W_KING || piece == Piece::B_KING){
        generate_king_moves(board, piece, index, moves);
    } else if (piece == Piece::W_PAWN || piece == Piece::B_PAWN){
        generate_pawn_moves(board, piece, index, moves);
    } else {
        generate_sliding_moves(board, piece, index, moves);
    }
}

void Engine::generate_sliding_moves(const Board &board, Piece piece, uint8_t index, std::vector<Move> &moves)
{
    //Implemet Shift and Mask Approach later for faster generation
    int startDirIndex = piece == Piece::B_BISHOP || piece == Piece::W_BISHOP ? 4 : 0;
    int endDirIndex = piece == Piece::B_ROOK || piece == Piece::W_ROOK ? 4 : 8;

    const Bitboard same_color = colorOf(piece) == Color::WHITE ? board.white_occupancy : board.black_occupancy;
    const Bitboard opp_color = colorOf(piece) == Color::WHITE ? board.black_occupancy : board.white_occupancy;

    for(int directionIndex = startDirIndex; directionIndex < endDirIndex; directionIndex++){
      for(int n = 0; n < num_squares_to_edge[index][directionIndex]; n++){
        int targetSquare = index + direction_offsets[directionIndex] * (n+1);

        if(targetSquare < 0){
          break;
        }

        const Bitboard target_bit = 1ULL << targetSquare;
        
        if((same_color & target_bit) != 0){
          break;
        }

        if((opp_color & target_bit) != 0){
            auto captured_piece = board.get_piece_at(targetSquare);
            // Construct the Move struct and add it to the list
            moves.push_back(Move{
                (uint8_t)piece,          
                (uint8_t)index,     
                (uint8_t)targetSquare,    
                captured_piece,                 
                std::nullopt,             
                false,                    
                false                    
            });
            break;
        }

        // Construct the Move struct and add it to the list
        moves.push_back(Move{
            (uint8_t)piece,          
            (uint8_t)index,     
            (uint8_t)targetSquare,    
            std::nullopt,                 
            std::nullopt,             
            false,                    
            false                    
        });
      }
    }
}

void Engine::generate_pawn_moves(const Board &board, Piece piece, uint8_t index, std::vector<Move> &moves)
{
    Color our_color = colorOf(piece);

    uint8_t direction = our_color == Color::WHITE ? North : South;
    int leftCap = (our_color == Color::WHITE ? +7 : -9);
    int rightCap = (our_color == Color::WHITE ? +9 : -7);

    uint8_t start_rank = (our_color == Color::WHITE ? 1 : 6);     // rank 2 or rank 7
    uint8_t promo_rank = (our_color == Color::WHITE ? 7 : 0);     // rank 8 or rank 1
    uint8_t ep_direction = direction;

    uint8_t single_push = index + direction;

    std::vector<Move> temp_moves;

    // ---------- 1. FORWARD MOVES ----------
    if(!board.get_piece_at(single_push).has_value()){
        temp_moves.push_back(Move{piece, index, single_push, std::nullopt, std::nullopt, false, false});

        if(rankOf(index) == start_rank){
            uint8_t double_push = index + 2*direction;
            if(!board.get_piece_at(double_push).has_value()){
                temp_moves.push_back(Move{piece, index, double_push, std::nullopt, std::nullopt, false, false});
            }
        }
    }

    // ---------- 2. CAPTURE MOVES ----------
    for(uint8_t diag_offset : {leftCap,rightCap}){
        uint8_t target = index + diag_offset;
        std::optional<Piece> target_piece = board.get_piece_at(target);

        if (target < 0 || target >= 64) continue;

        // Check file wrapping
        int fileDiff = ((index+diag_offset) % 8) - (index % 8);

        if (std::abs(fileDiff) != 1) continue;

        //Normal
        if(target_piece != std::nullopt && colorOf(target_piece.value()) != our_color){
            temp_moves.push_back(Move{piece, index, target, target_piece, std::nullopt, false, false});
        }

        // ---------- 3. EN PASSANT ----------
        if(target == board.enPassantSquare.value() && board.color_can_en_passant == our_color){
            std::optional<Piece> ep_piece = board.get_piece_at(target + (colorOf(piece) == Color::WHITE ? -8 : +8));
            temp_moves.push_back(Move{piece, index, target, target_piece, std::nullopt, true, false});
        }
    }

    // ---------- 4. PROMOTION ----------
    for(const auto& m : temp_moves){
        if(rankOf(m.to_square) == promo_rank){
            for(Piece promo : promotion_pieces(our_color)){
                moves.push_back(Move{m.piece, m.from_square, m.to_square, m.captured_piece, promo, m.is_enpassant, true});
            }
        } else {
            moves.push_back(m);
        }
    }
}

void Engine::generate_knight_moves(const Board &board, Piece piece, uint8_t index, std::vector<Move> &moves)
{
    int startDirIndex = 8;
    int endDirIndex = 16;

    for(int directionIndex = startDirIndex; directionIndex < endDirIndex; directionIndex++){
      uint8_t targetSquare = index + direction_offsets[directionIndex];

      if(targetSquare < 0 || targetSquare > 63) continue;

      // Check file wrapping
      int fileDiff = (targetSquare % 8) - (index % 8);
      if (std::abs(fileDiff) > 2) continue; // illegal wrap

      std::optional<Piece> target_piece = board.get_piece_at(targetSquare);

      if(target_piece.has_value() && colorOf(target_piece.value()) == colorOf(piece)) continue;

      moves.push_back(Move{piece, index, targetSquare, target_piece, std::nullopt, false, false});
     }
}

void Engine::generate_king_moves(const Board &board, Piece piece, uint8_t index, std::vector<Move> &moves)
{
    for(int directionIndex = 0; directionIndex < 8; directionIndex++){
      uint8_t targetSquare = index + direction_offsets[directionIndex];

      if(targetSquare < 0 || targetSquare > 63){
        continue;
      }

      // Check file wrapping
      int fileDiff = (targetSquare % 8) - (index % 8);
      if (std::abs(fileDiff) > 2) continue; // illegal wrap
      
      std::optional<Piece> target_piece = board.get_piece_at(targetSquare);

      if(target_piece.has_value() && colorOf(target_piece.value()) == colorOf(piece)) continue;

      moves.push_back(Move{piece, index, targetSquare, target_piece, std::nullopt, false, false});
    }

    //Add in castle moves
    generate_castle_moves(board, piece, index, moves);
}

void Engine::generate_castle_moves(const Board &board, Piece piece, uint8_t index, std::vector<Move> &moves)
{
    struct CastleInfo {
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
        moves.push_back(Move{
            board.get_piece_at(cs.kingFrom).value(),
            (uint8_t)cs.kingFrom,
            (uint8_t)cs.kingTo,
            std::nullopt,
            std::nullopt,
            false,
            true  // is_castle
        });
    }
}

Bitboard Engine::shift(Bitboard board, Direction direction)
{
    return (board << static_cast<int>(direction));
}


