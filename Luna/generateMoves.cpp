#include "generateMoves.h"
#include <chrono>
#include <iostream>
#include <sstream>
#include <cstring>
#include <cstdint>
#include <climits>
#include <algorithm>
#include <cmath>
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

    memset(historyTable,    0, sizeof(historyTable));
    memset(contHistTable,   0, sizeof(contHistTable));
    memset(countermoveTable,0, sizeof(countermoveTable));
    memset(killerMoves,     0, sizeof(killerMoves));

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
// SEE — STATIC EXCHANGE EVALUATION
// Returns the material gain/loss of a capture on toSq.
// Positive = winning capture, Negative = losing capture.
// ─────────────────────────────────────────────────────────
int GenerateMoves::seeCapture(const Board& board, Move move) const
{
    static const int seeVal[] = { 100, 300, 310, 500, 900, 20000 };

    int from   = move.getFrom();
    int toSq   = move.getTo();
    int target = board.getPieceAt(toSq);   // piece being captured
    int atter  = board.getPieceAt(from);   // piece doing the capturing

    if (target == -1) return 0; // nothing to capture
    if (atter  == -1) return 0;

    // gain[0] = value of piece captured on first capture
    int gain[32];
    int d = 0;
    gain[d] = seeVal[target];

    // Build occupancy and track attackers
    uint64_t occ       = board.occupied;
    uint64_t attackers = 0ULL; // all pieces attacking toSq

    // Collect all attackers of toSq from both sides
    attackers |= (pawnToMasks[0][toSq] & board.whitePawns);
    attackers |= (pawnToMasks[1][toSq] & board.blackPawns);
    attackers |= (knightMasks[toSq] & (board.whiteKnights | board.blackKnights));
    attackers |= (kingMasks[toSq]   & (board.whiteKing    | board.blackKing));
    attackers |= (getBishopAttacks(toSq, occ) & (board.whiteBishops | board.blackBishops |
                                                   board.whiteQueen  | board.blackQueen));
    attackers |= (getRookAttacks(toSq, occ)   & (board.whiteRooks   | board.blackRooks   |
                                                   board.whiteQueen  | board.blackQueen));

    // Side doing the first capture
    int side = board.sideToMove;

    // Remove the first capturing piece from occ
    occ &= ~(1ULL << from);
    attackers &= ~(1ULL << from);

    // Simulate the exchange
    while (true)
    {
        d++;
        gain[d] = seeVal[atter] - gain[d - 1];

        // Prune: if even the best case doesn't improve, stop
        if (std::max(-gain[d - 1], gain[d]) < 0) break;

        side ^= 1;

        // Find least valuable attacker for the current side
        uint64_t sidePieces = (side == 0) ? board.whitePieces : board.blackPieces;
        uint64_t sideAttackers = attackers & sidePieces;
        if (!sideAttackers) break;

        // Pick least valuable piece
        int nextPiece = -1;
        uint64_t nextBit = 0ULL;
        for (int p = PAWN; p <= KING; p++)
        {
            uint64_t bb = 0ULL;
            if (side == 0) {
                if (p == PAWN)   bb = board.whitePawns   & sideAttackers;
                if (p == KNIGHT) bb = board.whiteKnights & sideAttackers;
                if (p == BISHOP) bb = board.whiteBishops & sideAttackers;
                if (p == ROOK)   bb = board.whiteRooks   & sideAttackers;
                if (p == QUEEN)  bb = board.whiteQueen   & sideAttackers;
                if (p == KING)   bb = board.whiteKing    & sideAttackers;
            } else {
                if (p == PAWN)   bb = board.blackPawns   & sideAttackers;
                if (p == KNIGHT) bb = board.blackKnights & sideAttackers;
                if (p == BISHOP) bb = board.blackBishops & sideAttackers;
                if (p == ROOK)   bb = board.blackRooks   & sideAttackers;
                if (p == QUEEN)  bb = board.blackQueen   & sideAttackers;
                if (p == KING)   bb = board.blackKing    & sideAttackers;
            }
            if (bb) { nextPiece = p; nextBit = bb & -bb; break; }
        }
        if (nextPiece == -1) break;

        atter = nextPiece;

        // Remove this attacker, reveal X-ray attackers (sliders behind it)
        occ      &= ~nextBit;
        attackers &= ~nextBit;

        // Re-add any sliders that were behind this piece (X-ray)
        attackers |= (getBishopAttacks(toSq, occ) & (board.whiteBishops | board.blackBishops |
                                                       board.whiteQueen  | board.blackQueen))  & occ;
        attackers |= (getRookAttacks(toSq, occ)   & (board.whiteRooks   | board.blackRooks   |
                                                       board.whiteQueen  | board.blackQueen))  & occ;
    }

    // Minimax back through the gain array
    while (--d)
        gain[d - 1] = std::max(-gain[d], gain[d - 1]);

    return gain[0];
}

