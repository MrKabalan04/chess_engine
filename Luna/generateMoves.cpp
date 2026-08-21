#include "generateMoves.h"
#include <chrono>
#include <iostream>
#include <sstream>
#include <cstring>
#include <cstdint>
#include <climits>
#include <algorithm>
#include <cmath>
#include <functional>
#include <random>
#include "types.h"
using namespace std;

uint64_t GenerateMoves::rookTable[64][4096];
uint64_t GenerateMoves::bishopTable[64][512];
TTEntry GenerateMoves::transpositionTable[TT_SIZE];

int8_t GenerateMoves::egKQ[2][64][64][64];
int8_t GenerateMoves::egKR[2][64][64][64];
bool   GenerateMoves::egKQbuilt = false;
bool   GenerateMoves::egKRbuilt = false;
int8_t GenerateMoves::egKP[2][64][64][64];
bool   GenerateMoves::egKPbuilt = false;

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

// ???????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????
// ATTACK DETECTION
// ???????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????
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

// ???????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????
// LEGAL MOVE GENERATION
// ???????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????
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

// ???????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????
// ALL PSEUDO-LEGAL MOVES
// ???????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????
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

// ???????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????
// SEE ??? STATIC EXCHANGE EVALUATION
// Returns the material gain/loss of a capture on toSq.
// Positive = winning capture, Negative = losing capture.
// ???????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????
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

// ???????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????
// MOVE ORDERING
// ???????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????
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

    // Insertion sort (fast enough for ???256 moves)
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

    // Decay continuation history ??? it's large so use a pointer sweep
    int* p = &contHistTable[0][0][0][0];
    int  n = sizeof(contHistTable) / sizeof(int);
    for (int i = 0; i < n; ++i)
        p[i] >>= 2;
}

// ???????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????
// CAPTURE GENERATION
// ???????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????
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

// ???????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????
// MOBILITY EVALUATION
// Counts reachable squares for each piece, rewards having more options.
// Returns score from white's perspective (positive = white better).
// ???????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????
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

    // ?????? White mobility ????????????????????????????????????????????????????????????????????????????????????????????????????????????
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

    // ?????? Black mobility ????????????????????????????????????????????????????????????????????????????????????????????????????????????
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

