#include "generateMoves.h"
#include <chrono>
#include <iostream>
#include <cstring>
#include <cstdint>
#include <climits>
#include "types.h"
using namespace std;

uint64_t GenerateMoves::rookTable[64][4096];
uint64_t GenerateMoves::bishopTable[64][512];
TTEntry GenerateMoves::transpositionTable[TT_SIZE];

void GenerateMoves::init() {

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

    static const int knightOffsets[8] = { 17,15,10,6,-17,-15,-10,-6 };
    static const int kingOffsets[8]   = { 8,-8,1,-1,9,7,-7,-9 };

    memset(historyTable, 0, sizeof(historyTable));
    memset(killerMoves,  0, sizeof(killerMoves));

    for (int i = 0; i < 64; i++) {
        int col = i % 8;

        rookMagics[i]   = rookMagicsLocal[i];
        rookShifts[i]   = 64 - rookRelevantBits[i];
        bishopMagics[i] = bishopMagicsLocal[i];
        bishopShifts[i] = 64 - bishopRelevantBits[i];

        uint64_t knightMoves = 0ULL;
        for (int k = 0; k < 8; k++) {
            int t = i + knightOffsets[k];
            if (t < 0 || t >= 64) continue;
            if (abs((t % 8) - col) > 2) continue;
            knightMoves |= (1ULL << t);
        }
        knightMasks[i] = knightMoves;

        uint64_t kingMoves = 0ULL;
        for (int k = 0; k < 8; k++) {
            int t = i + kingOffsets[k];
            if (t < 0 || t >= 64) continue;
            if (abs((t % 8) - col) > 1) continue;
            kingMoves |= (1ULL << t);
        }
        kingMasks[i] = kingMoves;

        rookMasks[i]   = rookMask(i);
        bishopMasks[i] = bishopMask(i);
    }

    initPawnAttacks();
    initMagicTables();
}

// ─────────────────────────────────────────────────────────
// ATTACK DETECTION
// ─────────────────────────────────────────────────────────
bool GenerateMoves::isSquareAttacked(int sq, int attackerColor, const Board& board)
{
    uint64_t pawns   = (attackerColor == 0) ? board.whitePawns   : board.blackPawns;
    uint64_t knights = (attackerColor == 0) ? board.whiteKnights : board.blackKnights;
    uint64_t bishops = (attackerColor == 0) ? board.whiteBishops : board.blackBishops;
    uint64_t rooks   = (attackerColor == 0) ? board.whiteRooks   : board.blackRooks;
    uint64_t queens  = (attackerColor == 0) ? board.whiteQueen   : board.blackQueen;
    uint64_t king    = (attackerColor == 0) ? board.whiteKing    : board.blackKing;

    if (knightMasks[sq] & knights)  return true;
    if (kingMasks[sq]   & king)     return true;
    if (pawnToMasks[attackerColor][sq] & pawns) return true;
    if (getBishopAttacks(sq, board.occupied) & (bishops | queens)) return true;
    if (getRookAttacks(sq,   board.occupied) & (rooks   | queens)) return true;

    return false;
}

// ─────────────────────────────────────────────────────────
// LEGAL MOVE GENERATION
// ─────────────────────────────────────────────────────────
MoveList GenerateMoves::generateLegalMoves(Board& board, int side)
{
    MoveList pseudoLegal;
    generateAllMoves(board, side, pseudoLegal);

    MoveList legal;
    int opponent = side ^ 1;

    for (int i = 0; i < pseudoLegal.count; i++)
    {
        Move move = pseudoLegal.moves[i];
        board.makeMove(move);

        int kingSq = (side == 0)
            ? __builtin_ctzll(board.whiteKing)
            : __builtin_ctzll(board.blackKing);

        if (!isSquareAttacked(kingSq, opponent, board))
            legal.addMove(move);

        board.undoMove();
    }
    return legal;
}

