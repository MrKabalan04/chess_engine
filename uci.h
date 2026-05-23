#ifndef UCI_H
#define UCI_H

#include "board.h"
#include "generateMoves.h"
#include <string>

class UCI {
public:
    // Main processing loop designed to read standard GUI input pipes
    void uciLoop(Board& board, GenerateMoves& gen);

private:
    // Helper handlers to parse specialized sub-commands cleanly
    void parsePosition(const std::string& input, Board& board, GenerateMoves& gen);
    void parseGo(const std::string& input, Board& board, GenerateMoves& gen);
};

#endif