// ???????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????
// KING SAFETY EVALUATION
// Counts enemy pieces attacking squares near each king.
// Weighted by piece type and attacker count.
// Returns score from white's perspective.
// ???????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????
int GenerateMoves::evalKingSafety(const Board& board) const
{
    // Only meaningful in middlegame ??? fade out as pieces come off
    int phase =
        __builtin_popcountll(board.whiteKnights | board.blackKnights) * 1 +
        __builtin_popcountll(board.whiteBishops | board.blackBishops) * 1 +
        __builtin_popcountll(board.whiteRooks   | board.blackRooks)   * 2 +
        __builtin_popcountll(board.whiteQueen   | board.blackQueen)   * 4;
    if (phase > 24) phase = 24;

    // Attack weights per piece type
    static const int attackWeight[] = { 0, 2, 2, 3, 5, 0 }; // pawn,knight,bishop,rook,queen,king

    // Safety table: maps attacker_count (0-7) to penalty
    // Exponential ??? one attacker is annoying, three is deadly
    static const int safetyTable[] = { 0, 5, 15, 35, 65, 100, 140, 180 };

    uint64_t occ = board.occupied;

    auto kingZone = [&](int kingSq) -> uint64_t {
        // King zone = king square + all squares king can move to + one rank further
        uint64_t zone = kingMasks[kingSq] | (1ULL << kingSq);
        return zone;
    };

    int score = 0;

    // ?????? White king safety (black attacks near white king) ??????
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

    // ?????? Black king safety (white attacks near black king) ??????
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

// ???????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????
// FULL EVALUATION WRAPPER
// Combines material+PST (board.evaluate) + mobility + king safety.
// All search code calls this instead of board.evaluate() directly.
// ???????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????
int GenerateMoves::evalFull(Board& board) const
{
    int base     = board.evaluate();     // material + PST + pawn structure
    int mobility = evalMobility(board);  // piece mobility
    int safety   = evalKingSafety(board);// king attack scoring

    // ?????? Mop-up: drive enemy king to corner when winning big in endgame ????????????
    // Both mobility and safety are positive when good for white (white's POV).
    // We combine them with +, then flip for black to move ??? same convention.
    //
    // The search alone can't convert bare mating endgames (QR/KR/KBB vs K)
    // at web-time depth: a real mate line needs >25 quiet plies but at
    // 1.2s it only reaches depth ~14-18. So evaluation must STEER the win:
    // reward pushing the enemy king to the edge/corner and bringing the
    // winning king close. Once the king is pinned to the edge the mating
    // window drops to ~6-10 plies and the shallow search finds the mate.
    int mopup = 0;
    {
        int phase =
            __builtin_popcountll(board.whiteKnights | board.blackKnights) * 1 +
            __builtin_popcountll(board.whiteBishops | board.blackBishops) * 1 +
            __builtin_popcountll(board.whiteRooks   | board.blackRooks)   * 2 +
            __builtin_popcountll(board.whiteQueen   | board.blackQueen)   * 4;

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

            if (std::abs(diff) >= 300) {
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

                // Tiny-piece phase (phase <= 6: at most a queen or rook's worth of
                // material on the board): the winning king MUST run across
                // the board and join the attack, and can do so safely, so
                // allow very strong steering. In richer endings keep it
                // gentle because marching the king out is risky there.
                bool tiny = phase <= 6;

                int steer;
                if (tiny) {
                    int loserSq = (diff > 0) ? bk : wk;
                    int cdLoser = centerDist(loserSq);

                    // Stable gradient #1: constrict the enemy king. Count
                    // its legal king targets that our pieces defend ??? fewer
                    // escape squares is permanent progress the fleeing king
                    // cannot claw back between moves.
                    int kinMobile = 0;
                    {
                        uint64_t km = kingMasks[loserSq] & ~board.occupied;
                        // Squares attacked by the winning side's own forces.
                        uint64_t att = 0;
                        uint64_t p;
                        if (diff > 0) {
                            att |= kingMasks[wk];
                            p = board.whiteKnights; while (p) { int s = __builtin_ctzll(p); p &= p - 1; att |= knightMasks[s]; }
                            p = board.whitePawns;   while (p) { int s = __builtin_ctzll(p); p &= p - 1; att |= pawnFromMasks[0][s]; }
                            p = board.whiteRooks;   while (p) { int s = __builtin_ctzll(p); p &= p - 1; att |= getRookAttacks(s, board.occupied); }
                            p = board.whiteBishops; while (p) { int s = __builtin_ctzll(p); p &= p - 1; att |= getBishopAttacks(s, board.occupied); }
                            p = board.whiteQueen;   while (p) { int s = __builtin_ctzll(p); p &= p - 1; att |= getQueenAttacks(s, board.occupied); }
                        } else {
                            att |= kingMasks[bk];
                            p = board.blackKnights; while (p) { int s = __builtin_ctzll(p); p &= p - 1; att |= knightMasks[s]; }
                            p = board.blackPawns;   while (p) { int s = __builtin_ctzll(p); p &= p - 1; att |= pawnFromMasks[1][s]; }
                            p = board.blackRooks;   while (p) { int s = __builtin_ctzll(p); p &= p - 1; att |= getRookAttacks(s, board.occupied); }
                            p = board.blackBishops; while (p) { int s = __builtin_ctzll(p); p &= p - 1; att |= getBishopAttacks(s, board.occupied); }
                            p = board.blackQueen;   while (p) { int s = __builtin_ctzll(p); p &= p - 1; att |= getQueenAttacks(s, board.occupied); }
                        }
                        while (km) {
                            int s = __builtin_ctzll(km); km &= km - 1;
                            if (!(att >> s & 1ULL)) kinMobile++;
                        }
                    }

                    // Stable gradient #2: bring the winning major piece (or
                    // minors, when that's all we have) next to the king.
                    int pieceProx = 0;
                    {
                        uint64_t pieces = (diff > 0)
                            ? (board.whiteQueen | board.whiteRooks | board.whiteBishops | board.whiteKnights)
                            : (board.blackQueen | board.blackRooks | board.blackBishops | board.blackKnights);
                        int kR = loserSq >> 3, kF = loserSq & 7;
                        while (pieces) {
                            int s = __builtin_ctzll(pieces); pieces &= pieces - 1;
                            int d = std::max(std::abs((s >> 3) - kR), std::abs((s & 7) - kF));
                            if (d <= 2) pieceProx += (3 - d) * 10;
                        }
                    }

                    // Refuse the centre hard; the winning king also gets an
                    // immediate reward for settling next to the losing king.
                    int adjacency = kingDist <= 1 ? 220
                                 : kingDist <= 2 ? 120
                                 : kingDist <= 3 ? 50
                                 : 0;
                    int pull = std::max(0, 8 - kingDist) * 16;

                    steer = cdLoser * 48
                          + (cdLoser >= 6 ? 220 : 0)
                          + adjacency + pull
                          + (16 - kinMobile) * 14
                          + pieceProx;
                } else {
                    int cdLoser = centerDist((diff > 0) ? bk : wk);
                    steer = cdLoser * 26
                          + (cdLoser >= 6 ? 90 : 0)
                          + std::max(0, 5 - kingDist) * 5;
                }

                mopup += (diff > 0) ? steer : -steer;
            }
        }
    }

    int bonus = mobility + safety + mopup;

    // ?????? Quiet endgame bonus (equal material) ????????????????????????????????????????????????
    // The mop-up above only fires when one side is winning big, so in
    // even endgames the engine had no reason to activate its king and used
    // to shuffle aimlessly. Reward king centralisation in all low-piece
    // positions and give rooks a bonus when they guard a passed pawn
    // from behind (the classic winning endgame motif).
    {
        int phase2 =
            __builtin_popcountll(board.whiteKnights | board.blackKnights) * 1 +
            __builtin_popcountll(board.whiteBishops | board.blackBishops) * 1 +
            __builtin_popcountll(board.whiteRooks   | board.blackRooks)   * 2 +
            __builtin_popcountll(board.whiteQueen   | board.blackQueen)   * 4;

        if (phase2 <= 8)
        {
            auto cdist = [](int sq) -> int {
                int r = sq >> 3, f = sq & 7;
                return std::max(std::abs(2*r - 7), std::abs(2*f - 7));
            };
            int wk2 = __builtin_ctzll(board.whiteKing);
            int bk2 = __builtin_ctzll(board.blackKing);
            int act = cdist(bk2) - cdist(wk2);   // white central more = better
            bonus += act * 2;
        }

        // Rooks behind own passed pawn (white POV positive)
        auto fileMaskL = [](int f) -> uint64_t {
            return 0x0101010101010101ULL << f;
        };
        auto adjL = [&](int f) -> uint64_t {
            uint64_t m = fileMaskL(f);
            if (f > 0) m |= fileMaskL(f-1);
            if (f < 7) m |= fileMaskL(f+1);
            return m;
        };
        int rookSupport = 0;

        uint64_t wp = board.whitePawns;
        while (wp) {
            int sq = __builtin_ctzll(wp); wp &= wp - 1;
            int f = sq & 7, r = sq >> 3, passed = 1;
            for (int rr = r + 1; rr <= 7; rr++)
                if (board.blackPawns & (adjL(f) & (0xFFULL << (rr*8)))) { passed = 0; break; }
            if (!passed) continue;
            uint64_t rr = board.whiteRooks & fileMaskL(f);
            while (rr) {
                int rs = __builtin_ctzll(rr); rr &= rr - 1;
                if ((rs >> 3) < r) { rookSupport++; break; }
            }
        }

        uint64_t bp = board.blackPawns;
        while (bp) {
            int sq = __builtin_ctzll(bp); bp &= bp - 1;
            int f = sq & 7, r = sq >> 3, passed = 1;
            for (int rr = r - 1; rr >= 0; rr--)
                if (board.whitePawns & (adjL(f) & (0xFFULL << (rr*8)))) { passed = 0; break; }
            if (!passed) continue;
            uint64_t rr = board.blackRooks & fileMaskL(f);
            while (rr) {
                int rs = __builtin_ctzll(rr); rr &= rr - 1;
                if ((rs >> 3) > r) { rookSupport--; break; }
            }
        }

        bonus += rookSupport * 30;
    }

    // ?????? Positional terms (white POV positive) ?????????????????????????????????????????????
    // Classic, well-established eval features the engine was missing:
    //   ??? rooks on the 7th rank (and doubled fighting pair)
    //   ??? rooks on open / semi-open files
    //   ??? king pawn-shield (own pawns in front of a castled king)
    //   ??? protected knight outposts in enemy territory
    {
        const uint64_t notAFile = 0xFEFEFEFEFEFEFEFEULL;
        const uint64_t notHFile = 0x7F7F7F7F7F7F7F7FULL;

        int ph =
            __builtin_popcountll(board.whiteKnights | board.blackKnights) +
            __builtin_popcountll(board.whiteBishops | board.blackBishops) +
            __builtin_popcountll(board.whiteRooks   | board.blackRooks)   * 2 +
            __builtin_popcountll(board.whiteQueen   | board.blackQueen)   * 4;
        if (ph > 24) ph = 24;

        int posRook = 0;   // lifetime term (works in MG and EG)
        int posMG   = 0;   // middlegame-only term (shield + outposts)

        auto fileMaskF = [](int f) -> uint64_t {
            return 0x0101010101010101ULL << f;
        };

        auto rookTerm = [&](uint64_t rooks, uint64_t ownPawn, uint64_t enemyPawn, int sgn) {
            uint64_t rr = rooks; int on7 = 0;
            while (rr) {
                int rsq = __builtin_ctzll(rr); rr &= rr - 1;
                uint64_t fm = fileMaskF(rsq & 7);
                bool enemyNone = (enemyPawn & fm) == 0;
                bool ownNone   = (ownPawn   & fm) == 0;
                if (enemyNone && ownNone)       posRook += sgn * 16;   // open file
                else if (enemyNone)             posRook += sgn * 8;    // semi-open
                if ((sgn > 0 && (rsq >> 3) == 6) || (sgn < 0 && (rsq >> 3) == 1)) on7++;
            }
            posRook += sgn * (on7 >= 2 ? 60 : (on7 == 1 ? 30 : 0));
        };
        rookTerm(board.whiteRooks, board.whitePawns, board.blackPawns, +1);
        rookTerm(board.blackRooks, board.blackPawns, board.whitePawns, -1);

        auto shield = [&](int ksq, uint64_t pawns, int sgn) {
            int kr = ksq >> 3, kf = ksq & 7, cnt = 0;
            for (int i = 1; i <= 2; i++) {
                int rank = kr + sgn * i;
                if (rank < 0 || rank > 7) continue;
                uint64_t row = 0xFFULL << (rank * 8);
                uint64_t files = 0;
                for (int j = kf - 1; j <= kf + 1; j++)
                    if (j >= 0 && j < 8) files |= fileMaskF(j);
                cnt += __builtin_popcountll(pawns & row & files);
            }
            if (cnt > 5) cnt = 5;
            posMG += sgn * cnt * 9;
        };
        shield(__builtin_ctzll(board.whiteKing), board.whitePawns, +1);
        shield(__builtin_ctzll(board.blackKing), board.blackPawns, -1);

        auto outpost = [&](int ksq, uint64_t whiteKnights, uint64_t blackKnights,
                           uint64_t wPawnAtt, uint64_t bPawnAtt) {
            uint64_t kn = whiteKnights;
            while (kn) {
                int s = __builtin_ctzll(kn); kn &= kn - 1;
                if ((s >> 3) >= 4 && ((wPawnAtt >> s) & 1ULL) && !((bPawnAtt >> s) & 1ULL))
                    posMG += 20;
            }
            kn = blackKnights;
            while (kn) {
                int s = __builtin_ctzll(kn); kn &= kn - 1;
                if ((s >> 3) <= 3 && ((bPawnAtt >> s) & 1ULL) && !((wPawnAtt >> s) & 1ULL))
                    posMG -= 20;
            }
        };
        uint64_t wPawnAtt = ((board.whitePawns & notAFile) << 7) |
                            ((board.whitePawns & notHFile) << 9);
        uint64_t bPawnAtt = ((board.blackPawns & notAFile) >> 7) |
                            ((board.blackPawns & notHFile) >> 9);
        outpost(__builtin_ctzll(board.whiteKing), board.whiteKnights, board.blackKnights,
                wPawnAtt, bPawnAtt);

        bonus += posRook + posMG * ph / 24;
    }

    if (board.sideToMove == 1) bonus = -bonus;

    int total = base + bonus;

    // 50-move pressure: as the draw approaches, shrink the eval toward 0 so a
    // winning side feels urgency to progress instead of shuffling.
    if (board.halfMoveClock > 80)
        total = total * (100 - board.halfMoveClock) / 20;

    return total;
}

