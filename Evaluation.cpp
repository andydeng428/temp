#include "Evaluation.hpp"
#include "BitBoard.hpp"
#include "MaskGen.hpp"
#include <cstdint>

int materialScore[12] = {
    100,      // white pawn score
    300,      // white knight score
    350,      // white bishop score
    500,      // white rook score
    1000,     // white queen score
    10000,    // white king score
    -100,     // black pawn score
    -300,     // black knight score
    -350,     // black bishop score
    -500,     // black rook score
    -1000,    // black queen score
    -10000    // black king score
};

// Positional scores
const int pawn_score[64] = {
    90,  90,  90,  90,  90,  90,  90,  90,
    30,  30,  30,  40,  40,  30,  30,  30,
    20,  20,  20,  30,  30,  30,  20,  20,
    10,  10,  10,  20,  20,  10,  10,  10,
     5,   5,  10,  20,  20,   5,   5,   5,
     0,   0,   0,   5,   5,   0,   0,   0,
     0,   0,   0, -10, -10,   0,   0,   0,
     0,   0,   0,   0,   0,   0,   0,   0
};

const int knight_score[64] = {
    -5,   0,   0,   0,   0,   0,   0,  -5,
    -5,   0,   0,  10,  10,   0,   0,  -5,
    -5,   5,  20,  20,  20,  20,   5,  -5,
    -5,  10,  20,  30,  30,  20,  10,  -5,
    -5,  10,  20,  30,  30,  20,  10,  -5,
    -5,   5,  20,  10,  10,  20,   5,  -5,
    -5,   0,   0,   0,   0,   0,   0,  -5,
    -5, -10,   0,   0,   0,   0, -10,  -5
};

const int bishop_score[64] = {
     0,   0,   0,   0,   0,   0,   0,   0,
     0,   0,   0,   0,   0,   0,   0,   0,
     0,  20,   0,  10,  10,   0,  20,   0,
     0,   0,  10,  20,  20,  10,   0,   0,
     0,   0,  10,  20,  20,  10,   0,   0,
     0,  10,   0,   0,   0,   0,  10,   0,
     0,  30,   0,   0,   0,   0,  30,   0,
     0,   0, -10,   0,   0, -10,   0,   0
};

const int rook_score[64] = {
    50,  50,  50,  50,  50,  50,  50,  50,
    50,  50,  50,  50,  50,  50,  50,  50,
     0,   0,  10,  20,  20,  10,   0,   0,
     0,   0,  10,  20,  20,  10,   0,   0,
     0,   0,  10,  20,  20,  10,   0,   0,
     0,   0,  10,  20,  20,  10,   0,   0,
     0,   0,  10,  20,  20,   0,   0,   0,
     0,   0,   0,  20,  20,   0,   0,   0
};

const int king_score[64] = {
     0,   0,   0,   0,   0,   0,   0,   0,
     0,   0,   5,   5,   5,   5,   0,   0,
     0,   5,   5,  10,  10,   5,   5,   0,
     0,   5,  10,  20,  20,  10,   5,   0,
     0,   5,  10,  20,  20,  10,   5,   0,
     0,   0,   5,  10,  10,   5,   0,   0,
     0,   5,   5,  -5,  -5,   0,   5,   0,
     0,   0,   5,   0, -15,   0,  10,   0
};

const int mirror_score[128] = {
    a1, b1, c1, d1, e1, f1, g1, h1,
    a2, b2, c2, d2, e2, f2, g2, h2,
    a3, b3, c3, d3, e3, f3, g3, h3,
    a4, b4, c4, d4, e4, f4, g4, h4,
    a5, b5, c5, d5, e5, f5, g5, h5,
    a6, b6, c6, d6, e6, f6, g6, h6,
    a7, b7, c7, d7, e7, f7, g7, h7,
    a8, b8, c8, d8, e8, f8, g8, h8
};

// File and rank masks
uint64_t fileMasks[64];
uint64_t rankMasks[64];

// Pawn masks
uint64_t isolatedPawnMasks[64];
uint64_t whitePassedPawnMasks[64];
uint64_t blackPassedPawnMasks[64];

// Rank extraction
const int getRank[64] = {
    7, 7, 7, 7, 7, 7, 7, 7,
    6, 6, 6, 6, 6, 6, 6, 6,
    5, 5, 5, 5, 5, 5, 5, 5,
    4, 4, 4, 4, 4, 4, 4, 4,
    3, 3, 3, 3, 3, 3, 3, 3,
    2, 2, 2, 2, 2, 2, 2, 2,
    1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0
};

