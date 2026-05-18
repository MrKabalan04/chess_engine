#include "board.h"
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
    initZobrist();
    
    zobristHash = 0;

    for(int i = 0 ; i < 64 ; i++){
        int piece = getPieceAt(i);
        if(piece == -1) continue;
        int color = (whitePieces >> i) & 1ULL ? 0 : 1;
        zobristHash ^= zobristTable[color][piece][i];
    }
    int side = sideToMove;
    if (sideToMove == 1){
        zobristHash ^= zobristSideToMove;
    }

    int castling = castlingRights;
    if (castling & WHITE_CASTLING_KINGSIDE) zobristHash ^= zobristCastling[0];
    if (castling & WHITE_CASTLING_QUEENSIDE) zobristHash ^= zobristCastling[1];
    if (castling & BLACK_CASTLING_KINGSIDE) zobristHash ^= zobristCastling[2];
    if (castling & BLACK_CASTLING_QUEENSIDE) zobristHash ^=  zobristCastling[3];

    int enPassant = enPassantSquare;
    if (enPassant != -1) {
        int file = enPassant % 8;
        zobristHash ^= zobristEnPassant[file];
    }
    






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
    int to   = move.getTo();
    int type = move.getType();

    int side = sideToMove;
    int opponent = side ^ 1;

    int movingPiece = getPieceAt(from);
    if (movingPiece == -1) return;

    int pieceType = (movingPiece <= 5) ? movingPiece : (movingPiece - 6);
    int color     = (movingPiece <= 5) ? 0 : 1;

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

    updateOccupancy();

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

    int piece = getPieceAt(to);

    int pieceType = info.movedPiece;

    // remove moved piece from destination
    removePiece(to, pieceType, side == 0);

    // =========================
    // CASTLING
    // =========================
    if (type == CASTLE)
    {
        addPiece(from, KING, side == 0);

        // white
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
        // black
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

    updateOccupancy();
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
    int score = 0;
    uint64_t bb;

    // -------------------------
    // GAME PHASE (simple version)
    // -------------------------
    int phase = 0;

    phase += __builtin_popcountll(whiteQueen | blackQueen) * 4;
    phase += __builtin_popcountll(whiteRooks | blackRooks) * 2;
    phase += __builtin_popcountll(whiteBishops | blackBishops);
    phase += __builtin_popcountll(whiteKnights | blackKnights);

    bool endgame = (phase <= 8);

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

void Board::initZobrist()
{
   mt19937_64 rng(123456789); // fixed seed
   uint64_t randomNumber = rng();
    for (int color = 0; color < 2; color++)
    {
        for (int piece = 0; piece < 6; piece++)
        {
            for (int sq = 0; sq < 64; sq++)
            {
                zobristTable[color][piece][sq] = rng();
            }
        }
    }
    for (int i = 0; i < 4; i++){
        zobristCastling[i] = rng();
    }
        
    for (int i = 0; i < 8; i++){
        zobristEnPassant[i] = rng();
    }
        
    zobristSideToMove = rng();

}