// ???????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????
// OPENING BOOK
// ???????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????
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
    // We record every prefix ??? next_move pair so Luna plays book moves
    // as both white and black.
    static const char* lines[] = {
        // ?????? Ruy Lopez ????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????
        "e2e4 e7e5 g1f3 b8c6 f1b5 a7a6 b5a4 g8f6 e1g1 f8e7 f1e1 b7b5 a4b3 d7d6 c2c3 e8g8",
        "e2e4 e7e5 g1f3 b8c6 f1b5 g8f6 e1g1 f6e4 d2d4 f8e7 d1e2 d7d5",
        "e2e4 e7e5 g1f3 b8c6 f1b5 a7a6 b5c6 d7c6 d2d4 e5d4 d1d4 d8d4 f3d4",
        // ?????? Italian ??????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????
        "e2e4 e7e5 g1f3 b8c6 f1c4 f8c5 c2c3 g8f6 d2d4 e5d4 c3d4 c5b4 c1d2 b4d2 b1d2",
        "e2e4 e7e5 g1f3 b8c6 f1c4 f8c5 e1g1 g8f6 d2d3 d7d6 c2c3 a7a6 a2a4",
        "e2e4 e7e5 g1f3 b8c6 f1c4 g8f6 d2d3 f8c5 c2c3 d7d6 e1g1 a7a5",
        // ?????? Scotch ?????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????
        "e2e4 e7e5 g1f3 b8c6 d2d4 e5d4 f3d4 f8c5 c1e3 d8f6 c2c3 g8e7",
        "e2e4 e7e5 g1f3 b8c6 d2d4 e5d4 f3d4 g8f6 d4c6 b7c6 e4e5 d8e7 d1e2 f6d5 c2c4",
        // ?????? Sicilian Najdorf ???????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????
        "e2e4 c7c5 g1f3 d7d6 d2d4 c5d4 f3d4 g8f6 b1c3 a7a6 c1g5 e7e6 f2f4",
        "e2e4 c7c5 g1f3 d7d6 d2d4 c5d4 f3d4 g8f6 b1c3 a7a6 f1e2 e7e5 d4b3 f8e7 e1g1",
        // ?????? Sicilian Dragon ??????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????
        "e2e4 c7c5 g1f3 d7d6 d2d4 c5d4 f3d4 g8f6 b1c3 g7g6 c1e3 f8g7 f2f3 b8c6 d1d2 e8g8 e1c1",
        // ?????? Sicilian Scheveningen ????????????????????????????????????????????????????????????????????????????????????????????????????????????
        "e2e4 c7c5 g1f3 e7e6 d2d4 c5d4 f3d4 g8f6 b1c3 d7d6 f1e2 a7a6 e1g1 d8c7 f2f4",
        // ?????? French ?????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????
        "e2e4 e7e6 d2d4 d7d5 b1c3 g8f6 c1g5 f8e7 e4e5 f6d7 g5e7 d8e7 f2f4 a7a6 g1f3 c7c5",
        "e2e4 e7e6 d2d4 d7d5 b1d2 g8f6 e4e5 f6d7 f1d3 c7c5 c2c3 b8c6 g1e2 c5d4 c3d4",
        "e2e4 e7e6 d2d4 d7d5 e4d5 e6d5 g1f3 g8f6 f1d3 f8d6 e1g1 e8g8 c2c3 b8c6 b1d2",
        // ?????? Caro-Kann ????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????
        "e2e4 c7c6 d2d4 d7d5 b1c3 d5e4 c3e4 c8f5 e4g3 f5g6 h2h4 h7h6 g1f3 b8d7 h4h5 g6h7 f1d3",
        "e2e4 c7c6 d2d4 d7d5 e4d5 c6d5 c2c4 g8f6 b1c3 e7e6 g1f3 f8e7 c4d5 e6d5",
        // ?????? Pirc ???????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????
        "e2e4 d7d6 d2d4 g8f6 b1c3 g7g6 f2f4 f8g7 g1f3 e8g8 f1d3 b8c6 e1g1",
        // ?????? Queen's Gambit Declined ?????????????????????????????????????????????????????????????????????????????????????????????????????????
        "d2d4 d7d5 c2c4 e7e6 b1c3 g8f6 c1g5 f8e7 e2e3 e8g8 g1f3 b8d7 f1d3 d5c4 d3c4 c7c5",
        "d2d4 d7d5 c2c4 e7e6 g1f3 g8f6 b1c3 f8e7 c1f4 e8g8 e2e3 c7c5 d4c5 b8c6 a2a3",
        // ?????? Slav ???????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????
        "d2d4 d7d5 c2c4 c7c6 g1f3 g8f6 b1c3 d5c4 a2a4 c8f5 e2e3 e7e6 f1c4",
        "d2d4 d7d5 c2c4 c7c6 b1c3 g8f6 g1f3 e7e6 e2e3 a7a6 b2b3 b8d7 f1d3 d5c4 d3c4",
        // ?????? QGA ??????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????
        "d2d4 d7d5 c2c4 d5c4 g1f3 g8f6 e2e3 e7e6 f1c4 c7c5 e1g1 a7a6 d1e2 b8c6 d4c5",
        // ?????? King's Indian Defence ????????????????????????????????????????????????????????????????????????????????????????????????????????????
        "d2d4 g8f6 c2c4 g7g6 b1c3 f8g7 e2e4 d7d6 g1f3 e8g8 f1e2 e7e5 e1g1 b8c6 d4d5 c6e7 f3e1",
        "d2d4 g8f6 c2c4 g7g6 b1c3 f8g7 e2e4 d7d6 g1f3 e8g8 f1e2 e7e5 d4d5 a7a5 f3d2 b8a6 e1g1",
        // ?????? Nimzo-Indian ???????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????
        "d2d4 g8f6 c2c4 e7e6 b1c3 f8b4 e2e3 b7b6 g1e2 c8a6 a2a3 b4c3 e2c3 d7d5 b2b3",
        "d2d4 g8f6 c2c4 e7e6 b1c3 f8b4 d1c2 e8g8 a2a3 b4c3 c2c3 b7b6 c1g5",
        // ?????? Queen's Indian ????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????
        "d2d4 g8f6 c2c4 e7e6 g1f3 b7b6 g2g3 c8b7 f1g2 f8e7 e1g1 e8g8 b1c3 f6e4 d1c2 e4c3 c2c3",
        // ?????? Grunfeld ???????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????
        "d2d4 g8f6 c2c4 g7g6 b1c3 d7d5 c4d5 f6d5 e2e4 d5c3 b2c3 f8g7 f1c4 c7c5 g1f3 e8g8",
        "d2d4 g8f6 c2c4 g7g6 b1c3 d7d5 g1f3 f8g7 d1b3 d5c4 b3c4 e8g8 e2e4 c8g4 c1e3",
        // ?????? Catalan ?????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????
        "d2d4 g8f6 c2c4 e7e6 g1f3 d7d5 g2g3 f8e7 f1g2 e8g8 e1g1 d5c4 d1c2 a7a6 c2c4",
        // ?????? English ?????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????
        "c2c4 e7e5 b1c3 g8f6 g1f3 b8c6 g2g3 f8b4 f1g2 e8g8 e1g1 b4c3 b2c3 d7d6 d2d3",
        "c2c4 g8f6 b1c3 e7e6 g1f3 d7d5 d2d4 f8e7 c1f4 e8g8 e2e3 c7c5 d4c5 f8e8",
        // ?????? London ?????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????
        "d2d4 d7d5 g1f3 g8f6 c1f4 e7e6 e2e3 f8d6 f4d6 d8d6 b1d2 e8g8 f1d3 b8d7 e1g1 c7c5",
        "d2d4 g8f6 g1f3 e7e6 c1f4 d7d5 e2e3 f8d6 f4d6 d8d6 b1d2 e8g8 f1d3 c7c5 c2c3",
        // ?????? Replies as black ??????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????
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

        // Replay prefix 0..i, record position hash ??? moves[i+1]
        board.init();

        // ?????? Record startpos ??? moves[0] ??????????????????????????????????????????????????????????????????
        // The loop below only records "after move i ??? move i+1".
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

    // Pick the book reply. All entries share the same weight, so instead of
    // always choosing the first one (which made the engine play 1.e4/1...e5
    // literally every game) we shuffle among the max-weight moves for variety.
    static std::mt19937 rng(static_cast<unsigned>(std::chrono::high_resolution_clock::now()
                              .time_since_epoch().count()));
    const auto& vec = it->second;
    int bestW = 0;
    for (auto& e : vec) if (e.weight > bestW) bestW = e.weight;
    if (bestW < 1) bestW = 1;

    std::vector<uint16_t> options;
    for (auto& e : vec) if (e.weight == bestW) options.push_back(e.moveData);
    if (options.empty()) return Move(0,0,NORMAL);

    Move m; m.data = options[rng() % options.size()];
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
    if (board.isThreefoldRepetition()) return 0;

    // When in check we must search ALL evasions (stand-pat is not valid:
    // the position may be lost or even mated). Otherwise stand-pat + captures.
    bool qInCheck = isInCheck(board, board.sideToMove);
    int standPat = evalFull(board);

    MoveList moves;
    if (!qInCheck)
    {
        if (standPat >= beta) return beta;
        if (standPat > alpha) alpha = standPat;

        // Delta pruning: skip if even a queen capture can't raise alpha
        if (standPat + 975 < alpha) return alpha;

        generateCaptures(board, board.sideToMove, moves);
        orderCaptures(moves, board);
    }
    else
    {
        generateAllMoves(board, board.sideToMove, moves);
        orderMoves(moves, board, ply, Move(0, 0, NORMAL));
    }

    int side     = board.sideToMove;
    int opponent = side ^ 1;
    int legalCount = 0;

    for (int i = 0; i < moves.count; i++)
    {
        Move move = moves.moves[i];
        bool isPromotion = (move.getType() >= PROMOT_QUEEN);

        // SEE pruning only applies to captures in non-check nodes
        if (!qInCheck && !isPromotion && move.getType() != EN_PASSANT && seeCapture(board, move) < 0)
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

        legalCount++;
        int score = -quiescence(board, -beta, -alpha, ply + 1);
        board.undoMove();

        if (searchAborted) return 0;

        if (score >= beta) return beta;
        if (score > alpha) alpha = score;
    }

    // In check with no legal evasion = checkmate found at the horizon
    if (qInCheck && legalCount == 0)
        return -99000 + ply;

    return alpha;
}

