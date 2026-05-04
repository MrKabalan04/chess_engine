#include "generateMoves.h"
#include <iostream>
#include <cstring>
#include <cstdint>

using namespace std;

uint64_t GenerateMoves::rookTable[64][4096];

void GenerateMoves::init() {
    
    static const uint64_t rookMagicsLocal[64] = {
        0xa8ed120078004402ULL, 0x0002008010403001ULL, 0x0004060020100405ULL, 0x0001010020200408ULL,
        0x0004100020002801ULL, 0x0002008008004001ULL, 0x0001004002004001ULL, 0x8000400020001001ULL,
        0x0001004020082001ULL, 0x0008040008002001ULL, 0x0004008008002001ULL, 0x0004010040010005ULL,
        0x0004010040040001ULL, 0x0008020010004005ULL, 0x0001004020040005ULL, 0x0004004010041002ULL,
        0x0002100040001001ULL, 0x0004400080001001ULL, 0x0001080006000501ULL, 0x0001002010004005ULL,
        0x0004012008004001ULL, 0x0002004008001001ULL, 0x0002004010004001ULL, 0x0002004080004001ULL,
        0x0002104000080801ULL, 0x0001020200100808ULL, 0x0001002100100804ULL, 0x0002008080010001ULL,
        0x0002020080040081ULL, 0x0002020040040041ULL, 0x0002020040040001ULL, 0x0002004008004001ULL,
        0x0002004004000801ULL, 0x0002004001000802ULL, 0x0008080040010002ULL, 0x0008080040010001ULL,
        0x00010101010108ULL, 0x00010101010104ULL, 0x00010101010102ULL, 0x0008080040010002ULL,
        0x0008008000400801ULL, 0x0008004000400801ULL, 0x0004004000400801ULL, 0x0004004000401001ULL,
        0x0002004000401001ULL, 0x0008004000400401ULL, 0x00010101010102ULL, 0x00010101010101ULL,
        0x0004002000401001ULL, 0x0008001000401001ULL, 0x0008004000801001ULL, 0x00010101010104ULL,
        0x0004008020001001ULL, 0x0008004010001001ULL, 0x00010101010102ULL, 0x00010101010101ULL,
        0x00010101010101ULL, 0x00010101010101ULL, 0x00010101010101ULL, 0x00010101010101ULL,
        0x00010101010101ULL, 0x00010101010101ULL, 0x00010101010101ULL, 0x00010101010101ULL
    };

    static const int rookRelevantBits[64] = {
        12, 11, 11, 11, 11, 11, 11, 12,
        11, 10, 10, 10, 10, 10, 10, 11,
        11, 10, 10, 10, 10, 10, 10, 11,
        11, 10, 10, 10, 10, 10, 10, 11,
        11, 10, 10, 10, 10, 10, 10, 11,
        11, 10, 10, 10, 10, 10, 10, 11,
        11, 10, 10, 10, 10, 10, 10, 11,
        12, 11, 11, 11, 11, 11, 11, 12
    };

    for (int i = 0; i < 64; i++) {
        int col = i % 8;
        int row = i / 8;

        this->rookMagics[i] = rookMagicsLocal[i];
        this->rookShifts[i] = 64 - rookRelevantBits[i];  

        uint64_t knightMoves = 0ULL;
        uint64_t kingMoves = 0ULL;
        if (row <= 5 && col <= 6) knightMoves |= (1ULL << (i + 17));
        if (row <= 5 && col >= 1) knightMoves |= (1ULL << (i + 15));
        if (row <= 6 && col <= 5) knightMoves |= (1ULL << (i + 10));
        if (row <= 6 && col >= 2) knightMoves |= (1ULL << (i + 6));
        if (row >= 2 && col >= 1) knightMoves |= (1ULL << (i - 17));
        if (row >= 2 && col <= 6) knightMoves |= (1ULL << (i - 15));
        if (row >= 1 && col >= 2) knightMoves |= (1ULL << (i - 10));
        if (row >= 1 && col <= 5) knightMoves |= (1ULL << (i - 6));
        knightMasks[i] = knightMoves;

        if (row <= 6) kingMoves |= (1ULL << (i + 8));
        if (row >= 1) kingMoves |= (1ULL << (i - 8));
        if (col <= 6) kingMoves |= (1ULL << (i + 1));
        if (col >= 1) kingMoves |= (1ULL << (i - 1));
        if (row <= 6 && col <= 6) kingMoves |= (1ULL << (i + 9));
        if (row <= 6 && col >= 1) kingMoves |= (1ULL << (i + 7));
        if (row >= 1 && col <= 6) kingMoves |= (1ULL << (i - 7));
        if (row >= 1 && col >= 1) kingMoves |= (1ULL << (i - 9));
        kingMasks[i] = kingMoves;

        rookMasks[i] = rookMask(i);
    }

    initMagicTables();
}

