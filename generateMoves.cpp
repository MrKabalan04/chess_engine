#include "generateMoves.h"
#include <iostream>
#include <cstring>
#include <cstdint>
#include <climits>
#include "types.h"
using namespace std;

uint64_t GenerateMoves::rookTable[64][4096];
uint64_t GenerateMoves::bishopTable[64][512];

void GenerateMoves::init() {

    // =========================
    // ROOK MAGIC NUMBERS
    // =========================
    static const uint64_t rookMagicsLocal[64] = {
        0x8a80104000800020ULL, 0x140002000100040ULL, 0x2801880a0017001ULL, 0x100081001000420ULL,
        0x200020010080420ULL, 0x3001c0002010008ULL, 0x8480008002000100ULL, 0x2080088004402900ULL,
        0x800098204000ULL, 0x2024401000200040ULL, 0x100802000801000ULL, 0x120800800801000ULL,
        0x208808088000400ULL, 0x2802200800400ULL, 0x2200800100020080ULL, 0x801000060821100ULL,
        0x80044006422000ULL, 0x100808020004000ULL, 0x12108a0010204200ULL, 0x140848010000802ULL,
        0x481828014002800ULL, 0x8094004002004100ULL, 0x4010040010010802ULL, 0x20008806104ULL,
        0x100400080208000ULL, 0x2040002120081000ULL, 0x21200680100081ULL, 0x20100080080080ULL,
        0x2000a00200410ULL, 0x20080800400ULL, 0x80088400100102ULL, 0x80004600042881ULL,
        0x4040008040800020ULL, 0x440003000200801ULL, 0x4200011004500ULL, 0x188020010100100ULL,
        0x14800401802800ULL, 0x2080040080800200ULL, 0x124080204001001ULL, 0x200046502000484ULL,
        0x480400080088020ULL, 0x1000422010034000ULL, 0x30200100110040ULL, 0x100021010009ULL,
        0x2002080100110004ULL, 0x202008004008002ULL, 0x20020004010100ULL, 0x2048440040820001ULL,
        0x101002200408200ULL, 0x40802000401080ULL, 0x4008142004410100ULL, 0x2060820c0120200ULL,
        0x1001004080100ULL, 0x20c020080040080ULL, 0x2935610830022400ULL, 0x44440041009200ULL,
        0x280001040802101ULL, 0x2100190040002085ULL, 0x80c0084100102001ULL, 0x4024081001000421ULL,
        0x20030a0244872ULL, 0x12001008414402ULL, 0x2006104900a0804ULL, 0x1004081002402ULL
    };

    static const int rookRelevantBits[64] = {
        12,11,11,11,11,11,11,12,
        11,10,10,10,10,10,10,11,
        11,10,10,10,10,10,10,11,
        11,10,10,10,10,10,10,11,
        11,10,10,10,10,10,10,11,
        11,10,10,10,10,10,10,11,
        11,10,10,10,10,10,10,11,
        12,11,11,11,11,11,11,12
    };

    // =========================
    // BISHOP MAGIC NUMBERS
    // =========================
    static const uint64_t bishopMagicsLocal[64] = {
        0x40040844404084ULL,0x2004208a004208ULL,0x10190041080202ULL,0x108060845042010ULL,
        0x581104180800210ULL,0x2112080446200010ULL,0x1080820820060210ULL,0x3c0808410220200ULL,
        0x4050404440404ULL,0x21001420088ULL,0x24d0080801082102ULL,0x1020a0a020400ULL,
        0x40308200402ULL,0x4011002100800ULL,0x401484104104005ULL,0x801010402020200ULL,
        0x400210c3880100ULL,0x404022024108200ULL,0x810018200204102ULL,0x4002801a02003ULL,
        0x85040820080400ULL,0x810102c808880400ULL,0xe900410884800ULL,0x8002020480840102ULL,
        0x220200865090201ULL,0x2010100a02021202ULL,0x152048408022401ULL,0x20080002081110ULL,
        0x4001001021004000ULL,0x800040400a011002ULL,0xe4004081011002ULL,0x1c004001012080ULL,
        0x8004200962a00220ULL,0x8422100208500202ULL,0x2000402200300c08ULL,0x8646020080080080ULL,
        0x80020a0200100808ULL,0x2010004880111000ULL,0x623000a080011400ULL,0x42008c0340209202ULL,
        0x209188240001000ULL,0x400408a884001800ULL,0x110400a6080400ULL,0x1840060a44020800ULL,
        0x90080104000041ULL,0x201011000808101ULL,0x1a2208080504f080ULL,0x8012020600211212ULL,
        0x500861011240000ULL,0x180806108200800ULL,0x4000020e01040044ULL,0x300000261044000aULL,
        0x802241102020002ULL,0x20906061210001ULL,0x5a84841004010310ULL,0x4010801011c04ULL,
        0xa010109502200ULL,0x4a02012000ULL,0x500201010098b028ULL,0x8040002811040900ULL,
        0x28000010020204ULL,0x6000020202d0240ULL,0x8918844842082200ULL,0x4010011029020020ULL
    };

    static const int bishopRelevantBits[64] = {
        6,5,5,5,5,5,5,6,
        5,5,5,5,5,5,5,5,
        5,5,7,7,7,7,5,5,
        5,5,7,9,9,7,5,5,
        5,5,7,9,9,7,5,5,
        5,5,7,7,7,7,5,5,
        5,5,5,5,5,5,5,5,
        6,5,5,5,5,5,5,6
    };

    // =========================
    // OFFSETS (SAFE + FAST)
    // =========================
    static const int knightOffsets[8] = {
        17,15,10,6,
       -17,-15,-10,-6
    };

    static const int kingOffsets[8] = {
        8,-8,1,-1,
        9,7,-7,-9
    };

    // =========================
    // INIT LOOP
    // =========================
    for (int i = 0; i < 64; i++) {

        int col = i % 8;

        // -------------------------
        // MAGIC DATA
        // -------------------------
        rookMagics[i] = rookMagicsLocal[i];
        rookShifts[i] = 64 - rookRelevantBits[i];

        bishopMagics[i] = bishopMagicsLocal[i];
        bishopShifts[i] = 64 - bishopRelevantBits[i];

        // -------------------------
        // KNIGHT
        // -------------------------
        uint64_t knightMoves = 0ULL;

        for (int k = 0; k < 8; k++) {
            int t = i + knightOffsets[k];
            if (t < 0 || t >= 64) continue;

            int toCol = t % 8;
            if (abs(toCol - col) > 2) continue;

            knightMoves |= (1ULL << t);
        }

        knightMasks[i] = knightMoves;

        // -------------------------
        // KING
        // -------------------------
        uint64_t kingMoves = 0ULL;

        for (int k = 0; k < 8; k++) {
            int t = i + kingOffsets[k];
            if (t < 0 || t >= 64) continue;

            int toCol = t % 8;
            if (abs(toCol - col) > 1) continue;

            kingMoves |= (1ULL << t);
        }

        kingMasks[i] = kingMoves;

        // -------------------------
        // SLIDERS
        // -------------------------
        rookMasks[i] = rookMask(i);
        bishopMasks[i] = bishopMask(i);
    }

    initPawnAttacks();
    initMagicTables();
}

