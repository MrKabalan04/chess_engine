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

void printMoves(const MoveList& list)
{
    cout << "\nGenerated Moves:\n";

    for (int i = 0; i < list.count; i++)
    {
        Move m = list.moves[i];

        cout << sq(m.getFrom()) << " -> " << sq(m.getTo());

        if (m.getType() == CASTLE)
            cout << " (CASTLE)";

        cout << endl;
    }
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

    cout << "  --------------------------\n";
    cout << "    a  b  c  d  e  f  g  h\n";
}

void printCastlingRights(uint8_t rights)
{
    cout << "\nCASTLING RIGHTS:\n";

    if (rights & WHITE_CASTLING_KINGSIDE)
        cout << "White Kingside\n";

    if (rights & WHITE_CASTLING_QUEENSIDE)
        cout << "White Queenside\n";

    if (rights & BLACK_CASTLING_KINGSIDE)
        cout << "Black Kingside\n";

    if (rights & BLACK_CASTLING_QUEENSIDE)
        cout << "Black Queenside\n";

    if (rights == 0)
        cout << "None\n";
}

// =========================
// MAIN
// =========================

int main()
{
    GenerateMoves gen;
    gen.init();

    MoveList list;
    Board board;
   cout << "\n========== TEST 5 (PINNED PIECE TEST) ==========\n";

board.clearBoard();
board.sideToMove = 0;

// castling irrelevant here
board.castlingRights = 0;

// WHITE pieces
board.addPiece(4, KING, true);     // e1
board.addPiece(28, ROOK, true);    // e4 (PINNED)

// BLACK piece (pin attacker)
board.addPiece(60, ROOK, false);   // e8
board.addPiece(63, KING, false);   // e7 (blocking the pin)

board.updateOccupancy();

printBoard(board);

// generate legal moves ONLY for white
MoveList legal = gen.generateLegalMoves(board, 0);

printMoves(legal);

// 🔥 check if rook on e4 has moves
bool rookHasMoves = false;

for (int i = 0; i < legal.count; i++)
{
    Move m = legal.moves[i];
    if (m.getFrom() == 28)
        rookHasMoves = true;
}

cout << "\nPinned rook (e4) has legal moves? "
     << rookHasMoves << endl;

    return 0;
}