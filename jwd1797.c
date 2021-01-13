// WD1797 Implementation
// By: Joe Matta
// email: jmatta1980@hotmail.com
// November 2020

// Notes:
/* clock is assumed to be 1 MHz for single and double density mini-floppy disks
	(5.25" ) */
/* Z-DOS disks are 360k disks - 40 cylinders/2 heads (sides)/9 sectors per track/
	512 bytes per sector */
// A 300 RPM motor speed is also assumed
// NO write protect functionality

// jwd1797.c


#include <stdlib.h>
#include <stdio.h>
#include "jwd1797.h"
#include "utility_functions.h"

// ** ALL TIMINGS in microseconds **
// an index hole is encountered every 0.2 seconds with a 300 RPM drive
#define INDEX_HOLE_ENCOUNTER_US 200000
// index hole pulses last for 20 microseconds (WD1797 docs)
#define INDEX_HOLE_PULSE_US 20
// when non-busy status and HLD high, reset HLD after 15 index pulses
#define HLD_IDLE_RESET_LIMIT 15*INDEX_HOLE_ENCOUNTER_US
// head load timing (this can be set from 30-100 ms, depending on drive)
#define HEAD_LOAD_TIMING_LIMIT 60*1000	// set to 60 ms (60,000 us)
// verify time is 30 milliseconds for a 1MHz clock
#define VERIFY_HEAD_SETTLING_LIMIT 30*1000

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
	jwd_controller->dataShiftRegister = 0x00;
	jwd_controller->dataRegister = 0x00;
	jwd_controller->trackRegister = 0x00;
	jwd_controller->sectorRegister = 0x00;
	jwd_controller->commandRegister = 0x00;
	jwd_controller->statusRegister = 0x00;
	jwd_controller->CRCRegister = 0x00;

	jwd_controller->disk_img_index_pointer = 0;

	// jwd_controller->ready = 0;	// start drive not ready
	// jwd_controller->stepDirection = 0;	// start direction step out -> track 00

	jwd_controller->currentCommandName = "";
	jwd_controller->currentCommandType = 0;

	// TYPE I command bits
	jwd_controller->stepRate = 0;	// bits 0 and 1 determine the step rate
	jwd_controller->verifyFlag = 0;
	jwd_controller->headLoadFlag = 0;
	jwd_controller->trackUpdateFlag = 0;
	// TYPE II and III command bits
	jwd_controller->dataAddressMark = 0;
	jwd_controller->updateSSO = 0;
	jwd_controller->delay15ms = 0;
	jwd_controller->swapSectorLength = 0;
	jwd_controller->multipleRecords = 0;
	// TYPE IV command (forced interrupt) conditions
	jwd_controller->interruptNRtoR = 0;
	jwd_controller->interruptRtoNR = 0;
	jwd_controller->interruptIndexPulse = 0;
	jwd_controller->interruptImmediate = 0;
	// command step controls
	jwd_controller->command_action_done = 0;
	jwd_controller->command_done = 0;
	jwd_controller->head_settling_done = 0;

	jwd_controller->terminate_command = 0;

	jwd_controller->master_timer = 0.0;
	jwd_controller->index_pulse_timer = 0.0;
	jwd_controller->index_encounter_timer = 0.0;
	jwd_controller->step_timer = 0.0;
	jwd_controller->verify_head_settling_timer = 0.0;
	jwd_controller->command_typeII_timer = 0.0;
	jwd_controller->command_typeIII_timer = 0.0;
	jwd_controller->command_typeIV_timer = 0.0;
	jwd_controller->HLD_idle_reset_timer = 0.0;
	jwd_controller->HLT_timer = 0.0;

	jwd_controller->index_pulse_pin = 0;
	jwd_controller->ready_pin = 0;
	jwd_controller->tg43_pin = 0;
	jwd_controller->HLD_pin = 0;
	jwd_controller->HLT_pin = 0;
	jwd_controller->not_track00_pin = 0;
	jwd_controller->direction_pin = 0;
	// jwd_controller-> test_not_pin;

	jwd_controller->delayed_HLD = 0;
	jwd_controller->HLT_timer_active = 0;

	jwd_controller->drq = 0;
	jwd_controller->intrq = 0;

	jwd_controller->current_track = 0;

  // open current file "in" drive
  disk_img = fopen("z-dos-1.img", "rb");
}

