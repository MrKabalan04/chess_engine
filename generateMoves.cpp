#include "generateMoves.h"
#include <iostream>
#include <cstring>
#include <cstdint>

using namespace std;

uint64_t GenerateMoves::rookTable[64][4096];
uint64_t GenerateMoves::bishopTable[64][512];

void GenerateMoves::init() {



    
    // Pre-defined Magic Numbers for Rooks (Proven working set)
    static const uint64_t rookMagicsLocal[64] = {
        0x8a80104000800020ULL, 0x140002000100040ULL, 0x2801880a0017001ULL, 0x100081001000420ULL,
        0x200020010080420ULL, 0x3001c0002010008ULL, 0x8480008002000100ULL, 0x2080088004402900ULL,
        0x800098204000ULL, 0x2024401000200040ULL, 0x100802000801000ULL, 0x120800800801000ULL,
        0x208808088000400ULL, 0x2802200800400ULL, 0x2200800100020080ULL, 0x801000060821100ULL,
        0x80044006422000ULL, 0x100808020004000ULL, 0x12108a0010204200ULL, 0x140848010000802ULL,
        0x481828014002800ULL, 0x8094004002004100ULL, 0x4010040010010802ULL, 0x20008806104ULL,
        0x100400080208000ULL, 0x2040002120081000ULL, 0x21200680100081ULL, 0x20100080080080ULL,
        0x2000a00200410ULL, 0x20080800400ULL, 0x80088400100102ULL, 0x80004600042881ULL,
        0x4040008040800020ULL, 0x440003000200801ULL, 0x4200011004500ULL, 0x188020010100100ULL,
        0x14800401802800ULL, 0x2080040080800200ULL, 0x124080204001001ULL, 0x200046502000484ULL,
        0x480400080088020ULL, 0x1000422010034000ULL, 0x30200100110040ULL, 0x100021010009ULL,
        0x2002080100110004ULL, 0x202008004008002ULL, 0x20020004010100ULL, 0x2048440040820001ULL,
        0x101002200408200ULL, 0x40802000401080ULL, 0x4008142004410100ULL, 0x2060820c0120200ULL,
        0x1001004080100ULL, 0x20c020080040080ULL, 0x2935610830022400ULL, 0x44440041009200ULL,
        0x280001040802101ULL, 0x2100190040002085ULL, 0x80c0084100102001ULL, 0x4024081001000421ULL,
        0x20030a0244872ULL, 0x12001008414402ULL, 0x2006104900a0804ULL, 0x1004081002402ULL
    };

    static const int rookRelevantBits[64] = {
        12, 11, 11, 11, 11, 11, 11, 12,
        11, 10, 10, 10, 10, 10, 10, 11,
        11, 10, 10, 10, 10, 10, 10, 11,
        11, 10, 10, 10, 10, 10, 10, 11,
        11, 10, 10, 10, 10, 10, 10, 11,
        11, 10, 10, 10, 10, 10, 10, 11,
        11, 10, 10, 10, 10, 10, 10, 11,
        12, 11, 11, 11, 11, 11, 11, 12
    };

    // Pre-defined Magic Numbers for Bishops
    static const uint64_t bishopMagicsLocal[64] = {
        0x40040844404084ULL, 0x2004208a004208ULL, 0x10190041080202ULL, 0x108060845042010ULL,
        0x581104180800210ULL, 0x2112080446200010ULL, 0x1080820820060210ULL, 0x3c0808410220200ULL,
        0x4050404440404ULL, 0x21001420088ULL, 0x24d0080801082102ULL, 0x1020a0a020400ULL,
        0x40308200402ULL, 0x4011002100800ULL, 0x401484104104005ULL, 0x801010402020200ULL,
        0x400210c3880100ULL, 0x404022024108200ULL, 0x810018200204102ULL, 0x4002801a02003ULL,
        0x85040820080400ULL, 0x810102c808880400ULL, 0xe900410884800ULL, 0x8002020480840102ULL,
        0x220200865090201ULL, 0x2010100a02021202ULL, 0x152048408022401ULL, 0x20080002081110ULL,
        0x4001001021004000ULL, 0x800040400a011002ULL, 0xe4004081011002ULL, 0x1c004001012080ULL,
        0x8004200962a00220ULL, 0x8422100208500202ULL, 0x2000402200300c08ULL, 0x8646020080080080ULL,
        0x80020a0200100808ULL, 0x2010004880111000ULL, 0x623000a080011400ULL, 0x42008c0340209202ULL,
        0x209188240001000ULL, 0x400408a884001800ULL, 0x110400a6080400ULL, 0x1840060a44020800ULL,
        0x90080104000041ULL, 0x201011000808101ULL, 0x1a2208080504f080ULL, 0x8012020600211212ULL,
        0x500861011240000ULL, 0x180806108200800ULL, 0x4000020e01040044ULL, 0x300000261044000aULL,
        0x802241102020002ULL, 0x20906061210001ULL, 0x5a84841004010310ULL, 0x4010801011c04ULL,
        0xa010109502200ULL, 0x4a02012000ULL, 0x500201010098b028ULL, 0x8040002811040900ULL,
        0x28000010020204ULL, 0x6000020202d0240ULL, 0x8918844842082200ULL, 0x4010011029020020ULL
    };

    // Shifts for Bishop Magic Index calculation
    static const int bishopShiftsLocal[64] = {
        58, 59, 59, 59, 59, 59, 59, 58,
        59, 59, 59, 59, 59, 59, 59, 59,
        59, 59, 57, 57, 57, 57, 59, 59,
        59, 59, 57, 55, 55, 57, 59, 59,
        59, 59, 57, 55, 55, 57, 59, 59,
        59, 59, 57, 57, 57, 57, 59, 59,
        59, 59, 59, 59, 59, 59, 59, 59,
        58, 59, 59, 59, 59, 59, 59, 58
    };

    for (int i = 0; i < 64; i++) {
        int col = i % 8;
        int row = i / 8;

        // Map local magic data to class members
        this->rookMagics[i] = rookMagicsLocal[i];
        this->rookShifts[i] = 64 - rookRelevantBits[i]; 
        this->bishopMagics[i] = bishopMagicsLocal[i];
        this->bishopShifts[i] = bishopShiftsLocal[i]; 

        // Generate Leaping Piece Masks (Knight)
        uint64_t knightMoves = 0ULL;
        uint64_t kingMoves = 0ULL;
        
        if (row <= 5 && col <= 6) knightMoves |= (1ULL << (i + 17));
        if (row <= 5 && col >= 1) knightMoves |= (1ULL << (i + 15));
        if (row <= 6 && col <= 5) knightMoves |= (1ULL << (i + 10));
        if (row <= 6 && col >= 2) knightMoves |= (1ULL << (i + 6));
        if (row >= 2 && col >= 1) knightMoves |= (1ULL << (i - 17));
        if (row >= 2 && col <= 6) knightMoves |= (1ULL << (i - 15));
        if (row >= 1 && col >= 2) knightMoves |= (1ULL << (i - 10));
        if (row >= 1 && col <= 5) knightMoves |= (1ULL << (i - 6));
        knightMasks[i] = knightMoves;

        // Generate Leaping Piece Masks (King)
        if (row <= 6) kingMoves |= (1ULL << (i + 8));
        if (row >= 1) kingMoves |= (1ULL << (i - 8));
        if (col <= 6) kingMoves |= (1ULL << (i + 1));
        if (col >= 1) kingMoves |= (1ULL << (i - 1));
        if (row <= 6 && col <= 6) kingMoves |= (1ULL << (i + 9));
        if (row <= 6 && col >= 1) kingMoves |= (1ULL << (i + 7));
        if (row >= 1 && col <= 6) kingMoves |= (1ULL << (i - 7));
        if (row >= 1 && col >= 1) kingMoves |= (1ULL << (i - 9));
        kingMasks[i] = kingMoves;

        // Initialize Sliding Piece Masks (Crucial for Magic Bitboards)
        rookMasks[i] = rookMask(i);
        bishopMasks[i] = bishopMask(i);
    }

    initPawnAttacks();

    // Populate the final Attack Tables using the pre-initialized data
    initMagicTables();
}