bool GenerateMoves::isInCheck(const Board& board, int side)
{
    int kingSq = (side == 0)
        ? __builtin_ctzll(board.whiteKing)
        : __builtin_ctzll(board.blackKing);
    return isSquareAttacked(kingSq, side ^ 1, board);
}

bool GenerateMoves::isCheckmate(Board& board, int side)
{
    return isInCheck(board, side) && generateLegalMoves(board, side).count == 0;
}

bool GenerateMoves::isStalemate(Board& board, int side)
{
    return !isInCheck(board, side) && generateLegalMoves(board, side).count == 0;
}

// ─────────────────────────────────────────────────────────
// ALL PSEUDO-LEGAL MOVES
// ─────────────────────────────────────────────────────────
void GenerateMoves::generateAllMoves(const Board& board, int side, MoveList& list)
{
    uint64_t occupied       = board.occupied;
    uint64_t friendlyPieces = (side == 0) ? board.whitePieces : board.blackPieces;
    uint64_t opponentPieces = (side == 0) ? board.blackPieces : board.whitePieces;
    int enPassantSq         = board.enPassantSquare;

    if (side == 0)
    {
        uint64_t bb;
        bb = board.whitePawns;   while (bb) { int sq = __builtin_ctzll(bb); generatePawnMoves(sq,0,occupied,opponentPieces,list,enPassantSq); bb&=bb-1; }
        bb = board.whiteKnights; while (bb) { int sq = __builtin_ctzll(bb); generateLeapingMoves(sq,KNIGHT,friendlyPieces,list); bb&=bb-1; }
        bb = board.whiteBishops; while (bb) { int sq = __builtin_ctzll(bb); generateSlidingMoves(sq,BISHOP,occupied,friendlyPieces,list); bb&=bb-1; }
        bb = board.whiteRooks;   while (bb) { int sq = __builtin_ctzll(bb); generateSlidingMoves(sq,ROOK,occupied,friendlyPieces,list); bb&=bb-1; }
        bb = board.whiteQueen;   while (bb) { int sq = __builtin_ctzll(bb); generateSlidingMoves(sq,QUEEN,occupied,friendlyPieces,list); bb&=bb-1; }
        bb = board.whiteKing;    while (bb) { int sq = __builtin_ctzll(bb); generateKingMoves(sq,0,board,list); bb&=bb-1; }
    }
    else
    {
        uint64_t bb;
        bb = board.blackPawns;   while (bb) { int sq = __builtin_ctzll(bb); generatePawnMoves(sq,1,occupied,opponentPieces,list,enPassantSq); bb&=bb-1; }
        bb = board.blackKnights; while (bb) { int sq = __builtin_ctzll(bb); generateLeapingMoves(sq,KNIGHT,friendlyPieces,list); bb&=bb-1; }
        bb = board.blackBishops; while (bb) { int sq = __builtin_ctzll(bb); generateSlidingMoves(sq,BISHOP,occupied,friendlyPieces,list); bb&=bb-1; }
        bb = board.blackRooks;   while (bb) { int sq = __builtin_ctzll(bb); generateSlidingMoves(sq,ROOK,occupied,friendlyPieces,list); bb&=bb-1; }
        bb = board.blackQueen;   while (bb) { int sq = __builtin_ctzll(bb); generateSlidingMoves(sq,QUEEN,occupied,friendlyPieces,list); bb&=bb-1; }
        bb = board.blackKing;    while (bb) { int sq = __builtin_ctzll(bb); generateKingMoves(sq,1,board,list); bb&=bb-1; }
    }
}

// ─────────────────────────────────────────────────────────
// MOVE ORDERING
// ─────────────────────────────────────────────────────────
static int mvvLva(const Board& board, const Move& m)
{
    static const int val[] = { 100, 300, 310, 500, 900, 20000 };
    if (m.getType() == EN_PASSANT) return 0;  // PxP
    int v = board.getPieceAt(m.getTo());
    int a = board.getPieceAt(m.getFrom());
    int vv = (v != -1) ? val[v] : 0;
    int av = (a != -1) ? val[a] : 0;
    return 10 * vv - av;
}

