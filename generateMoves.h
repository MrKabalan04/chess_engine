#ifndef GENERATEMOVES_H
#define GENERATEMOVES_H
#include <iostream>
#include "board.h"
#include <cstdint>
using namespace std;

class Board;

class GenerateMoves{
    public:

        uint64_t knightMasks[64];
        uint64_t kingMasks[64];

        void init();
        void printBitBoard(uint64_t bitboard);

};




#endif