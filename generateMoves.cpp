#include "generateMoves.h"
#include "board.h"
#include <iostream>
#include <cstdint>
using namespace std;

void GenerateMoves::init() {
    for (int i = 0; i < 64; i++) {
        int col = i % 8;
        int row = i / 8;
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

        if (row <= 6){
            kingMoves |= (1ULL << (i + 8));
        }            
        if (row >= 1){
            kingMoves |= (1ULL << (i - 8));
        }            
        if (col <= 6){
            kingMoves |= (1ULL << (i + 1));
        }            
        if (col >= 1){
            kingMoves |= (1ULL << (i - 1));
        }            
        if (row <= 6 && col <= 6){
            kingMoves |= (1ULL << (i + 9));
        } 
        if (row <= 6 && col >= 1){
            kingMoves |= (1ULL << (i + 7));
        }
        if (row >= 1 && col <= 6){
            kingMoves |= (1ULL << (i - 7));
        } 
        if (row >= 1 && col >= 1){
            kingMoves |= (1ULL << (i - 9));
        } 

        kingMasks[i] = kingMoves;
    }    
}

void GenerateMoves::printBitBoard(uint64_t bitboard) {
    for (int row = 7; row >= 0; row--) {
        for (int col = 0; col < 8; col++) {
            int square = row * 8 + col;
            if ((bitboard >> square) & 1ULL)
                cout << " 1 ";
            else
                cout << " . ";
        }
        cout << endl;
    }
    cout << endl;
}