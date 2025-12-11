#include "uci.h"
#include "engine.h"
#include <sstream>
#include <iostream>

UCIEngine::UCIEngine() : board(), engine() {}

void UCIEngine::run()
{
    std::string line;
        
    while (std::getline(std::cin, line)) {
        if (line.empty()) continue;
        
        std::istringstream iss(line);
        std::string command;
        iss >> command;
        
        if (command == "uci") {
            handle_uci();
        }
        else if (command == "isready") {
            handle_isready();
        }
        else if (command == "ucinewgame") {
            handle_ucinewgame();
        }
        else if (command == "position") {
            handle_position(iss);
        }
        else if (command == "go") {
            handle_go(iss);
        }
        else if (command == "quit") {
            break;
        }
    }
}

void UCIEngine::handle_uci()
{
    std::cout << "id name MyChessEngine 1.0" << std::endl;
    std::cout << "id author Justin Hollister" << std::endl;
    // Options can go here (e.g., Hash size)
    std::cout << "uciok" << std::endl;
}

void UCIEngine::handle_isready()
{
    std::cout << "readyok" << std::endl;
}

void UCIEngine::handle_ucinewgame()
{
    board = Board();  // Reset to starting position
}

void UCIEngine::handle_position(std::istringstream& iss)
{
    std::string token;
    iss >> token;

    if(token == "startpos"){
        board = Board();  // Starting position
            
        // Check for moves
        iss >> token;  // Should be "moves" or empty
        if (token == "moves") {
            // Apply moves
            while (iss >> token) {
                apply_uci_move(token);
            }
        }
    } else if (token == "fen") {
            // Read FEN string
            std::string fen;
            std::string fen_token;
            
            // FEN has 6 parts, read until "moves" or end
            while (iss >> fen_token && fen_token != "moves") {
                if (!fen.empty()) fen += " ";
                fen += fen_token;
            }
            
            board.set_position_fen(fen);
            
            // If we stopped at "moves", apply them
            if (fen_token == "moves") {
                while (iss >> token) {
                    apply_uci_move(token);
                }
            }
        }
}

void UCIEngine::apply_uci_move(const std::string& uci_move)
{
    // Parse UCI move (e.g., "e2e4", "e7e8q")
    if (uci_move.length() < 4) return;

    int from_file = uci_move[0] - 'a';
    int from_rank = uci_move[1] - '1';
    int to_file = uci_move[2] - 'a';
    int to_rank = uci_move[3] - '1';

    int from_square = from_rank * 8 + from_file;
    int to_square = to_rank * 8 + to_file;

    // Generate legal moves and find matching one
    Move moves[MAX_NUMBER_OF_MOVES];
    int num_moves = engine.generate_legal_moves(board, moves);

    for (int i = 0; i < num_moves; i++) {
        if (moves[i].from_square == from_square && 
            moves[i].to_square == to_square) {
            
            // Check promotion
            if (uci_move.length() == 5) {
                char promo = uci_move[4];
                Piece expected_promo = parse_promotion(promo, board.sideToMove);
                
                if (moves[i].promoted_piece != expected_promo) {
                    continue;  // Wrong promotion
                }
            }
            
            // Found the move!
            board.make_move(moves[i]);
            return;
        }
    }
    
    std::cerr << "Invalid move: " << uci_move << std::endl;
}

void UCIEngine::handle_go(std::istringstream& iss)
{
    std::string token;
    int depth = 6;  // Default depth
    int movetime = 0;  // Time in milliseconds
    
    while (iss >> token) {
        if (token == "depth") {
            iss >> depth;
        }
        else if (token == "movetime") {
            iss >> movetime;
        }
        // Can add: wtime, btime, winc, binc for time management
    }
    
    // Search for best move
    Move best_move = search_position(depth);
    
    // Output best move in UCI format
    std::cout << "bestmove " << move_to_uci(best_move) << std::endl;
}

Piece UCIEngine::parse_promotion(char c, Color color)
{
    if (color == Color::WHITE) {
        switch(c) {
            case 'q': return Piece::W_QUEEN;
            case 'r': return Piece::W_ROOK;
            case 'b': return Piece::W_BISHOP;
            case 'n': return Piece::W_KNIGHT;
            default: return Piece::NONE;
        }
    } else {
        switch(c) {
            case 'q': return Piece::B_QUEEN;
            case 'r': return Piece::B_ROOK;
            case 'b': return Piece::B_BISHOP;
            case 'n': return Piece::B_KNIGHT;
            default: return Piece::NONE;
        }
    }
}

Move UCIEngine::search_position(int depth)
{
    Move moves[MAX_NUMBER_OF_MOVES];
    int num_moves = engine.generate_legal_moves(board, moves);
    
    if (num_moves == 0) {
        return Move{};  // No legal moves
    }
    
    Move best_move = moves[0];
    int best_score = -999999;
    
    for (int i = 0; i < num_moves; i++) {
        board.make_move(moves[i]);
        int score = -engine.negamaxAB(board, depth - 1, 1, -999999, 999999);
        board.undo_move();
        
        // Optional: send info
        if(false){
            std::cout << "info depth " << depth 
                    << " score cp " << score 
                    << " pv " << move_to_uci(moves[i]) 
                    << std::endl;
        }
        
        
        if (score > best_score) {
            best_score = score;
            best_move = moves[i];
        }
    }

    std::cout << "info depth " << depth 
                    << " score cp " << best_score 
                    << " pv " << move_to_uci(best_move) 
                    << std::endl;
    
    return best_move;
}

std::string UCIEngine::move_to_uci(const Move &move)
{
    std::string uci;
        
    // From square
    uci += char('a' + (move.from_square % 8));
    uci += char('1' + (move.from_square / 8));
    
    // To square
    uci += char('a' + (move.to_square % 8));
    uci += char('1' + (move.to_square / 8));
    
    // Promotion
    if (move.promoted_piece != Piece::NONE) {
        switch(move.promoted_piece) {
            case Piece::W_QUEEN:
            case Piece::B_QUEEN:  uci += 'q'; break;
            case Piece::W_ROOK:
            case Piece::B_ROOK:   uci += 'r'; break;
            case Piece::W_BISHOP:
            case Piece::B_BISHOP: uci += 'b'; break;
            case Piece::W_KNIGHT:
            case Piece::B_KNIGHT: uci += 'n'; break;
            default: break;
        }
    }
    
    return uci;
}

int main() {
    UCIEngine uci;
    uci.run();
    std::cout << "Done" << std::endl;
    return 0;
}
