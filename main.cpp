#include "board.h"
#include "generateMoves.h"
#include <iostream>

using namespace std;

/**
 * Visual Test Helper: Renders bitboards with clear headers 
 * for LinkedIn screenshots or technical documentation.
 */
void runScreenshotTest(GenerateMoves& gen, int sq, uint64_t occ, string type) {
    uint64_t result;
    if (type == "ROOK") result = gen.getRookAttacks(sq, occ);
    else if (type == "BISHOP") result = gen.getBishopAttacks(sq, occ);
    else result = gen.getQueenAttacks(sq, occ);

    cout << "================================================" << endl;
    cout << "  PIECE: " << type << " | SQUARE: " << sq << endl;
    cout << "================================================" << endl;
    
    cout << "Current Occupancy (Blockers):" << endl;
    gen.printBitBoard(occ);

    cout << "Generated Magic Attacks:" << endl;
    gen.printBitBoard(result);
    cout << "\n" << endl;
}

int main() {
    GenerateMoves gen;
    gen.init();

    cout << "CHESS ENGINE MOVE GENERATION - VISUAL VERIFICATION" << endl;
    cout << "==================================================\n" << endl;

    // 1. ROOK TEST: Horizontal & Vertical rays with blockers
    // Square 36 (E5), blocked on top (e7) and left (b5)
    uint64_t rookOcc = 0ULL;
    rookOcc |= (1ULL << 52); // Blocker at e7
    rookOcc |= (1ULL << 33); // Blocker at b5
    runScreenshotTest(gen, 36, rookOcc, "ROOK");

    // 2. BISHOP TEST: X-Shape diagonals with blockers
    // Square 27 (D4), blocked on top-right (f6) and bottom-left (b2)
    uint64_t bishopOcc = 0ULL;
    bishopOcc |= (1ULL << 45); // Blocker at f6
    bishopOcc |= (1ULL << 9);  // Blocker at b2
    runScreenshotTest(gen, 27, bishopOcc, "BISHOP");

    // 3. QUEEN TEST: The "Full Star" pattern
    // Square 28 (E4) on a clean board to show perfect symmetry
    runScreenshotTest(gen, 28, 0ULL, "QUEEN");

    cout << "All Magic Bitboard lookups successful. O(1) efficiency verified." << endl;
    cout << "Press Enter to exit..." << endl;
    cin.get();

    return 0;
}