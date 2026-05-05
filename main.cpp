#include "generateMoves.h"
#include <iostream>
#include <string>

using namespace std;

void runPawnTest(GenerateMoves& gen, int sq, int side, uint64_t occupied, uint64_t opponents, string description) {
    // Generate all potential moves
    uint64_t allMoves = gen.generatePawnMoves(sq, side, occupied, opponents);

    // Separate normal moves from promotion moves based on the rank
    uint64_t promotionMoves = 0ULL;
    uint64_t normalMoves = 0ULL;

    if (side == 0) { // White
        promotionMoves = allMoves & GenerateMoves::ROW_8;
        normalMoves = allMoves & ~GenerateMoves::ROW_8;
    } else { // Black
        promotionMoves = allMoves & GenerateMoves::ROW_1;
        normalMoves = allMoves & ~GenerateMoves::ROW_1;
    }

    cout << "====================================================" << endl;
    cout << " TEST: " << description << endl;
    cout << " Square: " << sq << " | Side: " << (side == 0 ? "White" : "Black") << endl;
    cout << "====================================================" << endl;

    cout << "Occupancy (Current Board State):" << endl;
    gen.printBitBoard(occupied);

    cout << "Normal Moves (Pushes & Captures):" << endl;
    gen.printBitBoard(normalMoves);

    if (promotionMoves) {
        cout << "PROMOTION MOVES (Targeting last rank):" << endl;
        gen.printBitBoard(promotionMoves);
    }
    cout << "\n" << endl;
}

int main() {
    GenerateMoves gen;
    gen.init();

    // --- CASE 1: WHITE STARTING POSITION (e2) ---
    // Should show: Single push (e3), Double push (e4), and Capture (d3)
    // IMPORTANT: We place a "Friend" on f3 to test that he CANNOT eat his own piece.
    uint64_t whiteSq = 12; // e2
    uint64_t opponents1 = (1ULL << 19); // Black piece on d3 (Capture)
    uint64_t friends1 = (1ULL << 21);    // White piece on f3 (Blocker - cannot eat)
    uint64_t occ1 = (1ULL << whiteSq) | opponents1 | friends1;
    runPawnTest(gen, whiteSq, 0, occ1, opponents1, "White e2: Double Move + Diagonal Capture + Diagonal Block");


    // --- CASE 2: WHITE PROMOTION WITH CAPTURE (e7) ---
    // Should show: Promotion via push (e8) and Promotion via capture (f8)
    uint64_t whitePromoPushSq = 52; // e7
    uint64_t opponents2 = (1ULL << 61); // Black piece on f8
    uint64_t occ2 = (1ULL << whitePromoPushSq) | opponents2;
    runPawnTest(gen, whitePromoPushSq, 0, occ2, opponents2, "White e7: Promotion via Push and Capture");


    // --- CASE 3: BLACK DOUBLE MOVE BLOCKED (d7) ---
    // Should show: Nothing. If d6 is blocked, he can't move to d6 OR d5.
    uint64_t blackSq = 51; // d7
    uint64_t blocker = (1ULL << 43); // Piece on d6
    uint64_t occ3 = (1ULL << blackSq) | blocker;
    runPawnTest(gen, blackSq, 1, occ3, 0ULL, "Black d7: Double Move Blocked by Piece on d6");

    cout << "Press Enter to finish tests..." << endl;
    cin.get();
    return 0;
}