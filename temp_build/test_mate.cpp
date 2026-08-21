// test_mate.cpp - which move survives after Qxh7, and why
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
    b.initFromFen("r1bq2rk/pp3pbp/2p1p1pQ/7P/3P4/2PB1N2/PP3PPR/2KR4 w - - 0 1");

    b.makeMove(Move(47, 55, NORMAL)); // Qxh7

    MoveList bm = gen.generateLegalMoves(b, 1);
    std::cout << "black legal replies: " << bm.count << "\n";
    for (int i = 0; i < bm.count; i++)
        std::cout << "  " << bm.moves[i].getFrom() << "->" << bm.moves[i].getTo()
                  << " type=" << bm.moves[i].getType() << "\n";

    // manually play each candidate reply the engine might consider and inspect
    // Kxh7 = 63->55
    Board c = b;
    c.makeMove(Move(63, 55, NORMAL));
    std::cout << "after Kxh7: blackKing=" << std::hex << c.blackKing << std::dec << "\n";
    std::cout << "  h7 attacked by white: " << gen.isSquareAttacked(55, 0, c) << "\n";
    std::cout << "  whiteRooks=" << std::hex << c.whiteRooks << " occupied=" << c.occupied << std::dec << "\n";

    uint64_t ray = gen.getRookAttacks(55, c.occupied);
    std::cout << "  rookAttacks(h7)=" << std::hex << ray << " & rooks=" << (ray & c.whiteRooks) << std::dec << "\n";
    return 0;
}
