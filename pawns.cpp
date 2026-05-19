#include "generateMoves.h"
#include <iostream>
#include <cstring>
#include <cstdint>
using namespace std;

void GenerateMoves::initPawnAttacks()
{
    for (int sq = 0; sq < 64; sq++)
    {
        uint64_t bit = 1ULL << sq;

        // attacks FROM a pawn located on sq
        pawnFromMasks[0][sq] =
            ((bit & ~COLUMN_H) << 9) |
            ((bit & ~COLUMN_A) << 7);

        pawnFromMasks[1][sq] =
            ((bit & ~COLUMN_A) >> 9) |
            ((bit & ~COLUMN_H) >> 7);

        // attacks TO the square sq by a pawn of given color
        // white pawns that attack sq are located at sq-7 and sq-9
        pawnToMasks[0][sq] =
            ((bit & ~COLUMN_A) >> 7) |
            ((bit & ~COLUMN_H) >> 9);

        // black pawns that attack sq are located at sq+7 and sq+9
        pawnToMasks[1][sq] =
            ((bit & ~COLUMN_H) << 9) |
            ((bit & ~COLUMN_A) << 7);
    }
}


void GenerateMoves::generatePawnMoves(int sq, int side, uint64_t occupied, uint64_t opponentPieces, MoveList& list, int enPassantSq) {
    uint64_t bit = (1ULL << sq);

    if (side == 0) { // WHITE PAWNS
        // 1. Single Push
        uint64_t singlePush = (bit << 8);
        if (singlePush && !(singlePush & occupied)) {
            // Check if the push results in a Promotion (reaching Row 8)
            if (singlePush & ROW_8) {
                list.addMove(Move(sq, sq + 8, PROMOT_QUEEN));
                list.addMove(Move(sq, sq + 8, PROMOT_ROOK));
                list.addMove(Move(sq, sq + 8, PROMOT_BISHOP));
                list.addMove(Move(sq, sq + 8, PROMOT_KNIGHT));
            } else {
                // Regular move
                list.addMove(Move(sq, sq + 8, NORMAL));
                
                // 2. Double Push (Only if the pawn is on its starting square and path is clear)
                uint64_t doublePush = (singlePush << 8);
                if ((bit & ROW_2) && !(doublePush & occupied)) {
                    list.addMove(Move(sq, sq + 16, DOUBLE_PUSH));
                }
            }
        }

        // 3. Normal Captures
        // Use precomputed attack masks (from-square) and intersect with squares occupied by opponent
        uint64_t attacks = (pawnFromMasks[0][sq] & opponentPieces);
        while (attacks) {
            int targetSq = __builtin_ctzll(attacks);
            // Check if capture results in promotion (pawn was on Row 7)
            if (bit & ROW_7) {
                list.addMove(Move(sq, targetSq, PROMOT_QUEEN));
                list.addMove(Move(sq, targetSq, PROMOT_ROOK));
                list.addMove(Move(sq, targetSq, PROMOT_BISHOP));
                list.addMove(Move(sq, targetSq, PROMOT_KNIGHT));
            } else {
                list.addMove(Move(sq, targetSq, NORMAL));
            }
            attacks &= (attacks - 1); // Clear the processed bit
        }

        // 4. En Passant Capture
        if (enPassantSq != -1) {
            uint64_t epBit = (1ULL << enPassantSq);
            // If the pawn's attack mask (from-square) hits the en passant target square
            if (pawnFromMasks[0][sq] & epBit) {
                list.addMove(Move(sq, enPassantSq, EN_PASSANT));
            }
        }
    } 
    else { // BLACK PAWNS
        // 1. Single Push
        uint64_t singlePush = (bit >> 8);
        if (singlePush && !(singlePush & occupied)) {
            // Check if push results in Promotion (reaching Row 1)
            if (singlePush & ROW_1) {
                list.addMove(Move(sq, sq - 8, PROMOT_QUEEN));
                list.addMove(Move(sq, sq - 8, PROMOT_ROOK));
                list.addMove(Move(sq, sq - 8, PROMOT_BISHOP));
                list.addMove(Move(sq, sq - 8, PROMOT_KNIGHT));
            } else {
                // Regular move
                list.addMove(Move(sq, sq - 8, NORMAL));
                
                // 2. Double Push (Only if on starting row)
                uint64_t doublePush = (singlePush >> 8);
                if ((bit & ROW_7) && !(doublePush & occupied)) {
                    list.addMove(Move(sq, sq - 16, DOUBLE_PUSH));
                }
            }
        }

        // 3. Normal Captures
        uint64_t attacks = (pawnFromMasks[1][sq] & opponentPieces);
        while (attacks) {
            int targetSq = __builtin_ctzll(attacks);
            // Check if capture results in promotion (pawn was on Row 2)
            if (bit & ROW_2) {
                list.addMove(Move(sq, targetSq, PROMOT_QUEEN));
                list.addMove(Move(sq, targetSq, PROMOT_ROOK));
                list.addMove(Move(sq, targetSq, PROMOT_BISHOP));
                list.addMove(Move(sq, targetSq, PROMOT_KNIGHT));
            } else {
                list.addMove(Move(sq, targetSq, NORMAL));
            }
            attacks &= (attacks - 1);
        }

        // 4. En Passant Capture
        if (enPassantSq != -1) {
            uint64_t epBit = (1ULL << enPassantSq);
            if (pawnFromMasks[1][sq] & epBit) {
                list.addMove(Move(sq, enPassantSq, EN_PASSANT));
            }
        }
    }
}