void GenerateMoves::orderMoves(MoveList& moves, Board& board, int ply, Move ttMove)
{
    int scores[256];

    for (int i = 0; i < moves.count; i++)
    {
        Move m = moves.moves[i];
        int  s = 0;

        if (m.data == ttMove.data)
        {
            scores[i] = 10'000'000;
            continue;
        }

        bool isCapture = ((board.occupied >> m.getTo()) & 1ULL) != 0;
        bool isEP      = m.getType() == EN_PASSANT;

        if (isCapture || isEP)
        {
            s = 1'000'000 + mvvLva(board, m);
        }
        else if (m.getType() >= PROMOT_QUEEN)
        {
            s = 900'000 + (m.getType() == PROMOT_QUEEN ? 900 : 300);
        }
        else if (ply < 32 && m.data == killerMoves[ply][0].data)
        {
            s = 800'000;
        }
        else if (ply < 32 && m.data == killerMoves[ply][1].data)
        {
            s = 799'000;
        }
        else
        {
            int hIdx = m.getFrom() * 64 + m.getTo();
            s = historyTable[hIdx];
        }

        scores[i] = s;
    }

    // Insertion sort — O(n²) but fine for ≤256 moves
    for (int i = 1; i < moves.count; i++)
    {
        Move m = moves.moves[i];
        int  s = scores[i];
        int  j = i - 1;
        while (j >= 0 && scores[j] < s)
        {
            moves.moves[j+1] = moves.moves[j];
            scores[j+1]      = scores[j];
            j--;
        }
        moves.moves[j+1] = m;
        scores[j+1]      = s;
    }
}

void GenerateMoves::orderCaptures(MoveList& list, const Board& board)
{
    static const int val[] = { 100, 300, 310, 500, 900, 20000 };
    int scores[256];

    for (int i = 0; i < list.count; i++)
    {
        Move m = list.moves[i];
        int v = board.getPieceAt(m.getTo());
        int a = board.getPieceAt(m.getFrom());
        scores[i] = 10 * ((v!=-1)?val[v]:0) - ((a!=-1)?val[a]:0);
    }

    for (int i = 1; i < list.count; i++)
    {
        Move m = list.moves[i];
        int  s = scores[i];
        int  j = i - 1;
        while (j >= 0 && scores[j] < s)
        {
            list.moves[j+1] = list.moves[j];
            scores[j+1]     = scores[j];
            j--;
        }
        list.moves[j+1] = m;
        scores[j+1]     = s;
    }
}

void GenerateMoves::ageHistory()
{
    for (int i = 0; i < 4096; ++i)
        historyTable[i] >>= 2;   // divide by 4 (faster decay than /2)
}

// ─────────────────────────────────────────────────────────
// CAPTURE GENERATION
// ─────────────────────────────────────────────────────────
void GenerateMoves::generateCaptures(const Board& board, int side, MoveList& list)
{
    MoveList all;
    generateAllMoves(board, side, all);

    uint64_t opponentPieces = (side == 0) ? board.blackPieces : board.whitePieces;

    for (int i = 0; i < all.count; i++)
    {
        Move move = all.moves[i];
        bool isCapture   = (1ULL << move.getTo()) & opponentPieces;
        bool isEnPassant = move.getType() == EN_PASSANT;
        bool isPromotion = move.getType() >= PROMOT_QUEEN;
        if (isCapture || isEnPassant || isPromotion)
            list.addMove(move);
    }
}

