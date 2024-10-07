#ifndef UTIL_HPP
#define UTIL_HPP

#include <iostream>
#include <cstdint>
#include "MoveGen.hpp"

extern const std::string squareToCoordinates[];

extern const std::string asciiPieces;

extern const std::string unicodePieces[12];

extern const std::unordered_map<char, int> charPieces;

extern const std::unordered_map<int, char> promotedPieces;

extern unsigned int randomState;

unsigned int getRandomU32BitNumber();
uint64_t getRandomU64BitNumber();
uint64_t generateMagicNumber();

int getTimeMs();

void printBoard();
void printBitboard(uint64_t bitBoard);
void printMove(int move);
void printMoveList(Moves *moveList);
void printAttackedSquares(BoardState& board, int side);

void parseFen(std::string fenC);
void printMoveScores(Moves *moveList);


#endif // INPUTOUTPUT_HPP