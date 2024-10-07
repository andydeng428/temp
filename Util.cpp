#include "Util.hpp"
#include "BitBoard.hpp"
#include "MoveGen.hpp"
#include "MaskGen.hpp"
#include "ZobristTable.hpp"
#include "Search.hpp"
#include <iostream>
#include <cstdint>
#include <sys/time.h>

const std::string squareToCoordinates[] = {
    "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
    "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
    "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
    "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
    "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
    "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
    "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
    "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
};

const std::string asciiPieces = "PNBRQKpnbrq";

const std::string unicodePieces[12] = {"♙", "♘", "♗", "♖", "♕", "♔", "♟︎", "♞", "♝", "♜", "♛", "♚"};

const std::unordered_map<char, int> charPieces = {
    {'P', P}, {'N', N}, {'B', B}, {'R', R}, {'Q', Q}, {'K', K},
    {'p', p}, {'n', n}, {'b', b}, {'r', r}, {'q', q}, {'k', k}
};

const std::unordered_map<int, char> promotedPieces = {
    {Q, 'q'},
    {R, 'r'},
    {B, 'b'},
    {N, 'n'},
    {q, 'q'},
    {r, 'r'},
    {b, 'b'},
    {n, 'n'}
};

unsigned int randomState = 1804289383;



//generate 32-bit pseudo legal number by using XOR shift algorithm
unsigned int getRandomU32BitNumber(){
    unsigned int number = randomState;

    //XOR shift algorithm
    number ^= number << 13;
    number ^= number >> 17;
    number ^= number << 5;

    randomState = number;
    return number;
}

//generate 64-bit pseudo legal numbers
uint64_t getRandomU64BitNumber(){
    // lets define 4 random numbers
    uint64_t n1, n2, n3, n4;

    n1 = static_cast<uint64_t>(getRandomU32BitNumber()) & 0xFFFF; 
    n2 = static_cast<uint64_t>(getRandomU32BitNumber()) & 0xFFFF; 
    n3 = static_cast<uint64_t>(getRandomU32BitNumber()) & 0xFFFF; 
    n4 = static_cast<uint64_t>(getRandomU32BitNumber()) & 0xFFFF;

    return n1 | (n2 << 16) | ( n3 << 32) | (n4 << 48);
}

uint64_t generateMagicNumber(){
    return getRandomU64BitNumber() & getRandomU64BitNumber() & getRandomU64BitNumber();
}

int getTimeMs()
{
    struct timeval timeValue;
    gettimeofday(&timeValue, NULL);
    return timeValue.tv_sec * 1000 + timeValue.tv_usec / 1000;
}

void printBoard(){
    // print offset
    std::cout<<"\n";

    // loop over board ranks
    for (int rank = 0; rank < 8; rank++)
    {
        // loop ober board files
        for (int file = 0; file < 8; file++)
        {
            // init square
            int square = rank * 8 + file;
            
            // print ranks
            if (!file)
                std::cout << "  " << 8 - rank << " ";
            
            // define piece variable
            int piece = -1;
            
            // loop over all piece bitboards
            for (int bb_piece = P; bb_piece <= k; bb_piece++)
            {
                if (getBit(board.bitBoard[bb_piece], square))
                    piece = bb_piece;
            }
            
            std::cout<< ((piece == -1) ? "." : unicodePieces[piece]) << " ";
        }
        
        // print new line every rank
        printf("\n");
    }
    
    // print board files
    printf("\n     a b c d e f g h\n\n");
    
    // print side to move
    printf("     Side:     %s\n", !board.side ? "white" : "black");
    
    // print enpassant square
    std::cout<< "     EnPassant:   " <<((board.enPassant != no_sq) ? squareToCoordinates[board.enPassant] : "no")<< std::endl;
    // print castling rights
    printf("     Castling:  %c%c%c%c\n\n", (board.castle & wk) ? 'K' : '-',
                                           (board.castle & wq) ? 'Q' : '-',
                                           (board.castle & bk) ? 'k' : '-',
                                           (board.castle & bq) ? 'q' : '-');
    // print hash key
    std::cout<< ("\n     Hashkey: ")<< board.hashKey << "\n";
}

void printBitboard(uint64_t bitBoard){
    for (int i = 0 ; i < 8 ; i++){ // ranks
        for (int k = 0 ; k < 8 ; k++){ // files
            //square index
            int square = i * 8 + k; 
            if (!k){
                std::cout << 8 - i << "  ";
            }
            std::cout << (getBit(bitBoard, square) ? 1 : 0) << " ";

        }
        std::cout<< std::endl;
    }
    std::cout << "   a b c d e f g h ";
    std::cout<< std::endl << bitBoard << std::endl;
}

void printMove(int move){
    if(getMovePromoted(move)){
            std::cout<<squareToCoordinates[getMoveSource(move)]
             << squareToCoordinates[getMoveTarget(move)]
             << promotedPieces.at(getMovePromoted(move));
    }
    else{
            std::cout<<squareToCoordinates[getMoveSource(move)]
             << squareToCoordinates[getMoveTarget(move)];
    }
}

