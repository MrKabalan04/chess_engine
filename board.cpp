#include "board.h"
#include <cassert>
#include <iostream>
#include <cstdint>
#include <random>

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
    assert(sq >= 0 && sq < 64);
    assert(pieceType >= PAWN && pieceType <= KING);

    uint64_t bit = (1ULL << sq);
    if (isWhite)
    {
        assert((whitePieces & bit) == 0ULL);  // ← ADD THIS LINE
        whitePieces |= bit;
        switch (pieceType)
        {
            case PAWN:   whitePawns   |= bit; break;
            case KNIGHT: whiteKnights |= bit; break;
            case BISHOP: whiteBishops |= bit; break;
            case ROOK:   whiteRooks   |= bit; break;
            case QUEEN:  whiteQueen   |= bit; break;
            case KING:   whiteKing    |= bit; break;
        }
    }
    else
    {
        assert((blackPieces & bit) == 0ULL);
        blackPieces |= bit;
        switch (pieceType)
        {
            case PAWN:   blackPawns   |= bit; break;
            case KNIGHT: blackKnights |= bit; break;
            case BISHOP: blackBishops |= bit; break;
            case ROOK:   blackRooks   |= bit; break;
            case QUEEN:  blackQueen   |= bit; break;
            case KING:   blackKing    |= bit; break;
        }
    }
    occupied |= bit;
}

