// test_root2.cpp - EXACT replica of getBestMove root loop w/ ordering + PVS
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

    gen.timeLimitMs = 100000;
    gen.searchStartTime = std::chrono::high_resolution_clock::now();
    gen.searchAborted = false;
    gen.nodesSearched = 0;

    MoveList legalRootMoves = gen.generateLegalMoves(b, 0);

    // Seed ordering from TT (empty here but mimic anyway)
    Move ttMove(0, 0, NORMAL);
    gen.orderMoves(legalRootMoves, b, 0, ttMove);

    int currentDepth = 1;
    int prevScore = 0;
    int delta = 25;
    int alpha = -999999;
    int beta  =  999999;

    Move absoluteBestMove = legalRootMoves.moves[0];
    Move bestMoveThisDepth = absoluteBestMove;
    int  bestScoreThisDepth = -999999;

    orderMoves_again:
    gen.orderMoves(legalRootMoves, b, 0, bestMoveThisDepth);

    for (int i = 0; i < legalRootMoves.count; i++)
    {
        Move move = legalRootMoves.moves[i];
        b.makeMove(move);

        int score;
        if (i == 0) {
            score = -gen.negamax(b, currentDepth - 1, -beta, -alpha, 1);
        } else {
            score = -gen.negamax(b, currentDepth - 1, -alpha - 1, -alpha, 1);
            if (!gen.searchAborted && score > alpha && score < beta)
                score = -gen.negamax(b, currentDepth - 1, -beta, -alpha, 1);
        }
        b.undoMove();

        std::cout << "i=" << i << " " << move.getFrom() << "->" << move.getTo()
                  << " score=" << score << " (best so far " << bestScoreThisDepth << ")\n";

        if (score > bestScoreThisDepth)
        {
            bestScoreThisDepth = score;
            bestMoveThisDepth  = move;
        }
        if (score > alpha) alpha = score;
    }

    std::cout << "\nBEST: " << bestMoveThisDepth.getFrom() << "->" << bestMoveThisDepth.getTo()
              << " score=" << bestScoreThisDepth << "\n";
    return 0;
}