void GenerateMoves::initMagicTables() {
    memset(rookTable, 0, sizeof(rookTable));

    for (int sq = 0; sq < 64; sq++) {
        uint64_t mask = rookMasks[sq];
        int bits = __builtin_popcountll(mask);
        int variations = 1 << bits;

        for (int i = 0; i < variations; i++) {
            uint64_t occ = setOccupancy(i, mask);
            int idx = (int)((occ * rookMagics[sq]) >> rookShifts[sq]);
            
            // Debugger for Rooks
            uint64_t actualAttacks = rookAttacksOnTheFly(sq, occ);
            if (rookTable[sq][idx] != 0ULL && rookTable[sq][idx] != actualAttacks) {
                printf("[ROOK] Collision! Sq: %d, Index: %d\n", sq, idx);
            }
            
            rookTable[sq][idx] = actualAttacks;
        }
    }

    memset(bishopTable, 0, sizeof(bishopTable));
    for(int sq = 0; sq < 64; sq++){
        uint64_t mask = bishopMasks[sq];
        int bits = __builtin_popcountll(mask);
        int variations = 1 << bits;

        for (int i = 0; i < variations; i++) {
            uint64_t occ = setOccupancy(i, mask);
            int idx = (int)((occ * bishopMagics[sq]) >> bishopShifts[sq]);

            // Debugger for Bishops
            uint64_t actualAttacks = bishopAttacksOnTheFly(sq, occ);
            if (bishopTable[sq][idx] != 0ULL && bishopTable[sq][idx] != actualAttacks) {
                printf("[BISHOP] Collision! Sq: %d, Index: %d\n", sq, idx);
            }

            bishopTable[sq][idx] = actualAttacks;
        }
    }
}

