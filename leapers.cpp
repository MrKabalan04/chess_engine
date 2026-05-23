#include "generateMoves.h"
#include <iostream>
#include <cstring>
#include <cstdint>
using namespace std;

void GenerateMoves::generateKingMoves(int sq, int side, const Board& board, MoveList& list) {
    uint64_t attacks = kingMasks[sq];
    uint64_t friendlyPieces = (side == 0) ? board.whitePieces : board.blackPieces;
    uint64_t legalDestinations = attacks & ~friendlyPieces;

    while (legalDestinations) {
        int targetSq = __builtin_ctzll(legalDestinations);
        list.addMove(Move(sq, targetSq, NORMAL));
        legalDestinations &= (legalDestinations - 1);
    }

    // === CASTLING ===
    // Only generate if the king is on its starting square.
    // These are pseudo-legal: legality (king not left in check) is
    // verified later by generateLegalMoves. But we DO check that the
    // king doesn't pass through or land on an attacked square here,
    // since that's a castling-specific rule, not a general legality rule.
    if (side == 0 && sq == 4) {
        // White kingside: e1->g1, need f1+g1 empty, e1+f1+g1 not attacked by black
        if ((board.castlingRights & WHITE_CASTLING_KINGSIDE) &&
            !(board.occupied & ((1ULL << 5) | (1ULL << 6))) &&
            !isSquareAttacked(4, 1, board) &&
            !isSquareAttacked(5, 1, board) &&
            !isSquareAttacked(6, 1, board)) {
            list.addMove(Move(4, 6, CASTLE));
        }
        // White queenside: e1->c1, need b1+c1+d1 empty, e1+d1+c1 not attacked by black
        if ((board.castlingRights & WHITE_CASTLING_QUEENSIDE) &&
            !(board.occupied & ((1ULL << 1) | (1ULL << 2) | (1ULL << 3))) &&
            !isSquareAttacked(4, 1, board) &&
            !isSquareAttacked(3, 1, board) &&
            !isSquareAttacked(2, 1, board)) {
            list.addMove(Move(4, 2, CASTLE));
        }
    }
    else if (side == 1 && sq == 60) {
        // Black kingside: e8->g8, need f8+g8 empty, e8+f8+g8 not attacked by white
        if ((board.castlingRights & BLACK_CASTLING_KINGSIDE) &&
            !(board.occupied & ((1ULL << 61) | (1ULL << 62))) &&
            !isSquareAttacked(60, 0, board) &&
            !isSquareAttacked(61, 0, board) &&
            !isSquareAttacked(62, 0, board)) {
            list.addMove(Move(60, 62, CASTLE));
        }
        // Black queenside: e8->c8, need b8+c8+d8 empty, e8+d8+c8 not attacked by white
        if ((board.castlingRights & BLACK_CASTLING_QUEENSIDE) &&
            !(board.occupied & ((1ULL << 57) | (1ULL << 58) | (1ULL << 59))) &&
            !isSquareAttacked(60, 0, board) &&
            !isSquareAttacked(59, 0, board) &&
            !isSquareAttacked(58, 0, board)) {
            list.addMove(Move(60, 58, CASTLE));
        }
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