// Penalty and bonus constants
const int doublePawnPenalty = -10;
const int isolatedPawnPenalty = -10;
const int passedPawnBonus[8] = {0, 10, 30, 50, 75, 100, 150, 200};
const int semiOpenFileScore = 10;
const int openFileScore = 15;
const int kingShieldBonus = 5;

uint64_t setFileRankMask(int fileNumber, int rankNumber)
{
    // file or rank mask
    uint64_t mask = 0ULL;
    
    // loop over ranks
    for (int rank = 0; rank < 8; rank++)
    {
        // loop over files
        for (int file = 0; file < 8; file++)
        {
            // init square
            int square = rank * 8 + file;
            
            if (fileNumber != -1)
            {
                // on file match
                if (file == fileNumber)
                    // set bit on mask
                    mask |= setBit(mask, square);
            }
            
            else if (rankNumber != -1)
            {
                // on rank match
                if (rank == rankNumber)
                    // set bit on mask
                    mask |= setBit(mask, square);
            }
        }
    }
    
    // return mask
    return mask;
}

// init evaluation masks
void initEvaluationMasks()
{
    /******** Init file masks ********/
    // loop over ranks
    for (int rank = 0; rank < 8; rank++)
    {
        // loop over files
        for (int file = 0; file < 8; file++)
        {
            // init square
            int square = rank * 8 + file;
            
            // init file mask for a current square
            fileMasks[square] |= setFileRankMask(file, -1);
        }
    }
    
    /******** Init rank masks ********/
    // loop over ranks
    for (int rank = 0; rank < 8; rank++)
    {
        // loop over files
        for (int file = 0; file < 8; file++)
        {
            // init square
            int square = rank * 8 + file;
            
            // init file mask for a current square
            rankMasks[square] |= setFileRankMask(-1, rank);
        }
    }
    
    /******** Init isolated masks ********/
    // loop over ranks
    for (int rank = 0; rank < 8; rank++)
    {
        // loop over files
        for (int file = 0; file < 8; file++)
        {
            // init square
            int square = rank * 8 + file;
            
            // init file mask for a current square
            isolatedPawnMasks[square] |= setFileRankMask(file - 1, -1);
            isolatedPawnMasks[square] |= setFileRankMask(file + 1, -1);
        }
    }
    
    /******** White passed masks ********/
    
    // loop over ranks
    for (int rank = 0; rank < 8; rank++)
    {
        // loop over files
        for (int file = 0; file < 8; file++)
        {
            // init square
            int square = rank * 8 + file;
            
            // init file mask for a current square
            whitePassedPawnMasks[square] |= setFileRankMask(file - 1, -1);
            whitePassedPawnMasks[square] |= setFileRankMask(file, -1);
            whitePassedPawnMasks[square] |= setFileRankMask(file + 1, -1);
            
            // loop over redundant ranks
            for (int i = 0; i < (8 - rank); i++)
                // reset redundant bits 
                whitePassedPawnMasks[square] &= ~rankMasks[(7 - i) * 8 + file];
        }
    }
    
    /******** Black passed masks ********/
    
    // loop over ranks
    for (int rank = 0; rank < 8; rank++)
    {
        // loop over files
        for (int file = 0; file < 8; file++)
        {
            // init square
            int square = rank * 8 + file;
            
            // init file mask for a current square
            blackPassedPawnMasks[square] |= setFileRankMask(file - 1, -1);
            blackPassedPawnMasks[square] |= setFileRankMask(file, -1);
            blackPassedPawnMasks[square] |= setFileRankMask(file + 1, -1);
            
            // loop over redundant ranks
            for (int i = 0; i < rank + 1; i++)
                // reset redundant bits 
                blackPassedPawnMasks[square] &= ~rankMasks[i * 8 + file];
        }
    }
}

