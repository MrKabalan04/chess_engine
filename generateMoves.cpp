#include "generateMoves.h"
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

    // IMPORTANT: use precomputed pawn-to-square masks
    if (pawnToMasks[attackerColor][sq] & pawns) return true;

    if (getBishopAttacks(sq, board.occupied) & (bishops | queens)) return true;
    if (getRookAttacks(sq, board.occupied) & (rooks | queens)) return true;

    return false;
}

MoveList GenerateMoves::generateLegalMoves(Board& board, int side)
{
    MoveList pseudoLegalMoves;
    generateAllMoves(board, side, pseudoLegalMoves);

    MoveList legalMoves;

    int opponent = side ^ 1;

    for (int i = 0; i < pseudoLegalMoves.count; i++)
    {
        Move move = pseudoLegalMoves.moves[i];

        board.makeMove(move);

        int kingSq = (side == 0)
            ? __builtin_ctzll(board.whiteKing)
            : __builtin_ctzll(board.blackKing);

        if (!isSquareAttacked(kingSq, opponent, board))
        {
            legalMoves.addMove(move);
        }

        board.undoMove();
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

bool GenerateMoves::isCheckmate(Board& board, int side){
    if (!isInCheck(board, side)){
        return false;
    }
    MoveList legalMoves = generateLegalMoves(board, side);
    return legalMoves.count == 0;
}

bool GenerateMoves::isStalemate( Board& board, int side){
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
    // QUEENS
    // =========================
    uint64_t queens = (side == 0) ? board.whiteQueen : board.blackQueen;

    while (queens)
    {
        int sq = __builtin_ctzll(queens);
        generateSlidingMoves(sq, QUEEN, occupied, friendlyPieces, list);
        queens &= (queens - 1);
    }

    // =========================
    // KING MOVES
    // =========================
    int kingSq = (side == 0)
        ? __builtin_ctzll(board.whiteKing)
        : __builtin_ctzll(board.blackKing);

    generateKingMoves(kingSq, side, board, list);

    // ======================================================
    // FAST CASTLING SAFETY CHECK (OPTIMIZED)
    // ======================================================

    // If king is in check → no castling
    if (isSquareAttacked(kingSq, enemy, board))
        return;

    const uint64_t occ = occupied;

    // =========================
    // WHITE CASTLING
    // =========================
    if (side == 0)
    {
        // KING SIDE
        if (board.castlingRights & WHITE_CASTLING_KINGSIDE)
        {
            if (!(occ & ((1ULL << 5) | (1ULL << 6))) &&
                !isSquareAttacked(4, enemy, board) &&
                !isSquareAttacked(5, enemy, board) &&
                !isSquareAttacked(6, enemy, board))
            {
                list.addMove(Move(4, 6, CASTLE));
            }
        }

        // QUEEN SIDE
        if (board.castlingRights & WHITE_CASTLING_QUEENSIDE)
        {
            if (!(occ & ((1ULL << 1) | (1ULL << 2) | (1ULL << 3))) &&
                !isSquareAttacked(4, enemy, board) &&
                !isSquareAttacked(3, enemy, board) &&
                !isSquareAttacked(2, enemy, board))
            {
                list.addMove(Move(4, 2, CASTLE));
            }
        }
    }

    // =========================
    // BLACK CASTLING
    // =========================
    else
    {
        // KING SIDE
        if (board.castlingRights & BLACK_CASTLING_KINGSIDE)
        {
            if (!(occ & ((1ULL << 61) | (1ULL << 62))) &&
                !isSquareAttacked(60, enemy, board) &&
                !isSquareAttacked(61, enemy, board) &&
                !isSquareAttacked(62, enemy, board))
            {
                list.addMove(Move(60, 62, CASTLE));
            }
        }

        // QUEEN SIDE
        if (board.castlingRights & BLACK_CASTLING_QUEENSIDE)
        {
            if (!(occ & ((1ULL << 57) | (1ULL << 58) | (1ULL << 59))) &&
                !isSquareAttacked(60, enemy, board) &&
                !isSquareAttacked(59, enemy, board) &&
                !isSquareAttacked(58, enemy, board))
            {
                list.addMove(Move(60, 58, CASTLE));
            }
        }
    }
}

Move GenerateMoves::getBestMove(Board& board, int maxDepth) {
    Move bestMove;
    MoveList legalMoves = generateLegalMoves(board, board.sideToMove);
    
    if (legalMoves.count == 0) return Move(); 

    // Clear the killer move tracking table to start completely fresh for this turn
    memset(killerMoves, 0, sizeof(killerMoves));

    for (int depth = 1; depth <= maxDepth; depth++) {
        int alpha = -1000000;
        int beta  =  1000000;
        int bestScore = -1000000;

        // --- FIXED: Sort the entire legal move list cleanly upfront ---
        // Distance from root is 0, so we pass 0 for ply
        orderMoves(legalMoves, board, 0); 

        // --- FIXED: Straightforward, highly optimized linear loop ---
        for (int i = 0; i < legalMoves.count; i++) {
            Move move = legalMoves.moves[i];
            
            board.makeMove(move);
            // Root ply is 0, so the first negamax branch layer starts at ply 1
            int score = -negamax(board, depth - 1, -beta, -alpha, 1);
            board.undoMove();

            if (score > bestScore) {
                bestScore = score;
                bestMove = move;
            }
            
            alpha = max(alpha, score);
        }
    }

    return bestMove;
}

void GenerateMoves::orderMoves(MoveList& list, const Board& board, int ply) {
    int scores[256] = {0};

    for (int i = 0; i < list.count; i++) {
        Move move = list.moves[i];
        int score = 0;

        // 1. Transposition Table Move
        uint64_t index = board.zobristHash % TT_SIZE;
        if (transpositionTable[index].zobristHash == board.zobristHash && 
            transpositionTable[index].bestMove.data == move.data) {
            scores[i] = 100000; 
            continue; 
        }

        int fromSq = move.getFrom();
        int toSq = move.getTo();
        uint64_t toBit = (1ULL << toSq);
        uint64_t fromBit = (1ULL << fromSq);

        // 2. MVV-LVA Captures
        bool isCapture = false;
        if (board.sideToMove == 0) { 
            if (toBit & board.blackQueen)         { score += 90000; isCapture = true; }
            else if (toBit & board.blackRooks)    { score += 50000; isCapture = true; }
            else if (toBit & board.blackBishops)  { score += 33000; isCapture = true; }
            else if (toBit & board.blackKnights)  { score += 32000; isCapture = true; }
            else if (toBit & board.blackPawns)    { score += 10000; isCapture = true; }
        } else { 
            if (toBit & board.whiteQueen)         { score += 90000; isCapture = true; }
            else if (toBit & board.whiteRooks)    { score += 50000; isCapture = true; }
            else if (toBit & board.whiteBishops)  { score += 33000; isCapture = true; }
            else if (toBit & board.whiteKnights)  { score += 32000; isCapture = true; }
            else if (toBit & board.whitePawns)    { score += 10000; isCapture = true; }
        }

        // 3. Killer Moves
        if (!isCapture) {
            if (move.data == killerMoves[ply][0].data)      score = 9000;
            else if (move.data == killerMoves[ply][1].data) score = 8000;
        }

        scores[i] = score;
    }

    // Sort everything upfront cleanly
    for (int i = 0; i < list.count - 1; i++) {
        for (int j = i + 1; j < list.count; j++) {
            if (scores[j] > scores[i]) {
                swap(scores[i], scores[j]);
                swap(list.moves[i], list.moves[j]);
            }
        }
    }
}

int GenerateMoves::quiescence(Board& board, int alpha, int beta, int depth, int ply)
{
    // 1. STAND PAT: If we stop here, is the current score good enough?
    int score = board.evaluate();

    if (score >= beta)
        return beta;
    if (score > alpha)
        alpha = score;

    // 2. SAFETY GUARD: Stop if we've gone too deep into tactical exchanges
    if (depth <= 0)
        return score;

    // 3. GENERATE ONLY CAPTURES
    MoveList captureMoves;
    generateCaptures(board, board.sideToMove, captureMoves);
    
    // Sort captures by MVV-LVA (Most Valuable Victim, Least Valuable Aggressor)
    orderCaptures(captureMoves, board);

    for (int i = 0; i < captureMoves.count; i++)
    {
        Move move = captureMoves.moves[i];

        board.makeMove(move);

        // --- INLINE LEGALITY CHECK ---
        int kingSq = (board.sideToMove == 1) 
            ? __builtin_ctzll(board.whiteKing) 
            : __builtin_ctzll(board.blackKing);

        if (isSquareAttacked(kingSq, board.sideToMove, board)) 
        {
            board.undoMove();
            continue; 
        }

        // 4. RECURSIVE SEARCH: Search the capture exchange
        score = -quiescence(board, -beta, -alpha, depth - 1, ply + 1);

        board.undoMove();

        // 5. ALPHA-BETA PRUNING
        if (score >= beta)
            return beta;
        
        if (score > alpha)
            alpha = score;
    }

    return alpha;
}

int GenerateMoves::negamax(Board& board, int depth, int alpha, int beta, int ply)
{
    uint64_t hashAtEntry = board.zobristHash;
    int index = hashAtEntry % TT_SIZE;
    TTEntry& entry = transpositionTable[index];

    // ======================================================
    // 1. TRANSPOSITION TABLE LOOKUP
    // ======================================================
    if (entry.zobristHash == hashAtEntry && entry.depth >= depth)
    {
        if (entry.flag == EXACT)
            return entry.score;

        if (entry.flag == BETA && entry.score >= beta)
            return beta;

        if (entry.flag == ALPHA && entry.score <= alpha)
            return alpha;
    }

    // ======================================================
    // 2. LEAF NODE (QUIESCENCE SEARCH)
    // ======================================================
    if (depth == 0)
        return quiescence(board, alpha, beta, 6, ply);

    bool inCheck = isInCheck(board, board.sideToMove);

    // ======================================================
    // 3. NULL MOVE PRUNING (NMP)
    // ======================================================
    if (depth >= 3 && !inCheck && ply > 0)
    {
        board.makeNullMove();

        int nullScore = -negamax(board, depth - 3, -beta, -beta + 1, ply + 1);

        board.undoNullMove();

        if (nullScore >= beta)
            return beta;
    }
    
    // ======================================================
    // 3.5 FUTILITY PRUNING 
    // ======================================================
    if (depth <= 2 && !inCheck && ply > 0)
    {
        int baseEval = board.evaluate();
        if (baseEval + 5000 <= alpha)
        {
            return quiescence(board, alpha, beta, 6, ply); 
        }
    }
    
    // ======================================================
    // 4. GENERATE & SORT PSEUDO-LEGAL MOVES 
    // ======================================================
    MoveList pseudoMoves;
    generateAllMoves(board, board.sideToMove, pseudoMoves);
    
    orderMoves(pseudoMoves, board, ply); 

    int originalAlpha = alpha;
    Move bestMove;
    int bestScore = INT_MIN + 1;
    int legalMovesSearched = 0;

    // ======================================================
    // 5. SEARCH LOOP (HIGHLY OPTIMIZED RE-SEARCH FILTER)
    // ======================================================
    for (int i = 0; i < pseudoMoves.count; i++)
    {
        Move move = pseudoMoves.moves[i];

        board.makeMove(move);

        // --- INLINE LEGALITY CHECK ---
        int kingSq = (board.sideToMove == 1) 
            ? __builtin_ctzll(board.whiteKing) 
            : __builtin_ctzll(board.blackKing);

        if (isSquareAttacked(kingSq, board.sideToMove, board)) 
        {
            board.undoMove();
            continue; 
        }

        legalMovesSearched++;
        int score;

        if (legalMovesSearched == 1)
        {
            // Search the PV move with full window bounds
            score = -negamax(board, depth - 1, -beta, -alpha, ply + 1);
        }
        else
        {
            // Try Late Move Reduction (LMR) on quiet moves down the list
            if (depth >= 3 && !inCheck && legalMovesSearched >= 4 && 
                (move.getType() == NORMAL || move.getType() == DOUBLE_PUSH))
            {
                int reduction = 1; 
                score = -negamax(board, depth - 1 - reduction, -alpha - 1, -alpha, ply + 1);
            }
            else
            {
                // Run a normal PVS scout search with a closed window
                score = -negamax(board, depth - 1, -alpha - 1, -alpha, ply + 1);
            }

            // --- TWO-STEP VERIFICATION RE-SEARCH GUARD ---
            if (score > alpha && score < beta)
            {
                // Step A: Re-search at full depth, but keep the window narrow first!
                score = -negamax(board, depth - 1, -alpha - 1, -alpha, ply + 1);

                // Step B: Only if it genuinely breaks past alpha do we pay for a full window evaluation
                if (score > alpha)
                {
                    score = -negamax(board, depth - 1, -beta, -alpha, ply + 1);
                }
            }
        }

        board.undoMove();

        if (score > bestScore)
        {
            bestScore = score;
            bestMove = move;
        }

        // --- BETA CUTOFF & KILLER MOVE REGISTRATION ---
        if (score >= beta)
        {
            if (move.getType() == NORMAL || move.getType() == DOUBLE_PUSH)
            {
                killerMoves[ply][1] = killerMoves[ply][0]; 
                killerMoves[ply][0] = move;               
            }
            break; 
        }

        alpha = max(alpha, score);
    }

    // ======================================================
    // 6. TERMINAL STATE DETECTION
    // ======================================================
    if (legalMovesSearched == 0)
    {
        if (inCheck)
            return -100000 + ply; 

        return 0; 
    }

    // ======================================================
    // 7. STORE RESULT IN TRANSPOSITION TABLE
    // ======================================================
    TTFlag flag;

    if (bestScore <= originalAlpha)      flag = ALPHA;
    else if (bestScore >= beta)          flag = BETA;
    else                                 flag = EXACT;

    if (depth > entry.depth || entry.zobristHash != hashAtEntry) 
    {
        entry.zobristHash = hashAtEntry;
        entry.score       = bestScore;
        entry.depth       = depth;
        entry.flag        = flag;
        entry.bestMove    = bestMove;
    }

    return bestScore;
}

void GenerateMoves::orderCaptures(MoveList& list, const Board& board) {
    // Piece values: PAWN=1, KNIGHT=3, BISHOP=3, ROOK=5, QUEEN=9, KING=100
    static const int pieceValues[] = { 100, 300, 300, 500, 900, 10000 };
    int scores[256];

    for (int i = 0; i < list.count; i++) {
        Move move = list.moves[i];
        
        int victimType = board.getPieceAt(move.getTo());
        int attackerType = board.getPieceAt(move.getFrom());
        
        // Use values instead of indices
        int victimVal = (victimType != -1) ? pieceValues[victimType] : 0;
        int attackerVal = (attackerType != -1) ? pieceValues[attackerType] : 0;
        
        // MVV-LVA: High victim value, low attacker value gets highest score
        scores[i] = (100 * victimVal) - attackerVal;
    }

    // Sort the list based on scores (simple bubble sort is fine for 5-10 moves)
    for (int i = 0; i < list.count - 1; i++) {
    int bestIdx = i;
    for (int j = i + 1; j < list.count; j++) {
        if (scores[j] > scores[bestIdx]) {
            bestIdx = j;
        }
    }
    if (bestIdx != i) {
        swap(list.moves[i], list.moves[bestIdx]);
        swap(scores[i], scores[bestIdx]);
        }
    }
}

void GenerateMoves::generateCaptures(const Board& board, int side, MoveList& list)
{
    MoveList all;
    generateAllMoves(board, side, all);  // pseudo-legal only, no board modification

    uint64_t opponentPieces = (side == 0) ? board.blackPieces : board.whitePieces;

    for (int i = 0; i < all.count; i++)
    {
        Move move = all.moves[i];
        bool isCapture = (1ULL << move.getTo()) & opponentPieces;
        bool isEnPassant = move.getType() == EN_PASSANT;

        if (isCapture || isEnPassant)
            list.addMove(move);
    }
}