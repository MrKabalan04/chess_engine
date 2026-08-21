// test_root.cpp - replicate root loop at depth 1, print every move's score
#include <iostream>
#include <cstdint>
#include "board.h"
#include "generateMoves.h"

int main() {
    Board b;
    GenerateMoves gen;
    gen.init();
    b.initZobrist();
    b.init();
    b.initFromFen("3r2k1/5ppp/8/8/8/8/5PPP/3R2K1 w - - 0 1");

    // mimic getBestMove internals needed for scoring
    gen.timeLimitMs = 100000;
    gen.searchStartTime = std::chrono::high_resolution_clock::now();
    gen.searchAborted = false;
    gen.nodesSearched = 0;

    MoveList ml = gen.generateLegalMoves(b, 0);
    std::cout << "legal moves: " << ml.count << "\n";

    for (int i = 0; i < ml.count; i++) {
        Move m = ml.moves[i];
        b.makeMove(m);
        int score = -gen.negamax(b, 0, -999999, 999999, 1);  // full window, depth 0 child
        b.undoMove();
        std::cout << "  " << m.getFrom() << "->" << m.getTo() << " type=" << m.getType()
                  << " score=" << score << "\n";
    }
    return 0;
}
