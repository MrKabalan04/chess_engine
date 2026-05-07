#include "generateMoves.h"
#include <iostream>
#include <string>

using namespace std;

void runPawnTest(GenerateMoves& gen, int sq, int side, uint64_t occupied, uint64_t opponents, string description) {
    MoveList list;     
    gen.generatePawnMoves(sq, side, occupied, opponents, list);

    cout << "====================================================" << endl;
    cout << " TEST: " << description << endl;
    cout << " Square: " << sq << " | Side: " << (side == 0 ? "White" : "Black") << endl;
    cout << " Total Moves Found: " << list.count << endl;
    cout << "====================================================" << endl;

    cout << "Occupancy (Current Board State):" << endl;
    gen.printBitBoard(occupied);

    cout << "List of moves found:" << endl;
    for (int i = 0; i < list.count; i++) {
        int from = list.moves[i].getFrom();
        int to = list.moves[i].getTo();
        int type = list.moves[i].getType();
        
        cout << "  -> Move " << i << ": " << from << " to " << to;
        
        if (type == DOUBLE_PUSH) cout << " (Double Push)";
        if (type >= PROMOT_QUEEN) cout << " (Promotion!)";
        cout << endl;
    }
    cout << "\n" << endl;
}

int main() {
    GenerateMoves gen;
    gen.init(); 

    // --- CASE 1: WHITE STARTING POSITION (e2) ---
    // e2 = 12. Black on d3 = 19. White blocker on f3 = 21.
    uint64_t whiteSq = 12; 
    uint64_t opponents1 = (1ULL << 19); 
    uint64_t friends1 = (1ULL << 21);    
    uint64_t occ1 = (1ULL << whiteSq) | opponents1 | friends1;
    runPawnTest(gen, whiteSq, 0, occ1, opponents1, "White e2: Double Move + Diagonal Capture");

    // --- CASE 2: WHITE PROMOTION (e7) ---
    // e7 = 52. Black piece on f8 = 61.
    uint64_t whitePromoPushSq = 52; 
    uint64_t opponents2 = (1ULL << 61); 
    uint64_t occ2 = (1ULL << whitePromoPushSq) | opponents2;
    runPawnTest(gen, whitePromoPushSq, 0, occ2, opponents2, "White e7: Promotion Pushes and Captures");

    // --- CASE 3: BLACK DOUBLE MOVE BLOCKED (d7) ---
    // d7 = 51. Piece on d6 = 43.
    uint64_t blackSq = 51; 
    uint64_t blocker = (1ULL << 43); 
    uint64_t occ3 = (1ULL << blackSq) | blocker;
    runPawnTest(gen, blackSq, 1, occ3, 0ULL, "Black d7: Double Move Blocked by d6");

    cout << "Press Enter to finish tests..." << endl;
    cin.get();
    return 0;
}