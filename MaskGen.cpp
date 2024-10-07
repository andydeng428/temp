#include "BitBoard.hpp"
#include "MaskGen.hpp"
#include "Magics.hpp"
#include <cstdint>


// Attack masks for pawns, knights, kings, bishops, and rooks
uint64_t pawnAttacks[2][64];
uint64_t knightAttacks[64];
uint64_t kingAttacks[64];
uint64_t bishopAttacks[64][512];
uint64_t rookAttacks[64][4096];
uint64_t bishopMasks[64];
uint64_t rookMasks[64];

// File masks
const uint64_t notInAFile = 18374403900871474942ULL;
const uint64_t notInHFile = 9187201950435737471ULL;
const uint64_t notInABFile = 18229723555195321596ULL;
const uint64_t notInHGFile = 4557430888798830399ULL;

// Relevancy mask, representing number of squares bishops and rooks can hop on that bit
const int bishopRelevantBit[64] = {
    6, 5, 5, 5, 5, 5, 5, 6, 
    5, 5, 5, 5, 5, 5, 5, 5, 
    5, 5, 7, 7, 7, 7, 5, 5, 
    5, 5, 7, 9, 9, 7, 5, 5, 
    5, 5, 7, 9, 9, 7, 5, 5, 
    5, 5, 7, 7, 7, 7, 5, 5, 
    5, 5, 5, 5, 5, 5, 5, 5, 
    6, 5, 5, 5, 5, 5, 5, 6
};

const int rookRelevantBit[64] = {
    12, 11, 11, 11, 11, 11, 11, 12, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    12, 11, 11, 11, 11, 11, 11, 12
};


uint64_t maskPawnAttacks(int side , int square){
    uint64_t attacks = 0ULL;

    uint64_t bitBoard = 0ULL;
    setBit(bitBoard, square);

    // white pawns
    if (!side){
        //handles capture to the right
        if ((bitBoard >> 7) & notInAFile){
            attacks |= (bitBoard >> 7); 
        }
        //handles capture to the left
        if ((bitBoard >> 9) & notInHFile){
            attacks |= (bitBoard >> 9); 
        }
    }
    // black pawns
    else{
        //handles capture to the right
        if ((bitBoard << 7) & notInHFile){
            attacks |= (bitBoard << 7); 
        }
        //handles capture to the left
        if ((bitBoard << 9) & notInAFile){
            attacks |= (bitBoard << 9); 
        }
    }
    return attacks;
}


uint64_t maskKnightAttacks(int square){
    uint64_t attacks = 0ULL;

    uint64_t bitBoard = 0ULL;
    setBit(bitBoard, square);

    attacks |= (bitBoard >> 17) & notInHFile;
    attacks |= (bitBoard >> 15) & notInAFile;
    attacks |= (bitBoard >> 10) & notInHGFile;
    attacks |= (bitBoard >> 6)  & notInABFile;
    attacks |= (bitBoard << 17) & notInAFile;
    attacks |= (bitBoard << 15) & notInHFile;
    attacks |= (bitBoard << 10) & notInABFile;
    attacks |= (bitBoard << 6)  & notInHGFile;
/*
    if ((bitBoard >> 17) & notInHFile) attacks |= (bitBoard >> 17);
    if ((bitBoard >> 15) & notInAFile) attacks |= (bitBoard >> 15);
    if ((bitBoard >> 10) & notInHGFile) attacks |= (bitBoard >> 10);
    if ((bitBoard >> 6) & notInABFile) attacks |= (bitBoard >> 6);
    if ((bitBoard << 17) & notInAFile) attacks |= (bitBoard << 17);
    if ((bitBoard << 15) & notInHFile) attacks |= (bitBoard << 15);
    if ((bitBoard << 10) & notInABFile) attacks |= (bitBoard << 10);
    if ((bitBoard << 6) & notInHGFile) attacks |= (bitBoard << 6);
*/
    return attacks;
}

uint64_t maskKingAttacks(int square){
    uint64_t attacks = 0ULL;

    // set bitboard square to mask all attacks`
    uint64_t bitBoard = 0ULL;
    setBit(bitBoard, square);

    attacks |= (bitBoard >> 8);              
    attacks |= (bitBoard >> 9) & notInHFile;
    attacks |= (bitBoard >> 7) & notInAFile; 
    attacks |= (bitBoard >> 1) & notInHFile;
    attacks |= (bitBoard << 8);              
    attacks |= (bitBoard << 9) & notInAFile;
    attacks |= (bitBoard << 7) & notInHFile;
    attacks |= (bitBoard << 1) & notInAFile;    
/*
    if (bitBoard >> 8) attacks |= (bitBoard >> 8);
    if ((bitBoard >> 9) & notInHFile) attacks |= (bitBoard >> 9);
    if ((bitBoard >> 7) & notInAFile) attacks |= (bitBoard >> 7);
    if ((bitBoard >> 1) & notInHFile) attacks |= (bitBoard >> 1);
    if (bitBoard << 8) attacks |= (bitBoard << 8);
    if ((bitBoard << 9) & notInAFile) attacks |= (bitBoard << 9);
    if ((bitBoard << 7) & notInHFile) attacks |= (bitBoard << 7);
    if ((bitBoard << 1) & notInAFile) attacks |= (bitBoard << 1);
*/
    // return attack map
    return attacks;
}