int evaluate(BoardState& board){
    // score variable
    int score = 0;

    // current pieces bitboard copy

    uint64_t bitboard;

    // int piece and square
    int piece, square;

    // double pawn penalty counter
    int doublePawns = 0;

    // loop over piece bitboards
    for (int bbPiece = P ; bbPiece <= k; bbPiece ++){
        bitboard = board.bitBoard[bbPiece];

        while (bitboard){
            piece = bbPiece;

            square = getLeastSigBitIndex(bitboard);

            score += materialScore[piece];

            //scpre psotoional peice scores
            switch(piece){
                //white pieces
                case P: 
                    score += pawn_score[square]; 

                    //double pawn penalty
                    doublePawns = countBits(board.bitBoard[P] & fileMasks[square]);

                    // on double pawns (triple, etc)
                    if (doublePawns > 1)
                        score += doublePawns * doublePawnPenalty;
                    
                    // on isolated pawn
                    if ((board.bitBoard[P] & isolatedPawnMasks[square]) == 0)
                        // give an isolated pawn penalty
                        score += isolatedPawnPenalty;
                    
                    // on passed pawn
                    if ((whitePassedPawnMasks[square] & board.bitBoard[p]) == 0)
                        // give passed pawn bonus
                        score += passedPawnBonus[getRank[square]];

                    break;
                case N: score += knight_score[square]; break;
                case B: 
                    score += bishop_score[square]; 
                    // mobility

                    score += countBits(getBishopAttacks(square, board.occupancies[both]));                    
                    
                    break;

                case R: 
                    // positional score
                    score += rook_score[square]; 

                    // semi open file bonus
                    if ((board.bitBoard[P] & fileMasks[square])== 0){
                        score += semiOpenFileScore;
                    }

                    // semi open file bonus
                    if (((board.bitBoard[P] | board.bitBoard[p]) & fileMasks[square]) == 0){
                        score += semiOpenFileScore;
                    }
                    break;

                case K: 
                    score += king_score[square]; 
                    // semi open file penalty
                    if ((board.bitBoard[P] & fileMasks[square] )== 0){
                        score -= semiOpenFileScore;
                    }

                    // semi open file penalty
                    if (((board.bitBoard[P] | board.bitBoard[p]) & fileMasks[square]) == 0){
                        score -= semiOpenFileScore;
                    }

                    // king safety bonus
                    score += countBits(kingAttacks[square] & board.occupancies[white]) * kingShieldBonus;

                    break;

                case Q:
                    score += countBits(getBishopAttacks(square, board.occupancies[both]));

                    break;              
                    
                // evaluate black pieces
                case p: 
                    score -= pawn_score[mirror_score[square]]; 
                
                    // positional score
                    score -= pawn_score[mirror_score[square]];

                    // double pawn penalty
                    doublePawns = countBits(board.bitBoard[p] & fileMasks[square]);
                    
                    // on double pawns (triple, etc)
                    if (doublePawns > 1)
                        score -= doublePawns * doublePawnPenalty;
                    
                    // on isolated pawns
                    if ((board.bitBoard[p] & isolatedPawnMasks[square]) == 0)
                        // give an isolated pawn penalty
                        score -= isolatedPawnPenalty;
                    
                    // on passed pawn
                    if ((blackPassedPawnMasks[square] & board.bitBoard[P]) == 0)
                        // give passed pawn bonus
                        score -= passedPawnBonus[getRank[mirror_score[square]]];
                break;


                case n: score -= knight_score[mirror_score[square]]; break;
                case b: 
                    score -= bishop_score[mirror_score[square]]; 

                    score -= countBits(getBishopAttacks(square, board.occupancies[both]));                    
                  
                
                    break;


                case r:

                    // positional score
                    score -= rook_score[mirror_score[square]];

                    // semi open file bonus
                    if ((board.bitBoard[p] & fileMasks[square] )== 0){
                        score -= semiOpenFileScore;
                    }

                    // semi open file bonus
                    if (((board.bitBoard[P] | board.bitBoard[p]) & fileMasks[square])== 0){
                        score -= semiOpenFileScore;
                    }
                    break;

                case k: 
                    score += king_score[mirror_score[square]]; 
                    // semi open file penalty
                    if ((board.bitBoard[p] & fileMasks[square]) == 0){
                        score += semiOpenFileScore;
                    }

                    // semi open file penalty
                    if (((board.bitBoard[P] | board.bitBoard[p]) & fileMasks[square])== 0){
                        score += semiOpenFileScore;
                    }

                    // king safety bonus
                    score -= countBits(kingAttacks[square] & board.occupancies[white]) * kingShieldBonus;

                    break;

                case q:
                    score -= countBits(getBishopAttacks(square, board.occupancies[both]));

                    break;  
            }

            //pop lsib
            popBit(bitboard, square);
        }
    }

    // return evaluation based on side
    return (board.side == white) ? score : -score;
}