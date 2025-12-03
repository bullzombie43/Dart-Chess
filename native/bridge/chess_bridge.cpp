#include "chess_bridge.h"
#include "board.h"
#include "engine.h"

static inline Board* handle_to_board(ChessBoardHandle handle){
    return static_cast<Board*>(handle);
}

static inline Engine* handle_to_engine(ChessEngineHandle handle){
    return static_cast<Engine*>(handle);
}

static inline ChessBoardHandle board_to_handle(Board* board){
    return static_cast<ChessBoardHandle>(board);
}

static inline ChessEngineHandle engine_to_handle(Engine* board){
    return static_cast<ChessEngineHandle>(board);
}

static void cpp_move_to_c_move(const Move& src, CMove* dst){
    dst->piece = static_cast<uint8_t>(src.piece);
    dst->from_square = src.from_square;
    dst->to_square = src.to_square;
    dst->captured_piece = static_cast<uint8_t>(src.captured_piece);
    dst->promoted_piece = static_cast<uint8_t>(src.promoted_piece);
    dst->is_enpassant = src.is_enpassant ? 1 : 0;
    dst->is_castling = src.is_castling ? 1 : 0;
}

static void c_move_to_cpp_move(const CMove* src, Move& dst) {
    dst.piece = static_cast<Piece>(src->piece);
    dst.from_square = src->from_square;
    dst.to_square = src->to_square;
    dst.captured_piece = static_cast<Piece>(src->captured_piece);
    dst.promoted_piece = static_cast<Piece>(src->promoted_piece);
    dst.is_enpassant = src->is_enpassant != 0;
    dst.is_castling = src->is_castling != 0;
}


ChessEngineHandle engine_create(void) {
    // Allocate new Engine object on the heap
    Engine* engine = new Engine();
    
    // Convert to opaque handle
    return engine_to_handle(engine);
}

void engine_destroy(ChessEngineHandle handle) {
    if (handle == nullptr) {
        return;
    }
    
    // Cast back to Engine* and delete
    Engine* engine = handle_to_engine(handle);
   
    delete engine;
}

 ChessBoardHandle board_create(void) {
    // Create new board in starting position
    Board* board = new (std::nothrow) Board();
    
    // Board() constructor sets up starting position, so this always succeeds
    return board_to_handle(board);
}

ChessBoardHandle board_create_from_fen(const char* fen) {
    // Validate input
    if (fen == nullptr) {
        return nullptr;
    }
    
    // Create empty board
    Board* board = new (std::nothrow) Board();
    if (board == nullptr) {
        return nullptr;  // Allocation failed (very rare)
    }
    
    // Try to set position from FEN
    // Your set_position_fen might throw exceptions on invalid FEN
    try {
        board->set_position_fen(std::string(fen));
        return board_to_handle(board);
    } catch (...) {
        // FEN parsing failed - clean up and return NULL
        delete board;
        return nullptr;
    }
}

void board_destroy(ChessBoardHandle handle) {
    if (handle == nullptr) {
        return;
    }
    
    Board* board = handle_to_board(handle);
    delete board;
}

uint8_t board_get_side_to_move(ChessBoardHandle handle){
    // Always validate handles in public API
    if (handle == nullptr) {
        throw std::runtime_error("Handle Cannot not be null in board_get_side_to_move");
    }

    Board* board = handle_to_board(handle);

    return static_cast<uint8_t>(board->sideToMove);
}

uint8_t board_get_piece_at(ChessBoardHandle handle, int square){
    if(handle == nullptr){
        throw std::runtime_error("Handle Cannot not be null in board_get_piece_at");
    }

    if(square < 0 || square > 63){
        throw std::runtime_error("Square must be between 0 and 63 in board_get_piece_at");
    }

    Board* board = handle_to_board(handle);

    return static_cast<uint8_t>(board->get_piece_at(square));
}