void GenerateMoves::initMagicTables() {
    memset(rookTable, 0, sizeof(rookTable));

    for (int sq = 0; sq < 64; sq++) {
        uint64_t mask = rookMasks[sq];
        int bits = __builtin_popcountll(mask);
        int variations = 1 << bits;

        for (int i = 0; i < variations; i++) {
            uint64_t occ = setOccupancy(i, mask);
            int idx = (int)((occ * rookMagics[sq]) >> rookShifts[sq]);
            rookTable[sq][idx] = rookAttacksOnTheFly(sq, occ);
        }
    }
}

uint64_t GenerateMoves::getRookAttacks(int sq, uint64_t occupied) {
    occupied &= rookMasks[sq];
    int idx = (int)((occupied * rookMagics[sq]) >> rookShifts[sq]);
    return rookTable[sq][idx];
}

void GenerateMoves::printBitBoard(uint64_t bitboard) {
    cout << "\n  a b c d e f g h\n  ----------------\n";
    for (int row = 7; row >= 0; row--) {
        cout << row + 1 << "|";
        for (int col = 0; col < 8; col++) {
            int sq = row * 8 + col;
            cout << (((bitboard >> sq) & 1ULL) ? " 1" : " .");
        }
        cout << " |" << row + 1 << "\n";
    }
    cout << "  ----------------\n  a b c d e f g h\n\n";
}

uint64_t GenerateMoves::rookMask(int sq) {
    uint64_t mask = 0ULL;
    int r = sq / 8, c = sq % 8;
    for (int i = c + 1; i <= 6; i++) mask |= (1ULL << (r * 8 + i));
    for (int i = c - 1; i >= 1; i--) mask |= (1ULL << (r * 8 + i));
    for (int i = r + 1; i <= 6; i++) mask |= (1ULL << (i * 8 + c));
    for (int i = r - 1; i >= 1; i--) mask |= (1ULL << (i * 8 + c));
    return mask;
}

uint64_t GenerateMoves::rookAttacksOnTheFly(int sq, uint64_t occ) {
    uint64_t attacks = 0ULL;
    int r = sq / 8, c = sq % 8;
    for (int i = c + 1; i <= 7; i++) { uint64_t b = (1ULL << (r * 8 + i)); attacks |= b; if (b & occ) break; }
    for (int i = c - 1; i >= 0; i--) { uint64_t b = (1ULL << (r * 8 + i)); attacks |= b; if (b & occ) break; }
    for (int i = r + 1; i <= 7; i++) { uint64_t b = (1ULL << (i * 8 + c)); attacks |= b; if (b & occ) break; }
    for (int i = r - 1; i >= 0; i--) { uint64_t b = (1ULL << (i * 8 + c)); attacks |= b; if (b & occ) break; }
    return attacks;
}

uint64_t GenerateMoves::setOccupancy(int index, uint64_t mask) {
    uint64_t occ = 0ULL;
    int bits = __builtin_popcountll(mask);
    for (int i = 0; i < bits; i++) {
        int sq = __builtin_ctzll(mask);
        mask &= mask - 1;
        if (index & (1 << i)) occ |= (1ULL << sq);
    }
    return occ;
}