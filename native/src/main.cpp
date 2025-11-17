#include <iostream>
#include <board.h>

int main(){
    std::cout << "Hello World" << std::endl;

    Board board = Board();
    board.set_position_fen("rnbqkbnr/pp1ppppp/8/2p5/4P3/5N2/PPPP1PPP/RNBQKB1R b KQkq - 1 2");

    board.print_board(std::cout);

    return 0;
}