void Board::removePiece(int sq, int pieceType, bool isWhite)
{
    assert(sq >= 0 && sq < 64);
    assert(pieceType >= PAWN && pieceType <= KING);

    uint64_t bit = (1ULL << sq);
    if (isWhite)
    {
        assert((whitePieces & bit) != 0ULL);
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
        assert((blackPieces & bit) != 0ULL);
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
    initZobrist();

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

    zobristHash = 0;
    for (int i = 0; i < 64; i++)
    {
        int piece = getPieceAt(i);
        if (piece == -1) continue;
        int color = ((whitePieces >> i) & 1ULL) ? 0 : 1;
        zobristHash ^= zobristTable[piece][color][i];
    }

    if (sideToMove == 1)
        zobristHash ^= zobristSideToMove;

    if (castlingRights & WHITE_CASTLING_KINGSIDE)  zobristHash ^= zobristCastling[0];
    if (castlingRights & WHITE_CASTLING_QUEENSIDE) zobristHash ^= zobristCastling[1];
    if (castlingRights & BLACK_CASTLING_KINGSIDE)  zobristHash ^= zobristCastling[2];
    if (castlingRights & BLACK_CASTLING_QUEENSIDE) zobristHash ^= zobristCastling[3];

    if (enPassantSquare != -1)
        zobristHash ^= zobristEnPassant[enPassantSquare % 8];

    positionHistory[0] = zobristHash;
    historyCount = 1;
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

    validateBoard();
}

void Board::validateBoard() const
{
    assert((whitePieces & blackPieces) == 0ULL && "White and black pieces overlap");
    assert(occupied == (whitePieces | blackPieces) && "Occupied board mismatch");
    assert((whitePawns & (whiteKnights | whiteBishops | whiteRooks | whiteQueen | whiteKing)) == 0ULL && "White piece types overlap");
    assert((blackPawns & (blackKnights | blackBishops | blackRooks | blackQueen | blackKing)) == 0ULL && "Black piece types overlap");
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
    int to   = move.getTo();
    int type = move.getType();

    int side = sideToMove;
    int opponent = side ^ 1;

    int movingPiece = getPieceAt(from);
    assert(movingPiece != -1);

    bool isPieceWhite = ((whitePieces >> from) & 1ULL) != 0ULL;
    assert(isPieceWhite == (side == 0));

    int pieceType = movingPiece;  // getPieceAt already returns 0-5
    int color = (side == 0) ? 0 : 1;  // use sideToMove, not movingPiece

    int captured = -1;

    // =========================
    // HANDLE EN PASSANT CAPTURE SQUARE
    // =========================
    if (type == EN_PASSANT)
    {
        int capSq = (side == 0) ? (to - 8) : (to + 8);
        captured = getPieceAt(capSq);
    }
    else
    {
        captured = getPieceAt(to);
    }

    // =========================
    // SAVE UNDO INFO
    // =========================
    UndoInfo& undo = undoStack[undoCount++];

    undo.move = move;
    undo.movedPiece = movingPiece;
    undo.capturedPiece = captured;

    undo.castlingRights = castlingRights;
    undo.enPassantSquare = enPassantSquare;
    undo.zobristHash = zobristHash;

    // =========================
    // ZOBRIST: REMOVE FROM SQUARE
    // =========================
    zobristHash ^= zobristTable[pieceType][color][from];

    // remove moving piece from board
    removePiece(from, pieceType, side == 0);

    // =========================
    // CASTLING
    // =========================
    if (type == CASTLE)
{
    // king move
    addPiece(to, KING, side == 0);
    zobristHash ^= zobristTable[KING][color][to];

    if (side == 0)
    {
        // white
        if (to == 6)
        {
            removePiece(7, ROOK, true);
            addPiece(5, ROOK, true);

            zobristHash ^= zobristTable[ROOK][0][7];
            zobristHash ^= zobristTable[ROOK][0][5];
        }
        else if (to == 2)
        {
            removePiece(0, ROOK, true);
            addPiece(3, ROOK, true);

            zobristHash ^= zobristTable[ROOK][0][0];
            zobristHash ^= zobristTable[ROOK][0][3];
        }
    }
    else
    {
        // black
        if (to == 62)
        {
            removePiece(63, ROOK, false);
            addPiece(61, ROOK, false);

            zobristHash ^= zobristTable[ROOK][1][63];
            zobristHash ^= zobristTable[ROOK][1][61];
        }
        else if (to == 58)
        {
            removePiece(56, ROOK, false);
            addPiece(59, ROOK, false);

            zobristHash ^= zobristTable[ROOK][1][56];
            zobristHash ^= zobristTable[ROOK][1][59];
        }
    }

    goto finalize;
}

    // =========================
    // CAPTURE (NORMAL)
    // =========================
    if (captured != -1 && type != EN_PASSANT)
    {
        int capType = (captured <= 5) ? captured : (captured - 6);

        zobristHash ^= zobristTable[capType][opponent][to];
        removePiece(to, capType, opponent == 0);

        if (capType == ROOK)
        {
            if (opponent == 0)
            {
                if (to == 0) castlingRights &= ~WHITE_CASTLING_QUEENSIDE;
                if (to == 7) castlingRights &= ~WHITE_CASTLING_KINGSIDE;
            }
            else
            {
                if (to == 56) castlingRights &= ~BLACK_CASTLING_QUEENSIDE;
                if (to == 63) castlingRights &= ~BLACK_CASTLING_KINGSIDE;
            }
        }
    }

    // =========================
    // EN PASSANT
    // =========================
    if (type == EN_PASSANT)
    {
        int capSq = (side == 0) ? (to - 8) : (to + 8);

        int capPiece = getPieceAt(capSq);
        if (capPiece != -1)
        {
            zobristHash ^= zobristTable[PAWN][opponent][capSq];
            removePiece(capSq, PAWN, opponent == 0);
        }

        addPiece(to, PAWN, side == 0);
        zobristHash ^= zobristTable[PAWN][color][to];

        goto finalize;
    }

    // =========================
    // PROMOTION
    // =========================
    if (type >= PROMOT_QUEEN)
    {
        int newType = QUEEN;

        if (type == PROMOT_ROOK)   newType = ROOK;
        if (type == PROMOT_BISHOP) newType = BISHOP;
        if (type == PROMOT_KNIGHT) newType = KNIGHT;

        addPiece(to, newType, side == 0);
        zobristHash ^= zobristTable[newType][color][to];

        goto finalize;
    }

    // =========================
    // NORMAL MOVE
    // =========================
    addPiece(to, pieceType, side == 0);
    zobristHash ^= zobristTable[pieceType][color][to];

finalize:

    // =========================
    // CASTLING RIGHTS UPDATE
    // =========================
    // Remove the old castling rights from the hash before any changes.
    if (castlingRights & WHITE_CASTLING_KINGSIDE)  zobristHash ^= zobristCastling[0];
    if (castlingRights & WHITE_CASTLING_QUEENSIDE) zobristHash ^= zobristCastling[1];
    if (castlingRights & BLACK_CASTLING_KINGSIDE)  zobristHash ^= zobristCastling[2];
    if (castlingRights & BLACK_CASTLING_QUEENSIDE) zobristHash ^= zobristCastling[3];

    if (pieceType == KING)
    {
        if (side == 0)
            castlingRights &= ~(WHITE_CASTLING_KINGSIDE | WHITE_CASTLING_QUEENSIDE);
        else
            castlingRights &= ~(BLACK_CASTLING_KINGSIDE | BLACK_CASTLING_QUEENSIDE);
    }
    else if (pieceType == ROOK)
    {
        if (side == 0)
        {
            if (from == 0) castlingRights &= ~WHITE_CASTLING_QUEENSIDE;
            if (from == 7) castlingRights &= ~WHITE_CASTLING_KINGSIDE;
        }
        else
        {
            if (from == 56) castlingRights &= ~BLACK_CASTLING_QUEENSIDE;
            if (from == 63) castlingRights &= ~BLACK_CASTLING_KINGSIDE;
        }
    }

    // If a rook capture removes a rook on a home square, update opponent castling rights too.
    if (captured != -1 && type != EN_PASSANT)
    {
        int capType = (captured <= 5) ? captured : (captured - 6);
        if (capType == ROOK)
        {
            if (opponent == 0)
            {
                if (to == 0) castlingRights &= ~WHITE_CASTLING_QUEENSIDE;
                if (to == 7) castlingRights &= ~WHITE_CASTLING_KINGSIDE;
            }
            else
            {
                if (to == 56) castlingRights &= ~BLACK_CASTLING_QUEENSIDE;
                if (to == 63) castlingRights &= ~BLACK_CASTLING_KINGSIDE;
            }
        }
    }

    // Re-add the current castling rights to the hash.
    if (castlingRights & WHITE_CASTLING_KINGSIDE)  zobristHash ^= zobristCastling[0];
    if (castlingRights & WHITE_CASTLING_QUEENSIDE) zobristHash ^= zobristCastling[1];
    if (castlingRights & BLACK_CASTLING_KINGSIDE)  zobristHash ^= zobristCastling[2];
    if (castlingRights & BLACK_CASTLING_QUEENSIDE) zobristHash ^= zobristCastling[3];

    // =========================
    // EN PASSANT UPDATE (ZOBRIST)
    // =========================
    if (enPassantSquare != -1)
        zobristHash ^= zobristEnPassant[enPassantSquare % 8];

    if (type == DOUBLE_PUSH)
    {
        enPassantSquare = (side == 0) ? (to - 8) : (to + 8);
        zobristHash ^= zobristEnPassant[enPassantSquare % 8];
    }
    else
    {
        enPassantSquare = -1;
    }

    // =========================
    // SIDE TO MOVE
    // =========================
    zobristHash ^= zobristSideToMove;
    sideToMove ^= 1;

    positionHistory[historyCount++] = zobristHash;
}

bool Board::isThreefoldRepetition()
{
    int count = 0;
    for (int i = 0; i < historyCount - 1; i++)
    {
        if (positionHistory[i] == zobristHash)
            count++;
    }
    return count >= 2;
}

void Board::undoMove()
{
    if (undoCount == 0) return;

    UndoInfo info = undoStack[--undoCount];

    Move move = info.move;

    int from = move.getFrom();
    int to   = move.getTo();
    int type = move.getType();

    // restore global state
    sideToMove ^= 1;
    castlingRights = info.castlingRights;
    enPassantSquare = info.enPassantSquare;
    zobristHash = info.zobristHash;

    int side = sideToMove;
    int opponent = side ^ 1;

    int pieceType = info.movedPiece;

    // =========================
    // REMOVE PIECE FROM DESTINATION
    // promotion: piece on 'to' is promoted piece not pawn
    // =========================
    if (type >= PROMOT_QUEEN)
    {
        int promotedType = QUEEN;
        if (type == PROMOT_ROOK)   promotedType = ROOK;
        if (type == PROMOT_BISHOP) promotedType = BISHOP;
        if (type == PROMOT_KNIGHT) promotedType = KNIGHT;
        removePiece(to, promotedType, side == 0);
    }
    else
    {
        removePiece(to, pieceType, side == 0);
    }

    // =========================
    // CASTLING
    // =========================
    if (type == CASTLE)
{
    addPiece(from, KING, side == 0);

    if (side == 0)
    {
        if (to == 6)
        {
            removePiece(5, ROOK, true);
            addPiece(7, ROOK, true);
        }
        else if (to == 2)
        {
            removePiece(3, ROOK, true);
            addPiece(0, ROOK, true);
        }
    }
    else
    {
        if (to == 62)
        {
            removePiece(61, ROOK, false);
            addPiece(63, ROOK, false);
        }
        else if (to == 58)
        {
            removePiece(59, ROOK, false);
            addPiece(56, ROOK, false);
        }
    }
}

    // =========================
    // PROMOTION
    // =========================
    else if (type >= PROMOT_QUEEN)
    {
        addPiece(from, PAWN, side == 0);

        if (info.capturedPiece != -1)
        {
            int capType =
                (info.capturedPiece <= 5)
                ? info.capturedPiece
                : info.capturedPiece - 6;

            addPiece(to, capType, opponent == 0);
        }
    }

    // =========================
    // EN PASSANT
    // =========================
    else if (type == EN_PASSANT)
    {
        addPiece(from, PAWN, side == 0);

        int capSq =
            (side == 0)
            ? (to - 8)
            : (to + 8);

        addPiece(capSq, PAWN, opponent == 0);
    }

    // =========================
    // NORMAL MOVE
    // =========================
    else
    {
        addPiece(from, pieceType, side == 0);

        if (info.capturedPiece != -1)
        {
            int capType =
                (info.capturedPiece <= 5)
                ? info.capturedPiece
                : info.capturedPiece - 6;

            addPiece(to, capType, opponent == 0);
        }
    }

    historyCount--;
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
    int mgScore = 0;
    int egScore = 0;

    // 1. Calculate Phase (0 = Opening, 256 = Deep Endgame)
    // We weight pieces to determine how "empty" the board is.
    int material = __builtin_popcountll(whiteQueen | blackQueen) * 4 +
                   __builtin_popcountll(whiteRooks | blackRooks) * 2 +
                   __builtin_popcountll(whiteBishops | blackBishops) * 1 +
                   __builtin_popcountll(whiteKnights | blackKnights) * 1;
    
    int phase = (material * 256) / 12; 
    if (phase > 256) phase = 256;

    // 2. Helper Lambda for scoring (keeps code clean)
    auto scorePiece = [&](uint64_t bb, const int* table, int value, bool isWhite) {
        while (bb) {
            int sq = __builtin_ctzll(bb);
            int tableSq = isWhite ? (sq ^ 56) : sq;
            int s = value + table[tableSq];
            if (isWhite) { mgScore += s; egScore += s; }
            else { mgScore -= s; egScore -= s; }
            bb &= bb - 1;
        }
    };

    // 3. Evaluate Material + PSTs
    scorePiece(whitePawns, pawnTable, 100, true);
    scorePiece(whiteKnights, knightTable, 320, true);
    scorePiece(whiteBishops, bishopTable, 330, true);
    scorePiece(whiteRooks, rookTable, 500, true);
    scorePiece(whiteQueen, queenTable, 900, true);
    
    // King is special: use Endgame table when phase is high
    int whiteKingSq = __builtin_ctzll(whiteKing);
    mgScore += 10000 + kingTable[whiteKingSq ^ 56];
    egScore += 10000 + kingEndgameTable[whiteKingSq ^ 56];

    scorePiece(blackPawns, pawnTable, 100, false);
    scorePiece(blackKnights, knightTable, 320, false);
    scorePiece(blackBishops, bishopTable, 330, false);
    scorePiece(blackRooks, rookTable, 500, false);
    scorePiece(blackQueen, queenTable, 900, false);
    
    int blackKingSq = __builtin_ctzll(blackKing);
    mgScore -= 10000 + kingTable[blackKingSq];
    egScore -= 10000 + kingEndgameTable[blackKingSq];

    // 4. The Tapered Blend
    // This transitions smoothly from MG to EG as phase increases
    int score = (mgScore * (256 - phase) + egScore * phase) / 256;

    return (sideToMove == 0) ? score : -score;
}

void Board::initZobrist()
{
    std::mt19937_64 rng(123456789);

    uint64_t* ptr = &zobristTable[0][0][0];

    for (int i = 0; i < 2 * 6 * 64; i++)
        ptr[i] = rng();

    for (int i = 0; i < 4; i++)
        zobristCastling[i] = rng();

    for (int i = 0; i < 8; i++)
        zobristEnPassant[i] = rng();

    zobristSideToMove = rng();
}

void Board::makeNullMove()
{
    // Save state onto the undo stack so undoNullMove can restore it perfectly
    UndoInfo& undo = undoStack[undoCount++];
    undo.castlingRights = castlingRights;
    undo.enPassantSquare = enPassantSquare;
    undo.zobristHash = zobristHash;
    undo.capturedPiece = -1;
    undo.movedPiece = -1;

    // Remove the current en passant square from hash if it exists
    if (enPassantSquare != -1)
        zobristHash ^= zobristEnPassant[enPassantSquare % 8];

    enPassantSquare = -1; // Clear EP square on a skipped turn
    
    // Switch sides and update hash
    zobristHash ^= zobristSideToMove;
    sideToMove ^= 1;

    // Track history to avoid desyncs during repetition checks
    positionHistory[historyCount++] = zobristHash;
}

void Board::undoNullMove()
{
    historyCount--;
    UndoInfo& info = undoStack[--undoCount];

    // Restore original parameters directly from stack
    sideToMove ^= 1;
    castlingRights = info.castlingRights;
    enPassantSquare = info.enPassantSquare;
    zobristHash = info.zobristHash;
}