// test_attack.cpp - probe board state + attack detection after Qxh7 in WAC.004
#include <iostream>
#include "board.h"
#include "generateMoves.h"

int main() {
    Board board;
    GenerateMoves gen;
    if (!board.initFromFen("r1bq2rk/pp3pbp/2p1p1pQ/7P/3P4/2PB1N2/PP3PPR/2KR4 w - - 0 1")) {
        std::cout << "FEN load failed\n"; return 1;
    }

    // find the white queen move h6->h7 among legal moves
    MoveList ml = gen.generateLegalMoves(board, 0);
    Move qxh7(0,0,NORMAL);
    int found = 0;
    for (int i = 0; i < ml.count; i++) {
        if (ml.moves[i].getFrom() == 47 && ml.moves[i].getTo() == 55) { // h6=47, h7=55
            qxh7 = ml.moves[i]; found++;
        }
    }
    std::cout << "h6h7 moves found: " << found << "\n";

    board.makeMove(qxh7);
    std::cout << "after Qxh7:\n";
    std::cout << "  piece at h7 (sq55): " << board.getPieceAt(55) << "\n";
    std::cout << "  whiteKing bb: " << std::hex << board.whiteKing << std::dec << "\n";
    std::cout << "  blackKing bb: " << std::hex << board.blackKing << std::dec << "\n";
    std::cout << "  whiteRooks bb: " << std::hex << board.whiteRooks << std::dec << "\n";
    std::cout << "  occupied   : " << std::hex << board.occupied << std::dec << "\n";
    std::cout << "  h7 attacked by white: " << gen.isSquareAttacked(55, 0, board) << "\n";
    std::cout << "  black in check      : " << gen.isInCheck(board, 1) << "\n";

    MoveList blackMoves = gen.generateLegalMoves(board, 1);
    std::cout << "  black legal moves: " << blackMoves.count << "\n";
    for (int i = 0; i < blackMoves.count; i++) {
        std::cout << "    " << blackMoves.moves[i].getFrom() << "->" << blackMoves.moves[i].getTo() << "\n";
    }
    return 0;
}