// ???????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????
// NEGAMAX WITH ALPHA-BETA
// ???????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????
int GenerateMoves::negamax(Board& board, int depth, int alpha, int beta, int ply)
{
    nodesSearched++;

    if ((nodesSearched & 255) == 0)
        checkTimeBudget();

    if (searchAborted) return 0;

    // ?????? Draw detection ????????????????????????????????????????????????????????????????????????????????????????????????????????????
    if (board.isInsufficientMaterial()) return 0;
    if (board.halfMoveClock >= 100) return 0;
    if (ply > 0 && board.isThreefoldRepetition()) return 0;

    int originalAlpha = alpha;
    bool isPV  = (beta - alpha) > 1;
    bool isRoot = (ply == 0);

    // ?????? TT Probe ??????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????
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

    // ?????? Check extension ?????????????????????????????????????????????????????????????????????????????????????????????????????????
    bool inCheck = isInCheck(board, board.sideToMove);
    if (inCheck && ply < 64) depth++;   // capped: unbounded extensions explode in perpetual-check lines

    // ?????? Static eval ?????????????????????????????????????????????????????????????????????????????????????????????????????????????????????
    int staticEval = evalFull(board);

    // Decisive endgames (one side up ??? ~4 pawns, few pieces left): the mating
    // net is long and quiet, so light pruning that would pass in middlegames
    // (RFP, LMR) chops the corridor out of the window and the engine ends up
    // shuffling while "winning". Detect it once here and disable those.
    bool decisiveEndgame = false;
    {
        int egP =
            __builtin_popcountll(board.whiteKnights | board.blackKnights) +
            __builtin_popcountll(board.whiteBishops | board.blackBishops) +
            __builtin_popcountll(board.whiteRooks   | board.blackRooks)   * 2 +
            __builtin_popcountll(board.whiteQueen   | board.blackQueen)   * 4;
        if (egP <= 12) {
            int wM = __builtin_popcountll(board.whitePawns)   * 100
                   + __builtin_popcountll(board.whiteKnights) * 320
                   + __builtin_popcountll(board.whiteBishops) * 330
                   + __builtin_popcountll(board.whiteRooks)   * 500
                   + __builtin_popcountll(board.whiteQueen)   * 900;
            int bM = __builtin_popcountll(board.blackPawns)   * 100
                   + __builtin_popcountll(board.blackKnights) * 320
                   + __builtin_popcountll(board.blackBishops) * 330
                   + __builtin_popcountll(board.blackRooks)   * 500
                   + __builtin_popcountll(board.blackQueen)   * 900;
            decisiveEndgame = std::abs(wM - bM) >= 400;
        }
    }

    // ?????? Reverse Futility Pruning ??????????????????????????????????????????????????????????????????????????????
    if (!inCheck && !isPV && depth >= 1 && depth <= 8 && !decisiveEndgame)
    {
        // 60*depth+30 is gentler than 80*depth: deep 7-plies pruning would
        // chop quiet-but-real threats out of the window in the endgame and
        // reward pointless shuffling. We now have ~1.2s/move to spend.
        if (staticEval - (60 * depth + 30) >= beta)
            return staticEval - (60 * depth + 30);
    }

    // ?????? Null Move Pruning ??????????????????????????????????????????????????????????????????????????????????????????????????????
    if (!inCheck && !isPV && depth >= 3 && ply > 0 && staticEval >= beta)
    {
        int bigPieces = __builtin_popcountll(
            (board.sideToMove == 0)
            ? (board.whiteKnights | board.whiteBishops | board.whiteRooks | board.whiteQueen)
            : (board.blackKnights | board.blackBishops | board.blackRooks | board.blackQueen));

        if (bigPieces >= 1)
        {
            // Gentler reduction: the old R (up to 9 at depth 12) pruned real
            // deep threats out of the window and caused missed tactics.
            int R = 3 + depth / 6 + std::min(2, (staticEval - beta) / 250);
            board.makeNullMove();
            int nullScore = -negamax(board, depth - 1 - R, -beta, -beta + 1, ply + 1);
            board.undoNullMove();
            if (!searchAborted && nullScore >= beta) return beta;
        }
    }

    // ?????? IID: Internal Iterative Deepening ???????????????????????????????????????????????????
    if (isPV && depth >= 6 && ttMove.data == 0)
    {
        negamax(board, depth - 4, alpha, beta, ply);
        if (searchAborted) return 0;
        if (transpositionTable[ttIndex].zobristHash == board.zobristHash)
            ttMove = transpositionTable[ttIndex].bestMove;
    }

    // ?????? Move generation ?????????????????????????????????????????????????????????????????????????????????????????????????????????
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

        // ?????? Late Move Pruning (LMP) ????????????????????????????????????????????????????????????????????????
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

        // ?????? Futility Pruning ??????????????????????????????????????????????????????????????????????????????????????????
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

        // ?????? Late Move Reductions (LMR) ???????????????????????????????????????????????????????????????
        if (!inCheck && depth >= 3 && legalCount > 2 && isQuiet && !decisiveEndgame)
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
                    // or target square was occupied ??? use the move type flags only
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

    // ?????? TT Store ??????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????
    if (!searchAborted)
    {
        TTFlag flag = EXACT;
        if (bestScore <= originalAlpha) flag = ALPHA;
        else if (bestScore >= beta)     flag = BETA;

        if (ttEntry.zobristHash != board.zobristHash ||
            ttEntry.depth <= depth ||
            ttEntry.age != searchAge)
        {
            // Mate scores are stored relative to the ROOT ply so that probes
            // at any ply can shift them back (probe does score -=/+ ply).
            // Storing raw node scores corrupts mate distances through
            // transpositions and produces phantom "mate N" results.
            int storeScore = bestScore;
            if (storeScore >  90000) storeScore += ply;
            else if (storeScore < -90000) storeScore -= ply;

            ttEntry.zobristHash = board.zobristHash;
            ttEntry.score       = storeScore;
            ttEntry.depth       = depth;
            ttEntry.flag        = flag;
            ttEntry.bestMove    = bestMove;
            ttEntry.age         = searchAge;
        }
    }

    return bestScore;
}

// ???????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????
// BARE-MATE RETROGRADE TABLEBASE (KQ-vs-K / KR-vs-K)
//
// Two fully-enumerated tables (queen, rook) built lazily by reverse
// retrograde BFS.  White is the strong side.  State key:
//     key = (stm<<18) | (wk<<12) | (bk<<6) | mq
// where stm = side to move (0 white, 1 black), wk = white king square,
// bk = black king square, mq = strong major square.  Value stored:
//     0 = draw / unknown / illegal
//     1 = black is checkmated (black to move, no legal move)
//     n>1 = plies until forced mate for white from this position.
// Black-strong positions are handled by the caller via a rank-flip
// (sq ^ 56) plus king-role swap (symmetric about colour).
//
// All kings-move legality is evaluated with bitboards; a "major capture"
// by the black king leaves the state space (K-vs-K draw), so those black
// states can never be losses.  Stalemates (either side) are draws.
// ???????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????
static inline int bareKey(int stm, int wk, int bk, int mq)
{
    return (stm << 18) | (wk << 12) | (bk << 6) | mq;
}

