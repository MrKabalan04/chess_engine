// test_magic.cpp - validate magic bitboard attack tables vs naive ray walk
#include <iostream>
#include <cstdint>
#include "board.h"
#include "generateMoves.h"

static uint64_t naiveRook(int sq, uint64_t occ) {
    uint64_t att = 0;
    int r = sq >> 3, f = sq & 7;
    static const int dr[4] = {1,-1,0,0}, df[4] = {0,0,1,-1};
    for (int d = 0; d < 4; d++) {
        int rr = r + dr[d], ff = f + df[d];
        while (rr >= 0 && rr < 8 && ff >= 0 && ff < 8) {
            int s = rr*8 + ff;
            att |= 1ULL << s;
            if (occ & (1ULL << s)) break;
            rr += dr[d]; ff += df[d];
        }
    }
    return att;
}
static uint64_t naiveBishop(int sq, uint64_t occ) {
    uint64_t att = 0;
    int r = sq >> 3, f = sq & 7;
    static const int dr[4] = {1,-1,1,-1}, df[4] = {1,-1,-1,1};
    for (int d = 0; d < 4; d++) {
        int rr = r + dr[d], ff = f + df[d];
        while (rr >= 0 && rr < 8 && ff >= 0 && ff < 8) {
            int s = rr*8 + ff;
            att |= 1ULL << s;
            if (occ & (1ULL << s)) break;
            rr += dr[d]; ff += df[d];
        }
    }
    return att;
}

int main() {
    Board b;
    GenerateMoves gen;
    gen.init();
    b.initZobrist();
    b.init();
    b.initFromFen("r1bq2rk/pp3pbp/2p1p1pQ/7P/3P4/2PB1N2/PP3PPR/2KR4 w - - 0 1");
    uint64_t occ = b.occupied;

    int badRook = 0, badBishop = 0;
    for (int sq = 0; sq < 64; sq++) {
        // test with several occupancy patterns
        uint64_t pats[4] = { occ, 0ULL, ~0ULL, (1ULL << (sq ^ 7)) | (1ULL << ((sq + 8) & 63)) };
        for (int pi = 0; pi < 4; pi++) {
            uint64_t o = pats[pi] & ~(1ULL << sq);
            if (gen.getRookAttacks(sq, o) != naiveRook(sq, o)) {
                if (badRook < 10) std::cout << "ROOK mismatch sq=" << sq << " pat=" << pi
                    << " magic=" << std::hex << gen.getRookAttacks(sq, o)
                    << " naive=" << naiveRook(sq, o) << std::dec << "\n";
                badRook++;
            }
            if (gen.getBishopAttacks(sq, o) != naiveBishop(sq, o)) {
                if (badBishop < 10) std::cout << "BISHOP mismatch sq=" << sq << " pat=" << pi
                    << " magic=" << std::hex << gen.getBishopAttacks(sq, o)
                    << " naive=" << naiveBishop(sq, o) << std::dec << "\n";
                badBishop++;
            }
        }
    }
    std::cout << "rook mismatches: " << badRook << ", bishop mismatches: " << badBishop << "\n";

    std::cout << "queen attacks h6(47) current occ: "
        << std::hex << gen.getQueenAttacks(47, occ) << std::dec << "\n";
    std::cout << "naive rook h6:   " << std::hex << naiveRook(47, occ) << std::dec << "\n";
    std::cout << "naive bishop h6: " << std::hex << naiveBishop(47, occ) << std::dec << "\n";
    return 0;
}
