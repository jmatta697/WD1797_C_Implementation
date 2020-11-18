// WD1797 Implementation
// By: Joe Matta
// email: jmatta1980@hotmail.com
// November 2020

// jwd1797.h

typedef struct {

unsigned char dataShiftRegister;
/* during the SEEK command, the dataRegister holds the address of the desired
  track position - otherwise it holds the assembled byte read from or writen
  to the dataShiftRegister */
unsigned char dataRegister;
/* holds track number of current read/write head position, increment every
  step-in toward track 76, and decrement every step-out toward track 00 */
unsigned char trackRegister;  // do not load when device is busy
/* holds desired sector position */
unsigned char sectorRegister; // do not load when device is busy
/* holds command currently being executed */
unsigned char commandRegister; // do not load when device is busy - except force int
/* holds device status information relevant to the previously executed command */
unsigned char statusRegister;
unsigned char CRCRegister;

// ready input from disk drive interface (0 = not ready, 1 = ready)
int ready;
// step direction output to disk drive interface (0 = out, 1 = in)
int stepDirection;
// step pulse output to disk drive interface (MFM - 2 microseconds, FM - 4)
int stepPulse;

// stepping motor rate - determined by TYPE I command bits 0 and 1
int stepRate;

char* currentCommandName;
int currentCommandType;
int verifyFlag;
int headLoadFlag;
int trackUpdateFlag;



} JWD1797;

JWD1797* newJWD1797();

void resetJWD1797(JWD1797*);
void writeJWD1797(JWD1797*, unsigned int addr, unsigned int value);
unsigned int readJWD1797(JWD1797*, unsigned int addr);
void doJWD1797Cycle(JWD1797*, double cycles);
void doJWD1797Command(JWD1797*);