// ─────────────────────────────────────────────────────────
// QUIESCENCE SEARCH
// ─────────────────────────────────────────────────────────
int GenerateMoves::quiescence(Board& board, int alpha, int beta, int ply)
{
    nodesSearched++;

    // Check time every 1024 nodes
    if ((nodesSearched & 1023) == 0)
        checkTimeBudget();

    if (searchAborted)
        return 0;

    if (board.isThreefoldRepetition())
        return 0;

    int standPat = board.evaluate();
    if (standPat >= beta) return beta;
    if (standPat > alpha) alpha = standPat;

    // Delta pruning: skip if even capturing the best possible piece can't raise alpha
    const int DELTA = 975;  // queen value
    if (standPat + DELTA < alpha) return alpha;

    MoveList captures;
    generateCaptures(board, board.sideToMove, captures);
    orderCaptures(captures, board);

    int side     = board.sideToMove;
    int opponent = side ^ 1;

    for (int i = 0; i < captures.count; i++)
    {
        Move move = captures.moves[i];

        board.makeMove(move);

        int kingSq = (side == 0)
            ? __builtin_ctzll(board.whiteKing)
            : __builtin_ctzll(board.blackKing);

        if (isSquareAttacked(kingSq, opponent, board))
        {
            board.undoMove();
            continue;
        }

        int score = -quiescence(board, -beta, -alpha, ply + 1);
        board.undoMove();

        if (searchAborted) return 0;

        if (score >= beta) return beta;
        if (score > alpha) alpha = score;
    }

    return alpha;
}

