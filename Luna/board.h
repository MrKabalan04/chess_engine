#ifndef BOARD_H
#define BOARD_H

#include <cstdint>
#include <string>
#include "types.h"

class Board {
public:

    // ===== PIECE BITBOARDS =====
    uint64_t whitePawns = 0ULL;
    uint64_t whiteKnights = 0ULL;
    uint64_t whiteBishops = 0ULL;
    uint64_t whiteRooks = 0ULL;
    uint64_t whiteQueen = 0ULL;
    uint64_t whiteKing = 0ULL;

    uint64_t blackPawns = 0ULL;
    uint64_t blackKnights = 0ULL;
    uint64_t blackBishops = 0ULL;
    uint64_t blackRooks = 0ULL;
    uint64_t blackQueen = 0ULL;
    uint64_t blackKing = 0ULL;

    // ===== OCCUPANCY =====
    uint64_t whitePieces = 0ULL;
    uint64_t blackPieces = 0ULL;
    uint64_t occupied = 0ULL;

    UndoInfo undoStack[500];
    int undoCount = 0;



    // ===== GAME STATE =====
    int sideToMove = 0; // 0 = white, 1 = black
    int enPassantSquare = -1;
    uint8_t castlingRights = 0;
    int halfMoveClock = 0;   // 50-move rule counter
    uint64_t positionHistory[500];
    int historyCount = 0;
    uint64_t zobristHash = 0;

    // ===== FUNCTIONS =====
    void init();
    bool initFromFen(const std::string& fen);
    void printBoard();
    void clearBoard();
    void updateOccupancy();
    void validateBoard() const;
    void sanityCheck() const;
    void makeNullMove();
    void undoNullMove();

    

    int evaluate();
    int getPieceAt(int sq) const;
    void addPiece(int sq, int pieceType, bool isWhite);
    void removePiece(int sq, int pieceType, bool isWhite);

    void makeMove(const Move& move);

    bool isOccupied(int sq) const;
    void initZobrist();
    bool isThreefoldRepetition() const;
    bool isInsufficientMaterial() const;
    void undoMove();

    uint64_t zobristTable[6][2][64];  // piece, color, type, square
    uint64_t zobristSideToMove;    // 1 number for side
    uint64_t zobristCastling[4];    // 4 castling rights
    uint64_t zobristEnPassant[8];    // 8 files

};

#endif