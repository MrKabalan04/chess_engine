#include "generateMoves.h"
#include <iostream>
#include <cstring>
#include <cstdint>
using namespace std;

uint64_t GenerateMoves::getQueenAttacks(int sq, uint64_t occupied) const {
    // A Queen's movement is the logical UNION of a Rook and a Bishop
    return getRookAttacks(sq, occupied) | getBishopAttacks(sq, occupied);
}

uint64_t GenerateMoves::getBishopAttacks(int sq, uint64_t occupied) const{
    occupied &= bishopMasks[sq];
    int idx = (int)((occupied * bishopMagics[sq]) >> bishopShifts[sq]);
    
    return bishopTable[sq][idx];
}

uint64_t GenerateMoves::getRookAttacks (int sq, uint64_t occupied) const {
occupied &= rookMasks[sq];
    int idx = (int)((occupied * rookMagics[sq]) >> rookShifts[sq]);
    return rookTable[sq][idx];
}

void GenerateMoves::generateSlidingMoves(int sq, PieceType type, uint64_t occupied, uint64_t friendlyPieces, MoveList& list){
    uint64_t attacks = 0ULL;
    if (type == ROOK){
        attacks = getRookAttacks(sq, occupied);
    }
    else if (type == BISHOP){
        attacks = getBishopAttacks(sq, occupied);
    }
    else if (type == QUEEN){
        attacks = getQueenAttacks(sq, occupied);
    }

    attacks = attacks & (~((uint64_t)friendlyPieces));

    while (attacks) {
        int targetSq = __builtin_ctzll(attacks);
        list.addMove(Move(sq, targetSq, NORMAL));
        attacks &= (attacks - 1);
    }

}