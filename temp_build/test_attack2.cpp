// test_attack2.cpp - inspect queen placement + generated moves from h6
#include <iostream>
#include "board.h"
#include "generateMoves.h"

int main() {
    Board board;
    GenerateMoves gen;
    if (!board.initFromFen("r1bq2rk/pp3pbp/2p1p1pQ/7P/3P4/2PB1N2/PP3PPR/2KR4 w - - 0 1")) {
        std::cout << "FEN load failed\n"; return 1;
    }

    const char* names[] = {"P","N","B","R","Q","K"};
    std::cout << "piece codes (0..5 = PNBRQK, -1 empty):\n";
    int qs[] = {47, 55, 15, 63, 54};
    for (int s : qs) std::cout << "  sq" << s << ": " << board.getPieceAt(s) << "\n";

    // full board dump
    std::cout << "board dump (rank 8 -> 1):\n";
    for (int r = 7; r >= 0; r--) {
        std::cout << "  ";
        for (int f = 0; f < 8; f++) {
            int p = board.getPieceAt(r*8+f);
            char c = (p < 0) ? '.' : names[p][0];
            std::cout << c;
        }
        std::cout << "\n";
    }

    MoveList ml = gen.generateLegalMoves(board, 0);
    std::cout << "white legal moves: " << ml.count << "\n";
    std::cout << "moves FROM sq47 (h6): ";
    for (int i = 0; i < ml.count; i++)
        if (ml.moves[i].getFrom() == 47)
            std::cout << ml.moves[i].getTo() << " ";
    std::cout << "\n";
    std::cout << "any move TO sq55 (h7): ";
    for (int i = 0; i < ml.count; i++)
        if (ml.moves[i].getTo() == 55)
            std::cout << ml.moves[i].getFrom() << " ";
    std::cout << "\n";
    return 0;
}
