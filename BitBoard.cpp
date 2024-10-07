#include "BitBoard.hpp"
#include <cstdint>

BoardState board;

int ply = 0;
uint64_t repetitionTable[1000] = {0};
int repetitionIndex = 0;
