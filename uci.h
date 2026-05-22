#ifndef UCI_H
#define UCI_H

#include "types.h"
#include <string>

std::string moveToString(Move m);
Move parseUCIMove(const std::string& moveStr, const MoveList& legalMoves);

#endif