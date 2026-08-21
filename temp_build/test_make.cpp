// test_make.cpp - makeMove/undoMove integrity per piece type
#include <iostream>
#include <cstdint>
#include "board.h"
#include "generateMoves.h"

static void dump(Board& b, const char* tag) {
    std::cout << tag << ": occ=" << std::hex << b.occupied
        << " wp=" << b.whitePawns << " wq=" << b.whiteQueen
        << " bp=" << b.blackPawns << std::dec
        << " stm=" << b.sideToMove << "\n";
}

int main() {
    Board b;
    GenerateMoves gen;
    gen.init();
    b.initZobrist();
    b.init();
    b.initFromFen("r1bq2rk/pp3pbp/2p1p1pQ/7P/3P4/2PB1N2/PP3PPR/2KR4 w - - 0 1");

    uint64_t occ0 = b.occupied, wq0 = b.whiteQueen, bp0 = b.blackPawns;

    // Qh6-g5 : quiet diagonal queen move (from 47 to 38)
    Move m(47, 38, NORMAL);
    std::cout << "before: whiteInCheck=" << gen.isInCheck(b, 0) << "\n";
    b.makeMove(m);
    std::cout << "after Qg5: whiteInCheck=" << gen.isInCheck(b, 0)
        << " wq=" << std::hex << b.whiteQueen << " (expect bit38=0x4000000000)"
        << " occ=" << b.occupied << std::dec << "\n";
    b.undoMove();
    std::cout << "after undo: occ match=" << (b.occupied == occ0)
        << " wq match=" << (b.whiteQueen == wq0)
        << " stm=" << b.sideToMove << "\n";

    // Qxh7 : capture (47 -> 55)
    Move m2(47, 55, NORMAL);
    b.makeMove(m2);
    std::cout << "after Qxh7: blackInCheck=" << gen.isInCheck(b, 1)
        << " h7piece=" << b.getPieceAt(55)
        << " bp=" << std::hex << b.blackPawns << std::dec << "\n";
    MoveList bm = gen.generateLegalMoves(b, 1);
    std::cout << "black legal replies: " << bm.count << " (expect 0 = mate)\n";
    b.undoMove();
    std::cout << "after undo2: occ match=" << (b.occupied == occ0)
        << " bp match=" << (b.blackPawns == bp0) << "\n";
    return 0;
}