bool GenerateMoves::isSquareAttacked(int sq, int attackerColor, const Board& board)
{
    uint64_t pawns   = (attackerColor == 0) ? board.whitePawns   : board.blackPawns;
    uint64_t knights = (attackerColor == 0) ? board.whiteKnights : board.blackKnights;
    uint64_t bishops = (attackerColor == 0) ? board.whiteBishops : board.blackBishops;
    uint64_t rooks   = (attackerColor == 0) ? board.whiteRooks   : board.blackRooks;
    uint64_t queens  = (attackerColor == 0) ? board.whiteQueen   : board.blackQueen;
    uint64_t king    = (attackerColor == 0) ? board.whiteKing    : board.blackKing;

    if (knightMasks[sq] & knights) return true;
    if (kingMasks[sq] & king) return true;

    // IMPORTANT: pawn attacks MUST be precomputed as "attacks TO sq"
    if (pawnMasks[attackerColor ^ 1][sq] & pawns) return true;

    if (getBishopAttacks(sq, board.occupied) & (bishops | queens)) return true;
    if (getRookAttacks(sq, board.occupied) & (rooks | queens)) return true;

    return false;
}

MoveList GenerateMoves::generateLegalMoves(const Board& board, int side)
{
    MoveList pseudoLegalMoves;
    generateAllMoves(board, side, pseudoLegalMoves);

    MoveList legalMoves;

    int opponent = side ^ 1;

    for (int i = 0; i < pseudoLegalMoves.count; i++)
    {
        Move move = pseudoLegalMoves.moves[i];

        Board copy = board;   // IMPORTANT: fresh copy per move
        copy.makeMove(move);

        int kingSq = (side == 0)
            ? __builtin_ctzll(copy.whiteKing)
            : __builtin_ctzll(copy.blackKing);

        if (!isSquareAttacked(kingSq, opponent, copy))
        {
            legalMoves.addMove(move);
        }
    }

    return legalMoves;
}