uint64_t maskBishopAttacks(int square){
    uint64_t attacks = 0ULL;
    // current file and rank
    int r, f;

    // target file and rank
    int tr = square / 8;
    int tf = square % 8;

    // do not mask the squares at the edge of the board because bishop movement is limited on edges
    for (r = tr + 1, f = tf + 1; r <= 6 && f <= 6; r++, f++) attacks |= (1ULL << (r * 8 + f));
    for (r = tr - 1, f = tf + 1; r >= 1 && f <= 6; r--, f++) attacks |= (1ULL << (r * 8 + f));
    for (r = tr + 1, f = tf - 1; r <= 6 && f >= 1; r++, f--) attacks |= (1ULL << (r * 8 + f));
    for (r = tr - 1, f = tf - 1; r >= 1 && f >= 1; r--, f--) attacks |= (1ULL << (r * 8 + f));
  
    return attacks;
}

uint64_t maskRookAttacks(int square){
    uint64_t attacks = 0ULL;
    // current file and rank
    int r, f;
    
    // target file and rank
    int tr = square / 8;
    int tf = square % 8;

    for (r = tr + 1; r <= 6; r++) attacks |= (1ULL << (r * 8 + tf));
    for (r = tr - 1; r >= 1; r--) attacks |= (1ULL << (r * 8 + tf));
    for (f = tf + 1; f <= 6; f++) attacks |= (1ULL << (tr * 8 + f));
    for (f = tf - 1; f >= 1; f--) attacks |= (1ULL << (tr * 8 + f));
    
    return attacks;
}

uint64_t bishopAttacksOnTheFly(int square, uint64_t block){
    uint64_t attacks = 0ULL;

    // current file and rank
    int r, f;

    // target file and rank
    int tr = square / 8;
    int tf = square % 8;

    // don't distinguish white and black, move generator will do that
    for (r = tr + 1, f = tf + 1; r <= 7 && f <= 7; r++, f++){
        attacks |= (1ULL << (r * 8 + f));
        if ((1ULL << (r * 8 + f)) & block){
            break;
        }
    }
    for (r = tr - 1, f = tf + 1; r >= 0 && f <= 7; r--, f++) {
        attacks |= (1ULL << (r * 8 + f));
        if ((1ULL << (r * 8 + f)) & block){
            break;
        }
    }
    for (r = tr + 1, f = tf - 1; r <= 7 && f >= 0; r++, f--) {
        attacks |= (1ULL << (r * 8 + f));
        if ((1ULL << (r * 8 + f)) & block){
            break;
        }
    }
    for (r = tr - 1, f = tf - 1; r >= 0 && f >= 0; r--, f--) {
        attacks |= (1ULL << (r * 8 + f));
        if ((1ULL << (r * 8 + f)) & block){
            break;
        }
    }
    return attacks;
}

uint64_t rookAttacksOnTheFly(int square, uint64_t block){
    uint64_t attacks = 0ULL;
    // current file and rank
    int r, f;
    
    // target file and rank
    int tr = square / 8;
    int tf = square % 8;

    for (r = tr + 1; r <= 7; r++) {
        attacks |= (1ULL << (r * 8 + tf));
        if ((1ULL << (r * 8 + tf)) & block){
            break;
        }
    }
    for (r = tr - 1; r >= 0; r--) {
        attacks |= (1ULL << (r * 8 + tf));
        if ((1ULL << (r * 8 + tf)) & block){
            break;
        }
    }
    for (f = tf + 1; f <= 7; f++) {
        attacks |= (1ULL << (tr * 8 + f));
        if ((1ULL << (tr * 8 + f)) & block){
            break;
        }
    }
    for (f = tf - 1; f >= 0; f--) {
        attacks |= (1ULL << (tr * 8 + f));
        if ((1ULL << (tr * 8 + f)) & block){
            break;
        }
    }
    
    return attacks;
}

