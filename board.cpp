#include <iostream>
#include <cstdint>
#include <cstring>
#include "board.h"
using namespace std;

    void Board::init(){
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

        }
        whitePieces = whitePawns | whiteRooks | whiteKnights | whiteBishops | whiteQueen | whiteKing;
        blackPieces = blackPawns | blackRooks | blackKnights | blackBishops | blackQueen | blackKing;
        occupied = whitePieces | blackPieces;
    }

    void Board::printBoard(){
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
                    piece = "wk";
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
                    piece = "bk";
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
    };

    void Board::clearBoard(){

    //White Pieces
    whitePawns = 0ULL;
    whiteRooks = 0ULL;
    whiteKnights = 0ULL;
    whiteBishops = 0ULL;
    whiteQueen = 0ULL;
    whiteKing = 0ULL;

    //Black Pieces
    blackPawns = 0ULL;
    blackRooks = 0ULL;
    blackKnights = 0ULL;
    blackBishops = 0ULL;
    blackQueen = 0ULL;
    blackKing = 0ULL;
    whitePieces = 0ULL;
    blackPieces = 0ULL;
    occupied = 0ULL;
}

    bool Board::isOccupied(int sq) const{
        return(occupied >> sq) & 1ULL;
    };

    int Board::getPieceColor(int sq) const{
        if ((whitePieces >> sq ) & 1ULL){
            return 1;
        }
        if ((blackPieces >> sq) & 1ULL){
            return 2;
        }
        return 0;
    };
    
    void Board::addPiece(int sq, int pieceType, bool isWhite){
        uint64_t bit = (1ULL << sq);
        if (isWhite){
            whitePieces |= bit;
        }
        if (isWhite) {
        whitePieces |= bit;
        switch (pieceType) {
            case 0: whiteRooks |= bit;   break;
            case 1: whiteKnights |= bit; break;
            case 2: whiteBishops |= bit; break;
            case 3: whiteQueen |= bit;   break;
            case 4: whiteKing |= bit;    break;
            case 5: whitePawns |= bit;   break; 
        }
    } 
    else {
        blackPieces |= bit;
        switch (pieceType) {
            case 0: blackRooks |= bit;   break;
            case 1: blackKnights |= bit; break;
            case 2: blackBishops |= bit; break;
            case 3: blackQueen |= bit;   break;
            case 4: blackKing |= bit;    break;
            case 5: blackPawns |= bit;   break;
        }
    }

    occupied |= bit;
        
    };