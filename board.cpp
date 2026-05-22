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
    assert((occupied & bit) == 0ULL); // Safety: square must be vacant

    if (isWhite)
    {
        assert((whitePieces & bit) == 0ULL);
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

    assert((occupied & bit) != 0ULL);
    occupied &= ~bit;
}

// =========================
// INIT START POSITION
// =========================
void Board::init()
{
    // 1. Wipe out any existing data completely
    whitePawns = whiteKnights = whiteBishops = whiteRooks = whiteQueen = whiteKing = 0ULL;
    blackPawns = blackKnights = blackBishops = blackRooks = blackQueen = blackKing = 0ULL;
    whitePieces = blackPieces = occupied = 0ULL;

    sideToMove = 0;
    enPassantSquare = -1;
    castlingRights = 15; // All 4 castling options active
    undoCount = 0;
    historyCount = 0;

    // 2. Initialize White Pawns strictly on Rank 2 (Squares 8 to 15)
    for (int i = 0; i < 8; i++) whitePawns |= (1ULL << (8 + i));
    
    // Initialize White Pieces strictly on Rank 1 (Squares 0 to 7)
    whiteRooks   |= (1ULL << 0) | (1ULL << 7); // a1, h1
    whiteKnights |= (1ULL << 1) | (1ULL << 6); // b1, g1
    whiteBishops |= (1ULL << 2) | (1ULL << 5); // c1, f1
    whiteQueen   |= (1ULL << 3);               // d1
    whiteKing    |= (1ULL << 4);               // e1

    // 3. Initialize Black Pawns strictly on Rank 7 (Squares 48 to 55)
    for (int i = 0; i < 8; i++) blackPawns |= (1ULL << (48 + i));
    
    // Initialize Black Pieces strictly on Rank 8 (Squares 56 to 63)
    blackRooks   |= (1ULL << 56) | (1ULL << 63); // a8, h8
    blackKnights |= (1ULL << 57) | (1ULL << 62); // b8, g8
    blackBishops |= (1ULL << 58) | (1ULL << 61); // c8, f8
    blackQueen   |= (1ULL << 59);                // d8
    blackKing    |= (1ULL << 60);                // e8

    // 4. Set container masks explicitly
    whitePieces = whitePawns | whiteKnights | whiteBishops | whiteRooks | whiteQueen | whiteKing;
    blackPieces = blackPawns | blackKnights | blackBishops | blackRooks | blackQueen | blackKing;
    occupied = whitePieces | blackPieces;

    // 5. Build initial Zobrist Hash safely without relying on helper methods
    initZobrist();
    zobristHash = 0ULL;

    // Hash White pieces
    for (int i = 8; i <= 15; i++)  zobristHash ^= zobristTable[PAWN][0][i];
    zobristHash ^= zobristTable[ROOK][0][0];   zobristHash ^= zobristTable[ROOK][0][7];
    zobristHash ^= zobristTable[KNIGHT][0][1]; zobristHash ^= zobristTable[KNIGHT][0][6];
    zobristHash ^= zobristTable[BISHOP][0][2]; zobristHash ^= zobristTable[BISHOP][0][5];
    zobristHash ^= zobristTable[QUEEN][0][3];  zobristHash ^= zobristTable[KING][0][4];

    // Hash Black pieces
    for (int i = 48; i <= 55; i++) zobristHash ^= zobristTable[PAWN][1][i];
    zobristHash ^= zobristTable[ROOK][1][56];   zobristHash ^= zobristTable[ROOK][1][63];
    zobristHash ^= zobristTable[KNIGHT][1][57]; zobristHash ^= zobristTable[KNIGHT][1][62];
    zobristHash ^= zobristTable[BISHOP][1][58]; zobristHash ^= zobristTable[BISHOP][1][61];
    zobristHash ^= zobristTable[QUEEN][1][59];  zobristHash ^= zobristTable[KING][1][60];

    // Add environmental flags to hash
    if (castlingRights & 1) zobristHash ^= zobristCastling[0];
    if (castlingRights & 2) zobristHash ^= zobristCastling[1];
    if (castlingRights & 4) zobristHash ^= zobristCastling[2];
    if (castlingRights & 8) zobristHash ^= zobristCastling[3];

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
    // 1. Ensure opposing sides do not share any squares
    assert((whitePieces & blackPieces) == 0ULL && "White and black pieces overlap");
    
    // 2. Ensure global occupancy represents exactly both sides combined
    assert(occupied == (whitePieces | blackPieces) && "Occupied board mismatch");
    
    // 3. Ensure individual piece sub-types do not bleed into each other
    assert((whitePawns & (whiteKnights | whiteBishops | whiteRooks | whiteQueen | whiteKing)) == 0ULL && "White piece types overlap");
    assert((blackPawns & (blackKnights | blackBishops | blackRooks | blackQueen | blackKing)) == 0ULL && "Black piece types overlap");

    // 🔥 NEW CRITICAL CHECKS: Ensure the side tracking bitboards match the item totals
    uint64_t calculatedWhite = whitePawns | whiteKnights | whiteBishops | whiteRooks | whiteQueen | whiteKing;
    uint64_t calculatedBlack = blackPawns | blackKnights | blackBishops | blackRooks | blackQueen | blackKing;

    assert(whitePieces == calculatedWhite && "whitePieces container does not match individual white piece bitboards");
    assert(blackPieces == calculatedBlack && "blackPieces container does not match individual black piece bitboards");
}


