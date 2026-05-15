#include <iostream>
#include "board.h"
#include "generateMoves.h"
#include "types.h"

using namespace std;

// =========================
// HELPERS
// =========================
string sq(int s) {
    string f = "abcdefgh";
    string r = "12345678";
    return string(1, f[s % 8]) + r[s / 8];
}

void printBoard(const Board& b) {
    cout << "\n    a  b  c  d  e  f  g  h\n";
    cout << "  --------------------------\n";

    for (int r = 7; r >= 0; r--) {
        cout << r + 1 << " | ";
        for (int f = 0; f < 8; f++) {
            int s = r * 8 + f;
            int p = b.getPieceAt(s);

            char c = '.';
            switch (p) {
                case PAWN: c = 'P'; break;
                case KNIGHT: c = 'N'; break;
                case BISHOP: c = 'B'; break;
                case ROOK: c = 'R'; break;
                case QUEEN: c = 'Q'; break;
                case KING: c = 'K'; break;
            }

            if (p == -1 && s == b.enPassantSquare)
            {
                c = '1';
            }

            cout << c << "  ";
        }
        cout << "|\n";
    }

    cout << "    a  b  c  d  e  f  g  h\n";
}

// =========================
// MAIN
// =========================
int main() {
    Board board;
    board.clearBoard();

    GenerateMoves gen;
    gen.init();

    // White pawn on e2, black pawn on d4 to test en passant
    board.addPiece(12, PAWN, true);   // e2
    board.addPiece(27, PAWN, false);  // d4
    board.updateOccupancy();

    cout << "Before double push:\n";
    printBoard(board);

    Move doublePush(12, 28, DOUBLE_PUSH);
    board.makeMove(doublePush);

    cout << "After double push:\n";
    printBoard(board);

    cout << "enPassantSquare: " << sq(board.enPassantSquare) << "\n";

    return 0;
}