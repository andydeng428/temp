#ifndef MOVEGEN_HPP
#define MOVEGEN_HPP

#include "BitBoard.hpp"
#include <cstring>

// Move encoding and decoding macros
#define encodeMove(source, target, piece, promoted, capture, double, enPassant, castling) \
    ((source) | ((target) << 6) | ((piece) << 12) | ((promoted) << 16) | ((capture) << 20) | ((double) << 21) | ((enPassant) << 22) | ((castling) << 23))

#define getMoveSource(move) (move & 0x3f)
#define getMoveTarget(move) ((move & 0xfc0) >> 6)
#define getMovePiece(move) ((move & 0xf000) >> 12)
#define getMovePromoted(move) ((move & 0xf0000) >> 16)
#define getMoveCapture(move) (move & 0x100000)
#define getMoveDouble(move) (move & 0x200000)
#define getMoveEnPassant(move) (move & 0x400000)
#define getMoveCastling(move) (move & 0x800000)

// Move list structure
struct Moves {
    int moves[256];
    int count;
};

// Declare the move list as an external variable
extern struct Moves moveList;

// Board state backup macros
#define copyBoard()  \
    BoardState boardCopy = board; \

#define restoreBoard() \
    board = boardCopy; \

// Enum for move types
enum { allMoves, onlyCaptures };

// Declare castling rights as an external constant array
extern const int castlingRights[64];

// Function declarations
int parseMove(char *moveString);
void addMove(Moves *moveList, int move);
int makeMove(BoardState & board, int move, int moveFlag);
void generateMove(const BoardState & board, Moves *moveList);

#endif // MOVEGEN_HPP