bool GenerateMoves::isInCheck(const Board& board, int side){
    int kingSq = (side == 0)
        ? __builtin_ctzll(board.whiteKing)
        : __builtin_ctzll(board.blackKing);
    int opponent = side ^ 1;
    return isSquareAttacked(kingSq, opponent, board);
}

bool GenerateMoves::isCheckmate(const Board& board, int side){
    if (!isInCheck(board, side)){
        return false;
    }
    MoveList legalMoves = generateLegalMoves(board, side);
    return legalMoves.count == 0;
}

bool GenerateMoves::isStalemate(const Board& board, int side){
    if (isInCheck(board, side)){
        return false;
    }
    MoveList legalMoves = generateLegalMoves(board, side);
    return legalMoves.count == 0;
}

void GenerateMoves::generateAllMoves(const Board& board, int side, MoveList& list)
{
    uint64_t friendlyPieces = (side == 0) ? board.whitePieces : board.blackPieces;
    uint64_t opponentPieces = (side == 0) ? board.blackPieces : board.whitePieces;
    uint64_t occupied = board.occupied;
    int enemy = side ^ 1;

    // =========================
    // PAWNS
    // =========================
    uint64_t pawns = (side == 0) ? board.whitePawns : board.blackPawns;

    while (pawns)
    {
        int sq = __builtin_ctzll(pawns);
        generatePawnMoves(sq, side, occupied, opponentPieces, list, board.enPassantSquare);
        pawns &= (pawns - 1);
    }

    // =========================
    // KNIGHTS
    // =========================
    uint64_t knights = (side == 0) ? board.whiteKnights : board.blackKnights;

    while (knights)
    {
        int sq = __builtin_ctzll(knights);
        generateLeapingMoves(sq, KNIGHT, friendlyPieces, list);
        knights &= (knights - 1);
    }

    // =========================
    // BISHOPS
    // =========================
    uint64_t bishops = (side == 0) ? board.whiteBishops : board.blackBishops;

    while (bishops)
    {
        int sq = __builtin_ctzll(bishops);
        generateSlidingMoves(sq, BISHOP, occupied, friendlyPieces, list);
        bishops &= (bishops - 1);
    }

    // =========================
    // ROOKS
    // =========================
    uint64_t rooks = (side == 0) ? board.whiteRooks : board.blackRooks;

    while (rooks)
    {
        int sq = __builtin_ctzll(rooks);
        generateSlidingMoves(sq, ROOK, occupied, friendlyPieces, list);
        rooks &= (rooks - 1);
    }

    // =========================
    // QUEEN
    // =========================
    uint64_t queens = (side == 0) ? board.whiteQueen : board.blackQueen;

    while (queens)
    {
        int sq = __builtin_ctzll(queens);
        generateSlidingMoves(sq, QUEEN, occupied, friendlyPieces, list);
        queens &= (queens - 1);
    }

    // =========================
    // KING MOVES (LEGAL ONLY)
    // =========================
    int kingSq = (side == 0)
        ? __builtin_ctzll(board.whiteKing)
        : __builtin_ctzll(board.blackKing);

    generateKingMoves(kingSq, side, board, list);

    // =========================
    // CASTLING
    // =========================

    // If king is in check → NO CASTLING
    if (isSquareAttacked(kingSq, enemy, board))
        return;

    // ======================================================
    // WHITE CASTLING
    // ======================================================
    if (side == 0)
    {
        // KING SIDE (e1 -> g1)
        if (board.castlingRights & WHITE_CASTLING_KINGSIDE)
        {
            if (!(occupied & ((1ULL << 5) | (1ULL << 6))) &&
                !isSquareAttacked(4, enemy, board) &&
                !isSquareAttacked(5, enemy, board) &&
                !isSquareAttacked(6, enemy, board))
            {
                list.addMove(Move(4, 6, CASTLE));
            }
        }

        // QUEEN SIDE (e1 -> c1)
        if (board.castlingRights & WHITE_CASTLING_QUEENSIDE)
        {
            if (!(occupied & ((1ULL << 1) | (1ULL << 2) | (1ULL << 3))) &&
                !isSquareAttacked(4, enemy, board) &&
                !isSquareAttacked(3, enemy, board) &&
                !isSquareAttacked(2, enemy, board))
            {
                list.addMove(Move(4, 2, CASTLE));
            }
        }
    }

    // ======================================================
    // BLACK CASTLING
    // ======================================================
    else
    {
        // KING SIDE (e8 -> g8)
        if (board.castlingRights & BLACK_CASTLING_KINGSIDE)
        {
            if (!(occupied & ((1ULL << 61) | (1ULL << 62))) &&
                !isSquareAttacked(60, enemy, board) &&
                !isSquareAttacked(61, enemy, board) &&
                !isSquareAttacked(62, enemy, board))
            {
                list.addMove(Move(60, 62, CASTLE));
            }
        }

        // QUEEN SIDE (e8 -> c8)
        if (board.castlingRights & BLACK_CASTLING_QUEENSIDE)
        {
            if (!(occupied & ((1ULL << 57) | (1ULL << 58) | (1ULL << 59))) &&
                !isSquareAttacked(60, enemy, board) &&
                !isSquareAttacked(59, enemy, board) &&
                !isSquareAttacked(58, enemy, board))
            {
                list.addMove(Move(60, 58, CASTLE));
            }
        }
    }
}

