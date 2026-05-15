#include <iostream>
#include "board.h"
#include "generateMoves.h"
#include "types.h"

using namespace std;

// =========================
// HELPERS
// =========================
string sq(int s)
{
    string f = "abcdefgh";
    string r = "12345678";
    return string(1, f[s % 8]) + r[s / 8];
}

void printBoard(const Board& b)
{
    cout << "\n    a  b  c  d  e  f  g  h\n";
    cout << "  --------------------------\n";

    for (int r = 7; r >= 0; r--)
    {
        cout << r + 1 << " | ";

        for (int f = 0; f < 8; f++)
        {
            int s = r * 8 + f;
            int p = b.getPieceAt(s);

            char c = '.';

            switch (p)
            {
                case PAWN:   c = 'P'; break;
                case KNIGHT: c = 'N'; break;
                case BISHOP: c = 'B'; break;
                case ROOK:   c = 'R'; break;
                case QUEEN:  c = 'Q'; break;
                case KING:   c = 'K'; break;
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
int main()
{
    Board board;
    board.clearBoard();

    GenerateMoves gen;
    gen.init();

    // ======================================================
    // SETUP: FULL CASTLING POSITION
    // ======================================================

    // White pieces
    board.addPiece(4, KING, true);   // e1
    board.addPiece(0, ROOK, true);   // a1
    board.addPiece(7, ROOK, true);   // h1

    // Black pieces
    board.addPiece(60, KING, false); // e8
    board.addPiece(56, ROOK, false); // a8
    board.addPiece(63, ROOK, false); // h8

    board.updateOccupancy();

    cout << "Before castling:\n";
    printBoard(board);

    // ======================================================
    // WHITE KING SIDE CASTLE (e1 -> g1)
    // ======================================================
    Move whiteCastleKingside(4, 6, CASTLE);
    board.makeMove(whiteCastleKingside);

    cout << "\nAfter WHITE kingside castling:\n";
    printBoard(board);

    cout << "Side to move: " << board.sideToMove << "\n";

    // ======================================================
    // BLACK QUEEN SIDE CASTLE (e8 -> c8)
    // ======================================================
    Move blackCastleQueenside(60, 58, CASTLE);
    board.makeMove(blackCastleQueenside);

    cout << "\nAfter BLACK queenside castling:\n";
    printBoard(board);

    cout << "Side to move: " << board.sideToMove << "\n";

    return 0;
}