// ─────────────────────────────────────────────────────────
// NEGAMAX WITH ALPHA-BETA
// ─────────────────────────────────────────────────────────
int GenerateMoves::negamax(Board& board, int depth, int alpha, int beta, int ply)
{
    nodesSearched++;

    if ((nodesSearched & 1023) == 0)
        checkTimeBudget();

    if (searchAborted)
        return 0;

    // Draw detection
    if (ply > 0 && board.isThreefoldRepetition())
        return 0;

    int originalAlpha = alpha;

    // ── TT Probe ──────────────────────────────────────────
    int ttIndex = board.zobristHash % TT_SIZE;
    TTEntry& ttEntry = transpositionTable[ttIndex];
    Move ttMove(0, 0, NORMAL);

    if (ttEntry.zobristHash == board.zobristHash)
    {
        ttMove = ttEntry.bestMove;

        if (ttEntry.depth >= depth)
        {
            int s = ttEntry.score;

            // Adjust mate scores for current ply
            if (s >  90000) s -= ply;
            if (s < -90000) s += ply;

            if (ttEntry.flag == EXACT)               return s;
            if (ttEntry.flag == ALPHA && s <= alpha) return alpha;
            if (ttEntry.flag == BETA  && s >= beta)  return beta;
        }
    }

    if (depth <= 0)
        return quiescence(board, alpha, beta, ply);

    // ── Check extension ───────────────────────────────────
    bool inCheck = isInCheck(board, board.sideToMove);
    if (inCheck) depth++;   // extend by 1 when in check

    // ── Null Move Pruning ──────────────────────────────────
    if (!inCheck && depth >= 3 && ply > 0)
    {
        int bigPieces = __builtin_popcountll(
            (board.sideToMove == 0)
            ? (board.whiteKnights | board.whiteBishops | board.whiteRooks | board.whiteQueen)
            : (board.blackKnights | board.blackBishops | board.blackRooks | board.blackQueen));

        if (bigPieces >= 1)
        {
            int R = (depth >= 6) ? 3 : 2;
            board.makeNullMove();
            int nullScore = -negamax(board, depth - 1 - R, -beta, -beta + 1, ply + 1);
            board.undoNullMove();

            if (!searchAborted && nullScore >= beta)
                return beta;
        }
    }

    // ── Move generation ───────────────────────────────────
    MoveList moves;
    generateAllMoves(board, board.sideToMove, moves);
    orderMoves(moves, board, ply, ttMove);

    int  bestScore  = -9999999;
    Move bestMove(0, 0, NORMAL);
    int  legalCount = 0;
    int  side       = board.sideToMove;
    int  opponent   = side ^ 1;

    for (int i = 0; i < moves.count; i++)
    {
        Move move = moves.moves[i];

        board.makeMove(move);

        // Skip illegal moves (king left in check)
        int kingSq = (side == 0)
            ? __builtin_ctzll(board.whiteKing)
            : __builtin_ctzll(board.blackKing);

        if (isSquareAttacked(kingSq, opponent, board))
        {
            board.undoMove();
            continue;
        }

        legalCount++;

        // ── Late Move Reductions (LMR) ─────────────────────
        int score;
        bool isCapture = (board.getPieceAt(move.getTo()) != -1) ||
                         (move.getType() == EN_PASSANT);
        // Note: after makeMove, side flipped, so we check the moving side's king, but
        // the capture flag is about the destination which is already set.
        // Re-check: isCapture should look at what was there before the move.
        // Use the occupied BB before the move — captured piece is now gone.
        // Simple heuristic: if material changed (captured piece removed), it's a capture.
        // We'll use a simpler check: type-based + the now-missing piece.
        bool isQuiet = (move.getType() == NORMAL || move.getType() == DOUBLE_PUSH);

        if (!inCheck && depth >= 3 && legalCount > 3 && isQuiet)
        {
            // Reduced search
            int R = 1 + (legalCount > 6 ? 1 : 0);
            score = -negamax(board, depth - 1 - R, -alpha - 1, -alpha, ply + 1);

            // If it beats alpha, re-search at full depth
            if (score > alpha)
                score = -negamax(board, depth - 1, -beta, -alpha, ply + 1);
        }
        else
        {
            score = -negamax(board, depth - 1, -beta, -alpha, ply + 1);
        }

        board.undoMove();

        if (searchAborted) return 0;

        if (score > bestScore)
        {
            bestScore = score;
            bestMove  = move;
        }

        if (score > alpha)
        {
            alpha = score;

            // History heuristic: only quiet moves that raise alpha
            if (isQuiet)
            {
                int hIdx = move.getFrom() * 64 + move.getTo();
                historyTable[hIdx] += depth * depth;
                // Cap to avoid overflow
                if (historyTable[hIdx] > 1'000'000)
                    historyTable[hIdx] = 1'000'000;
            }
        }

        if (alpha >= beta)
        {
            // Killer move: only quiet moves
            if (isQuiet && ply < 32)
            {
                if (killerMoves[ply][0].data != move.data)
                {
                    killerMoves[ply][1] = killerMoves[ply][0];
                    killerMoves[ply][0] = move;
                }
            }
            break;  // Beta cutoff
        }
    }

    // No legal moves = checkmate or stalemate
    if (legalCount == 0)
    {
        if (inCheck) return -99000 + ply;
        else         return 0;
    }

    // ── TT Store ──────────────────────────────────────────
    if (!searchAborted)
    {
        TTFlag flag = EXACT;
        if (bestScore <= originalAlpha) flag = ALPHA;
        else if (bestScore >= beta)     flag = BETA;

        ttEntry.zobristHash = board.zobristHash;
        ttEntry.score       = bestScore;
        ttEntry.depth       = depth;
        ttEntry.flag        = flag;
        ttEntry.bestMove    = bestMove;
    }

    return bestScore;
}