void GenerateMoves::buildBareMateTable(bool isRook)
{
    if (isRook ? egKRbuilt : egKQbuilt) return;
    if (isRook) egKRbuilt = true; else egKQbuilt = true;

    int8_t* T = isRook ? &egKR[0][0][0][0] : &egKQ[0][0][0][0];
    const int N = 1 << 19;

    std::vector<int8_t>  val(N, 0);
    std::vector<int16_t> down(N, 0);
    std::vector<uint8_t> canDraw(N, 0);
    std::vector<uint8_t> maxChild(N, 0);

    // line-clear test: is `target` attacked by the major at `from`
    // given the blocker bitboard `occ` (which must NOT contain `target`
    // or `from` themselves being endpoints... callers ensure blockers).
    auto attacked = [&](int from, int target, uint64_t blockers) -> bool {
        uint64_t occ = blockers | (1ULL << from);
        uint64_t att = isRook ? getRookAttacks(from, occ)
                              : getQueenAttacks(from, occ);
        return (att >> target) & 1ULL;
    };

    // ---- pass 1: down counts, canDraw flags, seed checkmates ----
    // Values are processed in strictly increasing order via buckets so the
    // DTM labels stay consistent (a black loss value is 1+max over its
    // children, a white win value is 1+min). KRK max DTM is ~32 plies.
    const int MAXD = 66;
    std::vector<std::vector<int>> bucket(MAXD + 2);

    for (int key = 0; key < N; key++) {
        int stm = key >> 18;
        int wk  = (key >> 12) & 63;
        int bk  = (key >> 6)  & 63;
        int mq  = key & 63;

        if (wk == bk || mq == wk || mq == bk) continue;
        if (kingMasks[wk] & (1ULL << bk)) continue;   // illegal: kings adjacent

        if (stm == 1) {
            // Black to move: build its legal king moves. Targets in-space
            // are counted for `down`; captures of the (undefended) major
            // exit to a draw so they set canDraw.
            bool inCheck = (kingMasks[wk] & (1ULL << bk))
                        || attacked(mq, bk, 1ULL << wk);
            bool hadCapture = false;
            int  kids = 0;
            uint64_t km = kingMasks[bk];
            while (km) {
                int s = __builtin_ctzll(km); km &= km - 1;
                if (s == wk) continue;                     // can't take the king
                if (s == mq) {
                    bool defended = (kingMasks[wk] & (1ULL << mq)) != 0;
                    if (!defended) hadCapture = true;      // takes major -> draw
                    continue;
                }
                if (kingMasks[wk] & (1ULL << s)) continue; // move adjacent white king
                if (attacked(mq, s, 1ULL << wk)) continue; // move into check from major
                kids++;
            }
            down[key] = kids;
            if (hadCapture) canDraw[key] = 1;
            if (kids == 0 && !hadCapture && inCheck) {
                val[key] = 1;                               // checkmate seed
                bucket[1].push_back(key);
            }
        }
    }

    // ---- retrograde BFS (value-ordered buckets) ----
    for (int d = 1; d <= MAXD; d++) {
        for (size_t bi = 0; bi < bucket[d].size(); bi++) {
            int key = bucket[d][bi];
            int v   = d;
            int stm = key >> 18;
            int wk  = (key >> 12) & 63;
            int bk  = (key >> 6)  & 63;
            int mq  = key & 63;

            if (stm == 1) {
                // Black loss solved: every WHITE predecessor is a win in v+1
                // plies (first discovery in value order is the minimum).
                // (a) white king moved last: predecessor king was adjacent to wk.
                uint64_t km = kingMasks[wk];
                while (km) {
                    int pwk = __builtin_ctzll(km); km &= km - 1;
                    if (pwk == bk || pwk == mq) continue;
                    if (kingMasks[pwk] & (1ULL << bk)) continue; // P invalid: bk in check
                    if (attacked(mq, bk, 1ULL << pwk)) continue; // P invalid: bk in check
                    int pkey = bareKey(0, pwk, bk, mq);
                    if (val[pkey] == 0 && v + 1 <= MAXD) {
                        val[pkey] = (int8_t)(v + 1);
                        bucket[v + 1].push_back(pkey);
                    }
                }
                // (b) major moved last: predecessor major on any clear square
                // along a ray from mq (stop at first of wk/bk).
                for (int dir = 1; dir <= 8; dir++) {
                    if (isRook && dir > 4) continue;     // rook has no diagonals
                    int rd = 0, cd = 0;
                    if      (dir == 1) { rd = -1; cd = 0; }
                    else if (dir == 2) { rd = 1;  cd = 0; }
                    else if (dir == 3) { rd = 0;  cd = -1; }
                    else if (dir == 4) { rd = 0;  cd = 1; }
                    else if (dir == 5) { rd = -1; cd = -1; }
                    else if (dir == 6) { rd = -1; cd = 1; }
                    else if (dir == 7) { rd = 1;  cd = -1; }
                    else               { rd = 1;  cd = 1; }
                    int r = mq >> 3, c = mq & 7;
                    while (true) {
                        r += rd; c += cd;
                        if (r < 0 || r > 7 || c < 0 || c > 7) break;
                        int sq = r * 8 + c;
                        if (sq == wk || sq == bk) break;      // blocked
                        // P = (0,wk,bk,pmq=sq); bk must not be in check there
                        if (!attacked(sq, bk, 1ULL << wk)) {
                            int pkey = bareKey(0, wk, bk, sq);
                            if (val[pkey] == 0 && v + 1 <= MAXD) {
                                val[pkey] = (int8_t)(v + 1);
                                bucket[v + 1].push_back(pkey);
                            }
                        }
                    }
                }
            } else {
                // White win solved: BLACK predecessors lose once every
                // in-space reply is a win (down hits 0 and no draw escape).
                uint64_t km = kingMasks[bk];
                while (km) {
                    int pbk = __builtin_ctzll(km); km &= km - 1;
                    if (pbk == wk || pbk == mq) continue;
                    if (kingMasks[wk] & (1ULL << bk)) continue; // S invalid: never
                    // legality of black move pbk->bk: bk must not be attacked
                    if (attacked(mq, bk, 1ULL << wk)) continue; // moved into check
                    if (kingMasks[pbk] & (1ULL << wk)) continue; // P invalid: wk in check
                    int pkey = bareKey(1, wk, pbk, mq);
                    if (val[pkey] == 0) {
                        down[pkey]--;
                        if ((int)maxChild[pkey] < v) maxChild[pkey] = (uint8_t)v;
                        if (down[pkey] == 0 && !canDraw[pkey]) {
                            int nv = (int)maxChild[pkey] + 1;
                            if (nv > MAXD) nv = MAXD;
                            val[pkey] = (int8_t)nv;
                            bucket[nv].push_back(pkey);
                        }
                    }
                }
            }
        }
    }

    for (int key = 0; key < N; key++)
        if (val[key]) T[key] = val[key];
}

int GenerateMoves::bareMateDTM(bool isRook, int stm, int wk, int bk, int mq) const
{
    if (wk == bk || mq == wk || mq == bk) return 0;
    if (kingMasks[wk] & (1ULL << bk)) return 0;
    const int8_t* T = isRook ? &egKR[0][0][0][0] : &egKQ[0][0][0][0];
    return T[bareKey(stm, wk, bk, mq)];
}

// Full controller: WHITE-strong DTM table queried in the correct
// orientation (black-strong positions use a rank-flip + king-role swap).
Move GenerateMoves::bareMateMove(Board &board)
{
    int winner = board.sideToMove;

    int wR = __builtin_popcountll(board.whiteRooks);
    int wQ = __builtin_popcountll(board.whiteQueen);
    int bR = __builtin_popcountll(board.blackRooks);
    int bQ = __builtin_popcountll(board.blackQueen);

    bool isRook;
    if (winner == 0) {
        if (bQ != 0 || bR != 0) return Move(0, 0, NORMAL);
        if (wQ == 1 && wR == 0)      isRook = false;
        else if (wR == 1 && wQ == 0) isRook = true;
        else return Move(0, 0, NORMAL);
    } else {
        if (wQ != 0 || wR != 0) return Move(0, 0, NORMAL);
        if (bQ == 1 && bR == 0)      isRook = false;
        else if (bR == 1 && bQ == 0) isRook = true;
        else return Move(0, 0, NORMAL);
    }

    int ourKing, theirKing, majorSq;
    if (winner == 0) {
        ourKing   = __builtin_ctzll(board.whiteKing);
        theirKing = __builtin_ctzll(board.blackKing);
        majorSq   = isRook ? __builtin_ctzll(board.whiteRooks)
                           : __builtin_ctzll(board.whiteQueen);
    } else {
        ourKing   = __builtin_ctzll(board.blackKing);
        theirKing = __builtin_ctzll(board.whiteKing);
        majorSq   = isRook ? __builtin_ctzll(board.blackRooks)
                           : __builtin_ctzll(board.blackQueen);
    }

    // white-strong frame
    int wkT, bkT, mqT;
    if (winner == 0) {
        wkT = ourKing; bkT = theirKing; mqT = majorSq;
    } else {
        wkT = ourKing ^ 56; bkT = theirKing ^ 56; mqT = majorSq ^ 56;
    }

    buildBareMateTable(isRook);
    // In the white-strong frame the strong side is always "white to move":
    // stm=0 for the current position, stm=1 for every child (our move hands
    // the turn to the weak side). Independent of which real colour is strong.
    int cur = bareMateDTM(isRook, 0, wkT, bkT, mqT);

    MoveList legal = generateLegalMoves(board, winner);
    if (legal.count == 0) return Move(0, 0, NORMAL);

    auto childVal = [&](int w, int b, int m) -> int {
        // w = strong king, b = weak king, m = major (physical squares);
        // child is weak-side-to-move (stm=1); black-strong needs the flip.
        if (winner == 0) return bareMateDTM(isRook, 1, w, b, m);
        return bareMateDTM(isRook, 1, w ^ 56, b ^ 56, m ^ 56);
    };

    Move best(0, 0, NORMAL);
    int bestVal = 300;

    for (int i = 0; i < legal.count; i++) {
        board.makeMove(legal.moves[i]);
        int wkC, bkC, mqC;
        if (winner == 0) {
            wkC = __builtin_ctzll(board.whiteKing);
            bkC = __builtin_ctzll(board.blackKing);
            mqC = isRook ? __builtin_ctzll(board.whiteRooks)
                         : __builtin_ctzll(board.whiteQueen);
        } else {
            wkC = __builtin_ctzll(board.blackKing);
            bkC = __builtin_ctzll(board.whiteKing);
            mqC = isRook ? __builtin_ctzll(board.blackRooks)
                         : __builtin_ctzll(board.blackQueen);
        }
        board.undoMove();
        int cv = childVal(wkC, bkC, mqC);
        if (cv > 0 && cv < bestVal) {
            bestVal = cv;
            best = legal.moves[i];
        }
    }

    if (cur > 0 && best.data != 0) {
        std::cout << "info string Tablebase mate (" << cur << " plies)\n";
        return best;
    }
    return Move(0, 0, NORMAL);
}

