#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <algorithm>
#include "board.h"
#include "generateMoves.h"
#include "types.h"

using namespace std;

// Structure to hold our benchmark test cases
struct PerftTestCase {
    string name;
    int depth;
    uint64_t expectedNodes;
};

// ============================================================================
// UI & TESTING HELPERS
// ============================================================================

// Converts a square index (0-63) to chess notation (e.g., 12 -> "e2")
string sq(int s)
{
    string f = "abcdefgh";
    string r = "12345678";
    return string(1, f[s % 8]) + r[s / 8];
}

// Converts a string coordinate (like "e2") back into an integer index (0-63)
int parseSquare(char file, char rank)
{
    int col = file - 'a';   // a-h → 0-7
    int row = rank - '1';   // 1-8 → 0-7
    return row * 8 + col;
}

// Renders the board state visually in the terminal console
void printBoard(const Board& b)
{
    cout << "\n   +-----------------+\n";
    for (int r = 7; r >= 0; r--)
    {
        cout << r + 1 << "  | ";
        for (int f = 0; f < 8; f++)
        {
            int s = r * 8 + f;
            char c = '.';

            // White Pieces (Uppercase)
            if      (b.whitePawns   & (1ULL << s)) c = 'P';
            else if (b.whiteKnights & (1ULL << s)) c = 'N';
            else if (b.whiteBishops & (1ULL << s)) c = 'B';
            else if (b.whiteRooks   & (1ULL << s)) c = 'R';
            else if (b.whiteQueen   & (1ULL << s)) c = 'Q';
            else if (b.whiteKing    & (1ULL << s)) c = 'K';
            // Black Pieces (Lowercase)
            else if (b.blackPawns   & (1ULL << s)) c = 'p';
            else if (b.blackKnights & (1ULL << s)) c = 'n';
            else if (b.blackBishops & (1ULL << s)) c = 'b';
            else if (b.blackRooks   & (1ULL << s)) c = 'r';
            else if (b.blackQueen   & (1ULL << s)) c = 'q';
            else if (b.blackKing    & (1ULL << s)) c = 'k';

            cout << c << " ";
        }
        cout << "|\n";
    }
    cout << "   +-----------------+\n";
    cout << "     a b c d e f g h\n\n";
}

// Recursive Perft node counter
uint64_t runUltimatePerft(int depth, Board& board, GenerateMoves& gen) {
    if (depth == 0) return 1ULL;

    MoveList pseudoMoves;
    gen.generateAllMoves(board, board.sideToMove, pseudoMoves);
    uint64_t nodes = 0;

    for (int i = 0; i < pseudoMoves.count; i++) {
        board.makeMove(pseudoMoves.moves[i]);
        
        int kingSq = (board.sideToMove == 1) 
            ? __builtin_ctzll(board.whiteKing) 
            : __builtin_ctzll(board.blackKing);

        if (!gen.isSquareAttacked(kingSq, board.sideToMove, board)) {
            nodes += runUltimatePerft(depth - 1, board, gen);
        }
        board.undoMove();
    }
    return nodes;
}

