#include "board.h"
#include <iostream>

using namespace std;

// =========================
// CLEAR BOARD
// =========================
void Board::clearBoard()
{
    whitePawns = whiteKnights = whiteBishops =
    whiteRooks = whiteQueen = whiteKing = 0ULL;

    blackPawns = blackKnights = blackBishops =
    blackRooks = blackQueen = blackKing = 0ULL;

    whitePieces = blackPieces = occupied = 0ULL;

    sideToMove = 0;
    enPassantSquare = -1;
    castlingRights = 0;
}

// =========================
// PRINT BOARD
// =========================
void Board::printBoard()
{
    cout << "a  b  c  d  e  f  g  h" << endl;
    for (int row = 7; row >= 0; row--)
    {
        cout << endl;
        for (int column = 0; column <= 7; column++)
        {
            int square = row * 8 + column;
            string piece = ". ";
            if ((whitePawns >> square) & 1ULL) piece = "WP";
            else if ((whiteRooks >> square) & 1ULL) piece = "WR";
            else if ((whiteKnights >> square) & 1ULL) piece = "WN";
            else if ((whiteBishops >> square) & 1ULL) piece = "WB";
            else if ((whiteQueen >> square) & 1ULL) piece = "WQ";
            else if ((whiteKing >> square) & 1ULL) piece = "WK";
            else if ((blackPawns >> square) & 1ULL) piece = "BP";
            else if ((blackRooks >> square) & 1ULL) piece = "BR";
            else if ((blackKnights >> square) & 1ULL) piece = "BN";
            else if ((blackBishops >> square) & 1ULL) piece = "BB";
            else if ((blackQueen >> square) & 1ULL) piece = "BQ";
            else if ((blackKing >> square) & 1ULL) piece = "BK";
            cout << piece << " ";
        }
    }
    cout << endl;
}

// =========================
// ADD PIECE
// =========================
void Board::addPiece(int sq, int pieceType, bool isWhite)
{
    uint64_t bit = (1ULL << sq);
    if (isWhite)
    {
        whitePieces |= bit;
        switch (pieceType)
        {
            case PAWN: whitePawns |= bit; break;
            case KNIGHT: whiteKnights |= bit; break;
            case BISHOP: whiteBishops |= bit; break;
            case ROOK: whiteRooks |= bit; break;
            case QUEEN: whiteQueen |= bit; break;
            case KING: whiteKing |= bit; break;
        }
    }
    else
    {
        blackPieces |= bit;
        switch (pieceType)
        {
            case PAWN: blackPawns |= bit; break;
            case KNIGHT: blackKnights |= bit; break;
            case BISHOP: blackBishops |= bit; break;
            case ROOK: blackRooks |= bit; break;
            case QUEEN: blackQueen |= bit; break;
            case KING: blackKing |= bit; break;
        }
    }
    occupied |= bit;
}

// =========================
// INIT START POSITION
// =========================
void Board::init()
{
    clearBoard();

    for (int i = 0; i < 8; i++)
    {
        whitePawns |= (1ULL << (8 + i));
        blackPawns |= (1ULL << (48 + i));

        switch (i)
        {
            case 0:
            case 7:
                whiteRooks |= (1ULL << i);
                blackRooks |= (1ULL << (56 + i));
                break;

            case 1:
            case 6:
                whiteKnights |= (1ULL << i);
                blackKnights |= (1ULL << (56 + i));
                break;

            case 2:
            case 5:
                whiteBishops |= (1ULL << i);
                blackBishops |= (1ULL << (56 + i));
                break;

            case 3:
                whiteQueen |= (1ULL << i);
                blackQueen |= (1ULL << (56 + i));
                break;

            case 4:
                whiteKing |= (1ULL << i);
                blackKing |= (1ULL << (56 + i));
                break;
        }
    }

    sideToMove = 0;
    enPassantSquare = -1;
    castlingRights = 15;

    updateOccupancy();
}

// =========================
// OCCUPANCY UPDATE
// =========================
void Board::updateOccupancy()
{
    whitePieces =
        whitePawns | whiteKnights | whiteBishops |
        whiteRooks | whiteQueen | whiteKing;

    blackPieces =
        blackPawns | blackKnights | blackBishops |
        blackRooks | blackQueen | blackKing;

    occupied = whitePieces | blackPieces;
}

// =========================
// PIECE QUERY
// =========================
int Board::getPieceAt(int sq) const
{
    uint64_t bit = 1ULL << sq;

    if (whitePawns & bit) return PAWN;
    if (whiteKnights & bit) return KNIGHT;
    if (whiteBishops & bit) return BISHOP;
    if (whiteRooks & bit) return ROOK;
    if (whiteQueen & bit) return QUEEN;
    if (whiteKing & bit) return KING;

    if (blackPawns & bit) return PAWN;
    if (blackKnights & bit) return KNIGHT;
    if (blackBishops & bit) return BISHOP;
    if (blackRooks & bit) return ROOK;
    if (blackQueen & bit) return QUEEN;
    if (blackKing & bit) return KING;

    return -1;
}

