#pragma once
#include "board.h"
#include "engine.h"

constexpr int DEFAULT_SEARCH_TIME_MS = 100;

class UCIEngine {
    private:
        Board board;
        Engine engine;

    public:
        UCIEngine();
        
        void run();

    private:
        void handle_uci();
        void handle_isready();
        void handle_ucinewgame();
        void handle_position(std::istringstream& iss);
        void apply_uci_move(const std::string& uci_move);
        void handle_go(std::istringstream& iss);

        Piece parse_promotion(char c, Color color);
        Move search_position(int depth);
        std::string move_to_uci(const Move& move);

};