int GenerateMoves::minimax(Board& board, int depth, bool isMaximizing){
    if (depth == 0) {
        return board.evaluate();
    }
    int bestScore = isMaximizing ? INT_MIN : INT_MAX;
    MoveList legalMoves = generateLegalMoves(board, board.sideToMove);
    for (int i = 0; i < legalMoves.count; i++) {
        Move move = legalMoves.moves[i];
        Board copy = board;
        copy.makeMove(move);
        int score = minimax(copy, depth - 1, !isMaximizing);
        if (isMaximizing) {
            bestScore = max(score, bestScore);
        } else {
            bestScore = min(score, bestScore);
        }
    }
    return bestScore;
}

Move GenerateMoves::getBestMove(Board& board, int depth)
{
    Move bestMove;
    bool isWhite = board.sideToMove == 0;
    int bestScore = isWhite ? INT_MIN : INT_MAX;

    MoveList legalMoves = generateLegalMoves(board, board.sideToMove);

    for (int i = 0; i < legalMoves.count; i++)
    {
        Move move = legalMoves.moves[i];
        Board copy = board;
        copy.makeMove(move);

        int score = minimax(copy, depth - 1, copy.sideToMove == 0);

        if (isWhite ? score > bestScore : score < bestScore)
        {
            bestScore = score;
            bestMove = move;
        }
    }

    return bestMove;
}