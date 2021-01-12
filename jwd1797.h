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
// int ready;
// step direction output to disk drive interface (0 = out->track00, 1 = in->track39)
// int stepDirection;
// step pulse output to disk drive interface (MFM - 2 microseconds, FM - 4)
// int stepPulse;

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

int command_action_done;  // flag indicates if the command action is done
int command_done; // flag indicating that entire command is done -
                  // including verification and delay -
int head_settling_done;
int terminate_command;

// TESTING
double master_timer;

// ALL timers in microseconds
double index_pulse_timer;
double index_encounter_timer;
double step_timer;
double verify_head_settling_timer;
double command_typeII_timer;
double command_typeIII_timer;
double command_typeIV_timer;
double HLD_idle_reset_timer;
double HLT_timer;

// DRIVE pins
int index_pulse_pin;
int ready_pin;
int tg43_pin;
int HLD_pin;
int HLT_pin;
int not_track00_pin;
int direction_pin;
// int not_test_pin;

int delayed_HLD;
int HLT_timer_active;

// computer interface pins
int drq;  /* also appears as status bit 1 during read/write operations. It is set
  high when a byte is assembled and transferred to the data register to be sent
  to the processor data bus on read operations. It is cleared when the data
  register is read by the processor. On writes, this is set high when a byte is
  transferred to the data shift register and another byte is requested from the
  processor to be written to disk. It is reset (cleared) when a new byte is loaded
  into the data register to be written. */
int intrq;  // attached to I0 of the slave 8259 interrupt controller in the Z-100.
  /* It is set at the completion of every command and is reset by reading the status
    register or by loading the command register with a new command. It is also
    set when a forced interrupt condition is met. */

int current_track;


} JWD1797;

JWD1797* newJWD1797();
void resetJWD1797(JWD1797*);
void writeJWD1797(JWD1797*, unsigned int, unsigned int);
unsigned int readJWD1797(JWD1797*, unsigned int);
void doJWD1797Cycle(JWD1797*, double);
void doJWD1797Command(JWD1797*);

void commandStep(JWD1797*, double);
void printAllRegisters(JWD1797*);
void printCommandFlags(JWD1797*);
void typeIStatusReset(JWD1797*);
void setupForcedIntCommand(JWD1797*);
void setupTypeICommand(JWD1797*);
void setTypeICommand(JWD1797*);
void setupTypeIICommand(JWD1797*);
void setTypeIICommand(JWD1797*);
void setupTypeIIICommand(JWD1797*);
void setTypeIIICommand(JWD1797*);
void printBusyMsg();
void updateTG43Signal(JWD1797*);
void handleIndexPulse(JWD1797*);
void handleHLDIdle(JWD1797*, double);
void handleHLTTimer(JWD1797*, double);
