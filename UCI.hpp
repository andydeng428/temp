#ifndef UCI_HPP
#define UCI_HPP

// External declarations for global variables
extern int quit;
extern int movesToGo;
extern int moveTime;
extern int timeUCI;
extern int inc;
extern int startTime;
extern int stopTime;
extern int timeSet;
extern int stopped;

// Function declarations
int inputWaiting();
void readInput();
void parsePosition(char *command);
void parseGo(char *command);
void uciLoop();
void communicate();

#endif // UCI_HPP