// =========================
// PIECE QUERY
// =========================
int Board::getPieceAt(int sq) const
{
    uint64_t bit = (1ULL << sq);

    // If the square isn't occupied at all, immediately return -1
    if ((occupied & bit) == 0ULL) {
        return -1;
    }

    // Check White bitboards
    if ((whitePieces & bit) != 0ULL) {
        if ((whitePawns & bit)   != 0ULL) return PAWN;
        if ((whiteKnights & bit) != 0ULL) return KNIGHT;
        if ((whiteBishops & bit) != 0ULL) return BISHOP;
        if ((whiteRooks & bit)   != 0ULL) return ROOK;
        if ((whiteQueen & bit)   != 0ULL) return QUEEN;
        if ((whiteKing & bit)    != 0ULL) return KING;
    }
    // Check Black bitboards
    else if ((blackPieces & bit) != 0ULL) {
        if ((blackPawns & bit)   != 0ULL) return PAWN;
        if ((blackKnights & bit) != 0ULL) return KNIGHT;
        if ((blackBishops & bit) != 0ULL) return BISHOP;
        if ((blackRooks & bit)   != 0ULL) return ROOK;
        if ((blackQueen & bit)   != 0ULL) return QUEEN;
        if ((blackKing & bit)    != 0ULL) return KING;
    }

    return -1;
}