// =========================
// OCCUPIED CHECK
// =========================
bool Board::isOccupied(int sq) const
{
    return (occupied >> sq) & 1ULL;
}

// =========================
// MAKE MOVE 
// =========================
void Board::makeMove(Move move)
{
    int from = move.getFrom();
    int to = move.getTo();
    int type = move.getType();

    int side = sideToMove;
    int opponent = side ^ 1;

    uint64_t fromBit = 1ULL << from;
    uint64_t toBit   = 1ULL << to;

    int movingPiece = getPieceAt(from);
    if (movingPiece == -1) return;

    // =========================
    // REMOVE PIECE (SAFE)
    // =========================
    if (side == 0)
    {
        if (movingPiece == PAWN)   whitePawns &= ~fromBit;
        if (movingPiece == KNIGHT) whiteKnights &= ~fromBit;
        if (movingPiece == BISHOP) whiteBishops &= ~fromBit;
        if (movingPiece == ROOK)   whiteRooks &= ~fromBit;
        if (movingPiece == QUEEN)  whiteQueen &= ~fromBit;
        if (movingPiece == KING)   whiteKing &= ~fromBit;
    }
    else
    {
        if (movingPiece == PAWN)   blackPawns &= ~fromBit;
        if (movingPiece == KNIGHT) blackKnights &= ~fromBit;
        if (movingPiece == BISHOP) blackBishops &= ~fromBit;
        if (movingPiece == ROOK)   blackRooks &= ~fromBit;
        if (movingPiece == QUEEN)  blackQueen &= ~fromBit;
        if (movingPiece == KING)   blackKing &= ~fromBit;
    }

    // =========================
    // CAPTURE
    // =========================
    int captured = getPieceAt(to);

    if (captured != -1)
    {
        if (opponent == 0)
        {
            if (captured == PAWN)   whitePawns &= ~toBit;
            if (captured == KNIGHT) whiteKnights &= ~toBit;
            if (captured == BISHOP) whiteBishops &= ~toBit;
            if (captured == ROOK)   whiteRooks &= ~toBit;
            if (captured == QUEEN)  whiteQueen &= ~toBit;
            if (captured == KING)   whiteKing &= ~toBit;
        }
        else
        {
            if (captured == PAWN)   blackPawns &= ~toBit;
            if (captured == KNIGHT) blackKnights &= ~toBit;
            if (captured == BISHOP) blackBishops &= ~toBit;
            if (captured == ROOK)   blackRooks &= ~toBit;
            if (captured == QUEEN)  blackQueen &= ~toBit;
            if (captured == KING)   blackKing &= ~toBit;
        }
    }

    // =========================
    // EN PASSANT
    // =========================
    if (type == EN_PASSANT)
    {
        int capSq = (side == 0) ? (to - 8) : (to + 8);
        uint64_t capBit = 1ULL << capSq;

        if (side == 0) blackPawns &= ~capBit;
        else           whitePawns &= ~capBit;
    }

    // =========================
    // PLACE PIECE
    // =========================
    if (type >= PROMOT_QUEEN)
    {
        if (side == 0)
        {
            if (type == PROMOT_QUEEN)  whiteQueen |= toBit;
            if (type == PROMOT_ROOK)   whiteRooks |= toBit;
            if (type == PROMOT_BISHOP) whiteBishops |= toBit;
            if (type == PROMOT_KNIGHT) whiteKnights |= toBit;
        }
        else
        {
            if (type == PROMOT_QUEEN)  blackQueen |= toBit;
            if (type == PROMOT_ROOK)   blackRooks |= toBit;
            if (type == PROMOT_BISHOP) blackBishops |= toBit;
            if (type == PROMOT_KNIGHT) blackKnights |= toBit;
        }
    }
    else
    {
        if (side == 0)
        {
            if (movingPiece == PAWN)   whitePawns |= toBit;
            if (movingPiece == KNIGHT) whiteKnights |= toBit;
            if (movingPiece == BISHOP) whiteBishops |= toBit;
            if (movingPiece == ROOK)   whiteRooks |= toBit;
            if (movingPiece == QUEEN)  whiteQueen |= toBit;
            if (movingPiece == KING)   whiteKing |= toBit;
        }
        else
        {
            if (movingPiece == PAWN)   blackPawns |= toBit;
            if (movingPiece == KNIGHT) blackKnights |= toBit;
            if (movingPiece == BISHOP) blackBishops |= toBit;
            if (movingPiece == ROOK)   blackRooks |= toBit;
            if (movingPiece == QUEEN)  blackQueen |= toBit;
            if (movingPiece == KING)   blackKing |= toBit;
        }
    }

    // =========================
    // FINAL UPDATE
    // =========================
    enPassantSquare = -1;
    sideToMove ^= 1;

    updateOccupancy();
}