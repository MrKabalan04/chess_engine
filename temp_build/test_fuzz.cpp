// test_fuzz.cpp - random playouts: make/undo must restore EXACT state incl. zobrist
#include <iostream>
#include <cstdint>
#include <random>
#include "board.h"
#include "generateMoves.h"

struct Snapshot {
    uint64_t wp, wn, wb, wr, wq, wk, bp, bn, bb, br, bq, bk, occ;
    int stm, ep, castle, halfmove, fullmove;
    uint64_t hash;
};

static Snapshot snap(Board& b) {
    Snapshot s;
    s.wp=b.whitePawns; s.wn=b.whiteKnights; s.wb=b.whiteBishops; s.wr=b.whiteRooks;
    s.wq=b.whiteQueen; s.wk=b.whiteKing;
    s.bp=b.blackPawns; s.bn=b.blackKnights; s.bb=b.blackBishops; s.br=b.blackRooks;
    s.bq=b.blackQueen; s.bk=b.blackKing;
    s.occ=b.occupied; s.stm=b.sideToMove; s.ep=b.enPassantSquare;
    s.castle=b.castlingRights; s.halfmove=b.halfMoveClock; s.fullmove=0;
    s.hash=b.zobristHash;
    return s;
}

static bool eq(const Snapshot& a, const Snapshot& b, std::string& why) {
    if (a.wp!=b.wp||a.wn!=b.wn||a.wb!=b.wb||a.wr!=b.wr||a.wq!=b.wq||a.wk!=b.wk||
        a.bp!=b.bp||a.bn!=b.bn||a.bb!=b.bb||a.br!=b.br||a.bq!=b.bq||a.bk!=b.bk) { why="bitboards"; return false; }
    if (a.occ!=b.occ) { why="occupied"; return false; }
    if (a.stm!=b.stm) { why="sideToMove"; return false; }
    if (a.ep!=b.ep)   { why="enPassant"; return false; }
    if (a.castle!=b.castle) { why="castleRights"; return false; }
    if (a.halfmove!=b.halfmove) { why="halfMoveClock"; return false; }
    if (a.fullmove!=b.fullmove) { why="fullMoveCounter"; return false; }
    if (a.hash!=b.hash) { why="zobristHash"; return false; }
    return true;
}

int main() {
    GenerateMoves gen;
    gen.init();

    const char* fens[] = {
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
        "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", // kiwipete
        "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",                            // pos3 ep tricks
        "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",     // pos4 promos
        "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8",            // pos5
        "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10",
    };

    std::mt19937_64 rng(12345);
    long totalMoves = 0, mismatches = 0;

    for (int trial = 0; trial < 3000; trial++) {
        Board b;
        b.initZobrist();
        b.init();
        b.initFromFen(fens[trial % 6]);

        Move path[400];
        int depth = 0;
        Snapshot start = snap(b);

        for (int step = 0; step < 200; step++) {
            MoveList ml = gen.generateLegalMoves(b, b.sideToMove);
            if (ml.count == 0) break;
            Move m = ml.moves[rng() % ml.count];
            path[depth++] = m;
            b.makeMove(m);
            totalMoves++;

            // periodic full undo-replay check
            if (step % 37 == 36 || step == 199) {
                Snapshot mid = snap(b);
                for (int i = depth - 1; i >= 0; i--) b.undoMove();
                Snapshot back = snap(b);
                std::string why;
                if (!eq(start, back, why)) {
                    mismatches++;
                    std::cout << "MISMATCH trial=" << trial << " step=" << step
                        << " field=" << why << "\n";
                    if (mismatches > 20) { std::cout << "too many\n"; return 1; }
                }
                // redo the moves to continue playout
                for (int i = 0; i < depth; i++) b.makeMove(path[i]);
                Snapshot again = snap(b);
                std::string why2;
                if (!eq(mid, again, why2)) {
                    mismatches++;
                    std::cout << "REDO-MISMATCH trial=" << trial << " step=" << step
                        << " field=" << why2 << "\n";
                    if (mismatches > 20) return 1;
                }
            }
        }
    }
    std::cout << "done: " << totalMoves << " moves played, " << mismatches << " mismatches\n";
    return 0;
}