// =========================
// IS THREEFOLD REPETITION
// =========================

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

    int pieceType = movingPiece;  
    int color = (side == 0) ? 0 : 1;

    int captured = -1;

    if (type == EN_PASSANT)
    {
        int capSq = (side == 0) ? (to - 8) : (to + 8);
        captured = getPieceAt(capSq);
    }
    else
    {
        captured = getPieceAt(to);
    }

    UndoInfo& undo = undoStack[undoCount++];
    undo.move = move;
    undo.movedPiece = movingPiece;
    undo.capturedPiece = captured;
    undo.castlingRights = castlingRights;
    undo.enPassantSquare = enPassantSquare;
    undo.zobristHash = zobristHash;

    zobristHash ^= zobristTable[pieceType][color][from];
    removePiece(from, pieceType, side == 0);

    // ==========================================
    // CASTLING BRANCH
    // ==========================================
    if (type == CASTLE)
    {
        addPiece(to, KING, side == 0);
        zobristHash ^= zobristTable[KING][color][to];

        if (side == 0) // White
        {
            if (to == 6) // Kingside
            {
                removePiece(7, ROOK, true); 
                addPiece(5, ROOK, true);
                zobristHash ^= zobristTable[ROOK][0][7]; 
                zobristHash ^= zobristTable[ROOK][0][5]; 
            }
            else if (to == 2) // Queenside
            {
                removePiece(0, ROOK, true); 
                addPiece(3, ROOK, true);
                zobristHash ^= zobristTable[ROOK][0][0]; 
                zobristHash ^= zobristTable[ROOK][0][3]; 
            }
        }
        else // Black
        {
            if (to == 62) // Kingside
            {
                removePiece(63, ROOK, false); 
                addPiece(61, ROOK, false);
                zobristHash ^= zobristTable[ROOK][1][63]; // ✅ Fixed square index
                zobristHash ^= zobristTable[ROOK][1][61]; // ✅ Fixed square index
            }
            else if (to == 58) // Queenside
            {
                removePiece(56, ROOK, false); 
                addPiece(59, ROOK, false);
                zobristHash ^= zobristTable[ROOK][1][56]; // ✅ Fixed square index
                zobristHash ^= zobristTable[ROOK][1][59]; // ✅ Fixed square index
            }
        }
        goto finalize;
    }

    // ==========================================
    // STANDARD CAPTURE BRANCH
    // ==========================================
    if (captured != -1 && type != EN_PASSANT)
    {
        int capType = captured; 
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

    // ==========================================
    // EN PASSANT BRANCH
    // ==========================================
    if (type == EN_PASSANT)
    {
        int capSq = (side == 0) ? (to - 8) : (to + 8);
        if (captured != -1)
        {
            zobristHash ^= zobristTable[PAWN][opponent][capSq];
            removePiece(capSq, PAWN, opponent == 0);
        }
        addPiece(to, PAWN, side == 0);
        zobristHash ^= zobristTable[PAWN][color][to];
        goto finalize;
    }

    // ==========================================
    // PROMOTION BRANCH
    // ==========================================
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

    // ==========================================
    // STANDARD MOVE STEP
    // ==========================================
    addPiece(to, pieceType, side == 0);
    zobristHash ^= zobristTable[pieceType][color][to];

finalize:

    // ==========================================
    // CASTLING RIGHTS UPDATE
    // ==========================================
    // ✅ FIXED: Added explicit array indices [0], [1], [2], [3]
    if (castlingRights & WHITE_CASTLING_KINGSIDE)  zobristHash ^= zobristCastling[0];
    if (castlingRights & WHITE_CASTLING_QUEENSIDE) zobristHash ^= zobristCastling[1];
    if (castlingRights & BLACK_CASTLING_KINGSIDE)  zobristHash ^= zobristCastling[2];
    if (castlingRights & BLACK_CASTLING_QUEENSIDE) zobristHash ^= zobristCastling[3];

    if (pieceType == KING)
    {
        if (side == 0) castlingRights &= ~(WHITE_CASTLING_KINGSIDE | WHITE_CASTLING_QUEENSIDE);
        else           castlingRights &= ~(BLACK_CASTLING_KINGSIDE | BLACK_CASTLING_QUEENSIDE);
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

    if (captured != -1 && type != EN_PASSANT && captured == ROOK)
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

    // ✅ FIXED: Re-add with explicit array indices [0], [1], [2], [3]
    if (castlingRights & WHITE_CASTLING_KINGSIDE)  zobristHash ^= zobristCastling[0];
    if (castlingRights & WHITE_CASTLING_QUEENSIDE) zobristHash ^= zobristCastling[1];
    if (castlingRights & BLACK_CASTLING_KINGSIDE)  zobristHash ^= zobristCastling[2];
    if (castlingRights & BLACK_CASTLING_QUEENSIDE) zobristHash ^= zobristCastling[3];

    if (enPassantSquare != -1) zobristHash ^= zobristEnPassant[enPassantSquare % 8];

    if (type == DOUBLE_PUSH)
    {
        enPassantSquare = (side == 0) ? (to - 8) : (to + 8);
        zobristHash ^= zobristEnPassant[enPassantSquare % 8];
    }
    else
    {
        enPassantSquare = -1;
    }

    zobristHash ^= zobristSideToMove;
    sideToMove ^= 1;
    positionHistory[historyCount++] = zobristHash;
}

// =========================
// UNDO MOVE
// =========================

void Board::undoMove()
{
    if (undoCount == 0) return;

    UndoInfo info = undoStack[--undoCount];
    Move move = info.move;

    int from = move.getFrom();
    int to   = move.getTo();
    int type = move.getType();

    // Restore global states safely
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
        removePiece(to, KING, side == 0);
        addPiece(from, KING, side == 0);

        if (side == 0) // White
        {
            if (to == 6) { removePiece(5, ROOK, true); addPiece(7, ROOK, true); }
            else if (to == 2) { removePiece(3, ROOK, true); addPiece(0, ROOK, true); }
        }
        else // Black
        {
            if (to == 62) { removePiece(61, ROOK, false); addPiece(63, ROOK, false); }
            else if (to == 58) { removePiece(59, ROOK, false); addPiece(56, ROOK, false); }
        }
    }
    else if (type >= PROMOT_QUEEN)
    {
        int promotedType = QUEEN;
        if (type == PROMOT_ROOK)   promotedType = ROOK;
        if (type == PROMOT_BISHOP) promotedType = BISHOP;
        if (type == PROMOT_KNIGHT) promotedType = KNIGHT;

        removePiece(to, promotedType, side == 0);
        addPiece(from, PAWN, side == 0);

        if (info.capturedPiece != -1)
        {
            addPiece(to, info.capturedPiece, opponent == 0);
        }
    }
    else if (type == EN_PASSANT)
    {
        removePiece(to, PAWN, side == 0);
        addPiece(from, PAWN, side == 0);

        int capSq = (side == 0) ? (to - 8) : (to + 8);
        addPiece(capSq, PAWN, opponent == 0);
    }
    else // NORMAL MOVES
    {
        removePiece(to, pieceType, side == 0);
        addPiece(from, pieceType, side == 0);

        if (info.capturedPiece != -1)
        {
            addPiece(to, info.capturedPiece, opponent == 0);
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
    // Use a fixed seed so your transposition table remains reproducible during testing
    std::mt19937_64 rng(123456789);

    // ✅ Explicitly initialize the 3D array matching your exact dimensions: [Piece][Color][Square]
    for (int piece = 0; piece < 6; piece++)
    {
        for (int color = 0; color < 2; color++)
        {
            for (int sq = 0; sq < 64; sq++)
            {
                zobristTable[piece][color][sq] = rng();
            }
        }
    }

    // Initialize Castling Rights (4 indices: 0 to 3)
    for (int i = 0; i < 4; i++)
    {
        zobristCastling[i] = rng();
    }

    // Initialize En Passant files (8 indices: 0 to 7)
    for (int i = 0; i < 8; i++)
    {
        zobristEnPassant[i] = rng();
    }

    // Initialize side to move flag
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
