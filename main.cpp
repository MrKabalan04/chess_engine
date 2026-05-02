#include "board.h"
#include "generateMoves.h"

int main() {
    Board board;
    board.init();
    board.printBoard();
    
 
    GenerateMoves gen;
    gen.init();

    cout << "Testing Knight Moves for Square 18 (C3):" << endl;
    gen.printBitBoard(gen.knightMasks[36]);

    
    cout << "Testing King Moves for Square 0 (A1):" << endl;
    gen.printBitBoard(gen.kingMasks[27]);
 
    
    cin.get();
 
    return 0;
}