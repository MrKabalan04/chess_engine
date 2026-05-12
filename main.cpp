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

    // ======================================================
    // PIECES SETUP
    // ======================================================

    // WHITE ROOK on a1 (sq 0)
    board.addPiece(0, ROOK, true);

    // BLACK BISHOP on h8 (sq 63)
    board.addPiece(63, BISHOP, false);

    // ======================================================
    // BLOCKERS (IMPORTANT TEST)
    // ======================================================

    // ROOK PATH
    board.addPiece(3, PAWN, true);     // d1 (friendly blocker)
    board.addPiece(24, KNIGHT, false); // a4 (enemy blocker)

    // BISHOP PATH
    board.addPiece(45, PAWN, true);    // f6 (friendly blocker)
    board.addPiece(54, KNIGHT, false); // g7 (enemy blocker)

    // ======================================================
    // PRINT BOARD
    // ======================================================

    cout << "\n=====================================\n";
    cout << "         SLIDING PIECE TEST          \n";
    cout << "=====================================\n";

    printBoard(board);

    // ======================================================
    // OCCUPANCY & COLORS
    // ======================================================

    uint64_t occ = board.whitePieces | board.blackPieces;
    uint64_t friendly = board.whitePieces;
    uint64_t enemy = board.blackPieces;

    MoveList list;

    // ======================================================
    // ROOK MOVES
    // ======================================================

    cout << "\n=====================================\n";
    cout << "          ROOK MOVES (a1)            \n";
    cout << "=====================================\n";

    gen.generateSlidingMoves(0, ROOK, occ, friendly, list);

    cout << "Moves count: " << list.count << "\n";

    for (int i = 0; i < list.count; i++) {
        cout << sq(list.moves[i].getFrom())
             << " -> "
             << sq(list.moves[i].getTo()) << "\n";
    }

    // ======================================================
    // CLEAR LIST
    // ======================================================
    list.count = 0;

    // ======================================================
    // BISHOP MOVES
    // ======================================================

    cout << "\n=====================================\n";
    cout << "         BISHOP MOVES (h8)           \n";
    cout << "=====================================\n";

    gen.generateSlidingMoves(63, BISHOP, occ, friendly, list);

    cout << "Moves count: " << list.count << "\n";

    for (int i = 0; i < list.count; i++) {
        cout << sq(list.moves[i].getFrom())
             << " -> "
             << sq(list.moves[i].getTo()) << "\n";
    }

    // ======================================================
    // CAPTURE TEST (EXPLICIT CHECK)
    // ======================================================

    cout << "\n=====================================\n";
    cout << "         CAPTURE TESTS               \n";
    cout << "=====================================\n";

    int enemySquare = 54;   // g7
    int friendlySquare = 45; // f6

    uint64_t bishopAttacks = gen.bishopAttacksOnTheFly(63, occ);

    cout << "Can bishop attack g7 (enemy)? ";
    cout << ((bishopAttacks & (1ULL << enemySquare)) ? "YES\n" : "NO\n");

    cout << "Can bishop attack f6 (friendly)? ";
    cout << ((bishopAttacks & (1ULL << friendlySquare)) ? "YES\n" : "NO\n");

    return 0;
}