void printMoveList(Moves *moveList){
    if (!moveList -> count){
        std::cout<< "no moves in move list";
        return;
    }
    std::cout<< "\n    move    piece   capture   double    enpass    castling\n\n";
    for (int i = 0 ; i < (moveList -> count) ; i ++){
        int move = moveList->moves[i];
        std::cout<< "    " <<squareToCoordinates[getMoveSource(move)]<<
                    squareToCoordinates[getMoveTarget(move)] <<
                    (getMovePromoted(move) ? promotedPieces.at(getMovePromoted(move)) : ' ') << "    " <<
                    asciiPieces[getMovePiece(move)] << "       " <<
                    (getMoveCapture(move) ? 1 : 0) << "       " <<
                    (getMoveDouble(move) ? 1 : 0) << "        " <<
                    (getMoveEnPassant(move) ? 1 : 0 )<< "          " <<
                    (getMoveCastling(move) ? 1 : 0) << "\n";

    }
    std::cout << "\n total number of moves:" << moveList->count;
}

void printAttackedSquares(BoardState& board, int side){
    for (int i = 0 ; i < 8 ; i++){ // ranks
        for (int k = 0 ; k < 8 ; k++){ // files
            //square index
            int square = i * 8 + k; 
            if (!k){
                std::cout << 8 - i << "  ";
            }
            std::cout<< (isSquareAttacked(board, square, side) ? 1 : 0) <<" ";

        }
        std::cout<< std::endl;
    }
    std::cout << "   a b c d e f g h ";
    std::cout<< std::endl << board.bitBoard << std::endl;
}

void printMoveScores(Moves *moveList){
    printf("     Move scores:\n\n");
    // loop over moves within a move list
    for (int count = 0; count < moveList->count; count++)
    {
        printf("     move: ");
        printMove(moveList->moves[count]);
        printf(" score: %d\n", scoreMove(board, moveList->moves[count]));
    }
}

void parseFen(std::string fenC) {
    //convert string into C-style string to use pointer arithmatic
    char copy[fenC.size() + 1];
    std::strcpy(copy, fenC.c_str());
    char *fen = copy;

    // Reset board position (bitboards)
    std::memset(board.bitBoard, 0ULL, sizeof(board.bitBoard));

    // Reset occupancies (bitboards)
    std::memset(board.occupancies, 0ULL, sizeof(board.occupancies));

    // Reset game state variables
    board.side = 0;
    board.enPassant = no_sq;
    board.castle = 0;

    // reset repetition index
    repetitionIndex = 0;

    // reset repetiion table
    memset(repetitionTable, 0ULL, sizeof(repetitionTable));

    // Loop over board ranks
    for (int rank = 0; rank < 8; rank++) {
        // Loop over board files
        for (int file = 0; file < 8; file++) {
            // Init current square
            int square = rank * 8 + file;

            // Match ASCII pieces within FEN string
            if ((*fen >= 'a' && *fen <= 'z') || (*fen >= 'A' && *fen <= 'Z')) {
                // Init piece type
                int piece = charPieces.at(*fen);

                // Set piece on corresponding bitboard
                setBit(board.bitBoard[piece], square);

                // Increment pointer to FEN string
                fen++;
            }

            // Match empty square numbers within FEN string
            if (*fen >= '0' && *fen <= '9') {
                // Init offset (convert char 0 to int 0)
                int offset = *fen - '0';

                // Define piece variable
                int piece = -1;

                // Loop over all piece bitboards
                for (int bbPiece = P; bbPiece <= k; bbPiece++) {
                    // If there is a piece on current square
                    if (getBit(board.bitBoard[bbPiece], square))
                        // Get piece code
                        piece = bbPiece;
                }

                // On empty current square
                if (piece == -1)
                    // Decrement file
                    file--;

                // Adjust file counter
                file += offset;

                // Increment pointer to FEN string
                fen++;
            }

            // Match rank separator
            if (*fen == '/')
                // Increment pointer to FEN string
                fen++;
        }
    }

    // Go to parsing side to move (increment pointer to FEN string)
    fen++;

    // Parse side to move
    (*fen == 'w') ? (board.side = white) : (board.side = black);

    // Go to parsing castling rights
    fen += 2;

    // Parse castling rights
    while (*fen != ' ') {
        switch (*fen) {
            case 'K': board.castle |= wk; break;
            case 'Q': board.castle |= wq; break;
            case 'k': board.castle |= bk; break;
            case 'q': board.castle |= bq; break;
            case '-': break;
        }

        // Increment pointer to FEN string
        fen++;
    }

    // Go to parsing en passant square (increment pointer to FEN string)
    fen++;

    // Parse en passant square
    if (*fen != '-') {
        // Parse en passant file & rank
        int file = fen[0] - 'a';
        int rank = 8 - (fen[1] - '0');

        // Init en passant square
        board.enPassant = rank * 8 + file;
    }

    // No en passant square
    else
        board.enPassant = no_sq;

    // Loop over white pieces bitboards
    for (int piece = P; piece <= K; piece++)
        // Populate white occupancy bitboard
        board.occupancies[white] |= board.bitBoard[piece];

    // Loop over black pieces bitboards
    for (int piece = p; piece <= k; piece++)
        // Populate black occupancy bitboard
        board.occupancies[black] |= board.bitBoard[piece];

    // Init all occupancies
    board.occupancies[both] |= board.occupancies[white];
    board.occupancies[both] |= board.occupancies[black];

    // init has key
    board.hashKey = generateHashKey(board);
}
