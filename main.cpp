#include "generateMoves.h"
#include <iostream>
#include <string>

using namespace std;

void testSlidingMoves(GenerateMoves& gen, int sq, PieceType type, uint64_t occupied, uint64_t friendly, string pieceName) {
    MoveList list;
    gen.generateSlidingMoves(sq, type, occupied, friendly, list);

    cout << "===== Testing " << pieceName << " at Square " << sq << " =====" << endl;
    cout << "Total Moves Found: " << list.count << endl;

    uint64_t movesBitboard = 0ULL;
    for (int i = 0; i < list.count; i++) {
        movesBitboard |= (1ULL << list.moves[i].getTo());
    }
    
    cout << "Moves Visualization (1 = possible move):" << endl;
    gen.printBitBoard(movesBitboard);
    cout << "==========================================\n" << endl;
}

int main() {
    GenerateMoves gen;
    gen.init(); 

    int testSq = 27; 

    
    
    uint64_t blockers = (1ULL << 43) | (1ULL << 25); 
    uint64_t friendly = (1ULL << 25); 
    
    uint64_t occupied = (1ULL << testSq) | blockers;

    cout << "--- BLOCKERS (Enemy at 43, Friendly at 25) ---" << endl;
    gen.printBitBoard(blockers);

    //ROOK
    testSlidingMoves(gen, testSq, ROOK, occupied, friendly, "ROOK");

    //BISHOP
    testSlidingMoves(gen, testSq, BISHOP, occupied, friendly, "BISHOP");

    //QUEEN
    testSlidingMoves(gen, testSq, QUEEN, occupied, friendly, "QUEEN");

    return 0;
}