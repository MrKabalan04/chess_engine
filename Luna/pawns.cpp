#include "generateMoves.h"
#include <cstdint>

using namespace std;

void GenerateMoves::initPawnAttacks()
{
    const uint64_t FILE_A = 0x0101010101010101ULL;
    const uint64_t FILE_H = 0x8080808080808080ULL;

    for (int sq = 0; sq < 64; sq++)
    {
        pawnToMasks[0][sq] = 0ULL;
        pawnToMasks[1][sq] = 0ULL;
        pawnFromMasks[0][sq] = 0ULL;
        pawnFromMasks[1][sq] = 0ULL;

        uint64_t bit = (1ULL << sq);

        // WHITE pawn attacks FROM square
        if ((bit & ~FILE_A) && sq <= 55)
            pawnFromMasks[0][sq] |= (1ULL << (sq + 7));

        if ((bit & ~FILE_H) && sq <= 54)
            pawnFromMasks[0][sq] |= (1ULL << (sq + 9));

        // BLACK pawn attacks FROM square
        if ((bit & ~FILE_H) && sq >= 8)
            pawnFromMasks[1][sq] |= (1ULL << (sq - 7));

        if ((bit & ~FILE_A) && sq >= 9)
            pawnFromMasks[1][sq] |= (1ULL << (sq - 9));

        // WHITE pawn attackers TO square
        if ((bit & ~FILE_A) && sq >= 9)
            pawnToMasks[0][sq] |= (1ULL << (sq - 9));

        if ((bit & ~FILE_H) && sq >= 8)
            pawnToMasks[0][sq] |= (1ULL << (sq - 7));

        // BLACK pawn attackers TO square
        // sq+7 goes upper-left (lower file): attacker pawn can't be on FILE_A
        if ((bit & ~FILE_A) && sq <= 54)
            pawnToMasks[1][sq] |= (1ULL << (sq + 7));

        // sq+9 goes upper-right (higher file): attacker pawn can't be on FILE_H
        if ((bit & ~FILE_H) && sq <= 55)
            pawnToMasks[1][sq] |= (1ULL << (sq + 9));
    }
}

void GenerateMoves::generatePawnMoves(
    int sq,
    int side,
    uint64_t occupied,
    uint64_t opponentPieces,
    MoveList& list,
    int enPassantSq)
{
    uint64_t bit = (1ULL << sq);

    if (side == 0) // WHITE
    {
        uint64_t one = bit << 8;

        if (!(one & occupied))
        {
            if (one & ROW_8)
            {
                list.addMove(Move(sq, sq + 8, PROMOT_QUEEN));
                list.addMove(Move(sq, sq + 8, PROMOT_ROOK));
                list.addMove(Move(sq, sq + 8, PROMOT_BISHOP));
                list.addMove(Move(sq, sq + 8, PROMOT_KNIGHT));
            }
            else
            {
                list.addMove(Move(sq, sq + 8, NORMAL));

                if ((bit & ROW_2) && !(occupied & (one << 8)))
                    list.addMove(Move(sq, sq + 16, DOUBLE_PUSH));
            }
        }

        uint64_t attacks = pawnFromMasks[0][sq] & opponentPieces;

        while (attacks)
        {
            int to = __builtin_ctzll(attacks);

            if (bit & ROW_7)
            {
                list.addMove(Move(sq, to, PROMOT_QUEEN));
                list.addMove(Move(sq, to, PROMOT_ROOK));
                list.addMove(Move(sq, to, PROMOT_BISHOP));
                list.addMove(Move(sq, to, PROMOT_KNIGHT));
            }
            else
            {
                list.addMove(Move(sq, to, NORMAL));
            }

            attacks &= attacks - 1;
        }

        if (enPassantSq != -1 &&
            (pawnFromMasks[0][sq] & (1ULL << enPassantSq)))
        {
            list.addMove(Move(sq, enPassantSq, EN_PASSANT));
        }
    }
    else // BLACK
    {
        uint64_t one = bit >> 8;

        if (!(one & occupied))
        {
            if (one & ROW_1)
            {
                list.addMove(Move(sq, sq - 8, PROMOT_QUEEN));
                list.addMove(Move(sq, sq - 8, PROMOT_ROOK));
                list.addMove(Move(sq, sq - 8, PROMOT_BISHOP));
                list.addMove(Move(sq, sq - 8, PROMOT_KNIGHT));
            }
            else
            {
                list.addMove(Move(sq, sq - 8, NORMAL));

                if ((bit & ROW_7) && !(occupied & (one >> 8)))
                    list.addMove(Move(sq, sq - 16, DOUBLE_PUSH));
            }
        }

        uint64_t attacks = pawnFromMasks[1][sq] & opponentPieces;

        while (attacks)
        {
            int to = __builtin_ctzll(attacks);

            if (bit & ROW_2)
            {
                list.addMove(Move(sq, to, PROMOT_QUEEN));
                list.addMove(Move(sq, to, PROMOT_ROOK));
                list.addMove(Move(sq, to, PROMOT_BISHOP));
                list.addMove(Move(sq, to, PROMOT_KNIGHT));
            }
            else
            {
                list.addMove(Move(sq, to, NORMAL));
            }

            attacks &= attacks - 1;
        }

        if (enPassantSq != -1 &&
            (pawnFromMasks[1][sq] & (1ULL << enPassantSq)))
        {
            list.addMove(Move(sq, enPassantSq, EN_PASSANT));
        }
    }
}