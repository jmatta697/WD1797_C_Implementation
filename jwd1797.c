// WD1797 Implementation
// By: Joe Matta
// email: jmatta1980@hotmail.com
// November 2020

// jwd1797.c

#include <stdlib.h>
#include <stdio.h>
#include "jwd1797.h"
#include "utility_functions.h"

// an index hole is encountered every 0.2 seconds with a 300 RPM drive
#define INDEX_HOLE_ENCOUNTER_US 200000
// index hole pulses last for 20 microseconds (WD1797 docs)
#define INDEX_HOLE_PULSE_US 20
// head load timing (HLT pin reamins low for this amount of time)
#define HEAD_LOAD_TIMING 50

// extern e8259_t* e8259_slave;

FILE* disk_img;

JWD1797* newJWD1797() {
	JWD1797* jwd_controller = (JWD1797*)malloc(sizeof(JWD1797));
	return jwd_controller;
}

/* INTRQ (pin connected to slave PIC IRQ3 in the Z100) is set to high at the
  completion of every command and when a force interrupt condition is met. It is
  reset (set to low) when the status register is read or when the commandRegister
  is loaded with a new command */

// the wd1797 has a 1MHz clock in the Z100 for 5.25" floppy

void resetJWD1797(JWD1797* jwd_controller) {
  // reset all fields and registers
  //...
	jwd_controller->statusRegister = 0x00;

	jwd_controller->master_timer = 0.0;
	jwd_controller->index_pulse_timer = 0.0;
	jwd_controller->index_encounter_timer = 0.0;
	jwd_controller->step_timer = 0.0;
	// index pulse (IP) pin from drive to controller
	jwd_controller->index_pulse = 0;
	jwd_controller->stepDirection = 0;
	// processor pins reset
	jwd_controller->drq = 0;
	jwd_controller->intrq = 0;

	jwd_controller->interruptNRtoR = 0;
	jwd_controller->interruptRtoNR = 0;
	jwd_controller->interruptIndexPulse = 0;
	jwd_controller->interruptImmediate = 0;

	jwd_controller->currentCommandName = "";
	jwd_controller->currentCommandType = 0;

	jwd_controller->current_track = 0;

  // open current file "in" drive
  disk_img = fopen("z-dos-1.img", "rb");
	// point to first byte on disk (byte 0x00 in loaded .img file)
	jwd_controller->disk_img_index_pointer = 0;
}

// read data from wd1797 according to port
unsigned int readJWD1797(JWD1797* jwd_controller, unsigned int port_addr) {

}

// write data to wd1797 based on port address
void writeJWD1797(JWD1797* jwd_controller, unsigned int port_addr, unsigned int value) {
  /* whenever a READ/WRITE command (command type II or III) is received, the READY
    input from the drive is sampled. If the ready signal is low (0), the command
    is not executed and an interrupt is generated. ALl type I commands are
    performed regardless of the state of the READY pin from the drive. */
	printf("\nWrite ");
	print_bin8_representation(value);
	printf("%s%X\n\n", " to wd1797/port: ", port_addr);

	switch(port_addr) {
		// command reg port
		case 0xb0:
			jwd_controller->commandRegister = value;
			doJWD1797Command(jwd_controller);
			break;
		// track reg port
		case 0xb1:
			break;
		// sector reg port
		case 0xb2:
			break;
		// data reg port
		case 0xb3:
			break;
		// control latch port
		case 0xb4:
			break;
		default:
			printf("%X is an invalid port!\n", port_addr);
	}

}

/* clocks the WD1797 chip (Z-100 uses a 1 MHz clock)
	a cycle should happen every 0.000001 seconds - every 1 microsecond
  main program will add the amount of calculated time from the previous
	instruction to the internal WD1797 timers */
void doJWD1797Cycle(JWD1797* w, double us) {
	// DEBUG clock
	w->master_timer += us;

	w->index_encounter_timer += us;
	// only increment index pulse timer if index pulse is high (1)
	if(w->index_pulse) {
		w->index_pulse_timer += us;
	}

	// check index pulse encountered
	if(!w->index_pulse && w->index_encounter_timer >= INDEX_HOLE_ENCOUNTER_US) {
		w->index_pulse = 1;
		// reset index hole encounter timer
		w->index_encounter_timer = 0.0;
	}
	// check index pulse timer
	if(w->index_pulse && w->index_pulse_timer >= INDEX_HOLE_PULSE_US) {
		w->index_pulse = 0;
		// reset index pulse timer
		w->index_pulse_timer = 0.0;
	}
	// check if command is still active and do command step if so...
	if(!w->command_done) {
		// do command step
		commandStep(w, us);
	}

	/* check if instruction is completed - generate INTERRUPT (INTRQ connected
	to I0 of slave interrupt controller -> I3 of master int controller */

}

