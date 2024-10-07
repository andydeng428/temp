#include "MoveGen.hpp"
#include "BitBoard.hpp"
#include "ZobristTable.hpp"
#include "MaskGen.hpp"


// Define the move list
struct Moves MoveList;

// Define the castling rights
const int castlingRights[64] = {
     7, 15, 15, 15,  3, 15, 15, 11,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    13, 15, 15, 15, 12, 15, 15, 14
}; // 15 means rook didn't move or is still alive

int parseMove(char *moveString){
    // we need to set the castling flags and all of that, thus lets just innitialize a moveList to take care of that
    Moves moveList[1];

    generateMove(board, moveList);

    int sourceSquare = (moveString[0] - 'a') + (8 - (moveString[1] - '0')) * 8;

    int targetSquare = (moveString[2] - 'a') + (8 - (moveString[3] - '0')) * 8;

    for (int moveCount = 0 ; moveCount < moveList->count ; moveCount ++){

        int move = moveList->moves[moveCount];

        if(sourceSquare == getMoveSource(move) && targetSquare == getMoveTarget(move)){
            int promotedPiece = getMovePromoted(move);

            if (promotedPiece)
            {
                // promoted to queen
                if ((promotedPiece == Q || promotedPiece == q) && moveString[4] == 'q')
                    // return legal move
                    return move;
                
                // promoted to rook
                else if ((promotedPiece == R || promotedPiece == r) && moveString[4] == 'r')
                    // return legal move
                    return move;
                
                // promoted to bishop
                else if ((promotedPiece == B || promotedPiece == b) && moveString[4] == 'b')
                    // return legal move
                    return move;
                
                // promoted to knight
                else if ((promotedPiece == N || promotedPiece == n) && moveString[4] == 'n')
                    // return legal move
                    return move;
                
                // continue the loop on possible wrong promotions (e.g. "e7e8f")
                continue;
            }

            return move;
            
        }
    
    }
    // return illegal move
    return 0;
}

void addMove(Moves *moveList, int move){
    moveList -> moves[moveList-> count] = move;
    moveList->count++;
}

