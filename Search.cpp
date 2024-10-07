#include "Search.hpp"
#include "MoveGen.hpp"
#include "Evaluation.hpp"
#include "ZobristTable.hpp"
#include "MaskGen.hpp"
#include "UCI.hpp"
#include "Util.hpp"
#include <cstdint>
#include <iostream>


uint64_t nodes = 0;

// MVV/LVA (Most Valuable Victim/Least Valuable Attacker) scoring
int mvv_lva[12][12] = {
    {105, 205, 305, 405, 505, 605, 105, 205, 305, 405, 505, 605},
    {104, 204, 304, 404, 504, 604, 104, 204, 304, 404, 504, 604},
    {103, 203, 303, 403, 503, 603, 103, 203, 303, 403, 503, 603},
    {102, 202, 302, 402, 502, 602, 102, 202, 302, 402, 502, 602},
    {101, 201, 301, 401, 501, 601, 101, 201, 301, 401, 501, 601},
    {100, 200, 300, 400, 500, 600, 100, 200, 300, 400, 500, 600},
    {105, 205, 305, 405, 505, 605, 105, 205, 305, 405, 505, 605},
    {104, 204, 304, 404, 504, 604, 104, 204, 304, 404, 504, 604},
    {103, 203, 303, 403, 503, 603, 103, 203, 303, 403, 503, 603},
    {102, 202, 302, 402, 502, 602, 102, 202, 302, 402, 502, 602},
    {101, 201, 301, 401, 501, 601, 101, 201, 301, 401, 501, 601},
    {100, 200, 300, 400, 500, 600, 100, 200, 300, 400, 500, 600}
};

// Killer and history moves
int killerMoves[2][maxPly] = {{0}};
int historyMoves[12][64] = {{0}};

// PV (Principal Variation) tables
int pvLength[maxPly] = {0};
int pvTable[maxPly][maxPly] = {{0}};
int followPV = 0, scorePV = 0;


void enablePVScoring(Moves* moveList){
    followPV = 0;

    // loop over moves within a movelist
    for (int count = 0 ; count < moveList->count; count++){
        // make sure we hit PV move
        if(pvTable[0][ply] == moveList->moves[count]){
            scorePV = 1;

            followPV = 1;
        }
    }
}

// score moves
int scoreMove(const BoardState& board, int move){
    // if PV move scoring is allowed
    if (scorePV){
        if (pvTable[0][ply] == move){
            // disable score PV flag
            scorePV = 0;
            // score PV move
            return 20000;
        }
    }
    // score capture move
    if (getMoveCapture(move)){
        // incase of enpassant capture we init as white pawn
        int targetPiece = P;
        
        // pick up bitboard piece index ranges depending on side
        int startPiece, endPiece;
        
        // pick up side to move
        if (board.side == white) { startPiece = p; endPiece = k; }
        else { startPiece = P; endPiece = K; }
        
        // loop over bitboards opposite to the current side to move
        for (int bbPiece = startPiece; bbPiece <= endPiece; bbPiece++)
        {
            // if there's a piece on the target square
            if (getBit(board.bitBoard[bbPiece], getMoveTarget(move)))
            {
                // remove it from corresponding bitboard
                targetPiece = bbPiece;
                break;
            }
        }
        // score move by MVV LVA 
        return mvv_lva[getMovePiece(move)][targetPiece] + 10000;

    }else{
        // killer move 
        //score 1st killer move
        if(killerMoves[0][ply] == move){
            return 9000;
        }
        else if(killerMoves[1][ply] == move){
            return 8000;
        }
        else{
            return historyMoves[getMovePiece(move)][getMoveTarget(move)];
        }

    }
    return 0;
}

/*  =======================
         Move ordering
    =======================
    
    1. PV move
    2. Captures in MVV/LVA
    3. 1st killer move
    4. 2nd killer move
    5. History moves
    6. Unsorted moves
*/


