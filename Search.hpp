#ifndef SEARCH_HPP
#define SEARCH_HPP

#include <cstdint>
#include "MoveGen.hpp"

extern uint64_t nodes;

#define mateValue 49000
#define mateScore 48000
#define INF 50000
#define maxPly 64

// Static tables, should be moved to .cpp
extern int mvv_lva[12][12];
extern int killerMoves[2][maxPly];
extern int historyMoves[12][64];
extern int pvLength[maxPly];
extern int pvTable[maxPly][maxPly];
extern int followPV, scorePV;

void enablePVScoring(Moves* moveList);
int scoreMove(const BoardState& board, int move);
void sortMoves(BoardState& board, Moves *moveList);
int isRepetition(const BoardState& board);
int quiescence(BoardState& board, int alpha, int beta);
int negamax(BoardState& board, int alpha, int beta, int depth);
void searchPosition(BoardState& board, int depth);

#endif // SEARCH_HPP
