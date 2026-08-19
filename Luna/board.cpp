#include "board.h"
#include <cassert>
#include <iostream>
#include <sstream>
#include <cstdint>
#include <random>
#include <algorithm>
#include <cmath>

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
// INIT FROM FEN
// =========================
bool Board::initFromFen(const std::string& fen)
{
    // Wipe board
    whitePawns = whiteKnights = whiteBishops = whiteRooks = whiteQueen = whiteKing = 0ULL;
    blackPawns = blackKnights = blackBishops = blackRooks = blackQueen = blackKing = 0ULL;
    whitePieces = blackPieces = occupied = 0ULL;
    enPassantSquare = -1;
    castlingRights  = 0;
    halfMoveClock   = 0;
    undoCount       = 0;
    historyCount    = 0;

    // Split into tokens
    std::istringstream ss(fen);
    std::string piecePart, sidePart, castlePart, epPart, halfStr;
    ss >> piecePart >> sidePart >> castlePart >> epPart >> halfStr;

    // 1. Piece placement
    int rank = 7, file = 0;
    for (char c : piecePart) {
        if (c == '/') { rank--; file = 0; continue; }
        if (c >= '1' && c <= '8') { file += c - '0'; continue; }

        int sq = rank * 8 + file;
        bool white = (c >= 'A' && c <= 'Z');
        char lc = white ? (c | 0x20) : c;

        PieceType pt;
        switch (lc) {
            case 'p': pt = PAWN;   break;
            case 'n': pt = KNIGHT; break;
            case 'b': pt = BISHOP; break;
            case 'r': pt = ROOK;   break;
            case 'q': pt = QUEEN;  break;
            case 'k': pt = KING;   break;
            default: return false;
        }
        addPiece(sq, pt, white);
        file++;
    }

    // 2. Side to move
    sideToMove = (sidePart == "b") ? 1 : 0;

    // 3. Castling rights
    if (castlePart != "-") {
        for (char c : castlePart) {
            if (c == 'K') castlingRights |= WHITE_CASTLING_KINGSIDE;
            if (c == 'Q') castlingRights |= WHITE_CASTLING_QUEENSIDE;
            if (c == 'k') castlingRights |= BLACK_CASTLING_KINGSIDE;
            if (c == 'q') castlingRights |= BLACK_CASTLING_QUEENSIDE;
        }
    }

    // 4. En passant
    if (epPart != "-" && epPart.size() >= 2) {
        int epFile = epPart[0] - 'a';
        int epRank = epPart[1] - '1';
        enPassantSquare = epRank * 8 + epFile;
    }

    // 4b. Half-move clock (50-move rule)
    if (!halfStr.empty()) {
        try { halfMoveClock = std::stoi(halfStr); } catch (...) { halfMoveClock = 0; }
    }

    // 5. Rebuild Zobrist hash from scratch
    zobristHash = 0ULL;
    auto hashBB = [&](uint64_t bb, int pt, int color) {
        while (bb) {
            int s = __builtin_ctzll(bb);
            zobristHash ^= zobristTable[pt][color][s];
            bb &= bb - 1;
        }
    };
    hashBB(whitePawns, PAWN, 0);   hashBB(blackPawns, PAWN, 1);
    hashBB(whiteKnights, KNIGHT, 0); hashBB(blackKnights, KNIGHT, 1);
    hashBB(whiteBishops, BISHOP, 0); hashBB(blackBishops, BISHOP, 1);
    hashBB(whiteRooks, ROOK, 0);   hashBB(blackRooks, ROOK, 1);
    hashBB(whiteQueen, QUEEN, 0);   hashBB(blackQueen, QUEEN, 1);
    hashBB(whiteKing, KING, 0);    hashBB(blackKing, KING, 1);

    if (castlingRights & WHITE_CASTLING_KINGSIDE)  zobristHash ^= zobristCastling[0];
    if (castlingRights & WHITE_CASTLING_QUEENSIDE) zobristHash ^= zobristCastling[1];
    if (castlingRights & BLACK_CASTLING_KINGSIDE)  zobristHash ^= zobristCastling[2];
    if (castlingRights & BLACK_CASTLING_QUEENSIDE) zobristHash ^= zobristCastling[3];
    if (enPassantSquare != -1)
        zobristHash ^= zobristEnPassant[enPassantSquare % 8];
    if (sideToMove == 1) zobristHash ^= zobristSideToMove;

    positionHistory[0] = zobristHash;
    historyCount = 1;
    return true;
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
void Board::addPiece(int sq, int pieceType, bool isWhite) {
    uint64_t bit = (1ULL << sq);
    if (isWhite) {
        whitePieces |= bit;
        if (pieceType == PAWN)   whitePawns |= bit;
        if (pieceType == KNIGHT) whiteKnights |= bit;
        if (pieceType == BISHOP) whiteBishops |= bit;
        if (pieceType == ROOK)   whiteRooks |= bit;
        if (pieceType == QUEEN)  whiteQueen |= bit;
        if (pieceType == KING)   whiteKing |= bit;
    } else {
        blackPieces |= bit;
        if (pieceType == PAWN)   blackPawns |= bit;
        if (pieceType == KNIGHT) blackKnights |= bit;
        if (pieceType == BISHOP) blackBishops |= bit;
        if (pieceType == ROOK)   blackRooks |= bit;
        if (pieceType == QUEEN)  blackQueen |= bit;
        if (pieceType == KING)   blackKing |= bit;
    }
    occupied |= bit;
}

void Board::removePiece(int sq, int pieceType, bool isWhite) {
    uint64_t mask = ~(1ULL << sq);
    if (isWhite) {
        whitePieces &= mask;
        if (pieceType == PAWN)   whitePawns &= mask;
        if (pieceType == KNIGHT) whiteKnights &= mask;
        if (pieceType == BISHOP) whiteBishops &= mask;
        if (pieceType == ROOK)   whiteRooks &= mask;
        if (pieceType == QUEEN)  whiteQueen &= mask;
        if (pieceType == KING)   whiteKing &= mask;
    } else {
        blackPieces &= mask;
        if (pieceType == PAWN)   blackPawns &= mask;
        if (pieceType == KNIGHT) blackKnights &= mask;
        if (pieceType == BISHOP) blackBishops &= mask;
        if (pieceType == ROOK)   blackRooks &= mask;
        if (pieceType == QUEEN)  blackQueen &= mask;
        if (pieceType == KING)   blackKing &= mask;
    }
    occupied &= mask;
}


// =========================
// ToFen
// =========================

std::string Board::getFen() const{
    int count = 0;
    std::string fen = "";
    for(int rank = 7; rank >= 0; rank--){
        for(int file = 0; file < 8; file++){
            int sq = rank * 8 + file;
            int piece = getPieceAt(sq);
            if(piece == -1){
                count++;
            } else {
                if(count > 0){
                    fen += std::to_string(count);
                    count = 0;
                }
                bool isWhite = (whitePieces >> sq) & 1ULL;
                char c;
                switch(piece){
                    case PAWN:   c = 'p'; break;
                    case KNIGHT: c = 'n'; break;
                    case BISHOP: c = 'b'; break;
                    case ROOK:   c = 'r'; break;
                    case QUEEN:  c = 'q'; break;
                    case KING:   c = 'k'; break;
                    default:     c = '?';
                }
                fen += isWhite ? toupper(c) : c;
            }
            
        }
        
        if(count > 0){
            fen += std::to_string(count);
            count = 0;
        }
        if(rank > 0) fen += '/';
    }
    fen += (sideToMove == 0) ? " w " : " b ";
    return fen;
}


// =========================
// INIT START POSITION
// =========================
void Board::init()
{
    // 1. Wipe out existing data
    whitePawns = whiteKnights = whiteBishops = whiteRooks = whiteQueen = whiteKing = 0ULL;
    blackPawns = blackKnights = blackBishops = blackRooks = blackQueen = blackKing = 0ULL;
    whitePieces = blackPieces = occupied = 0ULL;

    sideToMove = 0; 
    enPassantSquare = -1;
    castlingRights = 15; 
    halfMoveClock = 0;
    undoCount = 0;
    historyCount = 0;

    // 2. Setup pieces
    for (int i = 0; i < 8; i++) whitePawns |= (1ULL << (8 + i));
    whiteRooks |= (1ULL << 0) | (1ULL << 7);
    whiteKnights |= (1ULL << 1) | (1ULL << 6);
    whiteBishops |= (1ULL << 2) | (1ULL << 5);
    whiteQueen |= (1ULL << 3);
    whiteKing |= (1ULL << 4);

    for (int i = 0; i < 8; i++) blackPawns |= (1ULL << (48 + i));
    blackRooks |= (1ULL << 56) | (1ULL << 63);
    blackKnights |= (1ULL << 57) | (1ULL << 62);
    blackBishops |= (1ULL << 58) | (1ULL << 61);
    blackQueen |= (1ULL << 59);
    blackKing |= (1ULL << 60);

    whitePieces = whitePawns | whiteKnights | whiteBishops | whiteRooks | whiteQueen | whiteKing;
    blackPieces = blackPawns | blackKnights | blackBishops | blackRooks | blackQueen | blackKing;
    occupied = whitePieces | blackPieces;

    // 3. Build Zobrist Hash
    zobristHash = 0ULL;
    // Helper to hash pieces
    auto hashPiece = [&](uint64_t bb, int pieceType, int color) {
        while (bb) {
            int sq = __builtin_ctzll(bb);
            zobristHash ^= zobristTable[pieceType][color][sq];
            bb &= bb - 1;
        }
    };

    hashPiece(whitePawns, PAWN, 0); hashPiece(whiteKnights, KNIGHT, 0);
    hashPiece(whiteBishops, BISHOP, 0); hashPiece(whiteRooks, ROOK, 0);
    hashPiece(whiteQueen, QUEEN, 0); hashPiece(whiteKing, KING, 0);

    hashPiece(blackPawns, PAWN, 1); hashPiece(blackKnights, KNIGHT, 1);
    hashPiece(blackBishops, BISHOP, 1); hashPiece(blackRooks, ROOK, 1);
    hashPiece(blackQueen, QUEEN, 1); hashPiece(blackKing, KING, 1);

    // Castling
    if (castlingRights & 1) zobristHash ^= zobristCastling[0];
    if (castlingRights & 2) zobristHash ^= zobristCastling[1];
    if (castlingRights & 4) zobristHash ^= zobristCastling[2];
    if (castlingRights & 8) zobristHash ^= zobristCastling[3];
    
    // IMPORTANT: Include side to move in hash
    if (sideToMove == 1) zobristHash ^= zobristSideToMove;

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
int Board::getPieceAt(int sq) const {
    uint64_t bit = (1ULL << sq);
    if (whitePawns & bit) return PAWN;
    if (whiteKnights & bit) return KNIGHT;
    if (whiteBishops & bit) return BISHOP;
    if (whiteRooks & bit) return ROOK;
    if (whiteQueen & bit) return QUEEN; // Ensure this matches your member variable name
    if (whiteKing & bit) return KING;

    if (blackPawns & bit) return PAWN;
    if (blackKnights & bit) return KNIGHT;
    if (blackBishops & bit) return BISHOP;
    if (blackRooks & bit) return ROOK;
    if (blackQueen & bit) return QUEEN; // Ensure this matches your member variable name
    if (blackKing & bit) return KING;
    
    return -1;
}

// =========================
// IS THREEFOLD REPETITION
// =========================

bool Board::isThreefoldRepetition() const {
    // A game cannot repeat until at least 4 ply have been executed
    if (historyCount < 4) return false;

    int repetitions = 0;
    uint64_t currentHash = zobristHash;

    // Loop backward through the current game's history array only
    for (int i = historyCount - 2; i >= 0; i -= 2) {
        if (positionHistory[i] == currentHash) {
            repetitions++;
            if (repetitions >= 2) { // 3rd occurrence means threefold draw
                return true;
            }
        }
    }
    return false;
}

// =========================
// INSUFFICIENT MATERIAL
// =========================
bool Board::isInsufficientMaterial() const {
    // Any pawns, rooks, or queens → sufficient material
    if (whitePawns | blackPawns | whiteRooks | blackRooks | whiteQueen | blackQueen)
        return false;

    int wN = __builtin_popcountll(whiteKnights);
    int bN = __builtin_popcountll(blackKnights);
    int wB = __builtin_popcountll(whiteBishops);
    int bB = __builtin_popcountll(blackBishops);

    // K vs K
    if (!wN && !bN && !wB && !bB) return true;

    // K+N vs K  or  K vs K+N
    if (wB == 0 && bB == 0 && bN == 0 && wN == 1) return true;
    if (wB == 0 && bB == 0 && wN == 0 && bN == 1) return true;

    // K+B vs K  or  K vs K+B
    if (wN == 0 && bN == 0 && bB == 0 && wB == 1) return true;
    if (wN == 0 && bN == 0 && wB == 0 && bB == 1) return true;

    // K+B vs K+B (same coloured squares)
    if (wN == 0 && bN == 0 && wB == 1 && bB == 1) {
        // Same color if both bishops are on same parity square
        int wBsq = __builtin_ctzll(whiteBishops);
        int bBsq = __builtin_ctzll(blackBishops);
        if (((wBsq ^ bBsq) & 1) == 0) return true; // same diagonal color
    }

    return false;
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
void Board::makeMove(const Move& move) {
    int from = move.getFrom();
    int to   = move.getTo();
    int type = move.getType();
    int side = sideToMove;
    int opponent = side ^ 1;

    int movingPiece = getPieceAt(from);
    int captured    = (type == EN_PASSANT)
                        ? PAWN
                        : getPieceAt(to);

    // Always push undo FIRST — never return early after this point
    UndoInfo& undo       = undoStack[undoCount++];
    undo.move            = move;
    undo.movedPiece      = movingPiece;
    undo.capturedPiece   = captured;
    undo.castlingRights  = castlingRights;
    undo.enPassantSquare = enPassantSquare;
    undo.halfMoveClock   = halfMoveClock;
    undo.zobristHash     = zobristHash;

    // Guard: if there is no piece here the board is already corrupt,
    // but we must still keep make/undo balanced so the stack stays clean.
    if (movingPiece == -1) {
        // Push a no-op: just flip the side so undoMove restores correctly
        zobristHash ^= zobristSideToMove;
        sideToMove  ^= 1;
        positionHistory[historyCount++] = zobristHash;
        return;
    }

    // Remove moving piece from source
    zobristHash ^= zobristTable[movingPiece][side][from];
    removePiece(from, movingPiece, side == 0);

    // Remove captured piece if any
    if (captured != -1) {
        int capSq = (type == EN_PASSANT)
                        ? ((side == 0) ? (to - 8) : (to + 8))
                        : to;
        zobristHash ^= zobristTable[captured][opponent][capSq];
        removePiece(capSq, captured, opponent == 0);
    }

    // Determine final piece type (promotions change it)
    int finalType = movingPiece;
    if      (type == PROMOT_QUEEN)  finalType = QUEEN;
    else if (type == PROMOT_ROOK)   finalType = ROOK;
    else if (type == PROMOT_BISHOP) finalType = BISHOP;
    else if (type == PROMOT_KNIGHT) finalType = KNIGHT;

    if (type == CASTLE) {
        addPiece(to, KING, side == 0);
        zobristHash ^= zobristTable[KING][side][to];

        int rFrom = (to == 6) ? 7 : (to == 2) ? 0 : (to == 62) ? 63 : 56;
        int rTo   = (to == 6) ? 5 : (to == 2) ? 3 : (to == 62) ? 61 : 59;
        removePiece(rFrom, ROOK, side == 0);
        addPiece(rTo,      ROOK, side == 0);
        zobristHash ^= zobristTable[ROOK][side][rFrom];
        zobristHash ^= zobristTable[ROOK][side][rTo];
    } else {
        addPiece(to, finalType, side == 0);
        zobristHash ^= zobristTable[finalType][side][to];
    }

    // En passant square update
    if (enPassantSquare != -1)
        zobristHash ^= zobristEnPassant[enPassantSquare % 8];

    enPassantSquare = (type == DOUBLE_PUSH)
                        ? ((side == 0) ? (from + 8) : (from - 8))
                        : -1;

    if (enPassantSquare != -1)
        zobristHash ^= zobristEnPassant[enPassantSquare % 8];

    // 50-move rule clock: reset on pawn move or capture, else increment
    if (movingPiece == PAWN || captured != -1 || type == EN_PASSANT)
        halfMoveClock = 0;
    else
        halfMoveClock++;

    // Castling rights update
    uint8_t oldCastling = castlingRights;
    if (movingPiece == KING) {
        if (side == 0) castlingRights &= ~(WHITE_CASTLING_KINGSIDE | WHITE_CASTLING_QUEENSIDE);
        else           castlingRights &= ~(BLACK_CASTLING_KINGSIDE | BLACK_CASTLING_QUEENSIDE);
    }
    if (from == 0)  castlingRights &= ~WHITE_CASTLING_QUEENSIDE;
    if (from == 7)  castlingRights &= ~WHITE_CASTLING_KINGSIDE;
    if (from == 56) castlingRights &= ~BLACK_CASTLING_QUEENSIDE;
    if (from == 63) castlingRights &= ~BLACK_CASTLING_KINGSIDE;
    if (to   == 0)  castlingRights &= ~WHITE_CASTLING_QUEENSIDE;
    if (to   == 7)  castlingRights &= ~WHITE_CASTLING_KINGSIDE;
    if (to   == 56) castlingRights &= ~BLACK_CASTLING_QUEENSIDE;
    if (to   == 63) castlingRights &= ~BLACK_CASTLING_KINGSIDE;

    for (int i = 0; i < 4; i++)
        if (((oldCastling >> i) & 1) != ((castlingRights >> i) & 1))
            zobristHash ^= zobristCastling[i];

    zobristHash ^= zobristSideToMove;
    sideToMove  ^= 1;
    positionHistory[historyCount++] = zobristHash;
}
// =========================
// UNDO MOVE
// =========================

void Board::undoMove()
{
    if (undoCount == 0) return;

    // Pop the last move state from the stack tracker
    UndoInfo info = undoStack[--undoCount];
    Move move = info.move;

    int from = move.getFrom();
    int to   = move.getTo();
    int type = move.getType();

    // 1. Restore global environmental registers safely
    sideToMove ^= 1;
    castlingRights = info.castlingRights;
    enPassantSquare = info.enPassantSquare;
    halfMoveClock = info.halfMoveClock;
    zobristHash = info.zobristHash;

    int side = sideToMove;
    int opponent = side ^ 1;
    int pieceType = info.movedPiece;

    // ========================================================================
    // MUTUALLY EXCLUSIVE STATE RECOVERY (Prevents Bitboard Pollution)
    // ========================================================================
    if (type == CASTLE)
    {
        // Remove King from destination square and place back on origin
        removePiece(to, KING, side == 0);
        addPiece(from, KING, side == 0);

        // Put the Rooks back to their home corners
        if (side == 0) // White
        {
            if (to == 6)      { removePiece(5, ROOK, true); addPiece(7, ROOK, true); }
            else if (to == 2) { removePiece(3, ROOK, true); addPiece(0, ROOK, true); }
        }
        else // Black
        {
            if (to == 62)     { removePiece(61, ROOK, false); addPiece(63, ROOK, false); }
            else if (to == 58) { removePiece(59, ROOK, false); addPiece(56, ROOK, false); }
        }
    }
    else if (type >= PROMOT_QUEEN)
    {
        // Map promotion flags to target piece index types
        int promotedType = QUEEN;
        if (type == PROMOT_ROOK)   promotedType = ROOK;
        if (type == PROMOT_BISHOP) promotedType = BISHOP;
        if (type == PROMOT_KNIGHT) promotedType = KNIGHT;

        // Remove the promoted officer and place the home pawn back on origin
        removePiece(to, promotedType, side == 0);
        addPiece(from, PAWN, side == 0);

        // Re-add captured piece back to destination square if there was one
        if (info.capturedPiece != -1)
        {
            addPiece(to, info.capturedPiece, opponent == 0);
        }
    }
    else if (type == EN_PASSANT)
    {
        // Lift pawn off destination square, put back on origin
        removePiece(to, PAWN, side == 0);
        addPiece(from, PAWN, side == 0);

        // Re-materialize the captured victim pawn on its true row offset rank
        int capSq = (side == 0) ? (to - 8) : (to + 8);
        addPiece(capSq, PAWN, opponent == 0);
    }
    else // NORMAL MOVES & DOUBLE PUSHES
    {
        // Lift moving piece off destination square, put back on origin
        removePiece(to, pieceType, side == 0);
        addPiece(from, pieceType, side == 0);

        // Re-materialize standard target capture if there was one
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


    // Pawn: discourage backward pawns, reward center and advancement
    static const int pawnMG[64] = {
         0,  0,  0,  0,  0,  0,  0,  0,   // rank 1 (impossible for pawns but pad)
         5, 10, 10,-20,-20, 10, 10,  5,   // rank 2
         5, -5,-10,  0,  0,-10, -5,  5,   // rank 3
         0,  0,  0, 20, 20,  0,  0,  0,   // rank 4
         5,  5, 10, 25, 25, 10,  5,  5,   // rank 5
        10, 10, 20, 30, 30, 20, 10, 10,   // rank 6
        50, 50, 50, 50, 50, 50, 50, 50,   // rank 7
         0,  0,  0,  0,  0,  0,  0,  0    // rank 8
    };

    static const int pawnEG[64] = {
         0,  0,  0,  0,  0,  0,  0,  0,
        -5, -5, -5, -5, -5, -5, -5, -5,
         0,  0,  0,  0,  0,  0,  0,  0,
         5,  5,  5,  5,  5,  5,  5,  5,
        10, 10, 10, 10, 10, 10, 10, 10,
        20, 20, 20, 20, 20, 20, 20, 20,
        40, 40, 40, 40, 40, 40, 40, 40,
         0,  0,  0,  0,  0,  0,  0,  0
    };

    static const int knightTable[64] = {
        -50,-40,-30,-30,-30,-30,-40,-50,
        -40,-20,  0,  5,  5,  0,-20,-40,
        -30,  5, 10, 15, 15, 10,  5,-30,
        -30,  0, 15, 20, 20, 15,  0,-30,
        -30,  5, 15, 20, 20, 15,  5,-30,
        -30,  0, 10, 15, 15, 10,  0,-30,
        -40,-20,  0,  0,  0,  0,-20,-40,
        -50,-40,-30,-30,-30,-30,-40,-50,
    };

    static const int bishopTable[64] = {
        -20,-10,-10,-10,-10,-10,-10,-20,
        -10,  5,  0,  0,  0,  0,  5,-10,
        -10, 10, 10, 10, 10, 10, 10,-10,
        -10,  0, 10, 10, 10, 10,  0,-10,
        -10,  5,  5, 10, 10,  5,  5,-10,
        -10,  0,  5, 10, 10,  5,  0,-10,
        -10,  0,  0,  0,  0,  0,  0,-10,
        -20,-10,-10,-10,-10,-10,-10,-20,
    };

    static const int rookMGTable[64] = {
         0,  0,  0,  5,  5,  0,  0,  0,   // rank 1
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
         5, 10, 10, 10, 10, 10, 10,  5,   // rank 7
         0,  0,  0,  0,  0,  0,  0,  0    // rank 8
    };

    static const int queenTable[64] = {
        -20,-10,-10, -5, -5,-10,-10,-20,
        -10,  0,  5,  0,  0,  0,  0,-10,
        -10,  5,  5,  5,  5,  5,  0,-10,
          0,  0,  5,  5,  5,  5,  0, -5,
         -5,  0,  5,  5,  5,  5,  0, -5,
        -10,  0,  5,  5,  5,  5,  0,-10,
        -10,  0,  0,  0,  0,  0,  0,-10,
        -20,-10,-10, -5, -5,-10,-10,-20
    };

    // King middlegame: extremely strong preference for castled corners.
    // Center squares are catastrophic — penalties scale with centrality.
    // These values are from white's perspective (rank1 = index 0..7).
    static const int kingMG[64] = {
        // rank 1 — castled corners are safe, center is dangerous
         30, 40, 20,-20,-30,-20,  5, 30,
        // rank 2 — behind pawn structure is acceptable, center awful
         10, 10,-20,-40,-40,-20, 10, 10,
        // rank 3+ — any king here in MG is a disaster
        -30,-40,-50,-60,-60,-50,-40,-30,
        -40,-60,-70,-80,-80,-70,-60,-40,
        -50,-70,-80,-90,-90,-80,-70,-50,
        -50,-70,-80,-90,-90,-80,-70,-50,
        -50,-70,-80,-90,-90,-80,-70,-50,
        -50,-70,-80,-90,-90,-80,-70,-50,
    };

    // King endgame: centralize
    static const int kingEG[64] = {
        -50,-30,-30,-30,-30,-30,-30,-50,
        -30,-30,  0,  0,  0,  0,-30,-30,
        -30,-10, 20, 30, 30, 20,-10,-30,
        -30,-10, 30, 40, 40, 30,-10,-30,
        -30,-10, 30, 40, 40, 30,-10,-30,
        -30,-10, 20, 30, 30, 20,-10,-30,
        -30,-20,-10,  0,  0,-10,-20,-30,
        -50,-40,-30,-20,-20,-30,-40,-50,
    };


int Board::evaluate()
{
    int mg = 0, eg = 0;

    // ── Game phase ────────────────────────────────────────
    int phase =
        __builtin_popcountll(whiteKnights | blackKnights) * 1 +
        __builtin_popcountll(whiteBishops | blackBishops) * 1 +
        __builtin_popcountll(whiteRooks   | blackRooks)   * 2 +
        __builtin_popcountll(whiteQueen   | blackQueen)   * 4;
    if (phase > 24) phase = 24;
    int mgPhase = phase;
    int egPhase = 24 - phase;

    // ── PST scoring helper ────────────────────────────────
    auto addPST = [&](uint64_t bb, const int* mgT, const int* egT,
                      int mgVal, int egVal, bool white)
    {
        while (bb) {
            int sq  = __builtin_ctzll(bb);
            int psq = white ? sq : (sq ^ 56);
            if (white) { mg += mgVal + mgT[psq]; eg += egVal + egT[psq]; }
            else       { mg -= mgVal + mgT[psq]; eg -= egVal + egT[psq]; }
            bb &= bb - 1;
        }
    };

    // ── Material + PST ────────────────────────────────────
    addPST(whitePawns,   pawnMG,     pawnEG,     100, 110, true);
    addPST(blackPawns,   pawnMG,     pawnEG,     100, 110, false);
    addPST(whiteKnights, knightTable,knightTable, 320, 310, true);
    addPST(blackKnights, knightTable,knightTable, 320, 310, false);
    addPST(whiteBishops, bishopTable,bishopTable, 330, 340, true);
    addPST(blackBishops, bishopTable,bishopTable, 330, 340, false);
    addPST(whiteRooks,   rookMGTable,rookMGTable, 500, 520, true);
    addPST(blackRooks,   rookMGTable,rookMGTable, 500, 520, false);
    addPST(whiteQueen,   queenTable, queenTable,  900, 920, true);
    addPST(blackQueen,   queenTable, queenTable,  900, 920, false);

    // ── Bishop pair ───────────────────────────────────────
    if (__builtin_popcountll(whiteBishops) >= 2) { mg += 30; eg += 55; }
    if (__builtin_popcountll(blackBishops) >= 2) { mg -= 30; eg -= 55; }

    // ── King PST ──────────────────────────────────────────
    int wk = __builtin_ctzll(whiteKing);
    int bk = __builtin_ctzll(blackKing);
    mg += kingMG[wk];       eg += kingEG[wk];
    mg -= kingMG[bk ^ 56];  eg -= kingEG[bk ^ 56];

    // ── Pawn structure ────────────────────────────────────
    // Build file occupancy masks
    // fileMask[f] = all squares on file f
    // For each pawn, check: doubled, isolated, passed
    auto fileMask = [](int f) -> uint64_t {
        return 0x0101010101010101ULL << f;
    };
    auto adjacentFiles = [&](int f) -> uint64_t {
        uint64_t m = 0ULL;
        if (f > 0) m |= fileMask(f-1);
        if (f < 7) m |= fileMask(f+1);
        return m;
    };

    // Precompute file sets
    uint64_t wPawnFiles = 0ULL, bPawnFiles = 0ULL;
    {
        uint64_t bb = whitePawns;
        while (bb) { wPawnFiles |= fileMask(__builtin_ctzll(bb) & 7); bb &= bb-1; }
        bb = blackPawns;
        while (bb) { bPawnFiles |= fileMask(__builtin_ctzll(bb) & 7); bb &= bb-1; }
    }

    // White pawns
    {
        uint64_t bb = whitePawns;
        while (bb) {
            int sq   = __builtin_ctzll(bb);
            int file = sq & 7;
            int rank = sq >> 3;

            // Doubled pawn penalty
            if (__builtin_popcountll(whitePawns & fileMask(file)) > 1) {
                mg -= 8; eg -= 16;
            }

            // Isolated pawn penalty
            if (!(whitePawns & adjacentFiles(file))) {
                mg -= 12; eg -= 20;
            }

            // Passed pawn bonus
            // A pawn is passed if no enemy pawns on same or adjacent files ahead of it
            uint64_t frontSpan = 0ULL;
            for (int r = rank+1; r <= 7; r++)
                frontSpan |= ((fileMask(file) | adjacentFiles(file)) & (0xFFULL << (r*8)));
            if (!(blackPawns & frontSpan)) {
                // Bonus scales with how advanced the pawn is
                static const int passedBonus[8] = {0, 10, 15, 25, 45, 70, 100, 0};
                mg += passedBonus[rank];
                eg += passedBonus[rank] * 2;   // passed pawns much more important in EG

                // King proximity bonus in endgame: our king close to passer = good
                int wkRank = wk >> 3, wkFile = wk & 7;
                int bkRank = bk >> 3, bkFile = bk & 7;
                int wkDist = abs(wkRank - rank) + abs(wkFile - file);
                int bkDist = abs(bkRank - rank) + abs(bkFile - file);
                eg += (bkDist - wkDist) * 10;
            }

            bb &= bb - 1;
        }
    }

    // Black pawns
    {
        uint64_t bb = blackPawns;
        while (bb) {
            int sq   = __builtin_ctzll(bb);
            int file = sq & 7;
            int rank = sq >> 3;   // rank from white's side; black pawn at rank7 = starting, rank2 = promotion

            if (__builtin_popcountll(blackPawns & fileMask(file)) > 1) {
                mg += 8; eg += 16;
            }

            if (!(blackPawns & adjacentFiles(file))) {
                mg += 12; eg += 20;
            }

            // Passed pawn: no white pawns on same/adjacent files below it
            uint64_t frontSpan = 0ULL;
            for (int r = rank-1; r >= 0; r--)
                frontSpan |= ((fileMask(file) | adjacentFiles(file)) & (0xFFULL << (r*8)));
            if (!(whitePawns & frontSpan)) {
                // rank from black's perspective: rank8=0, rank1=7; black pawn at rank 2 (sq>>3==1) is advanced
                int blackRank = 7 - rank;
                static const int passedBonus[8] = {0, 10, 15, 25, 45, 70, 100, 0};
                mg -= passedBonus[blackRank];
                eg -= passedBonus[blackRank] * 2;

                int wkRank = wk >> 3, wkFile = wk & 7;
                int bkRank = bk >> 3, bkFile = bk & 7;
                int wkDist = abs(wkRank - rank) + abs(wkFile - file);
                int bkDist = abs(bkRank - rank) + abs(bkFile - file);
                eg -= (wkDist - bkDist) * 10;
            }

            bb &= bb - 1;
        }
    }

    // ── Rook on open / semi-open file ─────────────────────
    {
        uint64_t bb = whiteRooks;
        while (bb) {
            int file = __builtin_ctzll(bb) & 7;
            uint64_t fm = fileMask(file);
            if (!(whitePawns & fm)) {           // open or semi-open
                if (!(blackPawns & fm)) { mg += 20; eg += 15; }  // fully open
                else                    { mg += 10; eg +=  8; }  // semi-open
            }
            bb &= bb - 1;
        }
        bb = blackRooks;
        while (bb) {
            int file = __builtin_ctzll(bb) & 7;
            uint64_t fm = fileMask(file);
            if (!(blackPawns & fm)) {
                if (!(whitePawns & fm)) { mg -= 20; eg -= 15; }
                else                    { mg -= 10; eg -=  8; }
            }
            bb &= bb - 1;
        }
    }

    // ── King safety: comprehensive ────────────────────────
    {
        auto fileMaskLocal = [](int f) -> uint64_t {
            return 0x0101010101010101ULL << f;
        };

        // ── White king safety ──────────────────────────────
        {
            int wkFile = wk & 7;
            int wkRank = wk >> 3;

            // 1. Pawn shield: count pawns on ranks 2-3 in king's file + adjacent
            //    Works regardless of which rank king is on
            int shieldCount = 0;
            for (int df = -1; df <= 1; df++) {
                int f = wkFile + df;
                if (f < 0 || f > 7) continue;
                uint64_t fm = fileMaskLocal(f);
                // Pawns on rank 2 or 3 (closest shield)
                if (whitePawns & fm & (0xFFULL << 8))  shieldCount += 2;
                else if (whitePawns & fm & (0xFFULL << 16)) shieldCount += 1;
            }
            mg += shieldCount * 10;

            // 2. Penalty for open/semi-open files near king (enemy can attack)
            for (int df = -1; df <= 1; df++) {
                int f = wkFile + df;
                if (f < 0 || f > 7) continue;
                uint64_t fm = fileMaskLocal(f);
                bool noWhitePawn = !(whitePawns & fm);
                bool noBlackPawn = !(blackPawns & fm);
                if (noWhitePawn && noBlackPawn) mg -= 25; // fully open
                else if (noWhitePawn)           mg -= 15; // semi-open (our pawn gone)
            }

            // 3. Castling bonus: reward having castled or still having rights
            bool wHasCastled = (wkFile == 6 && wkRank == 0) ||
                               (wkFile == 2 && wkRank == 0);
            bool wCanCastle   = (castlingRights & (WHITE_CASTLING_KINGSIDE | WHITE_CASTLING_QUEENSIDE)) != 0;
            if (wHasCastled)   mg += 60;  // already castled: big bonus
            else if (wCanCastle) mg += 20; // rights intact: small bonus
            else                 mg -= 40; // lost rights without castling: penalty

            // 4. King on open file near center — dangerous
            if (wkRank > 0 && wkRank < 7) {
                // King walked to the center or far rank — extra penalty beyond PST
                int distToCenter = std::abs(wkFile - 3) + std::abs(wkFile - 4);
                int rankPenalty  = wkRank * 8; // higher rank = more dangerous
                mg -= rankPenalty + (distToCenter < 4 ? 20 : 0);
            }

            // 5. Queen off board reduces king danger — scale down if no queens
            bool blackQueenOn = blackQueen != 0ULL;
            if (!blackQueenOn) {
                // Halve all king safety penalties in queenless middlegame
                // (handled naturally by tapered eval, but add explicit bonus)
                mg += 30;
            }
        }

        // ── Black king safety ──────────────────────────────
        {
            int bkFile = bk & 7;
            int bkRank = bk >> 3;

            int shieldCount = 0;
            for (int df = -1; df <= 1; df++) {
                int f = bkFile + df;
                if (f < 0 || f > 7) continue;
                uint64_t fm = fileMaskLocal(f);
                if (blackPawns & fm & (0xFFULL << 48)) shieldCount += 2;
                else if (blackPawns & fm & (0xFFULL << 40)) shieldCount += 1;
            }
            mg -= shieldCount * 10;

            for (int df = -1; df <= 1; df++) {
                int f = bkFile + df;
                if (f < 0 || f > 7) continue;
                uint64_t fm = fileMaskLocal(f);
                bool noBlackPawn = !(blackPawns & fm);
                bool noWhitePawn = !(whitePawns & fm);
                if (noBlackPawn && noWhitePawn) mg += 25;
                else if (noBlackPawn)           mg += 15;
            }

            bool bHasCastled = (bkFile == 6 && bkRank == 7) ||
                               (bkFile == 2 && bkRank == 7);
            bool bCanCastle   = (castlingRights & (BLACK_CASTLING_KINGSIDE | BLACK_CASTLING_QUEENSIDE)) != 0;
            if (bHasCastled)   mg -= 60;
            else if (bCanCastle) mg -= 20;
            else                 mg += 40;

            if (bkRank > 0 && bkRank < 7) {
                int distToCenter = std::abs(bkFile - 3) + std::abs(bkFile - 4);
                int rankPenalty  = (7 - bkRank) * 8;
                mg += rankPenalty + (distToCenter < 4 ? 20 : 0);
            }

            bool whiteQueenOn = whiteQueen != 0ULL;
            if (!whiteQueenOn) mg -= 30;
        }
    }

    // ── Backward pawns ────────────────────────────────────
    // A pawn is backward if it can't be defended by another pawn
    // and the square ahead is controlled by an enemy pawn.
    {
        // White backward pawns
        uint64_t bb = whitePawns;
        while (bb) {
            int sq   = __builtin_ctzll(bb);
            int file = sq & 7;
            int rank = sq >> 3;

            // Is there a friendly pawn behind on adjacent files? (supporting)
            bool hasPawnSupport = false;
            uint64_t behindAdj = 0ULL;
            for (int r = 0; r < rank; r++) {
                if (file > 0) behindAdj |= (1ULL << (r*8 + file - 1));
                if (file < 7) behindAdj |= (1ULL << (r*8 + file + 1));
            }
            if (whitePawns & behindAdj) hasPawnSupport = true;

            // Is the stop square (rank+1) attacked by a black pawn?
            if (!hasPawnSupport && rank < 7) {
                int stopSq = sq + 8;
                uint64_t stopBit = 1ULL << stopSq;
                // Black pawn attacks the stop square
                bool stopAttacked = false;
                if (file > 0 && (blackPawns & (1ULL << (stopSq - 1 + 8)))) {} // not needed
                // simpler: check if any black pawn attacks stopSq
                if ((file > 0 && (blackPawns & (1ULL << (stopSq + 7)))) ||
                    (file < 7 && (blackPawns & (1ULL << (stopSq + 9)))))
                    stopAttacked = true;
                if (stopAttacked && !(whitePawns & fileMask(file) & ~(1ULL << sq))) {
                    mg -= 10; eg -= 16;
                }
            }
            bb &= bb - 1;
        }
        // Black backward pawns
        bb = blackPawns;
        while (bb) {
            int sq   = __builtin_ctzll(bb);
            int file = sq & 7;
            int rank = sq >> 3;

            bool hasPawnSupport = false;
            uint64_t behindAdj = 0ULL;
            for (int r = rank + 1; r <= 7; r++) {
                if (file > 0) behindAdj |= (1ULL << (r*8 + file - 1));
                if (file < 7) behindAdj |= (1ULL << (r*8 + file + 1));
            }
            if (blackPawns & behindAdj) hasPawnSupport = true;

            if (!hasPawnSupport && rank > 0) {
                int stopSq = sq - 8;
                bool stopAttacked = false;
                if ((file > 0 && (whitePawns & (1ULL << (stopSq - 9)))) ||
                    (file < 7 && (whitePawns & (1ULL << (stopSq - 7)))))
                    stopAttacked = true;
                if (stopAttacked && !(blackPawns & fileMask(file) & ~(1ULL << sq))) {
                    mg += 10; eg += 16;
                }
            }
            bb &= bb - 1;
        }
    }

    // ── Knight outpost bonus ──────────────────────────────
    // A knight on an advanced square that can't be attacked by enemy pawns
    {
        // Outpost squares: rank 4-6 for white (index 24-47), not attackable by black pawns
        uint64_t bb = whiteKnights;
        while (bb) {
            int sq   = __builtin_ctzll(bb);
            int file = sq & 7;
            int rank = sq >> 3;
            if (rank >= 4) {
                // Check that no black pawn can attack this square
                bool safe = true;
                for (int r = rank + 1; r <= 7; r++) {
                    if (file > 0 && (blackPawns & (1ULL << (r*8 + file - 1)))) { safe = false; break; }
                    if (file < 7 && (blackPawns & (1ULL << (r*8 + file + 1)))) { safe = false; break; }
                }
                if (safe) {
                    // Bigger bonus if supported by own pawn
                    bool supported = (file > 0 && (whitePawns & (1ULL << (sq - 9)))) ||
                                     (file < 7 && (whitePawns & (1ULL << (sq - 7))));
                    mg += supported ? 20 : 12;
                    eg += supported ? 25 : 15;
                }
            }
            bb &= bb - 1;
        }
        bb = blackKnights;
        while (bb) {
            int sq   = __builtin_ctzll(bb);
            int file = sq & 7;
            int rank = sq >> 3;
            if (rank <= 3) {
                bool safe = true;
                for (int r = rank - 1; r >= 0; r--) {
                    if (file > 0 && (whitePawns & (1ULL << (r*8 + file - 1)))) { safe = false; break; }
                    if (file < 7 && (whitePawns & (1ULL << (r*8 + file + 1)))) { safe = false; break; }
                }
                if (safe) {
                    bool supported = (file > 0 && (blackPawns & (1ULL << (sq + 7)))) ||
                                     (file < 7 && (blackPawns & (1ULL << (sq + 9))));
                    mg -= supported ? 20 : 12;
                    eg -= supported ? 25 : 15;
                }
            }
            bb &= bb - 1;
        }
    }

    // ── Rook on 7th rank ─────────────────────────────────
    {
        // White rook on rank 7 (index 48-55): enemy king on rank 8, or enemy pawns on rank 7
        uint64_t rank7mask = 0x00FF000000000000ULL;
        uint64_t rank8mask = 0xFF00000000000000ULL;
        uint64_t rank2mask = 0x000000000000FF00ULL;
        uint64_t rank1mask = 0x00000000000000FFULL;

        if (whiteRooks & rank7mask) {
            if ((blackKing & rank8mask) || (blackPawns & rank7mask)) {
                int cnt = __builtin_popcountll(whiteRooks & rank7mask);
                mg += 20 * cnt; eg += 30 * cnt;
            }
        }
        if (blackRooks & rank2mask) {
            if ((whiteKing & rank1mask) || (whitePawns & rank2mask)) {
                int cnt = __builtin_popcountll(blackRooks & rank2mask);
                mg -= 20 * cnt; eg -= 30 * cnt;
            }
        }

        // Connected rooks bonus (same rank or file, no pieces between)
        if (__builtin_popcountll(whiteRooks) >= 2) {
            int r1 = __builtin_ctzll(whiteRooks);
            int r2 = 63 - __builtin_clzll(whiteRooks);
            if (r1 != r2) {
                int r1rank = r1 >> 3, r2rank = r2 >> 3;
                int r1file = r1 & 7,  r2file = r2 & 7;
                if (r1rank == r2rank || r1file == r2file) { mg += 15; eg += 10; }
            }
        }
        if (__builtin_popcountll(blackRooks) >= 2) {
            int r1 = __builtin_ctzll(blackRooks);
            int r2 = 63 - __builtin_clzll(blackRooks);
            if (r1 != r2) {
                int r1rank = r1 >> 3, r2rank = r2 >> 3;
                int r1file = r1 & 7,  r2file = r2 & 7;
                if (r1rank == r2rank || r1file == r2file) { mg -= 15; eg -= 10; }
            }
        }
    }

    // ── Hanging piece penalty ─────────────────────────────
    // A piece is hanging if it is on a square attacked by the opponent
    // and not defended. Use a rough approximation: attacked by a pawn = hanging.
    {
        // White pieces attacked by black pawns
        uint64_t bPawnAtk = 0ULL;
        {
            uint64_t bp = blackPawns;
            while (bp) {
                int sq = __builtin_ctzll(bp);
                int f = sq & 7;
                if (f > 0) bPawnAtk |= (1ULL << (sq - 9));
                if (f < 7) bPawnAtk |= (1ULL << (sq - 7));
                bp &= bp - 1;
            }
        }
        // Penalise white pieces (not pawns/king) hanging on pawn-attacked squares
        uint64_t wHanging = (whiteKnights | whiteBishops | whiteRooks | whiteQueen) & bPawnAtk;
        while (wHanging) {
            int sq = __builtin_ctzll(wHanging);
            int pt = getPieceAt(sq);
            static const int hangVal[] = {0,30,30,50,80,0};
            if (pt >= 0 && pt < 6) { mg -= hangVal[pt]; eg -= hangVal[pt]; }
            wHanging &= wHanging - 1;
        }

        // Black pieces attacked by white pawns
        uint64_t wPawnAtk = 0ULL;
        {
            uint64_t wp = whitePawns;
            while (wp) {
                int sq = __builtin_ctzll(wp);
                int f = sq & 7;
                if (f > 0) wPawnAtk |= (1ULL << (sq + 7));
                if (f < 7) wPawnAtk |= (1ULL << (sq + 9));
                wp &= wp - 1;
            }
        }
        uint64_t bHanging = (blackKnights | blackBishops | blackRooks | blackQueen) & wPawnAtk;
        while (bHanging) {
            int sq = __builtin_ctzll(bHanging);
            int pt = getPieceAt(sq);
            static const int hangVal[] = {0,30,30,50,80,0};
            if (pt >= 0 && pt < 6) { mg += hangVal[pt]; eg += hangVal[pt]; }
            bHanging &= bHanging - 1;
        }
    }

    // ── Space bonus (middlegame only) ─────────────────────
    // Count squares in enemy half controlled by our pawns (pawn pushes)
    {
        // White: pawns advanced to ranks 3-5 in center files (c-f)
        uint64_t centerMask = 0x0000003C3C000000ULL; // ranks 4-5, files c-f
        uint64_t wSpace = whitePawns & centerMask;
        uint64_t bSpace = blackPawns & centerMask;
        int spaceDiff = __builtin_popcountll(wSpace) - __builtin_popcountll(bSpace);
        mg += spaceDiff * 4;
    }

    // ── Tapered eval ──────────────────────────────────────
    int total = (mg * mgPhase + eg * egPhase) / 24;

    // NOTE: mobility and king safety are added by GenerateMoves::evalFull()
    // which wraps this function — do not call evaluate() directly in the search.
    return (sideToMove == 0) ? total : -total;
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
    UndoInfo& undo = undoStack[undoCount++];
    undo.castlingRights  = castlingRights;
    undo.enPassantSquare = enPassantSquare;
    undo.halfMoveClock   = halfMoveClock;
    undo.zobristHash     = zobristHash;
    undo.capturedPiece   = -1;
    undo.movedPiece      = -1;

    if (enPassantSquare != -1)
        zobristHash ^= zobristEnPassant[enPassantSquare % 8];

    enPassantSquare = -1;
    halfMoveClock++;         // null move increments clock (no capture/pawn)

    zobristHash ^= zobristSideToMove;
    sideToMove ^= 1;
    positionHistory[historyCount++] = zobristHash;
}

void Board::undoNullMove()
{
    historyCount--;
    UndoInfo& info = undoStack[--undoCount];
    sideToMove       ^= 1;
    castlingRights    = info.castlingRights;
    enPassantSquare   = info.enPassantSquare;
    halfMoveClock     = info.halfMoveClock;
    zobristHash       = info.zobristHash;
}

// =========================
// SANITY CHECK
// =========================
void Board::sanityCheck() const {
    // 1. Piece bitboards must compose into their color aggregate
    uint64_t calcWhite = whitePawns | whiteKnights | whiteBishops |
                         whiteRooks | whiteQueen | whiteKing;
    uint64_t calcBlack = blackPawns | blackKnights | blackBishops |
                         blackRooks | blackQueen | blackKing;

    assert(whitePieces == calcWhite && "whitePieces desync");
    assert(blackPieces == calcBlack && "blackPieces desync");
    assert(occupied == (whitePieces | blackPieces) && "occupied desync");
    assert((whitePieces & blackPieces) == 0 && "color overlap");

    // 2. Exactly one king per side
    assert(__builtin_popcountll(whiteKing) == 1 && "white king count != 1");
    assert(__builtin_popcountll(blackKing) == 1 && "black king count != 1");

    // 3. No intra-color piece overlap
    uint64_t wAll[] = {whitePawns, whiteKnights, whiteBishops, whiteRooks, whiteQueen, whiteKing};
    for (int i = 0; i < 6; i++)
        for (int j = i+1; j < 6; j++)
            assert((wAll[i] & wAll[j]) == 0 && "white piece type overlap");

    uint64_t bAll[] = {blackPawns, blackKnights, blackBishops, blackRooks, blackQueen, blackKing};
    for (int i = 0; i < 6; i++)
        for (int j = i+1; j < 6; j++)
            assert((bAll[i] & bAll[j]) == 0 && "black piece type overlap");

    // 4. Side to move is valid
    assert((sideToMove == 0 || sideToMove == 1) && "invalid sideToMove");

    // 5. Castling rights consistency
    if (!(whiteKing & (1ULL << 4)))
        assert(!(castlingRights & (WHITE_CASTLING_KINGSIDE | WHITE_CASTLING_QUEENSIDE))
               && "white has castling rights but king not on e1");
    if (!(blackKing & (1ULL << 60)))
        assert(!(castlingRights & (BLACK_CASTLING_KINGSIDE | BLACK_CASTLING_QUEENSIDE))
               && "black has castling rights but king not on e8");

    // 6. En passant square validity
    if (enPassantSquare != -1) {
        if (sideToMove == 0) assert(enPassantSquare >= 40 && enPassantSquare <= 47
                                    && "white EP square not on rank 6");
        else                 assert(enPassantSquare >= 16 && enPassantSquare <= 23
                                    && "black EP square not on rank 3");
    }
}