void sortMoves(const BoardState& board, Moves* moveList){
    int moveScores[moveList->count];
 
    for (int count = 0 ; count < moveList->count; count++){
        moveScores[count] = scoreMove(board, moveList->moves[count]);
    }

    for (int currentMove = 0 ; currentMove < moveList->count ; currentMove++){
        for (int nextMove = currentMove + 1 ; nextMove < moveList->count ; nextMove++){
            if(moveScores[currentMove] < moveScores[nextMove]){
                int tempScore = moveScores[currentMove];
                moveScores[currentMove] = moveScores[nextMove];
                moveScores[nextMove] = tempScore;
                
                // swap moves
                int temp_move = moveList->moves[currentMove];
                moveList->moves[currentMove] = moveList->moves[nextMove];
                moveList->moves[nextMove] = temp_move;
            }
        }
    }
}

// position repetition detection
int isRepetition(const BoardState& board){
    for (int index = 0 ; index < repetitionIndex ; index ++){
        // if we found hash key that is same with current
        if (repetitionTable[index] == board.hashKey){
            return 1;
        }
    }
    return 0;
}

int quiescence(BoardState& board, int alpha, int beta){

    // every 2047 nodes 
    if ((nodes & 2047) == 0){
        communicate();
    }

    // increment nodes count
    nodes++;

    // we are too deep, thus there is overflow of arrays relying on max ply constant
    if (ply > maxPly - 1){
        return evaluate(board);
    }

    // evaluate position
    int evaluation = evaluate(board);

    if (evaluation >= beta){
        // node (move) fails high 
        return beta;
    }
    // found better move
    if (evaluation > alpha){
        // PV node (move)
        alpha = evaluation;
    }

    //create move list instance
    Moves moveList[1];

    // geenrate moves
    generateMove(board, moveList);

    // sort moves
    sortMoves(board, moveList);

    // loop over moves within a movelist
    for (int count = 0 ; count < moveList->count; count++){
        // copy board
        copyBoard();

        ply++;

        // increment repretiion index and score hash key
        repetitionIndex++;
        repetitionTable[repetitionIndex] = board.hashKey;

        // make sure to make only legal moves
        if (makeMove(board, moveList->moves[count], onlyCaptures) == 0){
            ply--;

            // decrement repetition index
            repetitionIndex--;

            //skip to next move
            continue;
        }

        int score = -quiescence(board ,-beta, - alpha);

        // decrement ply
        ply--;

        // decrement repetition index
        repetitionIndex--;


        // take move back
        restoreBoard();

        //return 0 if time si up 
        if (stopped == 1) return 0;

        // found better move
        if (score > alpha){
            // PV node (move)
            alpha = score;

            // failhard beta cutoff (beta cutoffs are when the search can be stopped because position is worse than a previously found move)
            if (score >= beta){
                // node (move) fails high 
                return beta;
            }
        }
    }
    // node that fails low
    return alpha;
}


const int fullDepthMoves = 4;
const int reductionLimit = 3;


