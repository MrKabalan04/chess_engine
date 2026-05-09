#include "generateMoves.h"
#include <iostream>

using namespace std;

void testLeapingMoves(GenerateMoves& gen, int sq, PieceType type, uint64_t friendly, string desc) {
    MoveList list;
    gen.generateLeapingMoves(sq, type, friendly, list);

    cout << "--- TEST: " << desc << " ---" << endl;
    cout << "Square: " << sq << " | Total Moves: " << list.count << endl;
    
    for (int i = 0; i < list.count; i++) {
        cout << "  Move " << i << ": To " << list.moves[i].getTo() << endl;
    }
    cout << "--------------------------------------\n" << endl;
}

int main() {
    GenerateMoves gen;
    gen.init(); 

    MoveList list;
    int knightSq = 18; // d3

    uint64_t myFriendlyPieces = (1ULL << 29); 

    cout << "Testing with Friendly Mask:" << endl;
    gen.printBitBoard(myFriendlyPieces);

    gen.generateLeapingMoves(knightSq, KNIGHT, myFriendlyPieces, list);

    cout << "Final Knight Moves Count: " << list.count << endl; 
    
    for (int i = 0; i < list.count; i++) {
        if (list.moves[i].getTo() == 29) {
            cout << "CRITICAL ERROR: Square 29 is STILL in the list!" << endl;
        }
    }

    return 0;
}