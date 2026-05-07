#ifndef TYPES_H
#define TYPES_H

#include <cstdint>

enum PieceType {PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING};

enum MoveType{
    NORMAL = 0,
    DOUBLE_PUSH = 1,
    EN_PASSANT = 2,
    CASTLE = 3,
    PROMOT_QUEEN = 4,
    PROMOT_ROOK = 5,
    PROMOT_BISHOP = 6,
    PROMOT_KNIGHT = 7
};


struct Move {
    uint16_t data;
    Move() : data(0) {}
    Move(int from, int to, MoveType type) {
        data = (from & 0x3F) | ((to & 0x3F) << 6) | ((type & 0xF) << 12);
    }

    int getFrom() const { return data & 0x3F; }
    int getTo() const { return (data >> 6) & 0x3F; }
    int getType() const { return (data >> 12) & 0xF; }
};

struct MoveList {
    Move moves[256];
    int count = 0;  

    void addMove(Move move){
        if (count < 256){
            moves[count] = move;
            count++;
        }
    }
};

#endif

