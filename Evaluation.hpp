#ifndef EVALUATION_HPP
#define EVALUATION_HPP

#include <cstdint>
#include "BitBoard.hpp"

// Material score
extern int materialScore[12];

// Positional scores
extern const int pawn_score[64];
extern const int knight_score[64];
extern const int bishop_score[64];
extern const int rook_score[64];
extern const int king_score[64];
extern const int mirror_score[128];

// File and rank masks
extern uint64_t fileMasks[64];
extern uint64_t rankMasks[64];

// Pawn masks
extern uint64_t isolatedPawnMasks[64];
extern uint64_t whitePassedPawnMasks[64];
extern uint64_t blackPassedPawnMasks[64];

// Rank extraction
extern const int getRank[64];

// Penalty and bonus constants
extern const int doublePawnPenalty;
extern const int isolatedPawnPenalty;
extern const int passedPawnBonus[8];
extern const int semiOpenFileScore;
extern const int openFileScore;
extern const int kingShieldBonus;

// Function declarations
uint64_t setFileRankMask(int fileNumber, int rankNumber);
void initEvaluationMasks();
int evaluate(const BoardState& board);

#endif // EVALUATION_HPP
