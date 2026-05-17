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
    cout << "\n";

    for (int r = 7; r >= 0; r--)
    {
        cout << r + 1 << "  ";

        for (int f = 0; f < 8; f++)
        {
            int s = r * 8 + f;

            char c = '.';

            // WHITE
            if (b.whitePawns & (1ULL << s)) c = 'P';
            else if (b.whiteKnights & (1ULL << s)) c = 'N';
            else if (b.whiteBishops & (1ULL << s)) c = 'B';
            else if (b.whiteRooks & (1ULL << s)) c = 'R';
            else if (b.whiteQueen & (1ULL << s)) c = 'Q';
            else if (b.whiteKing & (1ULL << s)) c = 'K';

            // BLACK
            else if (b.blackPawns & (1ULL << s)) c = 'p';
            else if (b.blackKnights & (1ULL << s)) c = 'n';
            else if (b.blackBishops & (1ULL << s)) c = 'b';
            else if (b.blackRooks & (1ULL << s)) c = 'r';
            else if (b.blackQueen & (1ULL << s)) c = 'q';
            else if (b.blackKing & (1ULL << s)) c = 'k';

            cout << c << " ";
        }

        cout << endl;
    }

    cout << "\n   a b c d e f g h\n\n";
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


int parseSquare(char file, char rank)
{
    int col = file - 'a';   // a-h → 0-7
    int row = rank - '1';   // 1-8 → 0-7

    return row * 8 + col;
}
// =========================
// MAIN
// =========================

int main()
{
    GenerateMoves gen;
    gen.init();

    Board board;
    board.init();

    

    while (true)
{
    printBoard(board);

    if (gen.isCheckmate(board, board.sideToMove))
    {
        cout << "Checkmate! "
             << ((board.sideToMove == 0) ? "Black" : "White")
             << " wins!" << endl;
        break;
    }

    if (gen.isStalemate(board, board.sideToMove))
    {
        cout << "Stalemate! It's a draw!" << endl;
        break;
    }

    // =====================================
    // HUMAN (WHITE)
    // =====================================

    if (board.sideToMove == 0)
    {
        MoveList legal =
            gen.generateLegalMoves(board, board.sideToMove);

        cout << "White to move (e2e4): ";

        string moveStr;
        cin >> moveStr;

        if (moveStr.length() != 4)
        {
            cout << "Invalid move format!" << endl;
            continue;
        }

        int from = parseSquare(moveStr[0], moveStr[1]);
        int to   = parseSquare(moveStr[2], moveStr[3]);

        if (from < 0 || from > 63 ||
            to < 0 || to > 63)
        {
            cout << "Invalid square input!" << endl;
            continue;
        }

        bool found = false;

        for (int i = 0; i < legal.count; i++)
        {
            Move m = legal.moves[i];

            if (m.getFrom() == from &&
                m.getTo() == to)
            {
                board.makeMove(m);
                found = true;
                break;
            }
        }

        if (!found)
        {
            cout << "Illegal move!" << endl;
            continue;
        }
    }

    // =====================================
    // ENGINE (BLACK)
    // =====================================

    else
    {
        cout << "Engine thinking...\n";
        cout << "Eval: " << board.evaluate() << endl;
        Move bestMove = gen.getBestMove(board, 3);

        cout << "Engine played: "
             << sq(bestMove.getFrom())
             << sq(bestMove.getTo())
             << endl;

        board.makeMove(bestMove);
        cout << "Eval: " << board.evaluate() << endl;
    }
}

    return 0;
}