// negamax alpha beta search
int negamax(BoardState& board, int alpha, int beta, int depth){
// variable score move
    int score;

    // define hash flag
    int hashFlag = hashFlagAlpha;

    // if position repetition occurs
    if (ply && isRepetition(board)){
        // returndraw score
        return 0;
    }
    // a hack by Pedro Castro to figure out whether the current node is POV node or not
    int pvNode = (beta - alpha > 1);

    // read hash entry if not a root ply and hash entry is available and current node is not PV move
    if (ply && (score = readHashEntry(board, alpha, beta, depth)) != noHashEntry && pvNode == 0){
        // if the move has already been searched (thus has a value), we just return score for this move 
        return score;
    }


    // every 2047 nodes 
    if ((nodes & 2047) == 0){
        communicate();
    }

    // init PV lenght
    pvLength[ply] = ply;


    // reccurision escape condition
    if (depth == 0){
        return quiescence(board, alpha, beta);
    }

    // we are too deep, hence theres a overflow of arrays of relying on max ply constant
    if (ply > maxPly - 1){
        return evaluate(board);
    }

    //increment nodes count
    nodes++;

    // is king in check
    int inCheck = isSquareAttacked(board, (board.side == white) ? getLeastSigBitIndex(board.bitBoard[K]) : getLeastSigBitIndex(board.bitBoard[k]), board.side ^ 1);
    
    // incrase search depth if king has been exposed into check
    if (inCheck) depth ++;

    // legal moves counter
    int legalMoves = 0;

    //create move list instance
    Moves moveList[1];

    // Null move pruning
    if (depth >= 3 && inCheck == 0 && ply){
        // preserve board state
        copyBoard();

        // increment ply
        ply++;

        // increment repretiion index and score hash key
        repetitionIndex++;
        repetitionTable[repetitionIndex] = board.hashKey;

        // hash enpassant if available
        if (board.enPassant != no_sq){
            board.hashKey ^= enpassantKeys[board.enPassant];
        }

        //  reset enpassant capture square
        board.enPassant = no_sq;

        //switch side, give opponent an extra move
        board.side ^= 1;

        // hash the side
        board.hashKey ^= sideKey;

        // search move with reduced depth to find beta cutoffs
        // depth - 1- R where R is reduction limit
        score = - negamax(board, -beta, -beta + 1, depth - 1 - 2);

        // decrement ply
        ply--;

        // decrement repetition index
        repetitionIndex--;

        //restore board state
        restoreBoard();

        //return 0 if time si up 
        if (stopped == 1) return 0;

        // fail hard beta cutoff
        if (score >= beta){
            // node returns fail high
            return beta;
        }
    }

    // geenrate moves
    generateMove(board, moveList);

    // if we are now following pv line
    if (followPV){
        //enable pv move scoring
        enablePVScoring(moveList);
    }

    // sort moves
    sortMoves(board, moveList);

    // moves searched in a move list
    int moveSearched = 0;
    
    // loop over moves within a movelist
    for (int count = 0 ; count < moveList->count; count++){
        // copy board
        copyBoard();

        ply++;

        // increment repretiion index and score hash key
        repetitionIndex++;
        repetitionTable[repetitionIndex] = board.hashKey;


        // make sure to make only legal moves
        if (makeMove(board, moveList->moves[count], allMoves) == 0){
            ply--;

            // decrement repetition index
            repetitionIndex--;

            //skip to next move
            continue;
        }

        // increment legal moves
        legalMoves++;

        // full depth search
        if (moveSearched == 0){
            // for all other types of nodes (moves) do normal alpha beta search
            score = -negamax(board, -beta, -alpha, depth - 1);
            // LMR
        }else{
            // conmdition to consider Late move reduction
            if (moveSearched >= fullDepthMoves 
                && depth >= reductionLimit 
                && inCheck == 0 
                && getMoveCapture(moveList->moves[count]) == 0
                && getMovePromoted(moveList->moves[count]) == 0){
                // search current moev with reduced depth
                score = -negamax(board, -alpha - 1, -alpha, depth - 2);
            }else{
                // hack to ensure that full depth search is done
                score = alpha + 1;
            }
            // Principal ariation search 
            if (score> alpha){
                 /* Once you've found a move with a score that is between alpha and beta,
                the rest of the moves are searched with the goal of proving that they are all bad.
                It's possible to do this a bit faster than a search that worries that one
                of the remaining moves might be good. */
                score = -negamax(board, -alpha - 1, -alpha, depth - 1);
            
                 /* If the algorithm finds out that it was wrong, and that one of the
                subsequent moves was better than the first PV move, it has to search again,
                in the normal alpha-beta manner.  This happens sometimes, and it's a waste of time,
                but generally not often enough to counteract the savings gained from doing the
                "bad move proof" search referred to earlier. */
                if((score > alpha) && (score < beta)){
                // research the move that has failed to be proved bad
                // with normal aphabeta score bounds
                    score = -negamax(board, -beta, - alpha, depth -1);
                }
            }
        }
        
        // decrement ply
        ply--;

        // decrement repetition index
        repetitionIndex--;

        // take move back
        restoreBoard();

        //return 0 if time si up 
        if (stopped == 1) return 0;

        //increment moves searched
        moveSearched++;

        // found better move
        if (score > alpha){
            // switch hash flag from storing score for fail low node
            hashFlag = hashFlagExact;


            // on quiet moves
            if (getMoveCapture(moveList->moves[count]) == 0){
                historyMoves[getMovePiece(moveList->moves[count])][getMoveTarget(moveList->moves[count])] += depth;
            }
            
            // PV node (move)
            alpha = score;

            //write PV move to PV table move
            pvTable[ply][ply] = moveList->moves[count];

            // copy move from deeper ply into a current ply's line
            for (int nextPly = ply + 1; nextPly < pvLength[ply + 1] ; nextPly++){
                // copy move from deeper ply into a current ply's line
                pvTable[ply][nextPly] = pvTable[ply + 1][nextPly];
            }

            //adjust PV length
            pvLength[ply] = pvLength[ply + 1];

            // failhard beta cutoff (beta cutoffs are when the search can be stopped because position is worse than a previously found move)
            if (score >= beta){
                // store hash entry with the score equals to beta
                writeHashEntry(board, beta, depth, hashFlagBeta);
    
                // on quiet moves
                if (getMoveCapture(moveList->moves[count]) == 0){
                    // store killer moves
                    killerMoves[1][ply] = killerMoves[0][ply];
                    killerMoves[0][ply] = moveList->moves[count];
                }
    
                // node (move) fails high 
                return beta;
            }
        }
    }
    // we dont have any legal moves to make
    if (legalMoves == 0){
        // when king is in check
            if (inCheck){
                return -mateValue + ply;
                /*in that position with 6 ply depth we can make two different checkmates - the short one and the "wrong" one. 
                if both paths have the same score we can not be sure that a better path will be taken, it depends on the order
                of search. by adding +ply value (which gets bigger the deeper we go) we ensure that mate that require less moves
                always has the biggest score, therefore it is prefered as a better path
                */
            }
        // when king is not in check
            else{
                // return stalemate score
                return 0;
            }
    }

    // store hash entry with the score equals to alpha
    writeHashEntry(board, alpha, depth, hashFlag);


    // node (move) fails low
    return alpha;
}



