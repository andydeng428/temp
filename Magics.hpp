#ifndef MAGICS_HPP
#define MAGICS_HPP

#include <cstdint>
#include <iostream>

// Function declarations
uint64_t findMagicNumber(int square, int relevantBits, int bishops);
void initMagicNumbers();

// Magic number declarations for bishops and rooks
extern const uint64_t bishopMagicNumbers[64];
extern const uint64_t rookMagicNumbers[64];

#endif // MAGICS_HPP
