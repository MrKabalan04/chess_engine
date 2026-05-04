#include "board.h"
#include "generateMoves.h"
#include <iostream>

using namespace std;

// Extended helper function to include Queen testing
void runVisualTest(GenerateMoves& gen, int sq, uint64_t occ, string type) {
    uint64_t result;

    if (type == "ROOK") result = gen.getRookAttacks(sq, occ);
    else if (type == "BISHOP") result = gen.getBishopAttacks(sq, occ);
    else if (type == "QUEEN") result = gen.getQueenAttacks(sq, occ);

    cout << "------------------------------------------------" << endl;
    cout << " TESTING: " << type << " ON SQUARE INDEX: " << sq << endl;
    cout << "------------------------------------------------" << endl;

    cout << "Occupancy Map (Blockers):";
    gen.printBitBoard(occ);

    cout << "Calculated Magic Attacks:";
    gen.printBitBoard(result);
    
    cout << "Test completed.\n" << endl;
}

int main() {
    GenerateMoves gen;
    gen.init();

    cout << "--- STARTING QUEEN VISUAL TESTS ---" << endl;

    /* 
       TEST 5: The "Full Star" (Queen at E4)
       Queen at center (28) on an empty board.
       Should show a perfect star shape (Horizontal, Vertical, and Diagonal).
    */
    runVisualTest(gen, 28, 0ULL, "QUEEN");

    /* 
       TEST 6: The "Blocked Queen"
       Queen at D4 (27) with a mix of blockers.
       - Blocker at D6 (Vertical stop)
       - Blocker at F4 (Horizontal stop)
       - Blocker at B2 (Diagonal stop)
    */
    uint64_t queenBlockers = 0ULL;
    queenBlockers |= (1ULL << 43); // D6
    queenBlockers |= (1ULL << 29); // F4
    queenBlockers |= (1ULL << 9);  // B2
    runVisualTest(gen, 27, queenBlockers, "QUEEN");

    cout << "Check the star shapes above. If they look correct, your slider logic is 100% complete." << endl;
    
    cout << "Press Enter to exit...";
    cin.get();

    return 0;
}