// ─────────────────────────────────────────────────────────
// ITERATIVE DEEPENING ROOT
// ─────────────────────────────────────────────────────────
Move GenerateMoves::getBestMove(Board& board, int maxDepth,
                                 long long myTimeLeftMs,
                                 long long incrementMs, int movesToGo)
{
    if (myTimeLeftMs <= 0) myTimeLeftMs = 5000;
    if (movesToGo    <= 0) movesToGo    = 40;

    // Sensible time allocation
    // Use roughly 1/(movesToGo) of remaining time, plus 75% of increment
    timeLimitMs = (myTimeLeftMs / (movesToGo + 5)) + (long long)(incrementMs * 0.75);

    // Hard caps
    if (timeLimitMs > 8000)                              timeLimitMs = 8000;
    if (timeLimitMs > (long long)(myTimeLeftMs * 0.75))  timeLimitMs = (long long)(myTimeLeftMs * 0.75);
    if (timeLimitMs < 20)                                timeLimitMs = 20;

    searchStartTime = std::chrono::high_resolution_clock::now();
    searchAborted   = false;
    nodesSearched   = 0;

    memset(killerMoves, 0, sizeof(killerMoves));
    ageHistory();

    MoveList legalRootMoves = generateLegalMoves(board, board.sideToMove);

    Move absoluteBestMove;
    if (legalRootMoves.count == 0) return absoluteBestMove;
    absoluteBestMove = legalRootMoves.moves[0];

    // Seed TT move ordering
    {
        Move ttMove;
        int idx = board.zobristHash % TT_SIZE;
        if (transpositionTable[idx].zobristHash == board.zobristHash)
            ttMove = transpositionTable[idx].bestMove;
        orderMoves(legalRootMoves, board, 0, ttMove);
    }

    int prevScore = 0;

    // ── Iterative Deepening ──────────────────────────────
    for (int currentDepth = 1; currentDepth <= maxDepth; currentDepth++)
    {
        // ── Aspiration Windows ────────────────────────────
        int delta  = 50;
        int alpha  = (currentDepth >= 4) ? prevScore - delta : -999999;
        int beta   = (currentDepth >= 4) ? prevScore + delta :  999999;

        Move  bestMoveThisDepth  = absoluteBestMove;
        int   bestScoreThisDepth = -999999;

        while (true)
        {
            bestScoreThisDepth = -999999;
            bestMoveThisDepth  = absoluteBestMove;
            int localAlpha     = alpha;

            for (int i = 0; i < legalRootMoves.count; i++)
            {
                Move move = legalRootMoves.moves[i];
                board.makeMove(move);

                int score = -negamax(board, currentDepth - 1, -beta, -localAlpha, 1);
                board.undoMove();

                checkTimeBudget();
                if (searchAborted) goto done;

                if (score > bestScoreThisDepth)
                {
                    bestScoreThisDepth = score;
                    bestMoveThisDepth  = move;
                }

                if (score > localAlpha)
                    localAlpha = score;

                if (localAlpha >= beta) break;
            }

            if (searchAborted) goto done;

            // Check aspiration window result
            if (bestScoreThisDepth <= alpha)
            {
                alpha -= delta;
                delta *= 2;
                if (alpha < -999999) alpha = -999999;
            }
            else if (bestScoreThisDepth >= beta)
            {
                beta  += delta;
                delta *= 2;
                if (beta > 999999) beta = 999999;
            }
            else
            {
                break;  // Score is within window — done
            }

            // If window is fully open, don't loop again
            if (alpha <= -999999 && beta >= 999999) break;
        }

        absoluteBestMove = bestMoveThisDepth;
        prevScore        = bestScoreThisDepth;

        // Update TT at root
        {
            int idx = board.zobristHash % TT_SIZE;
            transpositionTable[idx].zobristHash = board.zobristHash;
            transpositionTable[idx].score       = bestScoreThisDepth;
            transpositionTable[idx].depth       = currentDepth;
            transpositionTable[idx].flag        = EXACT;
            transpositionTable[idx].bestMove    = bestMoveThisDepth;
        }

        // Re-order root moves: best move first
        orderMoves(legalRootMoves, board, 0, bestMoveThisDepth);

        auto now     = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - searchStartTime).count();
        if (elapsed == 0) elapsed = 1;

        std::cout << "info depth " << currentDepth
                  << " score cp "  << bestScoreThisDepth
                  << " time "      << elapsed
                  << " nodes "     << nodesSearched
                  << " nps "       << (nodesSearched * 1000 / elapsed)
                  << std::endl;

        if (legalRootMoves.count == 1) break;

        // Time check: if we've used more than 60% of budget, don't start next depth
        auto elapsedNow = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::high_resolution_clock::now() - searchStartTime).count();
        if (elapsedNow >= timeLimitMs * 6 / 10) break;
    }

    done:
    return absoluteBestMove;
}