int makeMove(BoardState& board, int move, int moveFlag){
    // quiet moves
    if (moveFlag == allMoves){
        // preserve move state
        copyBoard();
        // parse move
        int sourceSquare = getMoveSource(move);
        int targetSquare = getMoveTarget(move);
        int piece = getMovePiece(move);
        int promotedPiece = getMovePromoted(move);
        int capture = getMoveCapture(move);
        int doublePush = getMoveDouble(move);
        int enpass = getMoveEnPassant(move);
        int castling = getMoveCastling(move);

        //make move
        popBit(board.bitBoard[piece], sourceSquare);
        setBit(board.bitBoard[piece], targetSquare);

        // hash peice(remove piece from source square and put it on target square from hash key)
        board.hashKey ^= pieceKeys[piece][sourceSquare]; // remove piece from source square in hash key
        board.hashKey ^= pieceKeys[piece][targetSquare]; // set piece to target square in hash key

        //capture moves
        if (capture){
            // pickup bitboard piece index ranges depending on side
            int startPiece, endPiece;

            if (board.side == white){
                startPiece = p;
                endPiece = k;
            }
            else{
                startPiece = P;
                endPiece = K;
            }

            // loop over bitboards opposite to the currend side to move
            //loop over from K N Q P B-> k n q p q, bassicly going through all of the pieces avaiable
            for (int bbPiece = startPiece; bbPiece <= endPiece ; bbPiece++){
                // if
                if (getBit(board.bitBoard[bbPiece], targetSquare)){
                    // if theres a piece on the target square remove that piece
                    popBit(board.bitBoard[bbPiece] , targetSquare);

                    // remove piece from hash key
                    board.hashKey ^= pieceKeys[bbPiece][targetSquare];
                    break;
                }

            }
        }

        // pawn promotion
        if (promotedPiece){
            //erase pawn from target square
            //popBit(bitBoard[(side == white) ? P : p], targetSquare);

            if (board.side == white){
                // erase pawn from target square
                popBit(board.bitBoard[P], targetSquare);

                //hash, remove pawn from hask key
                board.hashKey ^= pieceKeys[P][targetSquare];
            }else{
                // erase pawn from target square
                popBit(board.bitBoard[p], targetSquare);

                //hash, remove pawn from hask key
                board.hashKey ^= pieceKeys[p][targetSquare];
            }

            // set up promoted piece on board
            setBit(board.bitBoard[promotedPiece], targetSquare);

            // add promoted piece into hash key
            board.hashKey ^= pieceKeys[promotedPiece][targetSquare];
        }

        // enpassant captures
        if (enpass){
            //erase the pawn 
            (board.side == white) ? popBit(board.bitBoard[p], targetSquare + 8) : popBit(board.bitBoard[P], targetSquare - 8);

            if (board.side == white){
                // remove caputred pawn
                popBit(board.bitBoard[p], targetSquare + 8);

                // remove pawn from hash hey
                board.hashKey ^= pieceKeys[p][targetSquare + 8];
            }
            
            else{
                // remove caputred pawn
                popBit(board.bitBoard[P], targetSquare - 8);

                // remove pawn from hash hey
                board.hashKey ^= pieceKeys[P][targetSquare - 8];
            }
        }

        // hash enpassant (remove enpassanr square form hash key)
        if (board.enPassant != no_sq){
            board.hashKey ^= enpassantKeys[board.enPassant];
        }

        // reset enpass square 
        board.enPassant = no_sq;


        // handle double pawn push
        if (doublePush){
            //set enpassant square depending on side to move
            //(side == white) ? (enPassant = targetSquare + 8) : (enPassant = targetSquare - 8);

            if (board.side == white){
                // set enpassant square
                board.enPassant = targetSquare + 8;

                // hash enpassant
                board.hashKey ^= enpassantKeys[targetSquare + 8];

            }else{
                // set enpassant square
                board.enPassant = targetSquare - 8;

                // hash enpassant
                board.hashKey ^= enpassantKeys[targetSquare - 8];
            }
        }

        // handle castling
        if (castling){
            //switch target square
            switch(targetSquare){
                //white castle king side
                case(g1):
                    popBit(board.bitBoard[R], h1);
                    setBit(board.bitBoard[R], f1);

                    // hash rook
                    board.hashKey ^= pieceKeys[R][h1]; // remove rook from h1 from hash key
                    board.hashKey ^= pieceKeys[R][f1]; // put rook on f1 on hash key
                    break;
                //white castling queen side
                case(c1):
                    popBit(board.bitBoard[R], a1);
                    setBit(board.bitBoard[R], d1);

                    // hash rook
                    board.hashKey ^= pieceKeys[R][a1]; // remove rook from h1 from hash key
                    board.hashKey ^= pieceKeys[R][d1]; // put rook on f1 on hash key
                    break;
                //black castles king side
                case(g8):
                    popBit(board.bitBoard[r], h8);
                    setBit(board.bitBoard[r], f8);

                    // hash rook
                    board.hashKey ^= pieceKeys[r][h8]; // remove rook from h8 from hash key
                    board.hashKey ^= pieceKeys[r][f8]; // put rook on f8 on hash key
                    break;
                //black castles queen side
                case(c8):
                    popBit(board.bitBoard[r], a8);
                    setBit(board.bitBoard[r], d8);

                    // hash rook
                    board.hashKey ^= pieceKeys[r][a8]; // remove rook from a8 from hash key
                    board.hashKey ^= pieceKeys[r][d8]; // put rook on d8 on hash key
                    break;
            }
        }

        // hash castling
        board.hashKey ^= castleKeys[board.castle];

        //update castling rights
        board.castle &= castlingRights[sourceSquare];
        board.castle &= castlingRights[targetSquare];

        // hash castling, hashing it back
        board.hashKey ^= castleKeys[board.castle];

        //update occupancies
        //this may be inneficcient, we are completely resetting the occupancies
        memset(board.occupancies, 0ULL, 24);

        // loop over white pieces bitboards
        for (int bbPiece = P ; bbPiece <= K ; bbPiece++){
            board.occupancies[white] |= board.bitBoard[bbPiece];
        }

        for (int bbPiece = p ; bbPiece <= k ; bbPiece++){
            board.occupancies[black] |= board.bitBoard[bbPiece];
        }
        //update both occupancies
        board.occupancies[both] |= board.occupancies[white];
        board.occupancies[both] |= board.occupancies[black];

        // change sides
        board.side ^= 1;

        // hash side
        board.hashKey ^= sideKey;

        // ================== debug hash key incremental update ================
        // build hash key for the updated position from scratch
        //uint64_t hashFromScratch = generateHashKey();

        // if case if hashkey build form scatch doens match that was incrementally updated, we interupt execution
        //if (hashKey != hashFromScratch){
        //    std::cout<<"Make Move\n";
        //    std:: cout<< "move: " ; printMove(move);
        //    printBoard();
        //    std::cout<<"hash key should be: " << hashFromScratch;
        //    getchar();
        //}

        // make sure king is not in check
        if (isSquareAttacked(board, (board.side == white) ? getLeastSigBitIndex(board.bitBoard[k]) : getLeastSigBitIndex(board.bitBoard[K]) , board.side)){
            // move is illega, restore board
            restoreBoard();

            return 0;
        }
        else{
            return 1;
        }
    }
    // capture moves
    else{
        // make sure move is capture
        if (getMoveCapture(move))
            return makeMove(board, move, allMoves);
        
        else
            // move is not a capture
            return 0;
        
    }
}

