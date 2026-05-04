#ifndef GENERATEMOVES_H
#define GENERATEMOVES_H
#include <iostream>
#include "board.h"
#include <cstdint>
using namespace std;

class Board;

class GenerateMoves {
public:

    // Precomputed move masks for each piece type and square
    uint64_t knightMasks[64];
    uint64_t kingMasks[64];
    uint64_t rookMasks[64];
    uint64_t bishopMasks[64];

    // Magic bitboards for rook attacks
    uint64_t rookMagics[64];

    // Magic bitboards for bishop attacks
    uint64_t bishopMagics[64];

    // Attack tables for rook moves
    static uint64_t rookTable[64][4096];
    int rookShifts[64];

    // Attack tables for bishop moves
    static uint64_t bishopTable[64][512];
    int bishopShifts[64];

    void init();
    void printBitBoard(uint64_t bitboard);

    void initMagicTables();
    uint64_t rookMask(int sq);
    uint64_t rookAttacksOnTheFly(int sq, uint64_t occupied);
    uint64_t getRookAttacks(int sq, uint64_t occupied);

    uint64_t setOccupancy(int index, uint64_t mask);

    uint64_t bishopMask(int sq);
    uint64_t bishopAttacksOnTheFly(int sq, uint64_t occupied);
    uint64_t getBishopAttacks(int sq, uint64_t occupied);

    uint64_t getQueenAttacks(int sq, uint64_t occupied);
};

#endif