uint64_t GenerateMoves::getRookAttacks(int sq, uint64_t occupied) {
occupied &= rookMasks[sq];
    int idx = (int)((occupied * rookMagics[sq]) >> rookShifts[sq]);
    return rookTable[sq][idx];
}

void GenerateMoves::printBitBoard(uint64_t bitboard) {
    cout << "\n  a b c d e f g h\n  ----------------\n";
    for (int row = 7; row >= 0; row--) {
        cout << row + 1 << "|";
        for (int col = 0; col < 8; col++) {
            int sq = row * 8 + col;
            cout << (((bitboard >> sq) & 1ULL) ? " 1" : " .");
        }
        cout << " |" << row + 1 << "\n";
    }
    cout << "  ----------------\n  a b c d e f g h\n\n";
}

uint64_t GenerateMoves::rookMask(int sq) {
    uint64_t mask = 0ULL;
    int r = sq / 8, c = sq % 8;
    for (int i = c + 1; i <= 6; i++) mask |= (1ULL << (r * 8 + i));
    for (int i = c - 1; i >= 1; i--) mask |= (1ULL << (r * 8 + i));
    for (int i = r + 1; i <= 6; i++) mask |= (1ULL << (i * 8 + c));
    for (int i = r - 1; i >= 1; i--) mask |= (1ULL << (i * 8 + c));
    return mask;
}


uint64_t GenerateMoves::bishopMask(int sq){
    uint64_t mask = 0ULL;
    int r = sq / 8;
    int c = sq % 8;

    for (int tr = r + 1, tc = c + 1; tr <= 6 && tc <= 6; tr++, tc++) {
        mask |= (1ULL << (tr * 8 + tc));
    }
    for (int tr = r + 1, tc = c - 1; tr <= 6 && tc >= 1; tr++, tc--) {
        mask |= (1ULL << (tr * 8 + tc));
    }
    for (int tr = r - 1, tc = c + 1; tr >= 1 && tc <= 6; tr--, tc++) {
        mask |= (1ULL << (tr * 8 + tc));
    }
    for (int tr = r - 1, tc = c - 1; tr >= 1 && tc >= 1; tr--, tc--) {
        mask |= (1ULL << (tr * 8 + tc));
    }

    return mask;
}


