#include "generateMoves.h"
#include <iostream>
#include <cstring>
#include <cstdint>
using namespace std;

// Utility function to slide in a given direction until a blocker is hit
uint64_t slideInDirection(int sq, uint64_t occ, int rowStep, int colStep){
    int row = sq / 8;
    int col = sq % 8;
    uint64_t attacks = 0ULL;
    while (row >= 0 && row <= 7 && col >= 0 && col <= 7){
        row += rowStep;
        col += colStep;
        if (row < 0 || row > 7 || col < 0 || col > 7) break;
        uint64_t bit = (1ULL << (row * 8 + col));
        attacks |= bit;
        if (bit & occ) break;
    }
    return attacks;
}

inline int getIndexOfLSB(uint64_t bitboard) {
    if (bitboard == 0) return -1; 
    return __builtin_ctzll(bitboard);
}


//**************************MAGIC TABLE INITIALIZATION*******************************//
void GenerateMoves::initMagicTables() {
    memset(rookTable, 0, sizeof(rookTable));

    for (int sq = 0; sq < 64; sq++) {
        uint64_t mask = rookMasks[sq];
        int bits = __builtin_popcountll(mask);
        int variations = 1 << bits;

        for (int i = 0; i < variations; i++) {
            uint64_t occ = setOccupancy(i, mask);
            int idx = (int)((occ * rookMagics[sq]) >> rookShifts[sq]);
            
            // Debugger for Rooks
            uint64_t actualAttacks = rookAttacksOnTheFly(sq, occ);
            if (rookTable[sq][idx] != 0ULL && rookTable[sq][idx] != actualAttacks) {
                printf("[ROOK] Collision! Sq: %d, Index: %d\n", sq, idx);
            }
            
            rookTable[sq][idx] = actualAttacks;
        }
    }

    memset(bishopTable, 0, sizeof(bishopTable));
    for(int sq = 0; sq < 64; sq++){
        uint64_t mask = bishopMasks[sq];
        int bits = __builtin_popcountll(mask);
        int variations = 1 << bits;

        for (int i = 0; i < variations; i++) {
            uint64_t occ = setOccupancy(i, mask);
            int idx = (int)((occ * bishopMagics[sq]) >> bishopShifts[sq]);

            // Debugger for Bishops
            uint64_t actualAttacks = bishopAttacksOnTheFly(sq, occ);
            if (bishopTable[sq][idx] != 0ULL && bishopTable[sq][idx] != actualAttacks) {
                printf("[BISHOP] Collision! Sq: %d, Index: %d\n", sq, idx);
            }

            bishopTable[sq][idx] = actualAttacks;
        }
    }
}


//**************************************************************************************************//
// Utility function to set occupancy based on index and mask (Used for Magic Bitboard Initialization)
//**************************************************************************************************//
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

//**************************MASKS*************************************//
//********************************************************************//

//ROOK MASKS
uint64_t GenerateMoves::rookMask(int sq) {
    uint64_t mask = 0ULL;
    int r = sq / 8, c = sq % 8;
    for (int i = c + 1; i <= 6; i++) mask |= (1ULL << (r * 8 + i));
    for (int i = c - 1; i >= 1; i--) mask |= (1ULL << (r * 8 + i));
    for (int i = r + 1; i <= 6; i++) mask |= (1ULL << (i * 8 + c));
    for (int i = r - 1; i >= 1; i--) mask |= (1ULL << (i * 8 + c));
    return mask;
}

//BISHOP MASKS
uint64_t GenerateMoves::bishopMask(int sq){
    uint64_t mask = 0ULL;
    int r = sq / 8;
    int c = sq % 8;

    for (int tr = r + 1, tc = c + 1; tr <= 6 && tc <= 6; tr++, tc++) {
        mask |= (1ULL << (tr * 8 + tc));
    }
    for (int tr = r + 1, tc = c - 1; tr <= 6 && tc >= 1; tr++, tc--) {
        mask |= (1ULL << (tr * 8 + tc));
    }
    for (int tr = r - 1, tc = c + 1; tr >= 1 && tc <= 6; tr--, tc++) {
        mask |= (1ULL << (tr * 8 + tc));
    }
    for (int tr = r - 1, tc = c - 1; tr >= 1 && tc >= 1; tr--, tc--) {
        mask |= (1ULL << (tr * 8 + tc));
    }

    return mask;
}

//***********************ATTACKS ON FLY **************************//
//****************************************************************//

//Rook Attacks
uint64_t GenerateMoves::rookAttacksOnTheFly(int sq, uint64_t occ) {
    return slideInDirection(sq, occ, 0, 1) | // Right
           slideInDirection(sq, occ, 0, -1) | // Left
           slideInDirection(sq, occ, 1, 0) | // Down
           slideInDirection(sq, occ, -1, 0);  // Up
}

//Bishop Attacks
uint64_t GenerateMoves::bishopAttacksOnTheFly(int sq, uint64_t occ){
    return slideInDirection(sq, occ, 1, 1) | // Down-Right
           slideInDirection(sq, occ, 1, -1) | // Down-Left
           slideInDirection(sq, occ, -1, 1) | // Up-Right
           slideInDirection(sq, occ, -1, -1);  // Up-Left

}



//Print Bitboard (For Debugging)
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