// read data from wd1797 according to port
unsigned int readJWD1797(JWD1797* jwd_controller, unsigned int port_addr) {
	// printf("\nRead ");
	// printf("%s%X\n\n", " from wd1797/port: ", port_addr);

	unsigned int r_val = 0;

	switch(port_addr) {
		// status reg port
		case 0xb0:
			r_val = jwd_controller->statusRegister;
			break;
		// track reg port
		case 0xb1:
			r_val = jwd_controller->trackRegister;
			break;
		// sector reg port
		case 0xb2:
			break;
		// data reg port
		case 0xb3:
			break;
		// control latch reg port (write)
		case 0xb4:
			printf(" ** NOTICE: Reading from WD1797 control latch port 0xB4 (write only)!\n");
			break;
		// controller status port (read)
		case 0xb5:
			break;
		default:
			printf("%X is an invalid port!\n", port_addr);
	}
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
		// controller status port
		case 0xb5:
			printf(" ** NOTICE: Writing to WD1797 status port 0xB5 (read only)!\n");
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
	w->master_timer += us;	// @@@ DEBUG clock @@@

	// update not_track00_pin
	// check track and set not_track00_pin accordingly
	if(w->current_track == 0) {w->not_track00_pin = 0;}
	else {w->not_track00_pin = 1;}
	// update type I status bit 2 (S2) for track 00 status
	if(w->currentCommandType == 1 && !w->not_track00_pin) {
		w->statusRegister |= 0b00000100;
	}
	// clear TYPE I status bit 2 (S2) if not on track 00
	else {w->statusRegister &= 0b11111011;}

	handleIndexPulse(w, us);

	handleHLTTimer(w, us);
	// Type I status bit 5 (S5) will be set if HLD and HLT pins are high
	if(w->currentCommandType == 1 && w->HLD_pin && w->HLT_pin) {
		w->statusRegister |= 0b00100000;
	}
	// clear TYPE I status bit 5 (S5) if HLD and HLT both not high
	else {w->statusRegister &= 0b11011111;}

	// check if command is still active and do command step if so...
	if(!w->command_done) {
		commandStep(w, us);
	}
	// HLD pin will reset if drive is not busy and 15 index pulses happen
	handleHLDIdle(w, us);
}

/* WD1797 accepts 11 different commands - this function will register the
	command and set all paramenters associated with it */
void doJWD1797Command(JWD1797* w) {
	// if the 4 high bits are 0b1101, the command is a force interrupt
	if(((w->commandRegister>>4) & 15) == 13) {setupForcedIntCommand(w); return;}

	// if not TYPE IV (forced interrupt), get busy status bit from status register (bit 0)
	int busy = w->statusRegister & 1;
	// check busy status
	if(busy) {printBusyMsg(); return;}	// do not run command if busy

	/* determine if command in command register is a TYPE I command by checking
		if the 7 bit is a zero (noly TYPE I commands have a zero (0) in the 7 bit) */
	if(((w->commandRegister>>7) & 1) == 0) {
		setupTypeICommand(w);
		setTypeICommand(w);
	}
	/* Determine if command in command register is TYPE II
		 by checking the highest 3 bits. The two TYPE II commands have either 0b100
		 (Read Sector) or 0b101 (Write Sector) as the high 3 bits */
	else if(((w->commandRegister>>5) & 7) < 6) {
		printf("TYPE II Command in WD1797 command register..\n");
		// sample READY input from DRIVE
		if(!w->ready_pin) {
			// ** generate interrupt **
			w->intrq = 1; // MUST SEND INTERRUPT to slave int controller also...
			return; // do not execute command
		}
		setupTypeIICommand(w);
		setTypeIICommand(w);
	}
	/* Determine if command in command register is TYPE III
		 by checking the highest 3 bits. TYPE III commands have a higher value
		 then 5 in their shifted 3 high bits */
	else if(((w->commandRegister>>5) & 7) > 5) {
		printf("TYPE III Command in WD1797 command register..\n");
		// sample READY input from DRIVE
		if(!w->ready_pin) {
			// ** generate interrupt **
			w->intrq = 1; // MUST SEND INTERRUPT to slave int controller also...
			return; // do not execute command
		}
		setupTypeIIICommand(w);
		setTypeIIICommand(w);
	}
	// check command register error
	else {
		printf("%s\n", "Something went wrong! BAD COMMAND BITS in COMMAND REG!");
	}
}

