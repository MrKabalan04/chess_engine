#include "uci.h"
#include "types.h"
#include <string>

std::string moveToString(Move m) {
    std::string f = "abcdefgh";
    std::string r = "12345678";
    int from = m.getFrom();
    int to   = m.getTo();

    if (from < 0 || from > 63 || to < 0 || to > 63 || from == to) return "0000";

    std::string s;
    s += f[from % 8];
    s += r[from / 8];
    s += f[to % 8];
    s += r[to / 8];

    int type = m.getType();
    if (type == PROMOT_QUEEN)   s += "q";
    else if (type == PROMOT_ROOK)     s += "r";
    else if (type == PROMOT_BISHOP)   s += "b";
    else if (type == PROMOT_KNIGHT)   s += "n";
    
    return s;
}

Move parseUCIMove(const std::string& moveStr, const MoveList& legalMoves) {
    if (moveStr.length() < 4) return Move();

    int fromCol = moveStr[0] - 'a';
    int fromRow = moveStr[1] - '1';
    int toCol   = moveStr[2] - 'a';
    int toRow   = moveStr[3] - '1';

    int from = fromRow * 8 + fromCol;
    int to   = toRow   * 8 + toCol;

    if (from < 0 || from > 63 || to < 0 || to > 63 || from == to) return Move();

    char promo = (moveStr.length() == 5) ? moveStr[4] : 0;

    for (int i = 0; i < legalMoves.count; i++) {
        Move m = legalMoves.moves[i];
        if (m.getFrom() != from || m.getTo() != to) continue;

        if (promo != 0) {
            int t = m.getType();
            if (promo == 'q' && t != PROMOT_QUEEN) continue;
            if (promo == 'r' && t != PROMOT_ROOK) continue;
            if (promo == 'b' && t != PROMOT_BISHOP) continue;
            if (promo == 'n' && t != PROMOT_KNIGHT) continue;
        }
        return m;
    }
    return Move();
}