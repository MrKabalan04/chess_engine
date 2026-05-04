#include "board.h"
#include "generateMoves.h"

int main() {
    Board board;
    board.init();
    // board.printBoard();
    
 
    GenerateMoves gen;
    gen.init();

    // cout << "Testing Knight Moves for Square 18 (C3):" << endl;
    // gen.printBitBoard(gen.knightMasks[18]);

    
    // cout << "Testing King Moves for Square 0 (A1):" << endl;
    // gen.printBitBoard(gen.kingMasks[0]);
 

    //testing rook moves for square 28 (E4)
    int sq = 28; // E4
    uint64_t occ = 0ULL;
    occ |= (1ULL << 44); // E6
    occ |= (1ULL << 31); // H4
    occ |= (1ULL << 24); // B3

    cout << "Blockers (occupied squares):" << endl;
    gen.printBitBoard(occ);

    uint64_t fly = gen.rookAttacksOnTheFly(sq, occ);
    uint64_t magic = gen.getRookAttacks(sq, occ);

    cout << "On the Fly results:" << endl;
    gen.printBitBoard(fly);

    cout << "Magic Bitboard results:" << endl;
    gen.printBitBoard(magic);

  
    uint64_t emptyOcc = 0ULL; 
    uint64_t testAttacks = gen.rookAttacksOnTheFly(28, emptyOcc);
    gen.printBitBoard(testAttacks);
    
    cin.get();
 
    return 0;
}