// WD1797 Implementation
// By: Joe Matta
// email: jmatta1980@hotmail.com
// November 2020

// jwd1797.c

#include <stdlib.h>
#include <stdio.h>
# include "jwd1797.h"

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

// the wd1797 has a 1MHz clock in the Z100

// number of sectors per track can be 1-255
// number of tracks can be from 0-255
// IBM 3740 - 128 bytes/sector :: 26 sectors/track
/* System 34 - 256 bytes/sector :: 26 sectors/track
              1024 bytes/sector :: 8 sectors/track */


void resetJWD1797(JWD1797* jwd_controller) {
  // reset all fields and registers
  //...

  // open current file "in" drive
  disk_img = fopen("z-dos-1.img", "rb");
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
	printf("Write to wd1797: %X %X\n", port_addr, value);

	switch(port_addr) {
		case 0xb0 :	// command port
			jwd_controller->commandRegister = value;
			// w->statusPortInterrupt=0;
			doJWD1797Command(jwd_controller);
			break;
		case 0xb1 :
			break;
		case 0xb2 :
			break;
		case 0xb3 :
			break;
		case 0xb4 :
			break;
		default :
			printf("%X is an invalid port!\n", port_addr);
		}

}

void doJWD1797Cycle(JWD1797* w, double us) {

}

/* WD1797 accepts 11 different commands - this function will register the
	command and set all paramenters associated with it */
void doJWD1797Command(JWD1797* w) {
  /* command words should only be loaded into the command register when the
    BUSY status bit is off (status bit 0). One exception is the force interrupt.
    - When a command is being executed - the BUSY status is set
    - When a command is completed, an interrupt is generated and
    the BUSY status bit is reset */

  // TYPE I Commands - Restore, Seek, Step, Step-In, Step-Out
	// bit	--	def
	// 0		--	stepping motor rate0 (r0)
	// 1		--	stepping motor rate1 (r1)
	// 2		--	Verify on destination track? (V)
	// 3		-- 	Head load flag (0 = load head at beginning, 1 = unload head)
	// 4-7	--	RESTORE = 0000 / SEEK = 0001 (bits 7654)
	// --- T = Track update flag (0-do not update track register, 1-update)
	// 4-7	--	STEP = 001T / STEP-IN = 010T / STEP-OUT = 011T (bits 7654)

	/* determine if command in command register is a TYPE I command by checking
		if the 7 bit is a zero (all TYPE I commands have a zero (0) in the 7 bit) */
	if((w->commandRegister>>7) & 1 == 0) {
		printf("TYPE I Command in WD1797 command register..\n");
		w->currentCommandType = 1;
		// establish step rate options for 1MHz clock (only used with TYPE I cmds)
		int rates[] = {6, 12, 20, 30};
		// set flags according to command
		int rateBits = w->commandRegister&3;
		w->stepRate = rates[rateBits];
		w->verifyFlag = (w->commandRegister>>2) & 1;
		w->headLoadFlag = (w->commandRegister>>3) & 1;

		// get high bits of command register to determine the specific command
		int hbits = ((w->commandRegister>>4) & 15);	// examine 4 high bits

		if(hbits < 2) {	// RESTORE or SEEK command
			if(hbits&1 == 0) {	// RESTORE command
				// restore command detected
				w->currentCommandName = "RESTORE";
				printf("%s command in WD1797 command register\n", w->currentCommandName);
				// do restore stuff.....
			}
			else {	// SEEK command
				// seek command detected
				w->currentCommandName = "SEEK";
				printf("%s command in WD1797 command register\n", w->currentCommandName);
				// do seek stuff.....
			}
		}
		else {	// STEP, STEP-IN or STEP-OUT command
			// set track register update flag
			w->trackUpdateFlag = (w->commandRegister>>4) & 1;
			// determine which command by examining highest three bits of cmd reg
			int cmdID = (w->commandRegister>>5) & 3;
			if(cmdID == 1) {	// STEP
				// seek command detected
				w->currentCommandName = "STEP";
				printf("%s command in WD1797 command register\n", w->currentCommandName);
				// do seek stuff....
			}
			else if(cmdID == 2) {	//STEP-IN
				// seek command detected
				w->currentCommandName = "STEP-IN";
				printf("%s command in WD1797 command register\n", w->currentCommandName);
				// do seek stuff....
			}
			else {	// STEP-OUT
				// seek command detected
				w->currentCommandName = "STEP-OUT";
				printf("%s command in WD1797 command register\n", w->currentCommandName);
				// do seek stuff....
			}
		}

	}

}

int commandStep(JWD1797* w, double us) {

}


/* helper function to compute CRC */
void computeCRC(int initialValue, int* bytes, int len, int* result)
{
	unsigned short initial=(unsigned short)initialValue;

	unsigned short temp,a;
	unsigned short table[256];
	unsigned short poly=4129;

	for(int i=0; i<256; i++)
	{
		temp=0;
		a=(unsigned short)(i<<8);
		for(int j=0; j<8; j++)
		{
			if (((temp^a)&0x8000)!=0)
				temp=(unsigned short)((temp<<1)^poly);
			else
				temp<<=1;
		}
		table[i]=temp;
	}

	unsigned short crc=initial;
	for(int i=0; i<len; i++)
	{
		crc = (unsigned short)((crc<<8)^table[((crc>>8)^(0xff & bytes[i]))]);
	}
	result[0]=crc & 0xff;
	result[1]=(crc>>8)&0xff;
}
