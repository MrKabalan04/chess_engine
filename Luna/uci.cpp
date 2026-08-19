#include "uci.h"
#include <algorithm>
#include <iostream>
#include <sstream>
#include <vector>
#include <cstring>

using namespace std;

// Forward declarations of helpers defined in main.cpp to prevent linker errors
int parseSquare(char file, char rank);
string sq(int s);

void UCI::uciLoop(Board &board, GenerateMoves &gen) {
  string line;
  cin.clear();

  // Build opening book once at startup
  if (!gen.bookLoaded)
      gen.buildBook(board);

  while (getline(cin, line)) {
    if (line.empty()) continue;
    stringstream ss(line);
    string command;
    ss >> command;

    if (command == "uci") {
      cout << "id name Luna\n";
      cout << "id author Mr Kabalan\n";
      cout << "uciok\n" << flush;
    } else if (command == "isready") {
      cout << "readyok\n" << flush;
    } else if (command == "eval") {
      cout << "evalFull=" << gen.evalFull(board)
           << " evaluate=" << board.evaluate() << "\n" << flush;
    } else if (command == "ucinewgame") {
      board.init();
      gen.gamePly = 0;
      gen.searchAge = 0;
      memset(gen.transpositionTable, 0, sizeof(gen.transpositionTable));
      memset(gen.historyTable,       0, sizeof(gen.historyTable));
      memset(gen.contHistTable,      0, sizeof(gen.contHistTable));
      memset(gen.countermoveTable,   0, sizeof(gen.countermoveTable));
    } else if (command == "position") {
      parsePosition(line, board, gen);
    } else if (command == "go") {
      parseGo(line, board, gen);
    } else if (command == "quit") {
      break;
    }
  }
}

void UCI::parsePosition(const string &input, Board &board, GenerateMoves &gen) {
  stringstream ss(input);
  string token;
  ss >> token; // consume "position"

  ss >> token;
  if (token == "startpos") {
    board.init();
    gen.gamePly = 0;
  } else if (token == "fen") {
    // Collect up to 6 FEN fields (stop at "moves")
    string fenStr;
    int fields = 0;
    while (ss >> token && token != "moves" && fields < 6) {
      if (!fenStr.empty()) fenStr += ' ';
      fenStr += token;
      fields++;
    }
    if (!board.initFromFen(fenStr)) {
      std::cout << "info string Warning: Invalid FEN: " << fenStr << std::endl;
      board.init();
    }
    gen.gamePly = 0;
    // If we stopped at "moves", fall through; otherwise skip to moves below
    if (token != "moves") {
      // No moves section — update gamePly and return
      gen.gamePly = board.historyCount - 1;
      return;
    }
    goto parse_moves; // token is already "moves"
  }

  while (ss >> token)
    if (token == "moves") break;

parse_moves:
  while (ss >> token) {
    MoveList legalMoves = gen.generateLegalMoves(board, board.sideToMove);
    bool found = false;
    for (int i = 0; i < legalMoves.count; i++) {
      Move m = legalMoves.moves[i];
      string moveStr = sq(m.getFrom()) + sq(m.getTo());
      if (m.getType() >= PROMOT_QUEEN) {
        if (m.getType() == PROMOT_QUEEN)  moveStr += "q";
        else if (m.getType() == PROMOT_ROOK)   moveStr += "r";
        else if (m.getType() == PROMOT_BISHOP) moveStr += "b";
        else if (m.getType() == PROMOT_KNIGHT) moveStr += "n";
      }
      if (token == moveStr) {
        board.makeMove(m);
        found = true;
        break;
      }
    }
    if (!found)
      std::cout << "info string Warning: Illegal/Unknown move: " << token << std::endl;
  }
  // Update gamePly to reflect total moves played in this game
  gen.gamePly = board.historyCount - 1;
}

void UCI::parseGo(const string &input, Board &board, GenerateMoves &gen) {
  stringstream ss(input);
  string token;

  long long wtime = -1, btime = -1;
  long long winc = 0, binc = 0;
  long long movetime = -1;
  int depth = 64;  // high cap — time management will stop us
  int movesToGo = 0;

  while (ss >> token) {
    if (token == "wtime")      ss >> wtime;
    else if (token == "btime") ss >> btime;
    else if (token == "winc")  ss >> winc;
    else if (token == "binc")  ss >> binc;
    else if (token == "depth") ss >> depth;
    else if (token == "movestogo") ss >> movesToGo;
    else if (token == "movetime") ss >> movetime;
    else if (token == "infinite") depth = 64;
  }

  long long myTime = (board.sideToMove == 0) ? wtime : btime;
  long long myInc  = (board.sideToMove == 0) ? winc  : binc;

  // If no clock info at all, use a 5-second default
  if (myTime < 0 && movetime < 0) myTime = 5000;

  Move bestMove = gen.getBestMove(board, depth, myTime, myInc, movesToGo,
                                  movetime > 0 ? movetime : -1);

  string promoSuffix = "";
  if (bestMove.getType() == PROMOT_QUEEN)  promoSuffix = "q";
  if (bestMove.getType() == PROMOT_ROOK)   promoSuffix = "r";
  if (bestMove.getType() == PROMOT_BISHOP) promoSuffix = "b";
  if (bestMove.getType() == PROMOT_KNIGHT) promoSuffix = "n";

  // UCI: "0000" means no legal move (checkmate / stalemate). The engine's
  // null Move(0,0,NORMAL) would otherwise print "a1a1" and hang GUIs.
  if (bestMove.getFrom() == 0 && bestMove.getTo() == 0 && promoSuffix == "")
    cout << "bestmove 0000" << endl << flush;
  else
    cout << "bestmove " << sq(bestMove.getFrom()) << sq(bestMove.getTo())
         << promoSuffix << endl << flush;
}