void generateMove(const BoardState& board, Moves *moveList){
    moveList ->count = 0;

    // define source and target squares
    int sourceSquare, targetSquare;

    // define current piece bitboard copy and its attacks
    uint64_t bitboard, attacks;

    //we need to loop over all the bitboards
    for (int piece = P ; piece <= k ; piece++){
        //init piece bitboard copy
        bitboard = board.bitBoard[piece];

        // generate white pawns and white king castling moves
        if (board.side == white){
            // pick up white pawn bitboards index
            if (piece == P){
                // WE COULD OPTIMIZE THIS
                // loop over white pawns within white pawn bitboard
                while (bitboard){
                    // init source square
                    sourceSquare = getLeastSigBitIndex(bitboard);
                    //std::cout<< "white pawns: " << squareToCoordinates[sourceSquare] << "\n";

                    // init target square
                    targetSquare = sourceSquare - 8;

                    // generate quiet pawn moves
                    if (!(targetSquare < a8) && !getBit(board.occupancies[both], targetSquare)){
                        // pawn promotion
                        if (sourceSquare >= a7 && sourceSquare <= h7){
                            // add to move list
                            addMove(moveList, encodeMove(sourceSquare, targetSquare, piece, Q, 0, 0, 0, 0));
                            addMove(moveList, encodeMove(sourceSquare, targetSquare, piece, R, 0, 0, 0, 0));
                            addMove(moveList, encodeMove(sourceSquare, targetSquare, piece, B, 0, 0, 0, 0));
                            addMove(moveList, encodeMove(sourceSquare, targetSquare, piece, N, 0, 0, 0, 0));
                        }else{
                            // one square ahead
                            addMove(moveList, encodeMove(sourceSquare, targetSquare, piece, 0, 0, 0, 0, 0));
                            // two squares ahead
                            if ((sourceSquare >= a2 && sourceSquare <= h2) && !getBit(board.occupancies[both], targetSquare - 8)){
                                addMove(moveList, encodeMove(sourceSquare, targetSquare - 8, piece, 0, 0, 1, 0, 0));
                            } 
                        }
                    }

                    // init pawn attacks bitboard
                    attacks = pawnAttacks[board.side][sourceSquare] & board.occupancies[black];
                    // generate pawn captures
                    while (attacks){
                        //init target square
                        targetSquare = getLeastSigBitIndex(attacks);

                        if (sourceSquare >= a7 && sourceSquare <= h7){
                            // add to move list
                            addMove(moveList, encodeMove(sourceSquare, targetSquare, piece, Q, 1, 0, 0, 0));
                            addMove(moveList, encodeMove(sourceSquare, targetSquare, piece, R, 1, 0, 0, 0));
                            addMove(moveList, encodeMove(sourceSquare, targetSquare, piece, B, 1, 0, 0, 0));
                            addMove(moveList, encodeMove(sourceSquare, targetSquare, piece, N, 1, 0, 0, 0));
                        }else{
                            // one square ahead
                            addMove(moveList, encodeMove(sourceSquare, targetSquare, piece, 0, 1, 0, 0, 0));
                        }
                        popBit(attacks, targetSquare);
                    }

                    //generate enpassant captures
                    if (board.enPassant != no_sq){
                        // lookup pawn attacks and & with enpassant bit
                        uint64_t enPassantAttacks = pawnAttacks[board.side][sourceSquare] & (1ULL << board.enPassant);
                        // make sure enpassant capture available
                        if (enPassantAttacks){
                            //init enpassant capture target
                            int targetEnPassant = getLeastSigBitIndex(enPassantAttacks);
                            addMove(moveList, encodeMove(sourceSquare, targetEnPassant, piece, 0, 1, 0, 1, 0));
                        }
                    }

                    // pop leastsig1bit from piece bitbaord copy
                    popBit(bitboard, sourceSquare);
                }
            }
            //castling moves
            if (piece == K){
                //king side castling
                if (board.castle & wk){
                    // make sure squares between king and rook are emtpy
                    if (!getBit(board.occupancies[both], f1) && !getBit(board.occupancies[both], g1)){
                        // make sure king not in check and f1 square are not under attack
                        if(!isSquareAttacked(board, e1, black) && !isSquareAttacked(board, f1, black)){
                            addMove(moveList, encodeMove(e1, g1, piece, 0, 0, 0, 0, 1));

                        }
                    }
                }
                //queen side castling
                if (board.castle & wq){
                    // make sure squares between king and rook are emtpy
                    if (!getBit(board.occupancies[both], d1) && !getBit(board.occupancies[both], c1) && !getBit(board.occupancies[both], b1)){
                        // make sure king not in check and f1 square are not under attack
                        if(!isSquareAttacked(board ,e1, black) && !isSquareAttacked(board, d1, black)){ // deleted  && !isSquareAttacked(b1, black)
                            addMove(moveList, encodeMove(e1, c1, piece, 0, 0, 0, 0, 1));
                        }
                    }
                }
            }

        }
        //THIS IS WHEN BLACKKKKKKKK
        // generate black pawns and black king castling moves
        else{
            // pick up white pawn bitboards index
            if (piece == p){
                // WE COULD OPTIMIZE THIS
                // loop over white pawns within white pawn bitboard
                while (bitboard){
                    // init source square
                    sourceSquare = getLeastSigBitIndex(bitboard);
                    //std::cout<< "white pawns: " << squareToCoordinates[sourceSquare] << "\n";
                    // init target square
                    targetSquare = sourceSquare + 8;

                    // generate quiet pawn moves
                    if (!(targetSquare > h1) && !getBit(board.occupancies[both], targetSquare)){
                        // pawn promotion
                        if (sourceSquare >= a2 && sourceSquare <= h2){
                            // add to move list
                            addMove(moveList, encodeMove(sourceSquare, targetSquare, piece, q, 0, 0, 0, 0));
                            addMove(moveList, encodeMove(sourceSquare, targetSquare, piece, r, 0, 0, 0, 0));
                            addMove(moveList, encodeMove(sourceSquare, targetSquare, piece, b, 0, 0, 0, 0));
                            addMove(moveList, encodeMove(sourceSquare, targetSquare, piece, n, 0, 0, 0, 0));
                        }else{
                            // one square ahead
                            addMove(moveList, encodeMove(sourceSquare, targetSquare, piece, 0, 0, 0, 0, 0));
                            // two squares ahead
                            if ((sourceSquare >= a7 && sourceSquare <= h7) && !getBit(board.occupancies[both], targetSquare + 8)){
                                addMove(moveList, encodeMove(sourceSquare, targetSquare + 8, piece, 0, 0, 1, 0, 0));
                            }
                            
                        }
                    }

                    // init pawn attacks bitboard BLACK
                    attacks = pawnAttacks[board.side][sourceSquare] & board.occupancies[white];
                    // generate pawn captures
                    while (attacks){
                        //init target square
                        targetSquare = getLeastSigBitIndex(attacks);

                        if (sourceSquare >= a2 && sourceSquare <= h2){
                            // add to move list
                            addMove(moveList, encodeMove(sourceSquare, targetSquare, piece, q, 1, 0, 0, 0));
                            addMove(moveList, encodeMove(sourceSquare, targetSquare, piece, r, 1, 0, 0, 0));
                            addMove(moveList, encodeMove(sourceSquare, targetSquare, piece, b, 1, 0, 0, 0));
                            addMove(moveList, encodeMove(sourceSquare, targetSquare, piece, n, 1, 0, 0, 0));
                            
                        }else{
                            // one square ahead
                            addMove(moveList, encodeMove(sourceSquare, targetSquare, piece, 0, 1, 0, 0, 0));
                        }
                        popBit(attacks, targetSquare);
                    }

                    //generate enpassant captures
                    if (board.enPassant != no_sq){
                        // lookup pawn attacks and & with enpassant bit
                        uint64_t enPassantAttacks = pawnAttacks[board.side][sourceSquare] & (1ULL << board.enPassant);
                        // make sure enpassant capture available
                        if (enPassantAttacks){
                            //init enpassant capture target
                            int targetEnPassant = getLeastSigBitIndex(enPassantAttacks);
                            addMove(moveList, encodeMove(sourceSquare, targetEnPassant, piece, 0, 1, 0, 1, 0));
                        }
                    }

                    // pop leastsig1bit from piece bitbaord copy
                    popBit(bitboard, sourceSquare);
                }
            }

            if (piece == k){
                //king side castling
                if (board.castle & bk){
                    // make sure squares between king and rook are emtpy
                    if (!getBit(board.occupancies[both], f8) && !getBit(board.occupancies[both], g8)){
                        // make sure king not in check and f1 square are not under attack
                        if(!isSquareAttacked(board, e8, white) && !isSquareAttacked(board, f8, white)){
                            addMove(moveList, encodeMove(e8, g8, piece, 0, 0, 0, 0, 1));
                        }
                    }
                }
                //queen side castling
                if (board.castle & bq){
                    // make sure squares between king and queen are emtpy
                    if (!getBit(board.occupancies[both], d8) && !getBit(board.occupancies[both], c8)&& !getBit(board.occupancies[both], b8)){
                        // make sure king not in check and f8 square are not under attack
                        if(!isSquareAttacked(board, e8, white) && !isSquareAttacked(board, d8, white)){ //removed && !isSquareAttacked(b1, black)
                            addMove(moveList, encodeMove(e8, c8, piece, 0, 0, 0, 0, 1));
                        }
                    }
                }
            }
        }
        

        // generate knight moves

        if ((board.side == white) ? piece == N : piece == n){
            while (bitboard){
                sourceSquare = getLeastSigBitIndex(bitboard);

                // init piece attacks
                attacks = knightAttacks[sourceSquare] & ((board.side == white) ? ~ board.occupancies[white] : ~board.occupancies[black]);

                //loop over target squares availalbe from generated attacks
                while (attacks){
                    targetSquare = getLeastSigBitIndex(attacks);
                    

                    // quiet move
                    if (!getBit(((board.side == white) ? board.occupancies[black] : board.occupancies[white]), targetSquare)){
                        addMove(moveList, encodeMove(sourceSquare, targetSquare, piece, 0, 0, 0, 0, 0));
                    }
                    else{
                        // capture move
                        addMove(moveList, encodeMove(sourceSquare, targetSquare, piece, 0, 1, 0, 0, 0));
                    }
                    popBit(attacks, targetSquare);
                }

                popBit(bitboard, sourceSquare);
            }
        }
        // generate bishop moves
        if ((board.side == white) ? piece == B : piece == b){
            while (bitboard){
                sourceSquare = getLeastSigBitIndex(bitboard);

                // init piece attacks
                attacks = getBishopAttacks(sourceSquare, board.occupancies[both]) & ((board.side == white) ? ~board.occupancies[white] : ~board.occupancies[black]);

                //loop over target squares availalbe from generated attacks
                while (attacks){
                    targetSquare = getLeastSigBitIndex(attacks);
                    

                    // quiet move
                    if (!getBit(((board.side == white) ? board.occupancies[black] : board.occupancies[white]), targetSquare)){
                        addMove(moveList, encodeMove(sourceSquare, targetSquare, piece, 0, 0, 0, 0, 0));
                    }
                    else{
                        // capture move
                        addMove(moveList, encodeMove(sourceSquare, targetSquare, piece, 0, 1, 0, 0, 0));
                    }
                    popBit(attacks, targetSquare);
                }

                popBit(bitboard, sourceSquare);
            }
        }
        // generate rook moves
        if ((board.side == white) ? piece == R : piece == r){
            while (bitboard){
                sourceSquare = getLeastSigBitIndex(bitboard);

                // init piece attacks
                attacks = getRookAttacks(sourceSquare, board.occupancies[both]) & ((board.side == white) ? ~board.occupancies[white] : ~board.occupancies[black]);

                //loop over target squares availalbe from generated attacks
                while (attacks){
                    targetSquare = getLeastSigBitIndex(attacks);
                    

                    // quiet move
                    if (!getBit(((board.side == white) ? board.occupancies[black] : board.occupancies[white]), targetSquare)){
                        addMove(moveList, encodeMove(sourceSquare, targetSquare, piece, 0, 0, 0, 0, 0));
                    }
                    else{
                        // capture move
                        addMove(moveList, encodeMove(sourceSquare, targetSquare, piece, 0, 1, 0, 0, 0));
                    }
                    popBit(attacks, targetSquare);
                }

                popBit(bitboard, sourceSquare);
            }
        }
        // generate queen moves
        if ((board.side == white) ? piece == Q : piece == q){
            while (bitboard){
                sourceSquare = getLeastSigBitIndex(bitboard);

                // init piece attacks
                attacks = getQueenAttacks(sourceSquare, board.occupancies[both]) & ((board.side == white) ? ~board.occupancies[white] : ~board.occupancies[black]);

                //loop over target squares availalbe from generated attacks
                while (attacks){
                    targetSquare = getLeastSigBitIndex(attacks);
                    

                    // quiet move
                    if (!getBit(((board.side == white) ? board.occupancies[black] : board.occupancies[white]), targetSquare)){
                        addMove(moveList, encodeMove(sourceSquare, targetSquare, piece, 0, 0, 0, 0, 0));
                    }
                    else{
                        // capture move
                        addMove(moveList, encodeMove(sourceSquare, targetSquare, piece, 0, 1, 0, 0, 0));
                    }
                    popBit(attacks, targetSquare);
                }

                popBit(bitboard, sourceSquare);
            }
        }
        // generate king moves

        if ((board.side == white) ? piece == K : piece == k){
            while (bitboard){
                sourceSquare = getLeastSigBitIndex(bitboard);

                // init piece attacks
                attacks = kingAttacks[sourceSquare] & ((board.side == white) ? ~ board.occupancies[white] : ~board.occupancies[black]);

                //loop over target squares availalbe from generated attacks
                while (attacks){
                    targetSquare = getLeastSigBitIndex(attacks);
                    

                    // quiet move
                    if (!getBit(((board.side == white) ? board.occupancies[black] : board.occupancies[white]), targetSquare)){
                        addMove(moveList, encodeMove(sourceSquare, targetSquare, piece, 0, 0, 0, 0, 0));
                    }
                    else{
                        // capture move
                        addMove(moveList, encodeMove(sourceSquare, targetSquare, piece, 0, 1, 0, 0, 0));
                    }
                    popBit(attacks, targetSquare);
                }

                popBit(bitboard, sourceSquare);
            }
        }
    }
}
