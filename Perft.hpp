#ifndef PERFT_HPP
#define PERFT_HPP
#include "MoveGen.hpp"

int getTimeMS();
void perftDriver(BoardState& board, int depth);
void perftTest (BoardState& board, int depth);

#endif // PERFT_HPP