// execute command step if a command is active (not done)
// us is the time that passed since the last CPU instruction
void commandStep(JWD1797* w, double us) {
	/* do what needs to be done based on which command is still active and based
		on the timers */

  if(w->currentCommandName == "RESTORE") {
		// do "command action" step if not complete
		if(!w->command_action_done) {
			// check TR00 pin (this pin is updated in doJWD1797Cycle)
			if(!w->not_track00_pin) {	// indicates r/w head is over track 00
				w->trackRegister = 0;
				// generate interrupt
				w->intrq = 1;
				// e8259_set_irq0 (e8259_slave, 1);
				w->command_action_done = 1;	// indicate end of command action
				printf("%s\n", "RESTORED HEAD TO TRACK 00 - command action DONE");
				return;
			}
			// not at track 00 - increment step timer
			else {
				w->step_timer += us;
				/* check step timer - has it completed one step according to the step rate?
					Step rates are in milliseconds (ms), so step rate must be multipled by 1000
					to change it to microseconds (us). */
				if(w->step_timer >= (w->stepRate*1000)) {
					w->current_track--;
					// reset step timer
					w->step_timer = 0.0;
				}
			}
		}

		/* after all steps are done (reached track 00)
		 	take care of post command varifications and delays */
		else if(w->command_action_done) {
			// take care of delayed HLD
			if(w->delayed_HLD && w->HLD_pin == 0) {
				w->HLT_timer_active = 1;
				w->HLT_timer = 0.0;
				w->HLD_pin = 1;
				// one shot from HLD pin resets HLT pin
				w->HLT_pin = 0;
				w->HLD_idle_reset_timer = 0.0;
				// reset delayed HLD flag
				w->delayed_HLD = 0;
			}

			// if NO headload or yes headload and no verify
			if((!w->headLoadFlag || w->headLoadFlag) && !w->verifyFlag) {
				// no 30 ms delay and HLT is not sampled - command is done
				w->command_done = 1;
				w->statusRegister &= 0b11111110;	// reset (clear) busy status bit
				// no interrupt is generated because no verification operation takes place
				return;
			}
			// still waiting on verify head settling...
			else if((!w->headLoadFlag || w->headLoadFlag) && w->verifyFlag) {
				// if verify head settling has not occurred yet...
				if(!w->head_settling_done) {
					w->verify_head_settling_timer += us;
					// check if verify head settling is timed out
					if(w->verify_head_settling_timer >= VERIFY_HEAD_SETTLING_LIMIT) {
						// reset timer
						w->verify_head_settling_timer = 0.0;
						w->head_settling_done = 1;
						// assume verification operation is successful - generate interrupt
						w->intrq = 1;
						// e8259_set_irq0 (e8259_slave, 1);
					}
				}
				// head settling time is done. wait for HLT pin to go high...
				else if(w->head_settling_done) {
					// is HLT pin high?
					if(w->HLT_pin) {
						// head settling time has passed and the HLT pin is high
						// command is done
						w->command_done = 1;
						w->statusRegister &= 0b11111110;	// reset (clear) busy status bit
						return;
					}
				}

			}
		}
	}
	// end RESTORE

}


