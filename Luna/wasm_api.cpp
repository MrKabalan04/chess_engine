#include "board.h"
#include "generateMoves.h"
#include <emscripten/bind.h>

static Board board;
static GenerateMoves gen;

std::string sq(int s)
{
    return std::string(1, "abcdefgh"[s % 8]) +
           std::string(1, "12345678"[s / 8]);
}

void initEngine() {
    board.initZobrist();
    board.init();
    gen.init();
    gen.gamePly = 0;
    // Build the opening book once so the browser engine plays real
    // openings (as both colours) instead of a bare, deterministic search.
    if (!gen.bookLoaded)
        gen.buildBook(board);
}

int getSideToMove() {
    return board.sideToMove;
}

std::string getGameStatus() {
    if (gen.isCheckmate(board, board.sideToMove)) return "checkmate";
    if (gen.isStalemate(board, board.sideToMove)) return "stalemate";
    if (board.halfMoveClock >= 100) return "draw";
    return "playing";
}

std::string getFen() {
    return board.getFen();
}

std::string getLegalMoves(int square) {
    MoveList moves = gen.generateLegalMoves(board, board.sideToMove);
    std::string result = "";
    
    for (int i = 0; i < moves.count; i++) {
        Move m = moves.moves[i];
        if (m.getFrom() == square) {
            if (result != "") result += ",";
            result += std::to_string(m.getTo());
        }
    }
    
    return result;
}

std::string getAllLegalMoves() {
    MoveList moves = gen.generateLegalMoves(board, board.sideToMove);
    std::string result;
    for (int i = 0; i < moves.count; i++) {
        if (i > 0) result += ",";
        result += std::to_string(moves.moves[i].getFrom()) + "-" +
                  std::to_string(moves.moves[i].getTo());
    }
    return result;
}

std::string makeMove(int from, int to, std::string promo) {
    MoveList moves = gen.generateLegalMoves(board, board.sideToMove);
    
    for (int i = 0; i < moves.count; i++) {
        Move m = moves.moves[i];
        if (m.getFrom() == from && m.getTo() == to) {
            if (promo != "") {
                if (promo == "q" && m.getType() != PROMOT_QUEEN) continue;
                if (promo == "r" && m.getType() != PROMOT_ROOK) continue;
                if (promo == "b" && m.getType() != PROMOT_BISHOP) continue;
                if (promo == "n" && m.getType() != PROMOT_KNIGHT) continue;
            }
            board.makeMove(m);
            gen.gamePly++;   // track full-game ply count (time management)
            return board.getFen();
        }
    }
    
    return board.getFen();
}

std::string getBestMove() {
    // Fixed per-move budget for casual web play: ~1.2s of exclusive thinking
    // (previously the 5s clock + gamePly==0 gave the engine only ~73ms and a
    // depth-7 search, which is why it blundered and shuffled aimlessly).
    Move best = gen.getBestMove(board, 64, 5000, 0, 0, 1200);
    // UCI/engine convention: "0000" = no legal move (mate/stalemate),
    // not the raw squares of the null move ("a1a1").
    if (best.getFrom() == 0 && best.getTo() == 0) return "0000";
    return sq(best.getFrom()) + sq(best.getTo());
}

std::string undoMove() {
    if (gen.gamePly > 0) gen.gamePly--;
    board.undoMove();
    return board.getFen();
}

bool isInCheck() {
    return gen.isInCheck(board, board.sideToMove);
}

int getKingSquare() {
    int side = board.sideToMove;
    uint64_t kingBB = (side == 0) ? board.whiteKing : board.blackKing;
    int sq = __builtin_ctzll(kingBB);
    return sq;  // returns 0..63
}

EMSCRIPTEN_BINDINGS(luna) {
    emscripten::function("initEngine", &initEngine);
    emscripten::function("getFen", &getFen);
    emscripten::function("getLegalMoves", &getLegalMoves);
    emscripten::function("makeMove", &makeMove);
    emscripten::function("getBestMove", &getBestMove);
    emscripten::function("undoMove", &undoMove);
    emscripten::function("getGameStatus", &getGameStatus);
    emscripten::function("getSideToMove", &getSideToMove);
    emscripten::function("getAllLegalMoves", &getAllLegalMoves);
    emscripten::function("isInCheck", &isInCheck);
    emscripten::function("getKingSquare", &getKingSquare);
}