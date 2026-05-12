#include "generateMoves.h"
#include <iostream>
#include <cstring>
#include <cstdint>
using namespace std;

void GenerateMoves::generateKingMoves(int sq, int side, const Board& board, MoveList& list) {
    uint64_t attacks = kingMasks[sq];
    uint64_t friendlyPieces = (side == 0) ? board.whitePieces : board.blackPieces;
    uint64_t legalDestinations = attacks & ~friendlyPieces;
    
    int opponentColor = side ^ 1;

    while (legalDestinations) {
        int targetSq = __builtin_ctzll(legalDestinations); 
        
        if (!isSquareAttacked(targetSq, opponentColor, board)) {
            list.addMove(Move(sq, targetSq, NORMAL));
        }
        
        legalDestinations &= (legalDestinations - 1);
    }
}

void GenerateMoves::generateLeapingMoves(int sq, PieceType type, uint64_t friendlyPieces, MoveList& list) {
    uint64_t moves = (type == KNIGHT) ? knightMasks[sq] : kingMasks[sq];
    moves &= ~friendlyPieces;

    while (moves) {
        int targetSq = __builtin_ctzll(moves);
        list.addMove(Move(sq, targetSq, NORMAL));
        moves &= (moves - 1);
    }
}