//sarch position
void searchPosition(BoardState& board, int depth){
    int score = 0;
    // reset nodes counter
    nodes = 0;

    // reset time is up flag 
    stopped = 0;

    // reset follow PV flags
    followPV = 0;
    scorePV = 0;

    // clear helper data structures for search
    memset(killerMoves, 0, sizeof(killerMoves));
    memset(historyMoves, 0, sizeof(historyMoves));
    memset(pvTable, 0, sizeof(pvTable));
    memset(pvLength, 0, sizeof(pvLength));

    //define initial alpha beta bounds
    int alpha = -INF;
    int beta = INF;
    //iterative deepening
    for(int currentDepth = 1 ; currentDepth <= depth ; currentDepth++){
        //return 0 if time si up 
        if (stopped == 1) 
            break;

        //enable follow PV flag
        followPV = 1;
        
        // find best move within a given positon
        int score = negamax(board, alpha, beta, currentDepth);

        // we fell outside the window, so try again with full-width and same depth
        if ((score <= alpha) || (score >= beta)){
            alpha = -INF;
            beta = INF;
            continue;
        }

        // set up window for next iteration
        alpha = score - 50;
        beta = score + 50;

        if (score > -mateValue && score < -mateScore)
            std::cout << "info score mate " << (-(score + mateValue) / 2 - 1) << " depth " << currentDepth << " nodes " << nodes << " time " << getTimeMs() - startTime << " pv ";
        else if (score > mateScore && score < mateValue)
            std::cout << "info score mate " << ((mateValue - score) / 2 + 1) << " depth " << currentDepth << " nodes " << nodes << " time " << getTimeMs() - startTime << " pv ";
        else
            std::cout << "info score cp " << score << " depth " << currentDepth << " nodes " << nodes << " time " << getTimeMs() - startTime << " pv ";

        for (int count = 0 ; count < pvLength[0]; count++){
            printMove(pvTable[0][count]);
            std::cout<< " ";
        }

        std::cout<<" \n";
    }
        //best move placeholder
        std::cout<< "bestmove ";
        printMove(pvTable[0][0]);
        std::cout<< "\n";

}