/*
	+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	+++++++++++++++++ HELPER FUNCTIONS ++++++++++++++++++++++++++++++++++++++++++
	+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/

void setupTypeICommand(JWD1797* w) {
	// printf("TYPE I Command in WD1797 command register..\n");
	w->currentCommandType = 1;
	w->command_action_done = 0;
	w->command_done = 0;
	w->head_settling_done = 0;
	// set appropriate status bits for type I command
	typeIStatusReset(w);
	// establish step rate options (in ms) for 1MHz clock (only used with TYPE I cmds)
	int rates[] = {6, 12, 20, 30};
	// get rate bits
	int rateBits = w->commandRegister & 3;
	// set flags according to command bits
	w->stepRate = rates[rateBits];
	w->verifyFlag = (w->commandRegister>>2) & 1;
	w->headLoadFlag = (w->commandRegister>>3) & 1;
	// HLD set according to V and h flags of type I command
	if(!w->headLoadFlag && !w->verifyFlag) {w->HLD_pin = 0;}
	else if(w->headLoadFlag && !w->verifyFlag && w->HLD_pin == 0) {
		w->HLT_timer_active = 1;
		w->HLT_timer = 0.0;
		w->HLD_pin = 1;
		// one shot from HLD pin resets HLT pin
		w->HLT_pin = 0;
		w->HLD_idle_reset_timer = 0.0;

	}
	else if(!w->headLoadFlag && w->verifyFlag) {w->delayed_HLD = 1;}
	else if(w->headLoadFlag && w->verifyFlag && w->HLD_pin == 0) {
		w->HLT_timer_active = 1;
		w->HLT_timer = 0.0;
		w->HLD_pin = 1;
		// one shot from HLD pin resets HLT pin
		w->HLT_pin = 0;
		w->HLD_idle_reset_timer = 0.0;

	}
	// initialize command type I timer
	// w->command_typeI_timer = 0.0;
	// add appropriate time based on V flag (1 MHz clock) 30,000 us
	// if(w->verifyFlag) {w->command_typeI_timer += 30*1000;}
}

void setupTypeIICommand(JWD1797* w) {
	w->currentCommandType = 2;
	w->command_done = 0;
	// set busy status
	w->statusRegister |= 1;
	/* set TYPE II flags */
	w->updateSSO = (w->commandRegister>>1) & 1;
	w->delay15ms = (w->commandRegister>>2) & 1;
	w->swapSectorLength = (w->commandRegister>>3) & 1;
	w->multipleRecords = (w->commandRegister>>4) & 1;
	updateTG43Signal(w);
	w->HLD_pin = 1;
	// initialize HLD idle timer (set to 15 index pulses)
	w->HLD_idle_reset_timer = 0.0;
	// initialize command type I timer
	w->command_typeII_timer = 0.0;
	// add appropriate time based on E flag 15,000 us
	if(w->delay15ms) {w->command_typeII_timer += 15*1000;}
}

void setupTypeIIICommand(JWD1797* w) {
	w->currentCommandType = 3;
	w->command_done = 0;
	// set busy status
	w->statusRegister |= 1;
	/* set TYPE III flags */
	w->updateSSO = (w->commandRegister>>1) & 1;
	w->delay15ms = (w->commandRegister>>2) & 1;
	updateTG43Signal(w);
	w->HLD_pin = 1;
	// initialize HLD idle timer (set to 15 index pulses)
	w->HLD_idle_reset_timer = 0.0;
	// initialize command type I timer
	w->command_typeIII_timer = 0.0;
	// add appropriate time based on E flag 15,000 us
	if(w->delay15ms) {w->command_typeII_timer += 15*1000;}
}

void setupForcedIntCommand(JWD1797* w) {
	/* %%%%%% DEBUG/TESTING - clear all interrupt flags here  */
	w->interruptNRtoR = 0;
	w->interruptRtoNR = 0;
	w->interruptIndexPulse = 0;
	w->interruptImmediate = 0;
	// %%%%%%% DEBUG ABOVE ^^^^
	printf("TYPE IV Command in WD1797 command register (Force Interrupt)..\n");
	w->currentCommandType = 4;
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
		w->terminate_command = 1;
	}
}

void setTypeICommand(JWD1797* w) {
	// get 4 high bits of command register to determine the specific command
	int highBits = ((w->commandRegister>>4) & 15);	// examine 4 high bits

	if(highBits < 2) { // RESTORE or SEEK command
		if((highBits&1) == 0) {	// RESTORE command
			w->currentCommandName = "RESTORE";
			printf("%s command in WD1797 command register\n", w->currentCommandName);
		}
		else if((highBits&1) == 1) {	// SEEK command
			w->currentCommandName = "SEEK";
			printf("%s command in WD1797 command register\n", w->currentCommandName);
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
		}
		else if(cmdID == 2) {	//STEP-IN
			w->currentCommandName = "STEP-IN";
			printf("%s command in WD1797 command register\n", w->currentCommandName);
		}
		else if(cmdID == 3)  {	// STEP-OUT
			w->currentCommandName = "STEP-OUT";
			printf("%s command in WD1797 command register\n", w->currentCommandName);
		}
		// check error
		else {
			printf("%s\n", "Something went wrong! Cannot determine which TYPE I STEP command!");
		}
	}
}