// ??????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????
// KPK TABLEBASE (lone pawn vs bare king): distance to SAFE promotion
//
// Same retrograde bucket BFS as the bare-mate tables. White is the pawn
// side; state key = (stm<<18)|(wk<<12)|(bk<<6)|psq. Terminal: a white
// promotion push whose fresh queen cannot be captured by the bare king
// and which does not stalemate it (underpromotion is never needed in a
// won KPK). A black king capture of an undefended pawn exits to a drawn
// K-vs-K, so those states carry canDraw and can never become losses.
// Stalemates are draws. KPK has no pre-promotion checkmates (a lone pawn
// cannot mate), so there are no mate seeds. Black-strong positions are
// handled by the caller via a rank-flip (sq ^ 56).
// ??????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????
static inline int kpkKey(int stm, int wk, int bk, int ps)
{
    return (stm << 18) | (wk << 12) | (bk << 6) | ps;
}

void GenerateMoves::buildKpkTable()
{
    if (egKPbuilt) return;
    egKPbuilt = true;

    int8_t* T = &egKP[0][0][0][0];
    const int N = 1 << 19;

    std::vector<int8_t>  val(N, 0);
    std::vector<int16_t> down(N, 0);
    std::vector<uint8_t> canDraw(N, 0);
    std::vector<uint8_t> maxChild(N, 0);

    // squares attacked by a white pawn on s
    auto pawnAtt = [](int s) -> uint64_t {
        uint64_t a = 0;
        int f = s & 7;
        if (s + 7 < 64 && f > 0) a |= 1ULL << (s + 7);
        if (s + 9 < 64 && f < 7) a |= 1ULL << (s + 9);
        return a;
    };

    const int MAXD = 120;
    std::vector<std::vector<int>> bucket(MAXD + 2);

    // ---- pass 1: down counts, canDraw flags, seed safe promotions ----
    for (int key = 0; key < N; key++) {
        int stm = key >> 18;
        int wk  = (key >> 12) & 63;
        int bk  = (key >> 6)  & 63;
        int ps  = key & 63;

        if (wk == bk || ps == wk || ps == bk) continue;
        if (kingMasks[wk] & (1ULL << bk)) continue;
        int pr = ps >> 3;
        if (pr == 0 || pr == 7) continue;          // impossible pawn square

        if (stm == 0) {
            // seed: pawn on rank 7 with a SAFE promotion push
            if (pr != 6) continue;
            int q = ps + 8;
            if (q == bk) continue;                        // push blocked
            if (kingMasks[bk] & (1ULL << q)) continue;    // Q capturable
            uint64_t att = getQueenAttacks(q, 1ULL << wk);
            bool inCheck = (att >> bk) & 1ULL;
            bool hasMove = false;
            uint64_t km = kingMasks[bk];
            while (km) {
                int s = __builtin_ctzll(km); km &= km - 1;
                if (s == wk || s == q) continue;
                if (kingMasks[wk] & (1ULL << s)) continue;
                if ((att >> s) & 1ULL) continue;
                hasMove = true; break;
            }
            if (!hasMove && !inCheck) continue;           // stalemate: draw
            val[key] = 1;
            bucket[1].push_back(key);
        } else {
            // black to move: count in-space replies, flag pawn-capture escape
            uint64_t patt = pawnAtt(ps);
            bool hadCapture = false;
            int kids = 0;
            uint64_t km = kingMasks[bk];
            while (km) {
                int s = __builtin_ctzll(km); km &= km - 1;
                if (s == wk) continue;
                if (s == ps) {
                    if (!(kingMasks[wk] & (1ULL << ps))) hadCapture = true;
                    continue;
                }
                if (kingMasks[wk] & (1ULL << s)) continue;   // adjacent white K
                if ((patt >> s) & 1ULL) continue;            // pawn-controlled
                kids++;
            }
            down[key] = kids;
            if (hadCapture) canDraw[key] = 1;
        }
    }

    // ---- retrograde BFS (value-ordered buckets) ----
    for (int d = 1; d <= MAXD; d++) {
        for (size_t bi = 0; bi < bucket[d].size(); bi++) {
            int key = bucket[d][bi];
            int v   = d;
            int stm = key >> 18;
            int wk  = (key >> 12) & 63;
            int bk  = (key >> 6)  & 63;
            int ps  = key & 63;

            if (stm == 1) {
                // black cannot avoid promotion in v: every WHITE predecessor
                // wins in v+1 (first discovery in value order is minimal).
                // (a) white king moved last
                uint64_t km = kingMasks[wk];
                while (km) {
                    int pwk = __builtin_ctzll(km); km &= km - 1;
                    if (pwk == bk || pwk == ps) continue;
                    if (kingMasks[pwk] & (1ULL << bk)) continue; // P invalid
                    int pkey = kpkKey(0, pwk, bk, ps);
                    if (val[pkey] == 0 && v + 1 <= MAXD) {
                        val[pkey] = (int8_t)(v + 1);
                        bucket[v + 1].push_back(pkey);
                    }
                }
                // (b) pawn pushed last: single or double
                int pr = ps >> 3;
                if (pr >= 2) {
                    int pp = ps - 8;
                    if (pp != wk && pp != bk) {
                        int pkey = kpkKey(0, wk, bk, pp);
                        if (val[pkey] == 0 && v + 1 <= MAXD) {
                            val[pkey] = (int8_t)(v + 1);
                            bucket[v + 1].push_back(pkey);
                        }
                    }
                }
                if (pr == 3) {
                    int pp = ps - 16, mid = ps - 8;
                    if (mid != wk && mid != bk && pp != wk && pp != bk) {
                        int pkey = kpkKey(0, wk, bk, pp);
                        if (val[pkey] == 0 && v + 1 <= MAXD) {
                            val[pkey] = (int8_t)(v + 1);
                            bucket[v + 1].push_back(pkey);
                        }
                    }
                }
            } else {
                // white win solved: BLACK predecessors lose once every
                // in-space reply is a win (and no pawn-capture escape).
                uint64_t km = kingMasks[bk];
                uint64_t patt = pawnAtt(ps);
                while (km) {
                    int pbk = __builtin_ctzll(km); km &= km - 1;
                    if (pbk == wk || pbk == ps) continue;
                    if (kingMasks[wk] & (1ULL << pbk)) continue; // P invalid
                    if ((patt >> bk) & 1ULL) continue;   // moved into pawn check
                    int pkey = kpkKey(1, wk, pbk, ps);
                    if (val[pkey] == 0) {
                        down[pkey]--;
                        if ((int)maxChild[pkey] < v) maxChild[pkey] = (uint8_t)v;
                        if (down[pkey] == 0 && !canDraw[pkey]) {
                            int nv = (int)maxChild[pkey] + 1;
                            if (nv > MAXD) nv = MAXD;
                            val[pkey] = (int8_t)nv;
                            bucket[nv].push_back(pkey);
                        }
                    }
                }
            }
        }
    }

    for (int key = 0; key < N; key++)
        if (val[key]) T[key] = val[key];
}

int GenerateMoves::kpkDTM(int stm, int wk, int bk, int ps) const
{
    if (wk == bk || ps == wk || ps == bk) return 0;
    if (kingMasks[wk] & (1ULL << bk)) return 0;
    int pr = ps >> 3;
    if (pr == 0 || pr == 7) return 0;
    return egKP[stm][wk][bk][ps];
}