void initLeaperAttacks(){
    for (int square = 0 ; square < 64 ; square++){
        //initialize pawn attacks array only
        pawnAttacks[white][square] = maskPawnAttacks(white, square);
        pawnAttacks[black][square] = maskPawnAttacks(black, square);

        //initialize knight attacks array only
        knightAttacks[square] = maskKnightAttacks(square);

        //initialize king attack array only
        kingAttacks[square] = maskKingAttacks(square);
    }
}

uint64_t setOccupancy (int index, int bitsInMask, uint64_t attackMask){
    uint64_t occupancy = 0ULL;

    for (int i = 0 ; i < bitsInMask ; i++){
        int square = getLeastSigBitIndex(attackMask);
        popBit(attackMask, square);

        if (index & ( 1 << i)){
            occupancy |= (1ULL << square);
        }
    }
    return occupancy; 
} 

void initSliderAttacks(int bishop){
    for (int square = 0 ; square < 64 ; square++){
        bishopMasks[square] = maskBishopAttacks(square);
        rookMasks[square] = maskRookAttacks(square);

        u_int64_t attackMask = bishop? bishopMasks[square] : rookMasks[square];

        //init relavent occupancy bit coin
        int relevantBitsCount = countBits(attackMask);
        int occupancyIndicies = 1 << relevantBitsCount;

        for (int index = 0 ; index < occupancyIndicies ; index++){
            if (bishop){
                u_int64_t occupancy = setOccupancy(index, relevantBitsCount, attackMask);
                
                //init magic index
                int magicIndex = (occupancy * bishopMagicNumbers[square]) >> (64 - bishopRelevantBit[square]);

                bishopAttacks[square][magicIndex] = bishopAttacksOnTheFly(square, occupancy);
            }else{
                u_int64_t occupancy = setOccupancy(index, relevantBitsCount, attackMask);
                
                //init magic index
                int magicIndex = (occupancy * rookMagicNumbers[square]) >> (64 - rookRelevantBit[square]);

                rookAttacks[square][magicIndex] = rookAttacksOnTheFly(square, occupancy);
            }
        }

    }
}

uint64_t getBishopAttacks(int square, uint64_t occupancy){

    occupancy  &= bishopMasks[square];
    occupancy *= bishopMagicNumbers[square];
    occupancy >>= 64 - bishopRelevantBit[square];

    return bishopAttacks[square][occupancy];
}

uint64_t getRookAttacks(int square, uint64_t occupancy){
    occupancy  &= rookMasks[square];
    occupancy *= rookMagicNumbers[square];
    occupancy >>= 64 - rookRelevantBit[square];

    return rookAttacks[square][occupancy];
}

uint64_t getQueenAttacks(int square, uint64_t occupancy){
    uint64_t queenAttacks = 0ULL;
    u_int64_t rookOccunpancies = occupancy;
    u_int64_t bishopOccunpancies = occupancy;

    // combination of bishop and rook attacks
    bishopOccunpancies  &= bishopMasks[square];
    bishopOccunpancies *= bishopMagicNumbers[square];
    bishopOccunpancies >>= 64 - bishopRelevantBit[square];
    queenAttacks = bishopAttacks[square][bishopOccunpancies];
    rookOccunpancies  &= rookMasks[square];
    rookOccunpancies *= rookMagicNumbers[square];
    rookOccunpancies >>= 64 - rookRelevantBit[square];
    queenAttacks |= rookAttacks[square][rookOccunpancies];

    return queenAttacks;
}

int isSquareAttacked(const BoardState & board,int square, int side){
    // check if attacked by white pawns
    if ((side == white) && (pawnAttacks[black][square] & board.bitBoard[P]) != 0) return 1;

    // check if attacked by black pawns
    if ((side == black) && (pawnAttacks[white][square] & board.bitBoard[p]) != 0) return 1;

    //check if attacked by knight
    if ((knightAttacks[square] & ((side == white) ? board.bitBoard[N] : board.bitBoard[n])) != 0) return 1;
    
    // check if attakced by bishop
    if (getBishopAttacks(square, board.occupancies[both]) & ((side == white) ? board.bitBoard[B] : board.bitBoard[b])) return 1;

    // check if attakced by rook
    if (getRookAttacks(square, board.occupancies[both]) & ((side == white) ? board.bitBoard[R] : board.bitBoard[r])) return 1;


    // check if attakced by queen
    if (getQueenAttacks(square, board.occupancies[both]) & ((side == white) ? board.bitBoard[Q] : board.bitBoard[q])) return 1;

    //check if attacked by king 
    if ((kingAttacks[square] & ((side == white) ? board.bitBoard[K] : board.bitBoard[k])) != 0) return 1;
    return 0;
}