void setTypeIICommand(JWD1797* w) {
	// determine which command by examining highest three bits of cmd reg
	int cmdID = (w->commandRegister>>5) & 7;
	// check if READ SECTOR (high 3 bits == 0b100)
	if(cmdID == 4) {
		w->currentCommandName = "READ SECTOR";
		printf("%s command in WD1797 command register\n", w->currentCommandName);
	}
	else if(cmdID == 5) {
		w->currentCommandName = "WRITE SECTOR";
		printf("%s command in WD1797 command register\n", w->currentCommandName);
		// set Data Address Mark flag
		w->dataAddressMark = w->commandRegister & 1;
	}
	// check error
	else {
		printf("%s\n", "Something went wrong! Cannot determine which TYPE II command!");
	}
}

void setTypeIIICommand(JWD1797* w) {
	// determine which command by examining highest 4 bits of cmd reg
	int cmdID = (w->commandRegister>>4) & 15;
	// READ ADDRESS
	if(cmdID == 12) {
		w->currentCommandName = "READ ADDRESS";
		printf("%s command in WD1797 command register\n", w->currentCommandName);
	}
	// READ TRACK
	else if(cmdID == 14) {
		w->currentCommandName = "READ TRACK";
		printf("%s command in WD1797 command register\n", w->currentCommandName);
	}
	// WRITE TRACK
	else if(cmdID == 15) {
		w->currentCommandName = "WRITE TRACK";
		printf("%s command in WD1797 command register\n", w->currentCommandName);
	}
	// check error
	else {
		printf("%s\n", "Something went wrong! Cannot determine which TYPE III command!");
	}
}

void typeIStatusReset(JWD1797* w) {
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

void updateTG43Signal(JWD1797* w) {
	// update TG43 signal
	if(w->current_track > 43) {w->tg43_pin = 1;}
	else if(w->current_track <= 43) {w->tg43_pin = 0;}
}

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
		printf("%s\n", "INVALID COMMAND TYPE!");
	}
}

void handleIndexPulse(JWD1797* w, double time) {
	w->index_encounter_timer += time;
	// only increment index pulse timer if index pulse is high (1)
	if(w->index_pulse_pin) {w->index_pulse_timer += time;}

	if(!w->index_pulse_pin && w->index_encounter_timer >= INDEX_HOLE_ENCOUNTER_US) {
		w->index_pulse_pin = 1;
		// set IP status if TYPE I command is active
		if(w->currentCommandType == 1) {w->statusRegister |= 0b00000010;}
		// reset index hole encounter timer
		w->index_encounter_timer = 0.0;
	}
	// check index pulse timer
	if(w->index_pulse_pin && w->index_pulse_timer >= INDEX_HOLE_PULSE_US) {
		w->index_pulse_pin = 0;
		// clear IP status if TYPE I command is active
		if(w->currentCommandType == 1) {w->statusRegister &= 0b11111101;}
		// reset index pulse timer
		w->index_pulse_timer = 0.0;
	}
}

void handleHLDIdle(JWD1797* w, double time) {
	// if drive not busy (idle)
	int busy = w->statusRegister & 1;
	if(!busy && w->HLD_pin == 1) {
		// increase the HLD idle timer by the incoming processor instruction time
		w->HLD_idle_reset_timer += time;
		// check to see if HLD must be reset because of idle
		if(w->HLD_idle_reset_timer >= HLD_IDLE_RESET_LIMIT) {
			w->HLD_pin = 0;
			w->HLD_idle_reset_timer = 0.0;
		}
	}
	// if busy, make sure timer starts at 0.0 for start of next IDLE TIME count
	else if(busy) {
		w->HLD_idle_reset_timer = 0.0;
	}
}

void handleHLTTimer(JWD1797* w, double time) {
	// clock HLT delay timer if active
	if(w->HLT_timer_active) {
		w->HLT_timer += time;
		// set HLT pin if timer expired
		if(w->HLT_timer >= HEAD_LOAD_TIMING_LIMIT) {
			w->HLT_pin = 1;
			// reset timer
			w->HLT_timer = 0.0;
			w->HLT_timer_active = 0;
		}
	}
}