// Gated controller: exactly one pawn (ours), no other pieces, bare enemy
// king. Returns the shortest safe-promotion move, or null when the
// position doesn't match / is tablebase-drawn.
Move GenerateMoves::kpkMove(Board &board)
{
    int winner = board.sideToMove;
    uint64_t myPawns  = (winner == 0) ? board.whitePawns : board.blackPawns;
    uint64_t oppPawns = (winner == 0) ? board.blackPawns : board.whitePawns;
    uint64_t myOthers = (winner == 0)
        ? (board.whiteQueen | board.whiteRooks | board.whiteBishops | board.whiteKnights)
        : (board.blackQueen | board.blackRooks | board.blackBishops | board.blackKnights);
    uint64_t oppOthers = (winner == 0)
        ? (board.blackQueen | board.blackRooks | board.blackBishops | board.blackKnights)
        : (board.whiteQueen | board.whiteRooks | board.whiteBishops | board.whiteKnights);

    if (__builtin_popcountll(myPawns) != 1) return Move(0, 0, NORMAL);
    if (myOthers)                           return Move(0, 0, NORMAL);
    if (oppPawns || oppOthers)              return Move(0, 0, NORMAL);

    int ourKing, theirKing, pawnSq;
    if (winner == 0) {
        ourKing   = __builtin_ctzll(board.whiteKing);
        theirKing = __builtin_ctzll(board.blackKing);
    } else {
        ourKing   = __builtin_ctzll(board.blackKing);
        theirKing = __builtin_ctzll(board.whiteKing);
    }
    pawnSq = __builtin_ctzll(myPawns);

    // canonical white-strong frame
    int wkT, bkT, psT;
    if (winner == 0) { wkT = ourKing;     bkT = theirKing;     psT = pawnSq; }
    else             { wkT = ourKing ^ 56; bkT = theirKing ^ 56; psT = pawnSq ^ 56; }

    buildKpkTable();
    int cur = kpkDTM(0, wkT, bkT, psT);
    if (cur <= 0) return Move(0, 0, NORMAL);      // drawn / unreachable

    MoveList legal = generateLegalMoves(board, winner);
    if (legal.count == 0) return Move(0, 0, NORMAL);

    Move best(0, 0, NORMAL);
    int bestVal = 300;

    for (int i = 0; i < legal.count; i++) {
        Move m = legal.moves[i];
        int type = m.getType();

        int cv = 0;
        if (type == PROMOT_QUEEN) {
            // safety check mirrors the table seed test, evaluated live
            board.makeMove(m);
            int q  = m.getTo();
            int lk = __builtin_ctzll((winner == 0) ? board.blackKing
                                                   : board.whiteKing);
            int ok = __builtin_ctzll((winner == 0) ? board.whiteKing
                                                   : board.blackKing);
            uint64_t att = getQueenAttacks(q, 1ULL << ok);
            bool inCheck = (att >> lk) & 1ULL;
            bool hasMove = false;
            uint64_t km = kingMasks[lk];
            while (km) {
                int s = __builtin_ctzll(km); km &= km - 1;
                if (s == ok || s == q) continue;
                if (kingMasks[ok] & (1ULL << s)) continue;
                if ((att >> s) & 1ULL) continue;
                hasMove = true; break;
            }
            board.undoMove();
            if ((hasMove || inCheck) && !(kingMasks[lk] & (1ULL << q)))
                cv = 1;
        } else if (type != PROMOT_ROOK && type != PROMOT_BISHOP &&
                   type != PROMOT_KNIGHT) {
            board.makeMove(m);
            int wC, bC, pC;
            if (winner == 0) {
                wC = __builtin_ctzll(board.whiteKing);
                bC = __builtin_ctzll(board.blackKing);
                pC = board.whitePawns ? __builtin_ctzll(board.whitePawns) : -1;
            } else {
                wC = __builtin_ctzll(board.blackKing);
                bC = __builtin_ctzll(board.whiteKing);
                pC = board.blackPawns ? __builtin_ctzll(board.blackPawns) : -1;
            }
            board.undoMove();
            if (pC >= 0) {
                if (winner == 0) cv = kpkDTM(1, wC, bC, pC);
                else             cv = kpkDTM(1, wC ^ 56, bC ^ 56, pC ^ 56);
            }
        }
        // underpromotions: cv stays 0 (never required in a won KPK)

        if (cv > 0 && cv < bestVal) { bestVal = cv; best = m; }
    }

    if (best.data != 0) {
        std::cout << "info string KPK drive (" << cur << " plies to promotion)\n";
        return best;
    }
    return Move(0, 0, NORMAL);
}

