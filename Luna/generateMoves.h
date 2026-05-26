#ifndef GENERATEMOVES_H
#define GENERATEMOVES_H
#include "board.h"
#include "types.h"
#include <chrono>
#include <cstdint>
#include <iostream>

using namespace std;

class Board;

class GenerateMoves {
public:
  static const uint64_t COLUMN_A = 0x0101010101010101ULL;
  static const uint64_t COLUMN_H = 0x8080808080808080ULL;
  static const uint64_t ROW_1 = 0x00000000000000FFULL;
  static const uint64_t ROW_2 = 0x000000000000FF00ULL;
  static const uint64_t ROW_7 = 0x00FF000000000000ULL;
  static const uint64_t ROW_8 = 0xFF00000000000000ULL;
  uint8_t searchAge = 0;
  // History table: [from][to]
  int historyTable[4096];

  bool searchAborted = false;
  uint64_t nodesSearched = 0;

  long long timeLimitMs; // Make sure this is declared here
  std::chrono::time_point<std::chrono::high_resolution_clock> searchStartTime;

void checkTimeBudget()
{
    auto now = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        now - searchStartTime
    ).count();

    if (elapsed >= timeLimitMs)
        searchAborted = true;
}

  // Helper to get current time in ms
  long long getCurrentTimeMs() {
    auto now = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(
               now.time_since_epoch())
        .count();
  }

  static const int TT_SIZE = 1 << 21;
  static TTEntry transpositionTable[TT_SIZE];
  Move killerMoves[32][2];

  // Precomputed move masks for each piece type and square
  uint64_t knightMasks[64];
  uint64_t kingMasks[64];
  uint64_t rookMasks[64];
  uint64_t bishopMasks[64];

  // pawnFromMasks: attack squares FROM a pawn on [side][sq]
  // pawnToMasks: attack squares TO a square by a pawn of [side]
  uint64_t pawnFromMasks[2][64];
  uint64_t pawnToMasks[2][64];

  // Magic bitboards for rook attacks
  uint64_t rookMagics[64];

  // Magic bitboards for bishop attacks
  uint64_t bishopMagics[64];

  // Attack tables for rook moves
  static uint64_t rookTable[64][4096];
  int rookShifts[64];

  // Attack tables for bishop moves
  static uint64_t bishopTable[64][512];
  int bishopShifts[64];

  void init();
  void printBitBoard(uint64_t bitboard);
  void initMagicTables();
  void ageHistory();

  // ROOK
  uint64_t rookMask(int sq);
  uint64_t rookAttacksOnTheFly(int sq, uint64_t occupied);
  uint64_t getRookAttacks(int sq, uint64_t occupied) const;

  // Occupancy
  uint64_t setOccupancy(int index, uint64_t mask);

  // BISHOP
  uint64_t bishopMask(int sq);
  uint64_t bishopAttacksOnTheFly(int sq, uint64_t occupied);
  uint64_t getBishopAttacks(int sq, uint64_t occupied) const;

  // QUEEN
  uint64_t getQueenAttacks(int sq, uint64_t occupied) const;

  // KING
  void generateKingMoves(int sq, int side, const Board &board, MoveList &list);

  // Pawns
  void initPawnAttacks();
  void generatePawnMoves(int sq, int side, uint64_t occupied,
                         uint64_t opponentPieces, MoveList &list,
                         int enPassantSq);

  // Leapers (Knight and King)
  void generateLeapingMoves(int sq, PieceType type, uint64_t friendlyPieces,
                            MoveList &list);
  void generateSlidingMoves(int sq, PieceType type, uint64_t occupied,
                            uint64_t friendlyPieces, MoveList &list);

  // Attack Detection
  bool isSquareAttacked(int sq, int attackerColor, const Board &board);

  void generateAllMoves(const Board &board, int side, MoveList &list);
  MoveList generateLegalMoves(Board &board, int side);
  bool isInCheck(const Board &board, int side);
  bool isCheckmate(Board &board, int side);
  bool isStalemate(Board &board, int side);
  Move getBestMove(Board &board, int maxDepth, long long myTimeLeftMs = 5000,
                   long long incrementMs = 0, int movesToGo = 40);
  void orderMoves(MoveList& moves, Board& board, int ply, Move ttMove);
  void orderCaptures(MoveList &list, const Board &board);

  int negamax(Board &board, int depth, int alpha, int beta, int ply);
  int quiescence(Board& board, int alpha, int beta, int ply);
  void generateCaptures(const Board &board, int side, MoveList &list);

  // SEE — Static Exchange Evaluation
  int see(const Board& board, int toSq, int target, int fromSq, int atter) const;
  int seeCapture(const Board& board, Move move) const;

  // Mobility evaluation (called from board evaluate via friend or passed in)
  int evalMobility(const Board& board) const;
  int evalKingSafety(const Board& board) const;
  int evalFull(Board& board) const;
};

#endif