char* board_get_fen(ChessBoardHandle handle){
    if(handle == nullptr){
        throw std::runtime_error("Handle Cannot not be null in board_get_piece_at");
    }

    Board* board = handle_to_board(handle);

    std::string fen = board->getFen();

    //Allocate C string (size + 1 for null terminator)
    // IMPORTANT: Use malloc, not new[], so it matches free() in chess_free_string
    char* result = static_cast<char*>(malloc(fen.length() + 1));
    if (result == nullptr) {
        return nullptr;  // Allocation failed
    }
    
    // Copy string data
    std::strcpy(result, fen.c_str());

    return result;
}

//IMPORTANT USE FREE TO MATCH MALLOC

char* chess_move_to_string(const CMove *cmove)
{
    Move move;

    if(cmove == nullptr){
        return nullptr;
    }

    c_move_to_cpp_move(cmove, move);

    std::string cppstring = move_to_string(move);

    char* result = static_cast<char*>(malloc(cppstring.length() + 1));
    if (result == nullptr) {
        return nullptr;  // Allocation failed
    }

    std::strcpy(result, cppstring.c_str());

    return result;
}

void chess_free_string(char *str)
{
    // free() is already NULL-safe, but explicit check is clearer
    if (str != nullptr) {
        free(str);
    }
}

uint64_t chess_perft(ChessEngineHandle engine_handle, ChessBoardHandle board_handle, int32_t depth)
{
    if(engine_handle == nullptr || board_handle == nullptr ){
        throw std::runtime_error("Handle Cannot not be null in chess_perft");
    }

    Board* board = handle_to_board(board_handle);
    Engine* engine = handle_to_engine(engine_handle);

    return engine->perft(*board, depth);
}

int32_t engine_generate_legal_moves(ChessEngineHandle engine_handle, ChessBoardHandle board_handle, CMove* moves, int32_t max_moves){
    if(engine_handle == nullptr || board_handle == nullptr ){
        throw std::runtime_error("Handle Cannot not be null in engine_generate_legal_moves");
    }

    if(moves == nullptr){
        throw std::runtime_error("Moves Cannot not be null in engine_generate_legal_moves");
    }

    if(max_moves <= 0){
        throw std::runtime_error("Max Movex cannot be less than or equal to 0 in engine_generate_legal_moves");
    }

    Engine* engine = handle_to_engine(engine_handle);
    Board* board = handle_to_board(board_handle);

    // Allocate temporary buffer for C++ moves
    Move cpp_moves[MAX_LEGAL_MOVES];

    int count = engine->generate_legal_moves(*board, cpp_moves);

    // Clamp to max_moves (shouldn't happen with MAX_LEGAL_MOVES = 256)
    if (count > max_moves) {
        count = max_moves;
    }

    for (int i = 0; i < count; i++) {
        cpp_move_to_c_move(cpp_moves[i], &moves[i]);
    }
    
    return count;
}

void board_make_move(ChessBoardHandle handle, const CMove* move){
    if(handle == nullptr || move == nullptr){
        throw std::runtime_error("Handle or Move Cannot not be null in board_make_move");
    }

    Board* board = handle_to_board(handle);
    
    Move cpp_move;
    c_move_to_cpp_move(move, cpp_move);

    board->make_move(cpp_move);
}

void board_undo_move(ChessBoardHandle handle) {
    if (handle == nullptr) {
        throw std::runtime_error("Handle Cannot be null in chess_board_undo_move");
    }
    
    Board* board = handle_to_board(handle);
    board->undo_move();
}

uint8_t board_set_fen(ChessBoardHandle handle, const char *fen)
{
    if (handle == nullptr) {
        throw std::runtime_error("Handle Cannot be null in board_set_fen");
    }

    if(fen == nullptr){
        throw std::runtime_error("Fen Cannot be null in board_set_fen");
    }

    Board* board = handle_to_board(handle);

    std::string cppstring = static_cast<std::string>(fen);

    board->set_position_fen(cppstring);

    return 1;
}

uint8_t board_is_in_check(ChessBoardHandle handle) {
    if (handle == nullptr) {
        throw std::runtime_error("Handle Cannot be null in chess_board_undo_move");
    }
    
    Board* board = handle_to_board(handle);
    
    bool in_check = board->is_in_check(board->sideToMove);
    
    return in_check ? 1 : 0;
}






 