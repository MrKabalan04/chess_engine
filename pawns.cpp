#include "generateMoves.h"
#include <iostream>
#include <cstring>
#include <cstdint>
using namespace std;

void GenerateMoves::initPawnAttacks() 
{
    // Constant column boundaries to prevent pawn vectors from wrapping horizontally
    const uint64_t FILE_A = 0x0101010101010101ULL;
    const uint64_t FILE_H = 0x8080808080808080ULL;

    // Loop through all 64 squares on the grid
    for (int sq = 0; sq < 64; sq++) 
    {
        // Clear all arrays before writing bits
        pawnToMasks[0][sq] = 0ULL;
        pawnToMasks[1][sq] = 0ULL;
        pawnFromMasks[0][sq] = 0ULL;
        pawnFromMasks[1][sq] = 0ULL;

        uint64_t bit = (1ULL << sq);

        // ====================================================================
        // 1. POPULATE PAWN-FROM-MASKS (Where a pawn on 'sq' can attack to)
        // ====================================================================
        
        // WHITE PAWN ATTACKS FROM 'sq' (Moving up: sq + 7, sq + 9)
        if (sq <= 55) {
            if ((bit & ~FILE_A) != 0ULL) pawnFromMasks[0][sq] |= (1ULL << (sq + 7));
            if ((bit & ~FILE_H) != 0ULL) pawnFromMasks[0][sq] |= (1ULL << (sq + 9));
        }

        // BLACK PAWN ATTACKS FROM 'sq' (Moving down: sq - 9, sq - 7)
        if (sq >= 8) {
            if ((bit & ~FILE_A) != 0ULL) pawnFromMasks[1][sq] |= (1ULL << (sq - 9));
            if ((bit & ~FILE_H) != 0ULL) pawnFromMasks[1][sq] |= (1ULL << (sq - 7));
        }

        // ====================================================================
        // 2. POPULATE PAWN-TO-MASKS (Where pawns must stand to attack 'sq')
        // ====================================================================
        
        // WHITE ATTACKERS TARGETING 'sq' (Must stand below: sq - 9, sq - 7)
        if (sq >= 8) {
            if ((bit & ~FILE_A) != 0ULL) pawnToMasks[0][sq] |= (1ULL << (sq - 9));
            if ((bit & ~FILE_H) != 0ULL) pawnToMasks[0][sq] |= (1ULL << (sq - 7));
        }

        // BLACK ATTACKERS TARGETING 'sq' (Must stand above: sq + 7, sq + 9)
        if (sq <= 55) {
            if ((bit & ~FILE_A) != 0ULL) pawnToMasks[1][sq] |= (1ULL << (sq + 7));
            if ((bit & ~FILE_H) != 0ULL) pawnToMasks[1][sq] |= (1ULL << (sq + 9));
        }
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