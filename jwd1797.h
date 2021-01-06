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

// keep track of current byte being pointed to by the READ/WRITE head
int disk_img_index_pointer;

// ready input from disk drive interface (0 = not ready, 1 = ready)
int ready;
// step direction output to disk drive interface (0 = out->track00, 1 = in->track39)
int stepDirection;
// step pulse output to disk drive interface (MFM - 2 microseconds, FM - 4)
int stepPulse;

// stepping motor rate - determined by TYPE I command bits 0 and 1
int stepRate;

char* currentCommandName;
int currentCommandType;

// TYPE I command flags
int verifyFlag;
int headLoadFlag;
int trackUpdateFlag;

// TYPE II/III command flags
int dataAddressMark;
int updateSSO;
int delay15ms;
int swapSectorLength;
int multipleRecords;

// TYPE IV - interrupt condition flags
int interruptNRtoR;
int interruptRtoNR;
int interruptIndexPulse;
int interruptImmediate;

int command_done;

// TESTING
double master_timer;

double index_pulse_timer;
double index_encounter_timer;
double step_timer;
// index pulse (IP) pin from drive to controller
int index_pulse;
// DRIVE pins
int ready_pin;
int tg43_pin;
int HLD_pin;
int HLT_pin;
int track00_not_pin;
int direction_pin;
int test_not_pin;

// computer interface pins
int drq;
int intrq;

//
int current_track;


} JWD1797;

JWD1797* newJWD1797();
void resetJWD1797(JWD1797*);
void writeJWD1797(JWD1797*, unsigned int, unsigned int);
unsigned int readJWD1797(JWD1797*, unsigned int);
void doJWD1797Cycle(JWD1797*, double);
void doJWD1797Command(JWD1797*);

int commandStep(JWD1797*, double);
void printAllRegisters(JWD1797*);
void printCommandFlags(JWD1797*);
void type_I_Status_Reset(JWD1797*);
void setupForcedIntCommand(JWD1797*);
void setupTypeICommand(JWD1797*);
void setTypeICommand(JWD1797*);
void setupTypeIICommand(JWD1797*);
void setTypeIICommand(JWD1797*);
void setupTypeIIICommand(JWD1797*);
void setTypeIIICommand(JWD1797*);
void printBusyMsg();