/* WD1797 accepts 11 different commands - this function will register the
	command and set all paramenters associated with it */
void doJWD1797Command(JWD1797* w) {
	// get busy status bit from status register (bit 0)
	int busy = w->statusRegister & 1;

	// if the 4 high bits are 0b1101, the command is a force interrupt
	if(((w->commandRegister>>4) & 15) == 13) {
		setupForcedIntCommand(w);
	}
	/* determine if command in command register is a TYPE I command by checking
		if the 7 bit is a zero (noly TYPE I commands have a zero (0) in the 7 bit) */
	else if(((w->commandRegister>>7) & 1) == 0) {
		// check busy status
		if(busy) {
			printBusyMsg();
			return;
		}
		setupTypeICommand(w);
		setTypeICommand(w);
	}
	/* Determine if command in command register is TYPE II
		 by checking the highest 3 bits. The two TYPE II commands have either 0b100
		 (Read Sector) or 0b101 (Write Sector) as the high 3 bits */
	else if(((w->commandRegister>>5) & 7) < 6) {
		printf("TYPE II Command in WD1797 command register..\n");
		// check busy status
		if(busy) {
			printBusyMsg();
			return;
		}
		setupTypeIICommand(w);
		setTypeIICommand(w);
	}
	/* Determine if command in command register is TYPE III
		 by checking the highest 3 bits. TYPE III commands have a higher value
		 then 5 in their shifted 3 high bits */
	else if(((w->commandRegister>>5) & 7) > 5) {
		printf("TYPE III Command in WD1797 command register..\n");
		// check busy status
		if(busy) {
			printBusyMsg();
			return;
		}
		setupTypeIIICommand(w);
		setTypeIIICommand(w);
	}
	// check command register error
	else {
		printf("%s\n", "Something went wrong! BAD COMMAND BITS in COMMAND REG!");
	}
}

// execute command step
// us is the time that passed since the last CPU instruction
int commandStep(JWD1797* w, double us) {
	/* do what needs to be done based on which command is still active and based
		on certain timer */
		if(w->currentCommandName == "RESTORE") {
			//...
		}
}


/*
	+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	+++++++++++++++++ HELPER FUNCTIONS ++++++++++++++++++++++++++++++++++++++++++
	+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
/* helper function to compute CRC */
void computeCRC(int initialValue, int* bytes, int len, int* result) {
	unsigned short initial=(unsigned short)initialValue;
	unsigned short temp,a;
	unsigned short table[256];
	unsigned short poly=4129;

	for(int i=0; i<256; i++) {
		temp=0;
		a=(unsigned short)(i<<8);
		for(int j=0; j<8; j++) {
			if (((temp^a)&0x8000)!=0)
				temp=(unsigned short)((temp<<1)^poly);
			else
				temp<<=1;
		}
		table[i]=temp;
	}
	unsigned short crc=initial;
	for(int i=0; i<len; i++) {
		crc = (unsigned short)((crc<<8)^table[((crc>>8)^(0xff & bytes[i]))]);
	}
	result[0]=crc & 0xff;
	result[1]=(crc>>8)&0xff;
}

void printAllRegisters(JWD1797* w) {
	printf("\n%s\n", "WD1797 Registers:");
	printf("%s", "Status: ");
	print_bin8_representation(w->statusRegister);
	printf("\n%s", "Command: ");
	print_bin8_representation(w->commandRegister);
	printf("\n%s", "Sector: ");
	print_bin8_representation(w->sectorRegister);
	printf("\n%s", "Track: ");
	print_bin8_representation(w->trackRegister);
	printf("\n%s", "Data: ");
	print_bin8_representation(w->dataRegister);
	printf("\n%s", "DataShift: ");
	print_bin8_representation(w->dataShiftRegister);
	printf("\n%s", "CRC: ");
	print_bin8_representation(w->CRCRegister);
}