uint64_t GenerateMoves::bishopAttacksOnTheFly(int sq, uint64_t occ){
    uint64_t attacks = 0ULL;
    int r = sq / 8;
    int c = sq % 8;

    int tr = r + 1;
    int tc = c + 1;

     while (tr <= 7 && tc <= 7) {
        uint64_t b = (1ULL << (tr * 8 + tc));
        attacks |= b;
        if (b & occ) break;
        tr++;
        tc++;
    }

    tr = r + 1;
    tc = c - 1;
    while(tr <= 7 && tc >= 0){
        uint64_t b = (1ULL << (tr * 8 + tc));
        attacks |= b;
        if (b & occ) break;
        tr++;
        tc--;
    }   

    tr = r - 1;
    tc = c + 1;
    while (tr >= 0 && tc <= 7){
        uint64_t b = (1ULL << (tr * 8 + tc));
        attacks |= b;
        if (b & occ) break;
        tr--;
        tc++;
    }

    tr = r - 1;
    tc = c - 1;
    while (tr >= 0 && tc >= 0){
        uint64_t b = (1ULL << (tr * 8 + tc));
        attacks |= b;
        if (b & occ) break;
        tr--;
        tc--;
    }
    
    return attacks;

}

uint64_t GenerateMoves::getBishopAttacks(int sq, uint64_t occupied){
    occupied &= bishopMasks[sq];
    int idx = (int)((occupied * bishopMagics[sq]) >> bishopShifts[sq]);
    
    return bishopTable[sq][idx];
}



uint64_t GenerateMoves::rookAttacksOnTheFly(int sq, uint64_t occ) {
    uint64_t attacks = 0ULL;
    int r = sq / 8, c = sq % 8;
    for (int i = c + 1; i <= 7; i++) { 
        uint64_t b = (1ULL << (r * 8 + i)); 
        attacks |= b; 
        if (b & occ) 
        
        break; 
    }
    for (int i = c - 1; i >= 0; i--) {
        uint64_t b = (1ULL << (r * 8 + i)); 
        attacks |= b; 
        if (b & occ) 
        break; 
    }
    for (int i = r + 1; i <= 7; i++) { 
        uint64_t b = (1ULL << (i * 8 + c)); 
        attacks |= b; 
        if (b & occ) 
        break; 
    }
    for (int i = r - 1; i >= 0; i--) { 
        uint64_t b = (1ULL << (i * 8 + c)); 
        attacks |= b; 
        if (b & occ) 
        break; 
    }
    return attacks;
}

uint64_t GenerateMoves::setOccupancy(int index, uint64_t mask) {
    uint64_t occ = 0ULL;
    int bits = __builtin_popcountll(mask);
    for (int i = 0; i < bits; i++) {
        int sq = __builtin_ctzll(mask);
        mask &= mask - 1;
        if (index & (1 << i)) occ |= (1ULL << sq);
    }
    return occ;
}

uint64_t GenerateMoves::getQueenAttacks(int sq, uint64_t occupied) {
    // A Queen's movement is the logical UNION of a Rook and a Bishop
    return getRookAttacks(sq, occupied) | getBishopAttacks(sq, occupied);
}

void GenerateMoves::initPawnAttacks() {
    for (int sq = 0; sq <64; sq++){
        uint64_t bit = (1ULL << sq);
        pawnMasks[0][sq] = 0ULL;

        if( bit & ~COLUMN_A) pawnMasks[0][sq] |= (bit << 7); // White Pawn captures to the left
        if( bit & ~COLUMN_H) pawnMasks[0][sq] |= (bit << 9); // White Pawn captures to the right

        pawnMasks[1][sq] = 0ULL;

        if( bit & ~COLUMN_A) pawnMasks[1][sq] |= (bit >> 9); // Black Pawn captures to the left
        if( bit & ~COLUMN_H) pawnMasks[1][sq] |= (bit >> 7); // Black Pawn captures to the right    


    }
}