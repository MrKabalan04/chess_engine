#include "uci.h"
#include <algorithm>
#include <iostream>
#include <sstream>
#include <vector>

using namespace std;

// Forward declarations of helpers defined in main.cpp to prevent linker errors
int parseSquare(char file, char rank);
string sq(int s);

// Main input command monitoring framework loop
void UCI::uciLoop(Board &board, GenerateMoves &gen) {
  string line;

  // Clear status flags for standard input streams
  cin.clear();

  while (getline(cin, line)) {
    if (line.empty())
      continue;

    stringstream ss(line);
    string command;
    ss >> command;

    if (command == "uci") {
      // Identify engine properties to the host GUI software thread
      cout << "id name Luna\n";
      cout << "id author Mr Kabalan\n";
      cout << "uciok\n" << flush;
    } else if (command == "isready") {
      cout << "readyok\n" << flush;
    } else if (command == "ucinewgame") {
      board.init();
    } else if (command == "position") {
      parsePosition(line, board, gen);
    } else if (command == "go") {
      parseGo(line, board, gen);
    } else if (command == "quit") {
      break;
    }
  }
}

// Parses layout positions (e.g. "position startpos moves e2e4 e7e5")
void UCI::parsePosition(const string &input, Board &board, GenerateMoves &gen) {
  stringstream ss(input);
  string token;
  ss >> token; // consume "position"

  ss >> token;
  if (token == "startpos") {

    board.init();
  }

  // Skip tokens until we reach the "moves" list
  while (ss >> token) {
    if (token == "moves")
      break;
  }

  while (ss >> token) {
    MoveList legalMoves = gen.generateLegalMoves(board, board.sideToMove);
    bool found = false;

    for (int i = 0; i < legalMoves.count; i++) {
      Move m = legalMoves.moves[i];

      // Build the UCI string
      string moveStr = sq(m.getFrom()) + sq(m.getTo());

      if (m.getType() >= PROMOT_QUEEN) {
        if (m.getType() == PROMOT_QUEEN)
          moveStr += "q";
        else if (m.getType() == PROMOT_ROOK)
          moveStr += "r";
        else if (m.getType() == PROMOT_BISHOP)
          moveStr += "b";
        else if (m.getType() == PROMOT_KNIGHT)
          moveStr += "n";
      }

      if (token == moveStr) {
        board.makeMove(m);
        found = true;
        break;
      }

      // Move the debug print inside the loop to see what it's comparing against
      // std::cout << "DEBUG: Comparing " << token << " vs " << moveStr <<
      // std::endl;
    }

    if (!found) {
      std::cout << "info string Warning: Illegal/Unknown move: " << token
                << std::endl;
    }
  }
}

// Parses calculation clocks (e.g. "go wtime 300000 btime 280000 winc 2000 binc
// 2000")
void UCI::parseGo(const string &input, Board &board, GenerateMoves &gen) {
  stringstream ss(input);
  string token;

  long long wtime = -1, btime = -1;
  long long winc = 0, binc = 0;
  int depth = 10; // Fallback depth search cap
  int movesToGo = 40;

  while (ss >> token) {
    if (token == "wtime")
      ss >> wtime;
    else if (token == "btime")
      ss >> btime;
    else if (token == "winc")
      ss >> winc;
    else if (token == "binc")
      ss >> binc;
    else if (token == "depth")
      ss >> depth;
    else if (token == "movestogo")
      ss >> movesToGo;
  }

  // Allocate targeted timing windows relative to active side color
  long long myTime = (board.sideToMove == 0) ? wtime : btime;
  long long myInc = (board.sideToMove == 0) ? winc : binc;

  // Execute search routine utilizing our time management system
  Move bestMove = gen.getBestMove(board, depth, myTime, myInc, movesToGo);

  // Format special promotion indicators if necessary
  string promoSuffix = "";
  if (bestMove.getType() == PROMOT_QUEEN)
    promoSuffix = "q";
  if (bestMove.getType() == PROMOT_ROOK)
    promoSuffix = "r";
  if (bestMove.getType() == PROMOT_BISHOP)
    promoSuffix = "b";
  if (bestMove.getType() == PROMOT_KNIGHT)
    promoSuffix = "n";

  // Transmit computed move back up to host manager thread loop
  cout << "bestmove " << sq(bestMove.getFrom()) << sq(bestMove.getTo())
       << promoSuffix << endl
       << flush;
}