void printCommandFlags(JWD1797* w) {
	if(w->currentCommandType == 1) {
		printf("%s\n", "-- TYPE I COMMAND FLAGS --");
		printf("%s%d\n", "StepRate: ", w->stepRate);
		printf("%s%d\n", "Verify: ", w->verifyFlag);
		printf("%s%d\n", "HeadLoad: ", w->headLoadFlag);
		printf("%s%d\n", "TrackUpdate: ", w->trackUpdateFlag);
	}
	else if(w->currentCommandType == 2 || w->currentCommandType == 3) {
		printf("%s\n", "-- TYPE II/III COMMAND FLAGS --");
		printf("%s%d\n", "DataAddressMark: ", w->dataAddressMark);
		printf("%s%d\n", "UpdateSSO: ", w->updateSSO);
		printf("%s%d\n", "Delay15ms: ", w->delay15ms);
		printf("%s%d\n", "SwapSectorLength: ", w->swapSectorLength);
		printf("%s%d\n", "MultipleRecords: ", w->multipleRecords);
	}
	else if(w->currentCommandType == 4) {
		printf("%s\n", "-- TYPE IV COMMAND FLAGS/INTERRUPT CONDITIONS --");
		printf("%s%d\n", "InterruptNRtoR: ", w->interruptNRtoR);
		printf("%s%d\n", "InterruptRtoNR: ", w->interruptRtoNR);
		printf("%s%d\n", "InterruptIndexPulse: ", w->interruptIndexPulse);
		printf("%s%d\n", "InterruptImmediate: ", w->interruptImmediate);
	}
	else {
		printf("%s\n", "NO ACTIVE COMMAND!");
	}
}

void setupForcedIntCommand(JWD1797* w) {
	/* DEBUG/TESTING - clear all interrupt flags here  */
	w->interruptNRtoR = 0;
	w->interruptRtoNR = 0;
	w->interruptIndexPulse = 0;
	w->interruptImmediate = 0;
	// DEBUG ABOVE ^^^^
	printf("TYPE IV Command in WD1797 command register (Force Interrupt)..\n");
	w->currentCommandType = 4;
	// do force interrupt stuff...
	// get the interrupt condition bits (I0-I3) - the lowest 4 bits of the interrupt command
	int int_condition_bits = w->commandRegister & 15;
	// set interrupt condition(s)
	if(int_condition_bits & 1) {
		printf("%s\n", "INTRQ on NOT READY to READY transition");
		w->interruptNRtoR = 1;
	}
	if((int_condition_bits>>1) & 1) {
		printf("%s\n", "INTRQ on READY to NOT READY transition");
		w->interruptRtoNR = 1;
	}
	if((int_condition_bits>>2) & 1) {
		printf("%s\n", "INTRQ on INDEX PULSE");
		w->interruptIndexPulse = 1;
	}
	if((int_condition_bits>>3) & 1) {
		printf("%s\n", "INTRQ and IMMEDIATE INTERRUPT");
		w->interruptImmediate = 1;
	}
	if(int_condition_bits == 0) {
		printf("%s\n", "NO INTRQ and TERMINATE COMMAND IMMEDIATELY");
	}
}

void setupTypeICommand(JWD1797* w) {
	// printf("TYPE I Command in WD1797 command register..\n");
	w->currentCommandType = 1;
	w->command_done = 0;

	// set appropriate status bits for type I command
	type_I_Status_Reset(w);

	// establish step rate options for 1MHz clock (only used with TYPE I cmds)
	int rates[] = {6, 12, 20, 30};

	// get rate bit
	int rateBits = w->commandRegister & 3;
	// set flags according to command bits
	w->stepRate = rates[rateBits];
	w->verifyFlag = (w->commandRegister>>2) & 1;
	w->headLoadFlag = (w->commandRegister>>3) & 1;

	// HLD set according to V and h flags of type I command
	if(!w->headLoadFlag && !w->verifyFlag) {w->HLD_pin = 0;}
	else if(w->headLoadFlag && !w->verifyFlag) {w->HLD_pin = 1;}
}