// ============================================================================
// MAIN GAME LOOP WITH TIME CONTROL
// ============================================================================
int main() {
    GenerateMoves gen;
    gen.init();
    Board board;

    // Standard Official Chess Engine Test Benchmarks
    vector<PerftTestCase> tests = {
        { "Start Position - Depth 1", 1, 20ULL },
        { "Start Position - Depth 2", 2, 400ULL },
        { "Start Position - Depth 3", 3, 8902ULL },
        { "Start Position - Depth 4", 4, 197281ULL },
        { "Start Position - Depth 5", 5, 4865609ULL }
    };

    cout << "====================================================\n";
    cout << "   RUNNING ULTIMATE CHESS ENGINE INTEGRITY SUITE   \n";
    cout << "====================================================\n\n";

    bool systemStable = true;

    for (const auto& test : tests) {
        cout << "[TEST] Running: " << test.name << "..." << endl;
        board.init(); 

        auto start = chrono::high_resolution_clock::now();
        uint64_t resultNodes = runUltimatePerft(test.depth, board, gen);
        auto end = chrono::high_resolution_clock::now();
        
        chrono::duration<double> elapsed = end - start;

        if (resultNodes == test.expectedNodes) {
            cout << "   ✅ PASSED! Nodes: " << resultNodes << " | Time: " << elapsed.count() << "s" << endl;
            double nps = (double)resultNodes / elapsed.count() / 1000000.0;
            cout << "   Speed: " << nps << " MNPS\n\n";
        } else {
            cout << "   ❌ FAILED! Expected: " << test.expectedNodes << " | Got: " << resultNodes << "\n";
            systemStable = false;
            break; 
        }
    }

    cout << "====================================================\n";
    if (systemStable) {
        cout << "  🎉 SYSTEM 100% STABLE: Move generation is flawless!\n";
        cout << "  Ready to implement advanced Time Management.\n";
    } else {
        cout << "  ⚠️ FAULT DETECTED: Fix move generator mismatches before playing.\n";
        return -1;
    }
    cout << "====================================================\n\n";

    // Initialize board state layout for user gameplay session
    board.init();

    // Setup Match Time Controls (5 Minutes starting bank + 2-second increment per turn)
    long long whiteTimeRemainingMs = 300000; 
    long long blackTimeRemainingMs = 300000; 
    long long incrementMs = 2000;            

    int moveCounter = 1;

    while (true)
    {
        board.validateBoard();
        printBoard(board);

        // 1. Gather all legal moves
        MoveList legal = gen.generateLegalMoves(board, board.sideToMove);
        if (board.isThreefoldRepetition())
        {
            cout << "🏁 GAME OVER: Draw by Threefold repetition!\n";
            break;
        }

        // 2. Victory and Draw state validation filters
        if (legal.count == 0)
        {
            if (gen.isCheckmate(board, board.sideToMove))
            {
                cout << "🏁 GAME OVER: Checkmate! "
                     << (board.sideToMove == 0 ? "Black" : "White")
                     << " wins the match!\n";
            }
            else
            {
                cout << "🏁 GAME OVER: Draw by Stalemate!\n";
            }
            break;
        }

        // ====================================================================
        // HUMAN PLAYER TURN (WHITE)
        // ====================================================================
        if (board.sideToMove == 0)
        {
            cout << "Move #" << moveCounter << " [White to move (" << (whiteTimeRemainingMs / 1000) << "s left)]: ";
            
            auto turnStart = chrono::high_resolution_clock::now();
            string moveStr;
            if (!(cin >> moveStr))
                break;

            auto turnEnd = chrono::high_resolution_clock::now();
            long long timeSpent = chrono::duration_cast<chrono::milliseconds>(turnEnd - turnStart).count();
            
            // Deduct human decision time overhead and apply incremental padding
            whiteTimeRemainingMs -= timeSpent;
            whiteTimeRemainingMs += incrementMs;

            transform(moveStr.begin(), moveStr.end(), moveStr.begin(), ::tolower);
            if (moveStr == "quit" || moveStr == "exit") break;

            if (moveStr.length() != 4)
            {
                cout << "⚠️ Invalid format! Use notation structure like 'e2e4'.\n";
                continue;
            }

            int from = parseSquare(moveStr[0], moveStr[1]);
            int to   = parseSquare(moveStr[2], moveStr[3]);

            bool found = false;
            for (int i = 0; i < legal.count; i++)
            {
                Move m = legal.moves[i];
                if (m.getFrom() == from && m.getTo() == to)
                {
                    cout << " You played: " << sq(m.getFrom()) << " -> " << sq(m.getTo()) << endl;
                    board.makeMove(m);
                    found = true;
                    break;
                }
            }

            if (!found)
            {
                cout << "❌ That movement choice is illegal! Try again.\n";
                continue;
            }
        }

        // ====================================================================
        // TIME-MANAGED AI ENGINE TURN (BLACK)
        // ====================================================================
        else
        {
            cout << "Move #" << moveCounter << " [Black Engine is thinking...]\n";
            auto engineStart = chrono::high_resolution_clock::now();

            // Pass our newly configured time parameters directly down to our search tree
            Move bestMove = gen.getBestMove(board, 10, blackTimeRemainingMs, incrementMs);

            auto engineEnd = chrono::high_resolution_clock::now();
            long long timeSpent = chrono::duration_cast<chrono::milliseconds>(engineEnd - engineStart).count();
            
            // Deduct thinking cost and apply incremental turn rewards
            blackTimeRemainingMs -= timeSpent;
            blackTimeRemainingMs += incrementMs;

            // Security evaluation validation check
            bool valid = false;
            for (int i = 0; i < legal.count; i++)
            {
                if (legal.moves[i].getFrom() == bestMove.getFrom() &&
                    legal.moves[i].getTo()   == bestMove.getTo())
                {
                    valid = true;
                    bestMove = legal.moves[i]; 
                    break;
                }
            }

            if (!valid)
            {
                cout << "⚠️ CRITICAL ENGINE FAULT: Engine calculated an invalid legal move path. Stopping.\n";
                break;
            }

            cout << "🤖 Engine played: " << sq(bestMove.getFrom()) << " -> " << sq(bestMove.getTo())
                 << " | Clock Left: " << (blackTimeRemainingMs / 1000) << "s\n";

            board.makeMove(bestMove);
            moveCounter++;
        }
    }
    return 0;
}