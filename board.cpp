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

void Board::removePiece(int sq, int pieceType, bool isWhite){
    uint64_t bit = (1ULL << sq);
    if (isWhite)
    {
        whitePieces &= ~bit;
        switch (pieceType)
        {
            case PAWN: whitePawns &= ~bit; break;
            case KNIGHT: whiteKnights &= ~bit; break;
            case BISHOP: whiteBishops &= ~bit; break;
            case ROOK: whiteRooks &= ~bit; break;
            case QUEEN: whiteQueen &= ~bit; break;
            case KING: whiteKing &= ~bit; break;
        }
    }
    else
    {
        blackPieces &= ~bit;
        switch (pieceType)
        {
            case PAWN: blackPawns &= ~bit; break;
            case KNIGHT: blackKnights &= ~bit; break;
            case BISHOP: blackBishops &= ~bit; break;
            case ROOK: blackRooks &= ~bit; break;
            case QUEEN: blackQueen &= ~bit; break;
            case KING: blackKing &= ~bit; break;
        }
    }
    occupied &= ~bit;
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
    int movingPiece = getPieceAt(from);
    if (movingPiece == -1) return;

    // =========================
    // REMOVE PIECE (SAFE)
    // =========================
    removePiece(from, movingPiece, side == 0);

    // =========================
    // CAPTURE
    // =========================
    int captured = getPieceAt(to);

    if (captured != -1)
    {
        removePiece(to, captured, opponent == 0);
    }

    // =========================
    // EN PASSANT
    // =========================
    if (type == EN_PASSANT)
    {
        int capSq = (side == 0) ? (to - 8) : (to + 8);
        uint64_t capBit = 1ULL << capSq;
        removePiece(capSq, PAWN, opponent == 0);
    }

    // =========================
    // PLACE PIECE
    // =========================
    if (type >= PROMOT_QUEEN)
    {
        if (type == PROMOT_QUEEN)  addPiece(to, QUEEN, side == 0);
        if (type == PROMOT_ROOK)   addPiece(to, ROOK, side == 0);
        if (type == PROMOT_BISHOP) addPiece(to, BISHOP, side == 0);
        if (type == PROMOT_KNIGHT) addPiece(to, KNIGHT, side == 0);
    }
    else
    {
        addPiece(to, movingPiece, side == 0);
    }

    // =========================
    // FINAL UPDATE
    // =========================
    enPassantSquare = -1;
    sideToMove ^= 1;

    updateOccupancy();
}