void setTypeICommand(JWD1797* w) {
	// get 4 high bits of command register to determine the specific command
	int highBits = ((w->commandRegister>>4) & 15);	// examine 4 high bits

	if(highBits < 2) { // RESTORE or SEEK command
		if((highBits&1) == 0) {	// RESTORE command
			w->currentCommandName = "RESTORE";
			printf("%s command in WD1797 command register\n", w->currentCommandName);
			// do restore stuff.....
		}
		else if((highBits&1) == 1) {	// SEEK command
			w->currentCommandName = "SEEK";
			printf("%s command in WD1797 command register\n", w->currentCommandName);
			// do seek stuff.....
		}
		// check error
		else {
			printf("%s\n", "Something went wrong! Cannot determine RESTORE or SEEK!");
		}
	}
	else { // STEP, STEP-IN or STEP-OUT commands
		// set track register update flag
		w->trackUpdateFlag = (w->commandRegister>>4) & 1;
		// determine which command by examining highest three bits of cmd reg
		int cmdID = (w->commandRegister>>5) & 7;
		if(cmdID == 1) {	// STEP
			w->currentCommandName = "STEP";
			printf("%s command in WD1797 command register\n", w->currentCommandName);
			// do step stuff....
		}
		else if(cmdID == 2) {	//STEP-IN
			w->currentCommandName = "STEP-IN";
			printf("%s command in WD1797 command register\n", w->currentCommandName);
			// do step-in stuff....
		}
		else if(cmdID == 3)  {	// STEP-OUT
			w->currentCommandName = "STEP-OUT";
			printf("%s command in WD1797 command register\n", w->currentCommandName);
			// do step-out stuff....
		}
		// check error
		else {
			printf("%s\n", "Something went wrong! Cannot determine which TYPE I STEP command!");
		}
	}
}

void setupTypeIICommand(JWD1797* w) {
	w->currentCommandType = 2;
	w->command_done = 0;
	/* set TYPE II flags */
	w->updateSSO = (w->commandRegister>>1) & 1;
	w->delay15ms = (w->commandRegister>>2) & 1;
	w->swapSectorLength = (w->commandRegister>>3) & 1;
	w->multipleRecords = (w->commandRegister>>4) & 1;
}

void setTypeIICommand(JWD1797* w) {
	// determine which command by examining highest three bits of cmd reg
	int cmdID = (w->commandRegister>>5) & 7;
	// check if READ SECTOR (high 3 bits == 0b100)
	if(cmdID == 4) {
		w->currentCommandName = "READ SECTOR";
		printf("%s command in WD1797 command register\n", w->currentCommandName);
		// do read sector stuff....
	}
	else if(cmdID == 5) {
		w->currentCommandName = "WRITE SECTOR";
		printf("%s command in WD1797 command register\n", w->currentCommandName);
		// set Data Address Mark flag
		w->dataAddressMark = w->commandRegister & 1;
		// do write sector stuff....
	}
	// check error
	else {
		printf("%s\n", "Something went wrong! Cannot determine which TYPE II command!");
	}
}

void setupTypeIIICommand(JWD1797* w) {
	w->currentCommandType = 3;
	w->command_done = 0;
	/* set TYPE III flags */
	w->updateSSO = (w->commandRegister>>1) & 1;
	w->delay15ms = (w->commandRegister>>2) & 1;
}

void setTypeIIICommand(JWD1797* w) {
	// determine which command by examining highest 4 bits of cmd reg
	int cmdID = (w->commandRegister>>4) & 15;
	// READ ADDRESS
	if(cmdID == 12) {
		w->currentCommandName = "READ ADDRESS";
		printf("%s command in WD1797 command register\n", w->currentCommandName);
		// do read address stuff....
	}
	// READ TRACK
	else if(cmdID == 14) {
		w->currentCommandName = "READ TRACK";
		printf("%s command in WD1797 command register\n", w->currentCommandName);
		// do read track stuff....
	}
	// WRITE TRACK
	else if(cmdID == 15) {
		w->currentCommandName = "WRITE TRACK";
		printf("%s command in WD1797 command register\n", w->currentCommandName);
		// do write track stuff....
	}
	// check error
	else {
		printf("%s\n", "Something went wrong! Cannot determine which TYPE III command!");
	}
}

void type_I_Status_Reset(JWD1797* w) {
	// set BUSY bit
	w->statusRegister |= 0b00000001;
	// reset CRC ERROR, SEEK ERROR bits
	w->statusRegister &= 0b11100111;
	// reset DRQ, INTRQ pins
	w->drq = 0;
	w->intrq = 0;
}

void printBusyMsg() {
	printf("%s\n", "Cannot execute command placed into command register!");
	printf("%s\n", "Another command is currently processing! (BUSY STATUS)");
}
