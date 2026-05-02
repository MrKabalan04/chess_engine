#ifndef BOARD_H
#define BOARD_H
#include <iostream>
#include <cstdint>
using namespace std;
class Board{
    public:

        //white pieces
        uint64_t whitePawns = 0ULL;
        uint64_t whiteRooks = 0ULL;
        uint64_t whiteKnights = 0ULL;
        uint64_t whiteBishops = 0ULL;
        uint64_t whiteQueen = 0ULL;
        uint64_t whiteKing = 0ULL;
        
        //black pieces
        uint64_t blackPawns = 0ULL;
        uint64_t blackRooks = 0ULL;
        uint64_t blackKnights = 0ULL;
        uint64_t blackBishops = 0ULL;
        uint64_t blackQueen = 0ULL;
        uint64_t blackKing = 0ULL;

        //other
        uint64_t whitePieces = 0ULL;
        uint64_t blackPieces = 0ULL;
        uint64_t occupied = 0ULL;

        void init();
        void printBoard();
        void clearBoard();
        bool isOccupied(int sq) const;
        int getPieceColor(int sq) const;
        void addPiece(int sq, int pieceType, bool isWhite);
};

#endif