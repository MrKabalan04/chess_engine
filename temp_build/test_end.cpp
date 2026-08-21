// test_end.cpp - is the final convtest position mate or stalemate?
#include <iostream>
#include "board.h"
#include "generateMoves.h"

static bool apply(Board& b, GenerateMoves& gen, const char* uci) {
    MoveList ml = gen.generateLegalMoves(b, b.sideToMove);
    int from = (uci[0]-'a') + (uci[1]-'1')*8;
    int to   = (uci[2]-'a') + (uci[3]-'1')*8;
    for (int i = 0; i < ml.count; i++) {
        if (ml.moves[i].getFrom()==from && ml.moves[i].getTo()==to) {
            b.makeMove(ml.moves[i]);
            return true;
        }
    }
    std::cout << "ILLEGAL: " << uci << "\n";
    return false;
}

int main(int argc, char** argv) {
    Board b;
    GenerateMoves gen;
    gen.init();
    b.initZobrist();
    b.init();
    b.initFromFen(argv[1]);
    for (int i = 2; i < argc; i++)
        if (!apply(b, gen, argv[i])) return 1;
    int stm = b.sideToMove;
    bool inCheck = gen.isInCheck(b, stm);
    MoveList ml = gen.generateLegalMoves(b, stm);
    std::cout << "moves=" << ml.count << " inCheck=" << inCheck
              << " => " << (ml.count==0 ? (inCheck ? "CHECKMATE" : "STALEMATE") : "ongoing") << "\n";
    return 0;
}