// ─────────────────────────────────────────────────────────
// MOVE ORDERING
// ─────────────────────────────────────────────────────────
void GenerateMoves::orderMoves(MoveList& moves, Board& board, int ply, Move ttMove)
{
    // Get previous move info for countermove / continuation history
    int prevPiece = -1, prevTo = -1;
    if (board.undoCount > 0) {
        const UndoInfo& prev = board.undoStack[board.undoCount - 1];
        prevPiece = prev.movedPiece;
        prevTo    = prev.move.getTo();
    }

    int scores[256];

    for (int i = 0; i < moves.count; i++)
    {
        Move m = moves.moves[i];
        int  s = 0;

        // TT move always first
        if (m.data == ttMove.data)
        {
            scores[i] = 10'000'000;
            continue;
        }

        bool isCapture = ((board.occupied >> m.getTo()) & 1ULL) != 0;
        bool isEP      = m.getType() == EN_PASSANT;

        if (isCapture || isEP)
        {
            int seeScore = isEP ? 0 : seeCapture(board, m);
            if (seeScore >= 0)
                s = 1'000'000 + seeScore;
            else
                s = 500'000  + seeScore;
        }
        else if (m.getType() >= PROMOT_QUEEN)
        {
            s = 900'000 + (m.getType() == PROMOT_QUEEN ? 900 : 300);
        }
        else if (ply < 64 && m.data == killerMoves[ply][0].data)
        {
            s = 800'000;
        }
        else if (ply < 64 && m.data == killerMoves[ply][1].data)
        {
            s = 799'000;
        }
        else if (prevPiece >= 0 && prevPiece < 7 && prevTo >= 0 &&
                 countermoveTable[prevPiece][prevTo].data == m.data)
        {
            s = 798'000;  // countermove
        }
        else
        {
            int hIdx  = m.getFrom() * 64 + m.getTo();
            int curPiece = board.getPieceAt(m.getFrom());
            if (curPiece < 0) curPiece = 0;
            s = historyTable[hIdx];
            // Add continuation history bonus
            if (prevPiece >= 0 && prevPiece < 7 && prevTo >= 0 && curPiece < 7)
                s += contHistTable[prevPiece][prevTo][curPiece][m.getTo()];
        }

        scores[i] = s;
    }

    // Insertion sort (fast enough for ≤256 moves)
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
    int scores[256];

    for (int i = 0; i < list.count; i++)
    {
        Move m = list.moves[i];
        if (m.getType() == EN_PASSANT) { scores[i] = 0; continue; }
        scores[i] = seeCapture(board, m);
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
        historyTable[i] >>= 2;

    // Decay continuation history — it's large so use a pointer sweep
    int* p = &contHistTable[0][0][0][0];
    int  n = sizeof(contHistTable) / sizeof(int);
    for (int i = 0; i < n; ++i)
        p[i] >>= 2;
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
// MOBILITY EVALUATION
// Counts reachable squares for each piece, rewards having more options.
// Returns score from white's perspective (positive = white better).
// ─────────────────────────────────────────────────────────
int GenerateMoves::evalMobility(const Board& board) const
{
    // Mobility bonus per extra square (MG, EG)
    // Tuned per piece: knights and bishops benefit most from mobility
    static const int knightMobMG = 4,  knightMobEG = 4;
    static const int bishopMobMG = 3,  bishopMobEG = 5;
    static const int rookMobMG   = 2,  rookMobEG   = 3;
    static const int queenMobMG  = 1,  queenMobEG  = 2;

    // Baseline mobility counts (a piece with N squares scores N - baseline)
    // so a trapped piece scores negative
    static const int knightBase = 4;
    static const int bishopBase = 6;
    static const int rookBase   = 7;
    static const int queenBase  = 14;

    int mg = 0, eg = 0;

    // Phase for tapering (reuse same formula as board.evaluate)
    int phase =
        __builtin_popcountll(board.whiteKnights | board.blackKnights) * 1 +
        __builtin_popcountll(board.whiteBishops | board.blackBishops) * 1 +
        __builtin_popcountll(board.whiteRooks   | board.blackRooks)   * 2 +
        __builtin_popcountll(board.whiteQueen   | board.blackQueen)   * 4;
    if (phase > 24) phase = 24;
    int mgPhase = phase;
    int egPhase = 24 - phase;

    uint64_t occ = board.occupied;

    // ── White mobility ────────────────────────────────────
    {
        uint64_t bb = board.whiteKnights;
        while (bb) {
            int sq  = __builtin_ctzll(bb);
            int mob = __builtin_popcountll(knightMasks[sq] & ~board.whitePieces);
            mg += (mob - knightBase) * knightMobMG;
            eg += (mob - knightBase) * knightMobEG;
            bb &= bb - 1;
        }
        bb = board.whiteBishops;
        while (bb) {
            int sq  = __builtin_ctzll(bb);
            int mob = __builtin_popcountll(getBishopAttacks(sq, occ) & ~board.whitePieces);
            mg += (mob - bishopBase) * bishopMobMG;
            eg += (mob - bishopBase) * bishopMobEG;
            bb &= bb - 1;
        }
        bb = board.whiteRooks;
        while (bb) {
            int sq  = __builtin_ctzll(bb);
            int mob = __builtin_popcountll(getRookAttacks(sq, occ) & ~board.whitePieces);
            mg += (mob - rookBase) * rookMobMG;
            eg += (mob - rookBase) * rookMobEG;
            bb &= bb - 1;
        }
        bb = board.whiteQueen;
        while (bb) {
            int sq  = __builtin_ctzll(bb);
            int mob = __builtin_popcountll(getQueenAttacks(sq, occ) & ~board.whitePieces);
            mg += (mob - queenBase) * queenMobMG;
            eg += (mob - queenBase) * queenMobEG;
            bb &= bb - 1;
        }
    }

    // ── Black mobility ────────────────────────────────────
    {
        uint64_t bb = board.blackKnights;
        while (bb) {
            int sq  = __builtin_ctzll(bb);
            int mob = __builtin_popcountll(knightMasks[sq] & ~board.blackPieces);
            mg -= (mob - knightBase) * knightMobMG;
            eg -= (mob - knightBase) * knightMobEG;
            bb &= bb - 1;
        }
        bb = board.blackBishops;
        while (bb) {
            int sq  = __builtin_ctzll(bb);
            int mob = __builtin_popcountll(getBishopAttacks(sq, occ) & ~board.blackPieces);
            mg -= (mob - bishopBase) * bishopMobMG;
            eg -= (mob - bishopBase) * bishopMobEG;
            bb &= bb - 1;
        }
        bb = board.blackRooks;
        while (bb) {
            int sq  = __builtin_ctzll(bb);
            int mob = __builtin_popcountll(getRookAttacks(sq, occ) & ~board.blackPieces);
            mg -= (mob - rookBase) * rookMobMG;
            eg -= (mob - rookBase) * rookMobEG;
            bb &= bb - 1;
        }
        bb = board.blackQueen;
        while (bb) {
            int sq  = __builtin_ctzll(bb);
            int mob = __builtin_popcountll(getQueenAttacks(sq, occ) & ~board.blackPieces);
            mg -= (mob - queenBase) * queenMobMG;
            eg -= (mob - queenBase) * queenMobEG;
            bb &= bb - 1;
        }
    }

    return (mg * mgPhase + eg * egPhase) / 24;
}

// ─────────────────────────────────────────────────────────
// KING SAFETY EVALUATION
// Counts enemy pieces attacking squares near each king.
// Weighted by piece type and attacker count.
// Returns score from white's perspective.
// ─────────────────────────────────────────────────────────
int GenerateMoves::evalKingSafety(const Board& board) const
{
    // Only meaningful in middlegame — fade out as pieces come off
    int phase =
        __builtin_popcountll(board.whiteKnights | board.blackKnights) * 1 +
        __builtin_popcountll(board.whiteBishops | board.blackBishops) * 1 +
        __builtin_popcountll(board.whiteRooks   | board.blackRooks)   * 2 +
        __builtin_popcountll(board.whiteQueen   | board.blackQueen)   * 4;
    if (phase > 24) phase = 24;

    // Attack weights per piece type
    static const int attackWeight[] = { 0, 2, 2, 3, 5, 0 }; // pawn,knight,bishop,rook,queen,king

    // Safety table: maps attacker_count (0-7) to penalty
    // Exponential — one attacker is annoying, three is deadly
    static const int safetyTable[] = { 0, 5, 15, 35, 65, 100, 140, 180 };

    uint64_t occ = board.occupied;

    auto kingZone = [&](int kingSq) -> uint64_t {
        // King zone = king square + all squares king can move to + one rank further
        uint64_t zone = kingMasks[kingSq] | (1ULL << kingSq);
        return zone;
    };

    int score = 0;

    // ── White king safety (black attacks near white king) ──
    {
        int   wk      = __builtin_ctzll(board.whiteKing);
        uint64_t zone = kingZone(wk);
        int   attackCount  = 0;
        int   attackWeight_sum = 0;

        // Black knights
        uint64_t bb = board.blackKnights;
        while (bb) {
            int sq = __builtin_ctzll(bb);
            if (knightMasks[sq] & zone) {
                attackCount++;
                attackWeight_sum += attackWeight[KNIGHT];
            }
            bb &= bb - 1;
        }
        // Black bishops
        bb = board.blackBishops;
        while (bb) {
            int sq = __builtin_ctzll(bb);
            uint64_t attacks = getBishopAttacks(sq, occ);
            if (attacks & zone) {
                attackCount++;
                attackWeight_sum += attackWeight[BISHOP] *
                    __builtin_popcountll(attacks & zone);
            }
            bb &= bb - 1;
        }
        // Black rooks
        bb = board.blackRooks;
        while (bb) {
            int sq = __builtin_ctzll(bb);
            uint64_t attacks = getRookAttacks(sq, occ);
            if (attacks & zone) {
                attackCount++;
                attackWeight_sum += attackWeight[ROOK] *
                    __builtin_popcountll(attacks & zone);
            }
            bb &= bb - 1;
        }
        // Black queen
        bb = board.blackQueen;
        while (bb) {
            int sq = __builtin_ctzll(bb);
            uint64_t attacks = getQueenAttacks(sq, occ);
            if (attacks & zone) {
                attackCount++;
                attackWeight_sum += attackWeight[QUEEN] *
                    __builtin_popcountll(attacks & zone);
            }
            bb &= bb - 1;
        }

        int idx = std::min(attackCount, 7);
        score -= safetyTable[idx] + attackWeight_sum * 2;
    }

    // ── Black king safety (white attacks near black king) ──
    {
        int   bk      = __builtin_ctzll(board.blackKing);
        uint64_t zone = kingZone(bk);
        int   attackCount  = 0;
        int   attackWeight_sum = 0;

        uint64_t bb = board.whiteKnights;
        while (bb) {
            int sq = __builtin_ctzll(bb);
            if (knightMasks[sq] & zone) {
                attackCount++;
                attackWeight_sum += attackWeight[KNIGHT];
            }
            bb &= bb - 1;
        }
        bb = board.whiteBishops;
        while (bb) {
            int sq = __builtin_ctzll(bb);
            uint64_t attacks = getBishopAttacks(sq, occ);
            if (attacks & zone) {
                attackCount++;
                attackWeight_sum += attackWeight[BISHOP] *
                    __builtin_popcountll(attacks & zone);
            }
            bb &= bb - 1;
        }
        bb = board.whiteRooks;
        while (bb) {
            int sq = __builtin_ctzll(bb);
            uint64_t attacks = getRookAttacks(sq, occ);
            if (attacks & zone) {
                attackCount++;
                attackWeight_sum += attackWeight[ROOK] *
                    __builtin_popcountll(attacks & zone);
            }
            bb &= bb - 1;
        }
        bb = board.whiteQueen;
        while (bb) {
            int sq = __builtin_ctzll(bb);
            uint64_t attacks = getQueenAttacks(sq, occ);
            if (attacks & zone) {
                attackCount++;
                attackWeight_sum += attackWeight[QUEEN] *
                    __builtin_popcountll(attacks & zone);
            }
            bb &= bb - 1;
        }

        int idx = std::min(attackCount, 7);
        score += safetyTable[idx] + attackWeight_sum * 2;
    }

    // Taper: king safety only matters in middlegame
    return (score * phase) / 24;
}

// ─────────────────────────────────────────────────────────
// FULL EVALUATION WRAPPER
// Combines material+PST (board.evaluate) + mobility + king safety.
// All search code calls this instead of board.evaluate() directly.
// ─────────────────────────────────────────────────────────
int GenerateMoves::evalFull(Board& board) const
{
    int base     = board.evaluate();     // material + PST + pawn structure
    int mobility = evalMobility(board);  // piece mobility
    int safety   = evalKingSafety(board);// king attack scoring

    // ── Mop-up: drive enemy king to corner when winning big in endgame ────
    // Both mobility and safety are positive when good for white (white's POV).
    // We combine them with +, then flip for black to move — same convention.
    int mopup = 0;
    {
        int phase =
            __builtin_popcountll(board.whiteKnights | board.blackKnights) * 1 +
            __builtin_popcountll(board.whiteBishops | board.blackBishops) * 1 +
            __builtin_popcountll(board.whiteRooks   | board.blackRooks)   * 2 +
            __builtin_popcountll(board.whiteQueen   | board.blackQueen)   * 4;

        // Only apply in endgame-ish positions (phase <= 12)
        if (phase <= 12) {
            int wMat = __builtin_popcountll(board.whitePawns)   * 100
                     + __builtin_popcountll(board.whiteKnights) * 320
                     + __builtin_popcountll(board.whiteBishops) * 330
                     + __builtin_popcountll(board.whiteRooks)   * 500
                     + __builtin_popcountll(board.whiteQueen)   * 900;
            int bMat = __builtin_popcountll(board.blackPawns)   * 100
                     + __builtin_popcountll(board.blackKnights) * 320
                     + __builtin_popcountll(board.blackBishops) * 330
                     + __builtin_popcountll(board.blackRooks)   * 500
                     + __builtin_popcountll(board.blackQueen)   * 900;
            int diff = wMat - bMat;

            if (std::abs(diff) >= 200) {
                int wk  = __builtin_ctzll(board.whiteKing);
                int bk  = __builtin_ctzll(board.blackKing);
                int wkr = wk >> 3, wkf = wk & 7;
                int bkr = bk >> 3, bkf = bk & 7;

                // Chebyshev distance between kings (winning king should be close)
                int kingDist = std::max(std::abs(wkr - bkr), std::abs(wkf - bkf));

                // Chebyshev distance of losing king from board centre.
                // Corners return 7, true centre returns 1.
                auto centerDist = [](int sq) -> int {
                    int r = sq >> 3, f = sq & 7;
                    return std::max(std::abs(2*r - 7), std::abs(2*f - 7));
                };

                if (diff > 0) {
                    // White winning: push black king to corner, bring kings close
                    mopup += centerDist(bk) * 10 + (7 - kingDist) * 5;
                } else {
                    // Black winning: push white king to corner
                    mopup -= centerDist(wk) * 10 + (7 - kingDist) * 5;
                }
            }
        }
    }

    // FIX: was `mobility - safety` — both are from white's POV (positive=good for
    // white), so they must be added, not subtracted.  The minus sign was causing
    // the engine to (a) ignore attacking the enemy king and (b) reward having its
    // own king under fire.
    int bonus = mobility + safety + mopup;
    if (board.sideToMove == 1) bonus = -bonus;

    return base + bonus;
}

// ─────────────────────────────────────────────────────────
// OPENING BOOK
// ─────────────────────────────────────────────────────────
static std::string toUCI(Move m) {
    std::string s;
    s += "abcdefgh"[m.getFrom()%8];
    s += "12345678"[m.getFrom()/8];
    s += "abcdefgh"[m.getTo()%8];
    s += "12345678"[m.getTo()/8];
    if (m.getType()==PROMOT_QUEEN)  s+="q";
    if (m.getType()==PROMOT_ROOK)   s+="r";
    if (m.getType()==PROMOT_BISHOP) s+="b";
    if (m.getType()==PROMOT_KNIGHT) s+="n";
    return s;
}

void GenerateMoves::buildBook(Board& board) {
    // Each line is a sequence of UCI moves from startpos.
    // We record every prefix → next_move pair so Luna plays book moves
    // as both white and black.
    static const char* lines[] = {
        // ── Ruy Lopez ────────────────────────────────────────────────
        "e2e4 e7e5 g1f3 b8c6 f1b5 a7a6 b5a4 g8f6 e1g1 f8e7 f1e1 b7b5 a4b3 d7d6 c2c3 e8g8",
        "e2e4 e7e5 g1f3 b8c6 f1b5 g8f6 e1g1 f6e4 d2d4 f8e7 d1e2 d7d5",
        "e2e4 e7e5 g1f3 b8c6 f1b5 a7a6 b5c6 d7c6 d2d4 e5d4 d1d4 d8d4 f3d4",
        // ── Italian ──────────────────────────────────────────────────
        "e2e4 e7e5 g1f3 b8c6 f1c4 f8c5 c2c3 g8f6 d2d4 e5d4 c3d4 c5b4 c1d2 b4d2 b1d2",
        "e2e4 e7e5 g1f3 b8c6 f1c4 f8c5 e1g1 g8f6 d2d3 d7d6 c2c3 a7a6 a2a4",
        "e2e4 e7e5 g1f3 b8c6 f1c4 g8f6 d2d3 f8c5 c2c3 d7d6 e1g1 a7a5",
        // ── Scotch ───────────────────────────────────────────────────
        "e2e4 e7e5 g1f3 b8c6 d2d4 e5d4 f3d4 f8c5 c1e3 d8f6 c2c3 g8e7",
        "e2e4 e7e5 g1f3 b8c6 d2d4 e5d4 f3d4 g8f6 d4c6 b7c6 e4e5 d8e7 d1e2 f6d5 c2c4",
        // ── Sicilian Najdorf ─────────────────────────────────────────
        "e2e4 c7c5 g1f3 d7d6 d2d4 c5d4 f3d4 g8f6 b1c3 a7a6 c1g5 e7e6 f2f4",
        "e2e4 c7c5 g1f3 d7d6 d2d4 c5d4 f3d4 g8f6 b1c3 a7a6 f1e2 e7e5 d4b3 f8e7 e1g1",
        // ── Sicilian Dragon ──────────────────────────────────────────
        "e2e4 c7c5 g1f3 d7d6 d2d4 c5d4 f3d4 g8f6 b1c3 g7g6 c1e3 f8g7 f2f3 b8c6 d1d2 e8g8 e1c1",
        // ── Sicilian Scheveningen ────────────────────────────────────
        "e2e4 c7c5 g1f3 e7e6 d2d4 c5d4 f3d4 g8f6 b1c3 d7d6 f1e2 a7a6 e1g1 d8c7 f2f4",
        // ── French ───────────────────────────────────────────────────
        "e2e4 e7e6 d2d4 d7d5 b1c3 g8f6 c1g5 f8e7 e4e5 f6d7 g5e7 d8e7 f2f4 a7a6 g1f3 c7c5",
        "e2e4 e7e6 d2d4 d7d5 b1d2 g8f6 e4e5 f6d7 f1d3 c7c5 c2c3 b8c6 g1e2 c5d4 c3d4",
        "e2e4 e7e6 d2d4 d7d5 e4d5 e6d5 g1f3 g8f6 f1d3 f8d6 e1g1 e8g8 c2c3 b8c6 b1d2",
        // ── Caro-Kann ────────────────────────────────────────────────
        "e2e4 c7c6 d2d4 d7d5 b1c3 d5e4 c3e4 c8f5 e4g3 f5g6 h2h4 h7h6 g1f3 b8d7 h4h5 g6h7 f1d3",
        "e2e4 c7c6 d2d4 d7d5 e4d5 c6d5 c2c4 g8f6 b1c3 e7e6 g1f3 f8e7 c4d5 e6d5",
        // ── Pirc ─────────────────────────────────────────────────────
        "e2e4 d7d6 d2d4 g8f6 b1c3 g7g6 f2f4 f8g7 g1f3 e8g8 f1d3 b8c6 e1g1",
        // ── Queen's Gambit Declined ───────────────────────────────────
        "d2d4 d7d5 c2c4 e7e6 b1c3 g8f6 c1g5 f8e7 e2e3 e8g8 g1f3 b8d7 f1d3 d5c4 d3c4 c7c5",
        "d2d4 d7d5 c2c4 e7e6 g1f3 g8f6 b1c3 f8e7 c1f4 e8g8 e2e3 c7c5 d4c5 b8c6 a2a3",
        // ── Slav ─────────────────────────────────────────────────────
        "d2d4 d7d5 c2c4 c7c6 g1f3 g8f6 b1c3 d5c4 a2a4 c8f5 e2e3 e7e6 f1c4",
        "d2d4 d7d5 c2c4 c7c6 b1c3 g8f6 g1f3 e7e6 e2e3 a7a6 b2b3 b8d7 f1d3 d5c4 d3c4",
        // ── QGA ──────────────────────────────────────────────────────
        "d2d4 d7d5 c2c4 d5c4 g1f3 g8f6 e2e3 e7e6 f1c4 c7c5 e1g1 a7a6 d1e2 b8c6 d4c5",
        // ── King's Indian Defence ────────────────────────────────────
        "d2d4 g8f6 c2c4 g7g6 b1c3 f8g7 e2e4 d7d6 g1f3 e8g8 f1e2 e7e5 e1g1 b8c6 d4d5 c6e7 f3e1",
        "d2d4 g8f6 c2c4 g7g6 b1c3 f8g7 e2e4 d7d6 g1f3 e8g8 f1e2 e7e5 d4d5 a7a5 f3d2 b8a6 e1g1",
        // ── Nimzo-Indian ─────────────────────────────────────────────
        "d2d4 g8f6 c2c4 e7e6 b1c3 f8b4 e2e3 b7b6 g1e2 c8a6 a2a3 b4c3 e2c3 d7d5 b2b3",
        "d2d4 g8f6 c2c4 e7e6 b1c3 f8b4 d1c2 e8g8 a2a3 b4c3 c2c3 b7b6 c1g5",
        // ── Queen's Indian ────────────────────────────────────────────
        "d2d4 g8f6 c2c4 e7e6 g1f3 b7b6 g2g3 c8b7 f1g2 f8e7 e1g1 e8g8 b1c3 f6e4 d1c2 e4c3 c2c3",
        // ── Grunfeld ─────────────────────────────────────────────────
        "d2d4 g8f6 c2c4 g7g6 b1c3 d7d5 c4d5 f6d5 e2e4 d5c3 b2c3 f8g7 f1c4 c7c5 g1f3 e8g8",
        "d2d4 g8f6 c2c4 g7g6 b1c3 d7d5 g1f3 f8g7 d1b3 d5c4 b3c4 e8g8 e2e4 c8g4 c1e3",
        // ── Catalan ───────────────────────────────────────────────────
        "d2d4 g8f6 c2c4 e7e6 g1f3 d7d5 g2g3 f8e7 f1g2 e8g8 e1g1 d5c4 d1c2 a7a6 c2c4",
        // ── English ───────────────────────────────────────────────────
        "c2c4 e7e5 b1c3 g8f6 g1f3 b8c6 g2g3 f8b4 f1g2 e8g8 e1g1 b4c3 b2c3 d7d6 d2d3",
        "c2c4 g8f6 b1c3 e7e6 g1f3 d7d5 d2d4 f8e7 c1f4 e8g8 e2e3 c7c5 d4c5 f8e8",
        // ── London ───────────────────────────────────────────────────
        "d2d4 d7d5 g1f3 g8f6 c1f4 e7e6 e2e3 f8d6 f4d6 d8d6 b1d2 e8g8 f1d3 b8d7 e1g1 c7c5",
        "d2d4 g8f6 g1f3 e7e6 c1f4 d7d5 e2e3 f8d6 f4d6 d8d6 b1d2 e8g8 f1d3 c7c5 c2c3",
        // ── Replies as black ──────────────────────────────────────────
        "e2e4 e7e5",       "e2e4 c7c5",       "e2e4 e7e6",
        "e2e4 c7c6",       "e2e4 d7d6",       "e2e4 g8f6",
        "d2d4 d7d5",       "d2d4 g8f6",       "d2d4 e7e6",
        "c2c4 e7e5",       "c2c4 g8f6",       "c2c4 c7c5",
        "g1f3 d7d5",       "g1f3 g8f6",       "g1f3 c7c5",
        nullptr
    };

    board.initZobrist();
    book.clear();

    for (int l = 0; lines[l] != nullptr; l++) {
        // Parse moves
        std::vector<std::string> moves;
        std::string mv;
        std::istringstream iss(lines[l]);
        while (iss >> mv) moves.push_back(mv);
        if (moves.empty()) continue;

        // Replay prefix 0..i, record position hash → moves[i+1]
        board.init();

        // ── Record startpos → moves[0] ──────────────────────
        // The loop below only records "after move i → move i+1".
        // We must also record the very first move from the start position.
        {
            uint64_t startHash = board.zobristHash;
            MoveList legalStart = generateLegalMoves(board, board.sideToMove);
            for (int j = 0; j < legalStart.count; j++) {
                if (toUCI(legalStart.moves[j]) == moves[0]) {
                    auto& vec = book[startHash];
                    bool dup = false;
                    for (auto& e : vec) if (e.moveData == legalStart.moves[j].data) { dup = true; break; }
                    if (!dup) vec.push_back({legalStart.moves[j].data, 200});
                    break;
                }
            }
        }

        for (int i = 0; i < (int)moves.size() - 1; i++) {
            // Play move i
            MoveList legal = generateLegalMoves(board, board.sideToMove);
            bool found = false;
            for (int j = 0; j < legal.count; j++) {
                if (toUCI(legal.moves[j]) == moves[i]) {
                    board.makeMove(legal.moves[j]);
                    found = true;
                    break;
                }
            }
            if (!found) break;

            // Record: in this position, play moves[i+1]
            uint64_t hash = board.zobristHash;
            MoveList legal2 = generateLegalMoves(board, board.sideToMove);
            for (int j = 0; j < legal2.count; j++) {
                if (toUCI(legal2.moves[j]) == moves[i+1]) {
                    auto& vec = book[hash];
                    bool dup = false;
                    for (auto& e : vec) if (e.moveData == legal2.moves[j].data) { dup = true; break; }
                    if (!dup) vec.push_back({legal2.moves[j].data, 200});
                    break;
                }
            }
        }
    }

    board.init();
    bookLoaded = true;
    std::cout << "info string Opening book loaded: " << book.size() << " positions\n";
}

Move GenerateMoves::probeBook(const Board& board) const {
    if (!bookLoaded) return Move(0,0,NORMAL);
    auto it = book.find(board.zobristHash);
    if (it == book.end() || it->second.empty()) return Move(0,0,NORMAL);

    // Pick highest-weight move; ties broken by whichever appears first (insertion order = higher frequency)
    const auto& vec = it->second;
    int best = 0;
    for (int i = 1; i < (int)vec.size(); i++)
        if (vec[i].weight > vec[best].weight) best = i;

    Move m; m.data = vec[best].moveData;
    return m;
}


int GenerateMoves::quiescence(Board& board, int alpha, int beta, int ply)
{
    nodesSearched++;

    if ((nodesSearched & 255) == 0)
        checkTimeBudget();

    if (searchAborted) return 0;
    if (board.isInsufficientMaterial()) return 0;
    if (board.halfMoveClock >= 100) return 0;
    if (board.isThreefoldRepetition()) return -10;

    int standPat = evalFull(board);
    if (standPat >= beta) return beta;
    if (standPat > alpha) alpha = standPat;

    // Delta pruning: skip if even a queen capture can't raise alpha
    if (standPat + 975 < alpha) return alpha;

    MoveList captures;
    generateCaptures(board, board.sideToMove, captures);
    orderCaptures(captures, board);

    int side     = board.sideToMove;
    int opponent = side ^ 1;

    for (int i = 0; i < captures.count; i++)
    {
        Move move = captures.moves[i];
        bool isPromotion = (move.getType() >= PROMOT_QUEEN);

        
        if (!isPromotion && move.getType() != EN_PASSANT && seeCapture(board, move) < 0)
            continue;

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

    if ((nodesSearched & 255) == 0)
        checkTimeBudget();

    if (searchAborted) return 0;

    // ── Draw detection ────────────────────────────────────
    if (board.isInsufficientMaterial()) return 0;
    if (board.halfMoveClock >= 100) return 0;
    if (ply > 0 && board.isThreefoldRepetition()) return -10;

    int originalAlpha = alpha;
    bool isPV  = (beta - alpha) > 1;
    bool isRoot = (ply == 0);

    // ── TT Probe ──────────────────────────────────────────
    int ttIndex = (int)(board.zobristHash % TT_SIZE);
    TTEntry& ttEntry = transpositionTable[ttIndex];
    Move ttMove(0, 0, NORMAL);

    if (ttEntry.zobristHash == board.zobristHash)
    {
        ttMove = ttEntry.bestMove;
        if (!isRoot && ttEntry.depth >= depth)
        {
            int s = ttEntry.score;
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
    if (inCheck) depth++;

    // ── Static eval ───────────────────────────────────────
    int staticEval = evalFull(board);

    // ── Reverse Futility Pruning ──────────────────────────
    if (!inCheck && !isPV && depth >= 1 && depth <= 8)
    {
        if (staticEval - 80 * depth >= beta)
            return staticEval - 80 * depth;
    }

    // ── Null Move Pruning ──────────────────────────────────
    if (!inCheck && !isPV && depth >= 3 && ply > 0 && staticEval >= beta)
    {
        int bigPieces = __builtin_popcountll(
            (board.sideToMove == 0)
            ? (board.whiteKnights | board.whiteBishops | board.whiteRooks | board.whiteQueen)
            : (board.blackKnights | board.blackBishops | board.blackRooks | board.blackQueen));

        if (bigPieces >= 1)
        {
            int R = 3 + depth / 4 + std::min(3, (staticEval - beta) / 150);
            board.makeNullMove();
            int nullScore = -negamax(board, depth - 1 - R, -beta, -beta + 1, ply + 1);
            board.undoNullMove();
            if (!searchAborted && nullScore >= beta) return beta;
        }
    }

    // ── IID: Internal Iterative Deepening ─────────────────
    if (isPV && depth >= 6 && ttMove.data == 0)
    {
        negamax(board, depth - 4, alpha, beta, ply);
        if (searchAborted) return 0;
        if (transpositionTable[ttIndex].zobristHash == board.zobristHash)
            ttMove = transpositionTable[ttIndex].bestMove;
    }

    // ── Move generation ───────────────────────────────────
    MoveList moves;
    generateAllMoves(board, board.sideToMove, moves);

    {
        bool ttValid = false;
        for (int i = 0; i < moves.count; i++)
            if (moves.moves[i].data == ttMove.data) { ttValid = true; break; }
        if (!ttValid) ttMove = Move(0, 0, NORMAL);
    }

    orderMoves(moves, board, ply, ttMove);

    int  bestScore  = -9999999;
    Move bestMove(0, 0, NORMAL);
    int  legalCount = 0;
    int  quietCount = 0;
    int  side       = board.sideToMove;
    int  opponent   = side ^ 1;

    int prevPiece = -1, prevTo = -1;
    if (board.undoCount > 0) {
        const UndoInfo& prev = board.undoStack[board.undoCount - 1];
        prevPiece = prev.movedPiece; if (prevPiece < 0) prevPiece = 0;
        prevTo    = prev.move.getTo();
    }

    for (int i = 0; i < moves.count; i++)
    {
        Move move = moves.moves[i];

        bool isCapture   = ((board.occupied >> move.getTo()) & 1ULL) != 0
                        || move.getType() == EN_PASSANT;
        bool isPromotion = move.getType() >= PROMOT_QUEEN;
        bool isQuiet     = !isCapture && !isPromotion;

        // ── Late Move Pruning (LMP) ────────────────────────
        if (!inCheck && isQuiet && !isPV && depth <= 5 && legalCount > 0)
        {
            static const int lmpCount[] = {0, 8, 12, 18, 26, 36};
            if (quietCount >= lmpCount[depth]) continue;
        }

        board.makeMove(move);

        int kingSq = (side == 0)
            ? __builtin_ctzll(board.whiteKing)
            : __builtin_ctzll(board.blackKing);

        if (isSquareAttacked(kingSq, opponent, board))
        {
            board.undoMove();
            continue;
        }

        legalCount++;
        if (isQuiet) quietCount++;

        int movedPiece = board.undoStack[board.undoCount-1].movedPiece;
        if (movedPiece < 0) movedPiece = 0;

        // ── Futility Pruning ──────────────────────────────
        if (!inCheck && isQuiet && legalCount > 1 && depth <= 5 && !isPV)
        {
            static const int futilityMargin[] = { 0, 100, 200, 300, 450, 600 };
            if (staticEval + futilityMargin[depth] <= alpha)
            {
                board.undoMove();
                int hIdx = move.getFrom() * 64 + move.getTo();
                historyTable[hIdx] -= depth;
                if (historyTable[hIdx] < -1'000'000) historyTable[hIdx] = -1'000'000;
                continue;
            }
        }

        int score;

        // ── Late Move Reductions (LMR) ─────────────────────
        if (!inCheck && depth >= 3 && legalCount > 2 && isQuiet)
        {
            int R = (int)(0.75 + log((double)depth) * log((double)legalCount) / 2.5);
            if (!isPV) R++;
            R = std::max(1, std::min(R, depth - 2));

            score = -negamax(board, depth - 1 - R, -alpha - 1, -alpha, ply + 1);
            if (score > alpha)
                score = -negamax(board, depth - 1, -beta, -alpha, ply + 1);
        }
        else if (legalCount > 1 && isPV)
        {
            score = -negamax(board, depth - 1, -alpha - 1, -alpha, ply + 1);
            if (score > alpha && score < beta)
                score = -negamax(board, depth - 1, -beta, -alpha, ply + 1);
        }
        else
        {
            score = -negamax(board, depth - 1, -beta, -alpha, ply + 1);
        }

        board.undoMove();
        if (searchAborted) return 0;

        if (score > bestScore) { bestScore = score; bestMove = move; }

        if (score > alpha)
        {
            alpha = score;
            if (isQuiet)
            {
                int bonus = depth * depth;
                int hIdx  = move.getFrom() * 64 + move.getTo();
                historyTable[hIdx] += bonus;
                if (historyTable[hIdx] > 1'000'000) historyTable[hIdx] = 1'000'000;
                if (prevPiece >= 0 && prevPiece < 7 && movedPiece < 7)
                    contHistTable[prevPiece][prevTo][movedPiece][move.getTo()] += bonus;
            }
        }

        if (alpha >= beta)
        {
            if (isQuiet)
            {
                if (ply < 64 && killerMoves[ply][0].data != move.data)
                {
                    killerMoves[ply][1] = killerMoves[ply][0];
                    killerMoves[ply][0] = move;
                }
                if (prevPiece >= 0 && prevPiece < 7 && movedPiece < 7)
                    countermoveTable[prevPiece][prevTo] = move;

                // History malus for quiets searched before the cutoff
                int bonus = depth * depth;
                for (int j = 0; j < i; j++) {
                    Move prev2 = moves.moves[j];
                    // Only apply malus to quiet moves (no captures, no promotions)
                    // We can safely check the type; captures have getType()==EN_PASSANT
                    // or target square was occupied — use the move type flags only
                    bool prevIsQuiet = (prev2.getType() != EN_PASSANT)
                                    && (prev2.getType() < PROMOT_QUEEN);
                    // Also skip if it was a capture (target had a piece before we made it)
                    // Since board is restored, occupied reflects the original position
                    bool prevWasCapture = (board.occupied >> prev2.getTo()) & 1ULL;
                    if (prevIsQuiet && !prevWasCapture) {
                        int hIdx2 = prev2.getFrom() * 64 + prev2.getTo();
                        historyTable[hIdx2] -= bonus;
                        if (historyTable[hIdx2] < -1'000'000) historyTable[hIdx2] = -1'000'000;
                    }
                }
            }
            break;
        }
    }

    if (legalCount == 0)
        return inCheck ? (-99000 + ply) : 0;

    // ── TT Store ──────────────────────────────────────────
    if (!searchAborted)
    {
        TTFlag flag = EXACT;
        if (bestScore <= originalAlpha) flag = ALPHA;
        else if (bestScore >= beta)     flag = BETA;

        if (ttEntry.zobristHash != board.zobristHash ||
            ttEntry.depth <= depth ||
            ttEntry.age != searchAge)
        {
            ttEntry.zobristHash = board.zobristHash;
            ttEntry.score       = bestScore;
            ttEntry.depth       = depth;
            ttEntry.flag        = flag;
            ttEntry.bestMove    = bestMove;
            ttEntry.age         = searchAge;
        }
    }

    return bestScore;
}

// ─────────────────────────────────────────────────────────
// ITERATIVE DEEPENING ROOT
// ─────────────────────────────────────────────────────────
Move GenerateMoves::getBestMove(Board& board, int maxDepth,
                                 long long myTimeLeftMs,
                                 long long incrementMs, int movesToGo,
                                 long long movetimeMs)
{
    // ── Opening Book Probe ────────────────────────────────
    if (bookLoaded && gamePly < 30) {
        Move bookMove = probeBook(board);
        if (bookMove.data != 0) {
            // Validate it's legal before playing
            MoveList legal = generateLegalMoves(board, board.sideToMove);
            for (int i = 0; i < legal.count; i++) {
                if (legal.moves[i].data == bookMove.data) {
                    std::cout << "info string Book move: " << toUCI(bookMove) << "\n";
                    return bookMove;
                }
            }
        }
    }

    // ── Time Management ───────────────────────────────────
    if (movetimeMs > 0) {
        // Fixed time per move ("go movetime X") — use with small safety buffer
        timeLimitMs = movetimeMs - 20;
        if (timeLimitMs < 5) timeLimitMs = 5;
    } else {
        // Tournament / sudden-death clock.
        if (myTimeLeftMs <= 0) myTimeLeftMs = 1000;

        // Critical: keep a safety buffer so we never flag
        long long safeTime = myTimeLeftMs - 100;  // always keep 100ms in reserve
        if (safeTime < 50) safeTime = 50;

        long long baseTime;
        if (movesToGo > 0) {
            baseTime = safeTime / (movesToGo + 2);
        } else {
            int estimatedMovesLeft = std::max(10, 45 - gamePly / 2);
            baseTime = safeTime / (estimatedMovesLeft + 2);
        }

        // Add 80% of increment
        baseTime += (long long)(incrementMs * 0.8);

        // Phase multiplier
        double phaseMultiplier = 1.0;
        if      (gamePly < 10)  phaseMultiplier = 0.7;
        else if (gamePly < 20)  phaseMultiplier = 1.15;
        else if (gamePly < 40)  phaseMultiplier = 1.05;
        else                    phaseMultiplier = 0.9;

        timeLimitMs = (long long)(baseTime * phaseMultiplier);

        // Hard cap: never burn more than 15% of remaining safe time on one move
        long long hardCap = (long long)(safeTime * 0.15);
        if (timeLimitMs > hardCap)  timeLimitMs = hardCap;

        // Absolute ceiling
        if (timeLimitMs > 8000) timeLimitMs = 8000;

        // Graduated emergency floor — scales with available time
        long long minTime = std::min((long long)50, safeTime / 10);
        if (timeLimitMs < minTime) timeLimitMs = minTime;

        // Emergency throttle tiers
        if (myTimeLeftMs < 3000) timeLimitMs = std::min(timeLimitMs, myTimeLeftMs / 6);
        if (myTimeLeftMs < 1000) timeLimitMs = std::min(timeLimitMs, (long long)100);
        if (myTimeLeftMs < 300)  timeLimitMs = 30;
        if (myTimeLeftMs < 100)  timeLimitMs = 10;
    }

    searchStartTime = std::chrono::high_resolution_clock::now();
    searchAborted   = false;
    nodesSearched   = 0;
    searchAge++;  // age TT entries so replacement prefers fresh data

    memset(killerMoves, 0, sizeof(killerMoves));
    ageHistory();

    MoveList legalRootMoves = generateLegalMoves(board, board.sideToMove);
    if (legalRootMoves.count == 0) return Move(0, 0, NORMAL);

    // Instant move: only one legal move, no need to search
    if (legalRootMoves.count == 1) {
        std::cout << "info depth 1 score cp 0 time 1 nodes 1 nps 1000\n";
        return legalRootMoves.moves[0];
    }

    Move absoluteBestMove = legalRootMoves.moves[0];

    // Seed ordering from TT
    {
        Move ttMove(0, 0, NORMAL);
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
        int delta = 25;
        int alpha = (currentDepth >= 4) ? prevScore - delta : -999999;
        int beta  = (currentDepth >= 4) ? prevScore + delta :  999999;

        Move bestMoveThisDepth  = absoluteBestMove;
        int  bestScoreThisDepth = -999999;
        int  aspirationFails    = 0;

        while (true)
        {
            checkTimeBudget();
            if (searchAborted) goto done;

            bestScoreThisDepth = -999999;
            bestMoveThisDepth  = absoluteBestMove;

            // Re-order so the current best candidate is searched first.
            orderMoves(legalRootMoves, board, 0, bestMoveThisDepth);

            for (int i = 0; i < legalRootMoves.count; i++)
            {
                Move move = legalRootMoves.moves[i];
                board.makeMove(move);

                int score;
                if (i == 0) {
                    score = -negamax(board, currentDepth - 1, -beta, -alpha, 1);
                } else {
                    score = -negamax(board, currentDepth - 1, -alpha - 1, -alpha, 1);
                    if (!searchAborted && score > alpha && score < beta)
                        score = -negamax(board, currentDepth - 1, -beta, -alpha, 1);
                }
                board.undoMove();

                if (searchAborted) goto done;

                if (score > bestScoreThisDepth)
                {
                    bestScoreThisDepth = score;
                    bestMoveThisDepth  = move;
                }
                if (score > alpha) alpha = score;
            }

            if (searchAborted) goto done;

            // Widen window on fail — with a hard cap on retries
            if (bestScoreThisDepth <= prevScore - delta)
            {
                alpha = bestScoreThisDepth - delta;
                delta *= 2;
                aspirationFails++;
                if (aspirationFails >= 4 || alpha <= -999999) { alpha = -999999; beta = 999999; }
            }
            else if (bestScoreThisDepth >= prevScore + delta)
            {
                beta  = bestScoreThisDepth + delta;
                delta *= 2;
                aspirationFails++;
                if (aspirationFails >= 4 || beta >= 999999) { alpha = -999999; beta = 999999; }
            }
            else break;  // score inside window — done

            if (alpha <= -999999 && beta >= 999999) break;  // full window, no point retrying
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

        orderMoves(legalRootMoves, board, 0, bestMoveThisDepth);

        auto now     = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - searchStartTime).count();
        if (elapsed == 0) elapsed = 1;

        // Report mate scores properly
        std::string scoreStr;
        if (bestScoreThisDepth > 90000)
            scoreStr = "mate " + std::to_string((99000 - bestScoreThisDepth + 1) / 2);
        else if (bestScoreThisDepth < -90000)
            scoreStr = "mate -" + std::to_string((99000 + bestScoreThisDepth + 1) / 2);
        else
            scoreStr = "cp " + std::to_string(bestScoreThisDepth);

        std::cout << "info depth "  << currentDepth
                  << " score "      << scoreStr
                  << " time "       << elapsed
                  << " nodes "      << nodesSearched
                  << " nps "        << (nodesSearched * 1000 / elapsed)
                  << std::endl;

        auto elapsedNow = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::high_resolution_clock::now() - searchStartTime).count();

        // Hard abort if we're already over 90% of budget (shouldn't happen but safety net)
        if (elapsedNow >= timeLimitMs * 90 / 100) break;

        // Soft stop: don't start a new depth if we've used 50% of budget
        // (the next depth will take ~2-3x as long as the current one)
        if (elapsedNow >= timeLimitMs * 50 / 100) break;

        // If score is a mate, no need to search deeper
        if (bestScoreThisDepth > 90000 || bestScoreThisDepth < -90000) break;
    }

done:
    return absoluteBestMove;
}