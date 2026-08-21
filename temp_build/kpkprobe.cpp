#include "..\Luna\board.h"
#include "..\Luna\generateMoves.h"
#include <iostream>

int main() {
    Board board;
    board.initZobrist();
    board.init();
    GenerateMoves gen;
    gen.init();

    const char* fens[] = {
        "6k1/8/8/8/8/8/4P3/4K3 w - - 0 1",
        "8/8/8/4k3/8/8/4P3/4K3 w - - 0 1",
        "k7/P7/8/8/8/8/8/7K w - - 0 1",
        "4k3/8/8/8/8/8/8/4K2P w - - 0 1",
    };
    for (const char* f : fens) {
        board.initFromFen(f);
        Move m = gen.kpkMove(board);
        std::cout << f << "  ->  move.data=" << m.data << "\n";
    }

    std::cout << "raw Ke1/Kg8/Pe2   cur=" << (int)gen.kpkDTM(0, 4, 62, 12)  << "\n";
    std::cout << "raw Ke6/Kg8/Pe7   cur=" << (int)gen.kpkDTM(0, 44, 62, 52) << "\n";
    std::cout << "raw Ka6/Ka8/Pa7   cur=" << (int)gen.kpkDTM(0, 40, 56, 48) << "\n";
    return 0;
}