// ???????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????
// ITERATIVE DEEPENING ROOT
// ???????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????
// ???????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????
// DETERMINISTIC CONTROLLER: KQ-vs-K and KR-vs-K
//
// The shallow fixed-time search can't drive these to mate (the mating net
// needs ~25-30 quiet plies and at 1.2s we only reach ~14-18), and eval-only
// steering drowns once every move already scores +2500cp. So when we hold
// a queen or rook vs a bare king, play a hardcoded, always-converging
// strategy: check-and-confine with the major, keep the king between the
// major and the enemy king, march our king in, then the shallow mate scan
// (immediate-mate check below) fires when the net is small enough.
//
// Guarantees: no stalemate, never hangs the major, never delivers the major
// into an undefended king-capture, always finds an immediate mate when one
// exists in one ply.
// ???????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????
Move GenerateMoves::tryEndgameMateMove(Board &board)
{
    // ?? KPK: lone pawn vs bare king — proven drive to safe promotion ??
    // The search shuffles here instead of running the pawn; the tablebase
    // converts. Runs before the KQK/KRK gate (which requires no pawns).
    {
        Move kp = kpkMove(board);
        if (kp.data != 0) return kp;
    }

    // ?????? Detect KQ-vs-K / KR-vs-K, strong side to move ??????
    bool noPawnsMinors =
        (board.whitePawns | board.whiteKnights | board.whiteBishops
       | board.blackPawns | board.blackKnights | board.blackBishops) == 0;
    if (!noPawnsMinors) return Move(0, 0, NORMAL);

    int wR = __builtin_popcountll(board.whiteRooks);
    int bR = __builtin_popcountll(board.blackRooks);
    int wQ = __builtin_popcountll(board.whiteQueen);
    int bQ = __builtin_popcountll(board.blackQueen);

    bool meStrong = false, oppNone = false;
    if (board.sideToMove == 0) {
        meStrong  = (wQ == 1 && bQ == 0 && bR == 0) || (wR == 1 && wQ == 0 && bQ == 0 && bR == 0);
        oppNone   = (bQ == 0 && bR == 0);
    } else {
        meStrong  = (bQ == 1 && wQ == 0 && wR == 0) || (bR == 1 && bQ == 0 && wQ == 0 && wR == 0);
        oppNone   = (wQ == 0 && wR == 0);
    }
    if (!meStrong || !oppNone) return Move(0, 0, NORMAL);

    // ?? Retrograde tablebase controller (KQvK / KRvK) ??
    // Replaces the former forced-mate ID search: O(1) per move, provably
    // forced, stalemate-free shortest-distance mate. Falls through to the
    // greedy drive only for the rare positions the tables mark as draw.
    {
        Move tb = bareMateMove(board);
        if (tb.data != 0) {
            std::cout << "info string Tablebase mate drive\n";
            return tb;
        }
    }

    int winner = board.sideToMove;
    int loser  = 1 - winner;

    auto centerDist = [](int sq) -> int {
        int r = sq >> 3, f = sq & 7;
        return std::max(std::abs(2*r - 7), std::abs(2*f - 7));
    };

    MoveList moves = generateLegalMoves(board, winner);
    if (moves.count == 0) return Move(0, 0, NORMAL);

    Move best(0, 0, NORMAL);
    int bestScore = -1000000000;

    // ?? Forced-mate search (winner to move) ???????????????
    // The greedy one-ply pick misses mate-in-2/3 patterns (e.g. rook to the
    // mate rank, or king to opposition) and gets stuck in corner shuffles
    // once every move looks equal. Try an iterative-deepening mate search:
    // deep enough to see the real net (up to mate-in-10), pruning non-check /
    // non-approach moves so the tree stays small. Falls back to greedy.
    {
        const int MAX_D = 12;
        Move rootMove(0, 0, NORMAL);
        int bestFoundDepth = 0;
        long long mateNodes = 0;
        bool aborted = false;

        auto kdist = [](int a, int b) -> int {
            return std::max(std::abs((a >> 3) - (b >> 3)), std::abs((a & 7) - (b & 7)));
        };
        auto loserKingSq = [loser](Board& b) -> int {
            return __builtin_ctzll((loser == 0) ? b.whiteKing : b.blackKing);
        };
        auto ourKingSq = [winner](Board& b) -> int {
            return __builtin_ctzll((winner == 0) ? b.whiteKing : b.blackKing);
        };
        auto loserMobility = [&](Board& b) -> int {
            int lk = loserKingSq(b);
            uint64_t att = kingMasks[ourKingSq(b)], bb, pbb =
                (winner == 0) ? (b.whiteRooks | b.whiteQueen)
                              : (b.blackRooks | b.blackQueen);
            bb = pbb;
            while (bb) {
                int s = __builtin_ctzll(bb); bb &= bb - 1;
                att |= (winner == 0) ? getRookAttacks(s, b.occupied)
                                     : getRookAttacks(s, b.occupied);
                if (winner == 0 ? (b.whiteQueen >> s & 1ULL) : (b.blackQueen >> s & 1ULL))
                    att |= getQueenAttacks(s, b.occupied);
            }
            int mob = 0;
            uint64_t km = kingMasks[lk] & ~b.occupied;
            while (km) {
                int s = __builtin_ctzll(km); km &= km - 1;
                if (!(att >> s & 1ULL)) mob++;
            }
            return mob;
        };

        std::function<int(Board&, int, int)> ms =
            [&](Board& b, int mover, int d) -> int {
                // 2 = winner forces mate, 0 = unknown / not proven
                if (aborted) return 0;
                if (++mateNodes > 15000000) { aborted = true; return 0; }
                if (d == 0) return 0;
                MoveList L = generateLegalMoves(b, mover);
                if (L.count == 0) return 0;

                if (mover == winner) {
                    int oldKD = kdist(ourKingSq(b), loserKingSq(b));
                    int oldMob = loserMobility(b);
                    const bool wide = (d <= 4);   // closing sequence: try everything
                    for (int i = 0; i < L.count; i++) {
                        b.makeMove(L.moves[i]);
                        if (wide) {
                            // keep
                        } else {
                            MoveList R = generateLegalMoves(b, loser);
                            bool chk = isSquareAttacked(loserKingSq(b), winner, b);
                            int newKD = kdist(ourKingSq(b), loserKingSq(b));
                            (void)R;
                            if (!chk && newKD >= oldKD && loserMobility(b) >= oldMob) {
                                b.undoMove(); continue;   // pure shuffle — skip
                            }
                        }
                        MoveList R = generateLegalMoves(b, loser);
                        bool mated = (R.count == 0) && isSquareAttacked(loserKingSq(b), winner, b);
                        int r = mated ? 2 : ms(b, loser, d - 1);
                        b.undoMove();
                        if (r == 2) {
                            if (d > bestFoundDepth && d == MAX_D) rootMove = L.moves[i];
                            return 2;
                        }
                    }
                    return 0;
                } else {
                    for (int i = 0; i < L.count; i++) {
                        b.makeMove(L.moves[i]);
                        int r = ms(b, winner, d);
                        b.undoMove();
                        if (r != 2) return 0;   // some reply survives
                    }
                    return 2;
                }
            };

        for (int d = 1; d <= MAX_D && !aborted; d++) {
            MoveList root = generateLegalMoves(board, winner);
            Move found(0, 0, NORMAL);
            for (int i = 0; i < root.count; i++) {
                board.makeMove(root.moves[i]);
                MoveList R = generateLegalMoves(board, loser);
                bool mated = (R.count == 0) && isSquareAttacked(loserKingSq(board), winner, board);
                int r = mated ? 2 : ms(board, loser, d - 1);
                board.undoMove();
                if (r == 2) { found = root.moves[i]; break; }
            }
            if (!aborted && found.data != 0) {
                bestFoundDepth = d;
                rootMove = found;
            }
        }

        if (!aborted && rootMove.data != 0) {
            std::cout << "info string Endgame forced mate\n";
            return rootMove;
        }
    }

    for (int i = 0; i < moves.count; i++) {
        board.makeMove(moves.moves[i]);
        int loserKing = __builtin_ctzll((loser == 0) ? board.whiteKing : board.blackKing);
        int ourKing   = __builtin_ctzll((winner == 0) ? board.whiteKing : board.blackKing);

        bool inCheck = isSquareAttacked(loserKing, winner, board);
        MoveList replies = generateLegalMoves(board, loser);

        if (replies.count == 0) {
            if (inCheck) { board.undoMove(); return moves.moves[i]; } // mate!
            board.undoMove(); continue;                               // stalemate ??? never play it
        }

        // Never leave the major hanging on a square the enemy king can take.
        uint64_t ourMajor = (winner == 0)
            ? (board.whiteQueen | board.whiteRooks)
            : (board.blackQueen | board.blackRooks);
        if (ourMajor & kingMasks[loserKing]) {
            int msq = __builtin_ctzll(ourMajor);
            bool defended = (kingMasks[ourKing] & (1ULL << msq)) != 0;
            if (!defended) { board.undoMove(); continue; }
        }

        // Squares the loser king can still reach that we do not control.
        int mob = 0;
        {
            uint64_t att = 0, p;
            if (winner == 0) {
                att |= kingMasks[ourKing];
                p = board.whiteRooks;   while (p) { int s = __builtin_ctzll(p); p &= p - 1; att |= getRookAttacks(s, board.occupied); }
                p = board.whiteQueen;   while (p) { int s = __builtin_ctzll(p); p &= p - 1; att |= getQueenAttacks(s, board.occupied); }
            } else {
                att |= kingMasks[ourKing];
                p = board.blackRooks;   while (p) { int s = __builtin_ctzll(p); p &= p - 1; att |= getRookAttacks(s, board.occupied); }
                p = board.blackQueen;   while (p) { int s = __builtin_ctzll(p); p &= p - 1; att |= getQueenAttacks(s, board.occupied); }
            }
            uint64_t km = kingMasks[loserKing] & ~board.occupied;
            while (km) {
                int s = __builtin_ctzll(km); km &= km - 1;
                if (!(att >> s & 1ULL)) mob++;
            }
        }

        // Confine (corner it, shrink its escape squares), march the king in
        // while staying safely far (kDistance >= 2 avoids stalemate traps).
        // A flat check bonus just re-creates the endless shuffle, so checks
        // only win here through the confinement/approach they enable.
        int kDist = std::max(std::abs((ourKing >> 3) - (loserKing >> 3)),
                             std::abs((ourKing & 7) - (loserKing & 7)));
        int cd = centerDist(loserKing);
        // Cut weight: a major on the enemy king's rank/file (rook) or
        // adjacent (queen) confines it. Score = corner(major weight) +
        // confinement + king approach (heavily weighted so the march to the
        // net beats rook/queen shuffling).
        bool cut = (ourMajor == 0) ? false : false;
        {
            uint64_t m = ourMajor;
            while (m) {
                int msq = __builtin_ctzll(m); m &= m - 1;
                int r1 = msq >> 3, f1 = msq & 7;
                int r2 = loserKing >> 3, f2 = loserKing & 7;
                if (r1 == r2 || f1 == f2) cut = true;
            }
        }
        int score = cd * 200
                  + (8 - mob) * 18
                  + 280 - 45 * kDist
                  + (kDist <= 2 ? 340 : 0)
                  + (cut ? 40 : 0)
                  + (inCheck ? (mob >= 7 ? 15 : 0) : 0);
        if (score > bestScore) { bestScore = score; best = moves.moves[i]; }
        board.undoMove();
    }

    if (best.data != 0) {
        std::cout << "info string Endgame mate drive\n";
        return best;
    }
    return moves.moves[0];
}

Move GenerateMoves::getBestMove(Board& board, int maxDepth,
                                 long long myTimeLeftMs,
                                 long long incrementMs, int movesToGo,
                                 long long movetimeMs)
{
    // ?????? Opening Book Probe ????????????????????????????????????????????????????????????????????????????????????????????????
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

    // ?????? Deterministic bare-king mate drive (KQvK / KRvK) ??????
    // Search can't close these (mate needs >25 plies); the controller can.
    Move endgameMate = tryEndgameMateMove(board);
    if (endgameMate.data != 0) return endgameMate;

    // ?????? Time Management ?????????????????????????????????????????????????????????????????????????????????????????????????????????
    if (movetimeMs > 0) {
        // Fixed time per move ("go movetime X") — respect the requested
        // time exactly (small safety buffer). Previously an "endgame boost"
        // multiplied this past the requested limit, making the engine take
        // up to 1.6x longer than the caller asked for.
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

        // Graduated emergency floor ??? scales with available time
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

    // ?????? Iterative Deepening ??????????????????????????????????????????????????????????????????????????????????????????
    for (int currentDepth = 1; currentDepth <= maxDepth; currentDepth++)
    {
        // ?????? Aspiration Windows ????????????????????????????????????????????????????????????????????????????????????
        int delta = 25;
        int alpha = (currentDepth >= 4) ? prevScore - delta : -999999;
        int beta  = (currentDepth >= 4) ? prevScore + delta :  999999;

        Move bestMoveThisDepth  = absoluteBestMove;
        int  bestScoreThisDepth = -999999;
        int  aspirationFails    = 0;

        // Best move from the previous aspiration pass. Retries must START
        // from this move (searched first) — resetting to the stale
        // absoluteBestMove let the first-generated quiet move win every
        // tie in retry passes and discard a found mate.
        Move passBest = absoluteBestMove;

        while (true)
        {
            checkTimeBudget();
            if (searchAborted) goto done;

            bestScoreThisDepth = -999999;
            bestMoveThisDepth  = passBest;

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

            passBest = bestMoveThisDepth;

            // A mate score is exact — never aspiration-retry it. Retrying
            // re-searches every root move against a saturated window where
            // all bounds tie, and the tie degenerates to move order.
            if (bestScoreThisDepth > 90000 || bestScoreThisDepth < -90000)
                break;

            // Widen window on fail ??? with a hard cap on retries
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
            else break;  // score inside window ??? done

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

        // Soft stop: don't start a new depth if we've used 65% of budget.
        // (Starting a depth we don't finish is safe — the abort path returns
        // the last fully-searched best move — so we can afford to use more
        // of the allocated time than a conservative 50% rule.)
        auto elapsedNow = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::high_resolution_clock::now() - searchStartTime).count();

        if (elapsedNow >= timeLimitMs * 65 / 100) break;

        // If score is a mate, no need to search deeper
        if (bestScoreThisDepth > 90000 || bestScoreThisDepth < -90000) break;
    }

done:
    return absoluteBestMove;
}
