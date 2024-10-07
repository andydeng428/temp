#ifndef BITBOARD_HPP
#define BITBOARD_HPP

#include <cstdint>
#include <unordered_map>
#include <string>

// Useful Debug positions
#define emptyBoard "8/8/8/8/8/8/8/8 w - - "
#define startPosition "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 "
#define trickyPosition "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1 "
#define killerPosition "rnbqkb1r/pp1p1pPp/8/2p1pP2/1P1P4/3P3P/P1P1P3/RNBQKBNR w KQkq e6 0 1"
#define cmkPosition "r2q1rk1/ppp2ppp/2n1bn2/2b1p3/3pP3/3P1NPP/PPP1NPB1/R1BQ1RK1 b - - 0 9 "
#define repetitions "2r3k1/R7/8/1R6/8/8/P4KPP/8 w - - 0 40 "


enum {
    a8, b8, c8, d8, e8, f8, g8, h8,
    a7, b7, c7, d7, e7, f7, g7, h7,
    a6, b6, c6, d6, e6, f6, g6, h6,
    a5, b5, c5, d5, e5, f5, g5, h5,
    a4, b4, c4, d4, e4, f4, g4, h4,
    a3, b3, c3, d3, e3, f3, g3, h3,
    a2, b2, c2, d2, e2, f2, g2, h2,
    a1, b1, c1, d1, e1, f1, g1, h1, no_sq
};

enum {
    white,
    black, 
    both
};

enum { P, N, B, R, Q, K, p, n, b, r, q, k };

/*
0001 1 white king can castle to king side
0010 2 white king can castle to queen side
0100 4 black king can castle to king side
1000 8 black king can castle to queen side */
enum { wk = 1, wq = 2, bk = 4, bq = 8 };

enum {
    rook,
    bishop
};

#define getBit(bitBoard, square) (bitBoard & (1ULL << (square)))
#define setBit(bitBoard, square) (bitBoard |= (1ULL << (square)))
#define popBit(bitBoard, square) ((bitBoard) &= ~(1ULL << (square)))


struct BoardState{
    uint64_t bitBoard[12] = {0};
    uint64_t occupancies[3] = {0};
    int side = white;
    int enPassant = no_sq;
    int castle = 0;
    uint64_t hashKey = 0;

    //BoardState(const BoardState& other){}
};
extern BoardState board;

// half move counter
extern int ply;

// repetition table
extern uint64_t repetitionTable[1000];

// repetition index
extern int repetitionIndex;

static inline int countBits(uint64_t bitBoard) {
    return __builtin_popcountll(bitBoard);
}

static inline int getLeastSigBitIndex(uint64_t bitBoard){
    if (bitBoard == 0) return -1;
    return __builtin_ctzll(bitBoard);
}

#endif // BITBOARD_HPP