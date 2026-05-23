#include "board.h"
#include "generateMoves.h"
#include "uci.h"
#include <iostream>
#include <string>

std::string sq(int s)
{
    return std::string(1, "abcdefgh"[s % 8]) +
           std::string(1, "12345678"[s / 8]);
}

std::string moveToString(const Move& m)
{
    std::string s;
    s += "abcdefgh"[m.getFrom() % 8];
    s += "12345678"[m.getFrom() / 8];
    s += "abcdefgh"[m.getTo() % 8];
    s += "12345678"[m.getTo() / 8];

    if (m.getType() == PROMOT_QUEEN)  s += "q";
    if (m.getType() == PROMOT_ROOK)   s += "r";
    if (m.getType() == PROMOT_BISHOP) s += "b";
    if (m.getType() == PROMOT_KNIGHT) s += "n";

    return s;
}

int main()
{
    GenerateMoves gen;
    gen.init();

    Board board;
    board.initZobrist();
    board.init();

    UCI uci;
    uci.uciLoop(board, gen);

    return 0;
}