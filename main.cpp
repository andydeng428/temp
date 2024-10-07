#include <iostream>
#include "BitBoard.hpp"
#include "Magics.hpp"
#include "ZobristTable.hpp"
#include "UCI.hpp"
#include "MaskGen.hpp"
#include "Evaluation.hpp"



int main(){
    initLeaperAttacks();
    initSliderAttacks(bishop);
    initSliderAttacks(rook);
    // init random keys for hasing
    initRandomKeys();
    // clear hash table
    clearHashTable();
    // init evaluation masks
    initEvaluationMasks();

    uciLoop();
    return 0;
}