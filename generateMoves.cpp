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
            // Use SEE to distinguish winning/losing captures
            int seeScore = isEP ? 0 : seeCapture(board, m);
            if (seeScore >= 0)
                s = 1'000'000 + seeScore;   // winning/equal capture
            else
                s = 500'000  + seeScore;    // losing capture — still above quiets but below killers
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

    // Insertion sort
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

    // base is already from side-to-move perspective
    // mobility and safety are from white's perspective, so flip if black to move
    int bonus = mobility - safety;
    if (board.sideToMove == 1) bonus = -bonus;

    return base + bonus;
}

// ─────────────────────────────────────────────────────────
// QUIESCENCE SEARCH
// ─────────────────────────────────────────────────────────
int GenerateMoves::quiescence(Board& board, int alpha, int beta, int ply)
{
    nodesSearched++;

    if ((nodesSearched & 1023) == 0)
        checkTimeBudget();

    if (searchAborted) return 0;
    if (board.isThreefoldRepetition()) return 0;

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

        // Skip SEE-losing captures in qsearch (they only waste time)
        if (move.getType() != EN_PASSANT && seeCapture(board, move) < 0)
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

    if ((nodesSearched & 1023) == 0)
        checkTimeBudget();

    if (searchAborted) return 0;

    // Draw detection
    if (ply > 0 && board.isThreefoldRepetition()) return 0;

    int originalAlpha = alpha;
    bool isPV = (beta - alpha) > 1; // PV node has a full window

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

    // ── Static eval (used by pruning techniques) ──────────
    int staticEval = evalFull(board);

    // ── Reverse Futility Pruning (Static Null Move) ───────
    // If static eval beats beta by a big margin, we're probably too good — cut off
    if (!inCheck && !isPV && depth >= 1 && depth <= 6)
    {
        int rfpMargin = 120 * depth;
        if (staticEval - rfpMargin >= beta)
            return staticEval - rfpMargin;
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
            int R = 3 + (depth >= 6 ? 1 : 0);
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

    // Validate TT move — only use it if it's actually in the generated list
    // A stale/colliding TT entry can produce a completely bogus move
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
    int  side       = board.sideToMove;
    int  opponent   = side ^ 1;

    for (int i = 0; i < moves.count; i++)
    {
        Move move = moves.moves[i];

        // ── Compute move flags BEFORE makeMove ────────────
        bool isCapture   = ((board.occupied >> move.getTo()) & 1ULL) != 0
                        || move.getType() == EN_PASSANT;
        bool isPromotion = move.getType() >= PROMOT_QUEEN;
        bool isQuiet     = !isCapture && !isPromotion;

        board.makeMove(move);

        // Skip illegal moves (king left in check) — MUST happen before any pruning
        int kingSq = (side == 0)
            ? __builtin_ctzll(board.whiteKing)
            : __builtin_ctzll(board.blackKing);

        if (isSquareAttacked(kingSq, opponent, board))
        {
            board.undoMove();
            continue;
        }

        legalCount++;

        // ── Futility Pruning ──────────────────────────────
        // Only prune AFTER we know the move is legal
        if (!inCheck && isQuiet && legalCount > 1 && depth <= 3 && !isPV)
        {
            static const int futilityMargin[] = { 0, 150, 300, 500 };
            if (staticEval + futilityMargin[depth] <= alpha)
            {
                board.undoMove();
                continue;
            }
        }

        int score;

        // ── Late Move Reductions (LMR) ─────────────────────
        if (!inCheck && depth >= 3 && legalCount > 3 && isQuiet)
        {
            // Scale reduction with depth and move count
            int R = 1;
            if (legalCount > 6)  R = 2;
            if (legalCount > 12) R = 3;
            if (depth >= 8)      R++;
            R = std::min(R, depth - 2); // never reduce into qsearch directly

            score = -negamax(board, depth - 1 - R, -alpha - 1, -alpha, ply + 1);

            // Full re-search if LMR move beats alpha
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

            if (isQuiet)
            {
                int hIdx = move.getFrom() * 64 + move.getTo();
                historyTable[hIdx] += depth * depth;
                if (historyTable[hIdx] > 1'000'000)
                    historyTable[hIdx] = 1'000'000;
            }
        }

        if (alpha >= beta)
        {
            if (isQuiet && ply < 32)
            {
                if (killerMoves[ply][0].data != move.data)
                {
                    killerMoves[ply][1] = killerMoves[ply][0];
                    killerMoves[ply][0] = move;
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

    timeLimitMs = (myTimeLeftMs / (movesToGo + 5)) + (long long)(incrementMs * 0.75);
    if (timeLimitMs > 8000)                             timeLimitMs = 8000;
    if (timeLimitMs > (long long)(myTimeLeftMs * 0.75)) timeLimitMs = (long long)(myTimeLeftMs * 0.75);
    if (timeLimitMs < 20)                               timeLimitMs = 20;

    searchStartTime = std::chrono::high_resolution_clock::now();
    searchAborted   = false;
    nodesSearched   = 0;

    memset(killerMoves, 0, sizeof(killerMoves));
    ageHistory();

    MoveList legalRootMoves = generateLegalMoves(board, board.sideToMove);
    if (legalRootMoves.count == 0) return Move(0, 0, NORMAL);

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
        int delta = 50;
        int alpha = (currentDepth >= 4) ? prevScore - delta : -999999;
        int beta  = (currentDepth >= 4) ? prevScore + delta :  999999;

        Move bestMoveThisDepth  = absoluteBestMove;
        int  bestScoreThisDepth = -999999;

        while (true)
        {
            bestScoreThisDepth = -999999;
            bestMoveThisDepth  = absoluteBestMove;

            for (int i = 0; i < legalRootMoves.count; i++)
            {
                Move move = legalRootMoves.moves[i];
                board.makeMove(move);

                // Full window for every root move — correct and simple
                int score = -negamax(board, currentDepth - 1, -beta, -alpha, 1);
                board.undoMove();

                checkTimeBudget();
                if (searchAborted) goto done;

                if (score > bestScoreThisDepth)
                {
                    bestScoreThisDepth = score;
                    bestMoveThisDepth  = move;
                }
                if (score > alpha)
                    alpha = score;
                if (alpha >= beta) break;
            }

            if (searchAborted) goto done;

            // Widen window and retry on fail-low or fail-high
            if (bestScoreThisDepth <= prevScore - delta)
            {
                alpha  = bestScoreThisDepth - delta;
                delta *= 2;
                if (alpha < -999999) alpha = -999999;
            }
            else if (bestScoreThisDepth >= prevScore + delta)
            {
                beta   = bestScoreThisDepth + delta;
                delta *= 2;
                if (beta > 999999) beta = 999999;
            }
            else
            {
                break; // Score inside window — done
            }

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

        orderMoves(legalRootMoves, board, 0, bestMoveThisDepth);

        auto now     = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - searchStartTime).count();
        if (elapsed == 0) elapsed = 1;

        std::cout << "info depth "  << currentDepth
                  << " score cp "   << bestScoreThisDepth
                  << " time "       << elapsed
                  << " nodes "      << nodesSearched
                  << " nps "        << (nodesSearched * 1000 / elapsed)
                  << std::endl;

        if (legalRootMoves.count == 1) break;

        // Don't start a new depth if we've used more than 60% of time budget
        auto elapsedNow = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::high_resolution_clock::now() - searchStartTime).count();
        if (elapsedNow >= timeLimitMs * 6 / 10) break;
    }

done:
    return absoluteBestMove;
}