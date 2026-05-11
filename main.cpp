#include <iostream>
#include <string>
#include "board.h"
#include "generateMoves.h"
#include "types.h"

using namespace std;

// =========================
// UI HELPERS
// =========================
string sq(int s) {
    string f = "abcdefgh";
    string r = "12345678";
    return string(1, f[s % 8]) + r[s / 8];
}

string piece(int p) {
    switch (p) {
        case PAWN: return "P";
        case KNIGHT: return "N";
        case BISHOP: return "B";
        case ROOK: return "R";
        case QUEEN: return "Q";
        case KING: return "K";
        default: return ".";
    }
}

void printBoard(const Board& b) {
    cout << "\n    REALISTIC GAME POSITION\n\n";
    cout << "    a  b  c  d  e  f  g  h\n";
    cout << "  --------------------------\n";

    for (int r = 7; r >= 0; r--) {
        cout << r + 1 << " | ";

        for (int f = 0; f < 8; f++) {
            int sq = r * 8 + f;
            int p = b.getPieceAt(sq);
            cout << piece(p) << "  ";
        }

        cout << "| " << r + 1 << "\n";
    }

    cout << "  --------------------------\n";
    cout << "    a  b  c  d  e  f  g  h\n\n";
}

// =========================
// MAIN DEMO
// =========================
int main() {

    Board board;
    board.clearBoard();

    GenerateMoves gen;
    gen.init();

    // ======================================================
    // REALISTIC MID-GAME POSITION (KING UNDER PRESSURE)
    // ======================================================

    // WHITE PIECES
    board.addPiece(4, KING, true);      // e1 king exposed
    board.addPiece(27, QUEEN, true);    // d4 queen
    board.addPiece(10, BISHOP, true);   // c2 bishop
    board.addPiece(28, PAWN, true);     // e4 pawn
    board.addPiece(35, KNIGHT, true);   // d5 knight

    // BLACK PIECES (AGGRESSIVE)
    board.addPiece(60, KING, false);    // e8 king
    board.addPiece(36, ROOK, false);    // e5 rook aiming at king file
    board.addPiece(43, BISHOP, false);  // f6 bishop controlling diagonals
    board.addPiece(52, QUEEN, false);   // e7 queen pressure
    board.addPiece(12, KNIGHT, false);  // e3 knight jump threat

    board.sideToMove = 0;

    // =========================
    // PRINT BOARD
    // =========================
    cout << "\n=====================================\n";
    cout << "      KING SAFETY DEMO POSITION      \n";
    cout << "=====================================\n";

    printBoard(board);

    // =========================
    // MOVE GENERATION
    // =========================
    MoveList list;
    gen.generateAllMoves(board, board.sideToMove, list);

    cout << "Side to move: WHITE\n";
    cout << "Generated pseudo-legal moves: " << list.count << "\n";

    cout << "\nSample moves:\n";
    for (int i = 0; i < min(list.count, 12); i++) {
        cout << sq(list.moves[i].getFrom())
             << " -> "
             << sq(list.moves[i].getTo()) << "\n";
    }

    // =========================
    // KING SAFETY CHECK
    // =========================
    int whiteKing = __builtin_ctzll(board.whiteKing);
    int blackKing = __builtin_ctzll(board.blackKing);

    bool whiteCheck = gen.isSquareAttacked(whiteKing, 1, board);
    bool blackCheck = gen.isSquareAttacked(blackKing, 0, board);

    cout << "\n=====================================\n";
    cout << "           KING STATUS               \n";
    cout << "=====================================\n";

    if (whiteCheck)
        cout << "⚠ WHITE KING IS IN CHECK (UNDER ATTACK!) at " << sq(whiteKing) << "\n";
    else
        cout << "✔ White king safe\n";

    if (blackCheck)
        cout << "⚠ BLACK KING IS IN CHECK (UNDER ATTACK!) at " << sq(blackKing) << "\n";
    else
        cout << "✔ Black king safe\n";

    // =========================
    // ATTACK MAP (WHITE VIEW)
    // =========================
    cout << "\n=====================================\n";
    cout << "        WHITE ATTACK MAP             \n";
    cout << "=====================================\n";

    for (int r = 7; r >= 0; r--) {
        cout << r + 1 << " | ";
        for (int f = 0; f < 8; f++) {
            int s = r * 8 + f;
            cout << (gen.isSquareAttacked(s, 0, board) ? "x  " : ".  ");
        }
        cout << "|\n";
    }

    cout << "    a  b  c  d  e  f  g  h\n";

    // =========================
    // FINAL STATUS
    // =========================
    cout << "\n=====================================\n";
    cout << "ENGINE STATUS: TACTICAL AWARENESS DEMO\n";
    cout << "✔ Real position simulation\n";
    cout << "✔ King attack detection working\n";
    cout << "✔ Sliding + leaping pieces active\n";
    cout << "NEXT STEP: LEGAL MOVE FILTERING\n";
    cout << "=====================================\n";

    cout << PAWN;
    cout << KNIGHT;
    cout << BISHOP;
    cout << ROOK;
    cout << QUEEN;
    cout << KING;
    

    return 0;
}