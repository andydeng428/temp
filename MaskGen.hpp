#ifndef MASKGEN_HPP
#define MASKGEN_HPP

#include <cstdint>

// Declare the attack masks for pawns, knights, kings, bishops, and rooks
extern uint64_t pawnAttacks[2][64];
extern uint64_t knightAttacks[64];
extern uint64_t kingAttacks[64];
extern uint64_t bishopAttacks[64][512];
extern uint64_t rookAttacks[64][4096];
extern uint64_t bishopMasks[64];
extern uint64_t rookMasks[64];

// File masks
extern const uint64_t notInAFile;
extern const uint64_t notInHFile;
extern const uint64_t notInABFile;
extern const uint64_t notInHGFile;

// Relevancy masks for bishops and rooks
extern const int bishopRelevantBit[64];
extern const int rookRelevantBit[64];

// Function declarations
uint64_t maskPawnAttacks(int side, int square);
uint64_t maskKnightAttacks(int square);
uint64_t maskKingAttacks(int square);
uint64_t maskBishopAttacks(int square);
uint64_t maskRookAttacks(int square);
uint64_t bishopAttacksOnTheFly(int square, uint64_t block);
uint64_t rookAttacksOnTheFly(int square, uint64_t block);
uint64_t setOccupancy(int index, int bitsInMask, uint64_t attackMask);
void initLeaperAttacks();
uint64_t getBishopAttacks(int square, uint64_t occupancy);
uint64_t getRookAttacks(int square, uint64_t occupancy);
uint64_t getQueenAttacks(int square, uint64_t occupancy);
void initSliderAttacks(int bishop);
int isSquareAttacked(const BoardState& board, int square, int side);

#endif // MASKGEN_HPP
