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

void Board::removePiece(int sq, int pieceType, bool isWhite)
{
    uint64_t bit = (1ULL << sq);

    if (isWhite)
    {
        whitePieces &= ~bit;

        switch (pieceType)
        {
            case PAWN:   whitePawns &= ~bit; break;
            case KNIGHT: whiteKnights &= ~bit; break;
            case BISHOP: whiteBishops &= ~bit; break;
            case ROOK:   whiteRooks &= ~bit; break;
            case QUEEN:  whiteQueen &= ~bit; break;
            case KING:   whiteKing &= ~bit; break;
        }
    }
    else
    {
        blackPieces &= ~bit;

        switch (pieceType)
        {
            case PAWN:   blackPawns &= ~bit; break;
            case KNIGHT: blackKnights &= ~bit; break;
            case BISHOP: blackBishops &= ~bit; break;
            case ROOK:   blackRooks &= ~bit; break;
            case QUEEN:  blackQueen &= ~bit; break;
            case KING:   blackKing &= ~bit; break;
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
    // REMOVE PIECE FROM START
    // =========================
    removePiece(from, movingPiece, side == 0);

    // =========================
    // CASTLING (SPECIAL CASE)
    // =========================
    if (type == CASTLE)
    {
        // PLACE KING
        addPiece(to, KING, side == 0);

        // WHITE
        if (side == 0)
        {
            if (from == 4 && to == 6)
            {
                removePiece(7, ROOK, true);
                addPiece(5, ROOK, true);
            }
            else if (from == 4 && to == 2)
            {
                removePiece(0, ROOK, true);
                addPiece(3, ROOK, true);
            }
        }
        // BLACK
        else
        {
            if (from == 60 && to == 62)
            {
                removePiece(63, ROOK, false);
                addPiece(61, ROOK, false);
            }
            else if (from == 60 && to == 58)
            {
                removePiece(56, ROOK, false);
                addPiece(59, ROOK, false);
            }
        }
    }
    else
    {
        // =========================
        // CAPTURE
        // =========================
        int captured = getPieceAt(to);

        if (captured != -1)
        {
            removePiece(to, captured, opponent == 0);

            // rook capture → remove castling rights
            if (captured == ROOK)
            {
                if (to == 0)  castlingRights &= ~WHITE_CASTLING_QUEENSIDE;
                if (to == 7)  castlingRights &= ~WHITE_CASTLING_KINGSIDE;
                if (to == 56) castlingRights &= ~BLACK_CASTLING_QUEENSIDE;
                if (to == 63) castlingRights &= ~BLACK_CASTLING_KINGSIDE;
            }
        }

        // =========================
        // EN PASSANT
        // =========================
        if (type == EN_PASSANT)
        {
            int capSq = (side == 0) ? (to - 8) : (to + 8);
            removePiece(capSq, PAWN, opponent == 0);
        }

        // =========================
        // PROMOTION OR NORMAL MOVE
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
    }

    // =========================
    // CASTLING RIGHTS UPDATE
    // =========================
    if (movingPiece == KING)
    {
        if (side == 0)
            castlingRights &= ~(WHITE_CASTLING_KINGSIDE | WHITE_CASTLING_QUEENSIDE);
        else
            castlingRights &= ~(BLACK_CASTLING_KINGSIDE | BLACK_CASTLING_QUEENSIDE);
    }
    else if (movingPiece == ROOK)
    {
        if (side == 0)
        {
            if (from == 0) castlingRights &= ~WHITE_CASTLING_QUEENSIDE;
            else if (from == 7) castlingRights &= ~WHITE_CASTLING_KINGSIDE;
        }
        else
        {
            if (from == 56) castlingRights &= ~BLACK_CASTLING_QUEENSIDE;
            else if (from == 63) castlingRights &= ~BLACK_CASTLING_KINGSIDE;
        }
    }

    // =========================
    // EN PASSANT SQUARE UPDATE
    // =========================
    if (type == DOUBLE_PUSH)
        enPassantSquare = (side == 0) ? (to - 8) : (to + 8);
    else
        enPassantSquare = -1;

    // =========================
    // FINALIZE TURN
    // =========================
    sideToMove ^= 1;
    updateOccupancy();
}

// =========================
// EVALUATION
// =========================

// ===== PIECES TABLE =====
    static const int pawnTable[64] = {
            0,  0,  0,  0,  0,  0,  0,  0,
            50, 50, 50, 50, 50, 50, 50, 50,
            10, 10, 20, 30, 30, 20, 10, 10,
            5,  5, 10, 25, 25, 10,  5,  5,
            0,  0,  0, 20, 20,  0,  0,  0,
            5, -5,-10,  0,  0,-10, -5,  5,
            5, 10, 10,-20,-20, 10, 10,  5,
            0,  0,  0,  0,  0,  0,  0,  0
        };         

    static const int knightTable[64] = {
            -50,-40,-30,-30,-30,-30,-40,-50,
            -40,-20,  0,  0,  0,  0,-20,-40,
            -30,  0, 10, 15, 15, 10,  0,-30,
            -30,  5, 15, 20, 20, 15,  5,-30,
            -30,  0, 15, 20, 20, 15,  0,-30,
            -30,  5, 10, 15, 15, 10,  5,-30,
            -40,-20,  0,  5,  5,  0,-20,-40,
            -50,-40,-30,-30,-30,-30,-40,-50,
        };

    static const int bishopTable[64] = {
            -20,-10,-10,-10,-10,-10,-10,-20,
            -10,  0,  0,  0,  0,  0,  0,-10,
            -10,  0,  5, 10, 10,  5,  0,-10,
            -10,  5,  5, 10, 10,  5,  5,-10,
            -10,  0, 10, 10, 10, 10,  0,-10,
            -10, 10, 10, 10, 10, 10, 10,-10,
            -10,  5,  0,  0,  0,  0,  5,-10,
            -20,-10,-10,-10,-10,-10,-10,-20,
        };

    static const int rookTable[64] = {
        0,  0,  0,  0,  0,  0,  0,  0,
        5, 10, 10, 10, 10, 10, 10,  5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        0,  0,  0,  5,  5,  0,  0,  0
    };

    static const int queenTable[64] = {
        -20,-10,-10, -5, -5,-10,-10,-20,
        -10,  0,  0,  0,  0,  0,  0,-10,
        -10,  0,  5,  5,  5,  5,  0,-10,
        -5,  0,  5,  5,  5,  5,  0, -5,
        0,  0,  5,  5,  5,  5,  0, -5,
        -10,  5,  5,  5,  5,  5,  0,-10,
        -10,  0,  5,  0,  0,  0,  0,-10,
        -20,-10,-10, -5, -5,-10,-10,-20
    };

    static const int kingTable[64] = {
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -20,-30,-30,-40,-40,-30,-30,-20,
    -10,-20,-20,-20,-20,-20,-20,-10,
    20, 20,  0,  0,  0,  0, 20, 20,
    20, 30, 10,  0,  0, 10, 30, 20
    };

    static const int kingEndgameTable[64] = {
        -50,-40,-30,-20,-20,-30,-40,-50,
        -30,-20,-10,  0,  0,-10,-20,-30,
        -30,-10, 20, 30, 30, 20,-10,-30,
        -30,-10, 30, 40, 40, 30,-10,-30,
        -30,-10, 30, 40, 40, 30,-10,-30,
        -30,-10, 20, 30, 30, 20,-10,-30,
        -30,-30,  0,  0,  0,  0,-30,-30,
        -50,-30,-30,-30,-30,-30,-30,-50
    };  


int Board::evaluate()
{
    int score = 0;
    uint64_t bb;

    // WHITE (flip with ^ 56)
    bb = whitePawns;
    while (bb) { int sq = __builtin_ctzll(bb); score += 100 + pawnTable[sq ^ 56]; bb &= bb - 1; }

    bb = whiteKnights;
    while (bb) { int sq = __builtin_ctzll(bb); score += 320 + knightTable[sq ^ 56]; bb &= bb - 1; }

    bb = whiteBishops;
    while (bb) { int sq = __builtin_ctzll(bb); score += 330 + bishopTable[sq ^ 56]; bb &= bb - 1; }

    bb = whiteRooks;
    while (bb) { int sq = __builtin_ctzll(bb); score += 500 + rookTable[sq ^ 56]; bb &= bb - 1; }

    bb = whiteQueen;
    while (bb) { int sq = __builtin_ctzll(bb); score += 900 + queenTable[sq ^ 56]; bb &= bb - 1; }

    bb = whiteKing;
    while (bb) { int sq = __builtin_ctzll(bb); score += 10000 + kingTable[sq ^ 56]; bb &= bb - 1; }

    // BLACK (use sq directly)
    bb = blackPawns;
    while (bb) { int sq = __builtin_ctzll(bb); score -= 100 + pawnTable[sq]; bb &= bb - 1; }

    bb = blackKnights;
    while (bb) { int sq = __builtin_ctzll(bb); score -= 320 + knightTable[sq]; bb &= bb - 1; }

    bb = blackBishops;
    while (bb) { int sq = __builtin_ctzll(bb); score -= 330 + bishopTable[sq]; bb &= bb - 1; }

    bb = blackRooks;
    while (bb) { int sq = __builtin_ctzll(bb); score -= 500 + rookTable[sq]; bb &= bb - 1; }

    bb = blackQueen;
    while (bb) { int sq = __builtin_ctzll(bb); score -= 900 + queenTable[sq]; bb &= bb - 1; }

    bb = blackKing;
    while (bb) { int sq = __builtin_ctzll(bb); score -= 10000 + kingTable[sq]; bb &= bb - 1; }

    return score;
}

