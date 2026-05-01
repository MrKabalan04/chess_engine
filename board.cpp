#include <iostream>
#include <cstdint>
#include <cstring>
using namespace std;

class Board {
    public:

        //white pieces
        uint64_t whitePawns = 0ULL;
        uint64_t whiteRooks = 0ULL;
        uint64_t whiteKnights = 0ULL;
        uint64_t whiteBishops = 0ULL;
        uint64_t whiteQueen = 0ULL;
        uint64_t whiteKing = 0ULL;
        
        //black pieces
        uint64_t blackPawns = 0ULL;
        uint64_t blackRooks = 0ULL;
        uint64_t blackKnights = 0ULL;
        uint64_t blackBishops = 0ULL;
        uint64_t blackQueen = 0ULL;
        uint64_t blackKing = 0ULL;

        //other
        uint64_t whitePieces = 0ULL;
        uint64_t blackPieces = 0ULL;
        uint64_t occupied = 0ULL;

        void init(){
            for(int i = 0; i < 8; i++){

                whitePawns |= (1ULL << (8 + i));
                blackPawns |= (1ULL << (48 + i));

                switch(i){
                    case 0:
                    case 7:
                        whiteRooks |= (1ULL << i);
                        blackRooks |= (1ULL << (56 + i));
                        break;
                    case 1:
                    case 6:
                        whiteKnights |= (1ULL << i);
                        blackKnights |= (1ULL << (56 + i));
                        break;
                    case 2:
                    case 5:
                        whiteBishops |= (1ULL << i);
                        blackBishops |= (1ULL << (56 + i));
                        break;
                    case 3:
                        whiteQueen |= (1ULL << i);
                        blackQueen |= (1ULL << (56 + i));
                        break;
                    case 4:
                        whiteKing |= (1ULL << i);
                        blackKing |= (1ULL << (56 + i));
                        break;
            }

            whitePieces = whitePawns | whiteRooks | whiteKnights | whiteBishops | whiteQueen | whiteKing;
            blackPieces = blackPawns | blackRooks | blackKnights | blackBishops | blackQueen | blackKing;
            occupied = whitePieces | blackPieces;
        }
    }

    void printBoard(){
        cout << "a  b  c  d  e  f  g  h" << endl;
        for( int row = 7; row >= 0; row--){
            cout << endl;
            for( int column=0; column <= 7; column++){
                int square = row * 8 + column;
                string piece = ". ";
                if((whitePawns >> square) & 1ULL){
                    piece = "WP";
                }else if((whiteRooks >> square) & 1ULL){
                    piece = "WR";
                }else if((whiteKnights >> square) & 1ULL){
                    piece = "WN";
                }else if((whiteBishops >> square) & 1ULL){
                    piece = "WB";
                }else if((whiteQueen >> square) & 1ULL){
                    piece = "WQ";
                }else if((whiteKing >> square) & 1ULL){  
                    piece = "WK";
                }else if((blackPawns >> square) & 1ULL){
                    piece = "BP";
                }else if((blackRooks >> square) & 1ULL){
                    piece = "BR";
                }else if((blackKnights >> square) & 1ULL){
                    piece = "BN";
                }else if((blackBishops >> square) & 1ULL){
                    piece = "BB";
                }else if((blackQueen >> square) & 1ULL){
                    piece = "BQ";
                }else if((blackKing >> square) & 1ULL){
                    piece = "BK";
                }
                cout << piece << " ";
            }
        } 
        cout << endl;
    }

};

int main(){
    Board board;
    board.init();
    board.printBoard();
    return 0;
}