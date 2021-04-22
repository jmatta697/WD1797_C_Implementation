// JWD1797 TEST FUNCTIONS

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "jwd1797.h"
#include "utility_functions.h"

void restoreTestPrintHelper(JWD1797*);
void readSectorPrintHelper(JWD1797*);
void typeIVerifyPrintHelper(JWD1797*);
void seekTestPrintHelper(JWD1797*);
void readTrackTestPrintHelper(JWD1797*);

/* ------------- TEST FUNCTIONS ---------------- */

/* test the WD1797 master clock - this test makes sure the incoming instruction
  times are being registered by the JWD1797's master DEBUG clock */
void masterClockTest(JWD1797* jwd1797, double instr_times[]) {
  printf("\n\n%s\n\n", "-------------- MASTER CLOCK TEST --------------");
  printf("\n\n%s\n\n", "press <ENTER> key to continue...");
  while(getchar() != '\n') {};
  resetJWD1797(jwd1797);
  for(int i = 0; i < 10; i++) {
    usleep(250000);
    // simulate random instruction time by picking from instruction_times list
    // pass instruction time elapsed to WD1797
    double instr_t = instr_times[rand()%7];
    printf("%f\n", instr_t);
    doJWD1797Cycle(jwd1797, instr_t);
    printf("%s%f\n","Master CLOCK: ", jwd1797->master_timer);
  }
}

void getFByteTest(JWD1797* jwd1797, double instr_times[]) {
  printf("\n\n%s\n\n", "-------------- getFDiskByte() TEST (320k disk)--------------");
  printf("\n\n%s\n\n", "press <ENTER> key to continue...");
  while(getchar() != '\n') {};

  unsigned char read_byte;
  unsigned char compare_byte;
  // adjust these for testing different conditions
  jwd1797->current_track = 4;
  jwd1797->sso_pin = 1;

  for(int i = 0; i < 200; i++) {
    // simulate random instruction time by picking from instruction_times list
    double instr_t = instr_times[rand()%7];
    // printf("%f\n", instr_t);
    doJWD1797Cycle(jwd1797, instr_t); // pass instruction time elapsed to WD1797
    // only print when a new byte is read
    if(jwd1797->new_byte_read_signal_) {
      printf("%s", "Rotational byte pointer: ");
      printf("%lu\n", jwd1797->rotational_byte_pointer);
      printf("%s", "MASTER CLOCK: ");
      printf("%f -- ", jwd1797->master_timer);
      read_byte = getFDiskByte(jwd1797);
      printf("%02X\n", read_byte);
      compare_byte = jwd1797->formattedDiskArray[(jwd1797->current_track*(5768*2))
        + (jwd1797->sso_pin*5768) + jwd1797->rotational_byte_pointer];
      if(read_byte == compare_byte) {
        printf("%s\n", "\tBYTE CONFIRMED");
      }
      else {
        printf("%s\n", "\tWRONG BYTE READ");
      }
      usleep(50000);
    }
  }
}

/* tests that incoming commands affect the correct flags and are received as
  expected */
void commandWriteTests(JWD1797* jwd1797) {
  printf("\n\n%s\n", "-------------- COMMAND INTAKE TEST --------------");
  printf("\n\n%s\n\n", "press <ENTER> key to continue...");
  while(getchar() != '\n') {};
  /* command write tests... */
  int port = 0xb0;
  // Restore - rate-6, verify, load head
  resetJWD1797(jwd1797);
  writeJWD1797(jwd1797, port, 0b00001100);
  printCommandFlags(jwd1797);
  usleep(500000);
  // Restore - rate-20, no verify, load head
  resetJWD1797(jwd1797);
  writeJWD1797(jwd1797, port, 0b00001010);
  printCommandFlags(jwd1797);
  usleep(500000);
  // Seek - rate-20, no verify, load head
  resetJWD1797(jwd1797);
  writeJWD1797(jwd1797, port, 0b00011010);
  printCommandFlags(jwd1797);
  usleep(500000);
  // Seek - rate-12, verify, unload head
  resetJWD1797(jwd1797);
  writeJWD1797(jwd1797, port, 0b00010101);
  printCommandFlags(jwd1797);
  usleep(500000);
  // Step - rate-20, no verify, load head, update track reg
  resetJWD1797(jwd1797);
  writeJWD1797(jwd1797, port, 0b00111010);
  printCommandFlags(jwd1797);
  usleep(500000);
  // Step - rate-30, verify, unload head, no track reg update
  resetJWD1797(jwd1797);
  writeJWD1797(jwd1797, port, 0b00100111);
  printCommandFlags(jwd1797);
  usleep(500000);
  // StepIn - rate-30, no verify, load head, update track reg
  resetJWD1797(jwd1797);
  writeJWD1797(jwd1797, port, 0b01011011);
  printCommandFlags(jwd1797);
  usleep(500000);
  // StepOut - rate-12, verify, unload head, no track reg update
  resetJWD1797(jwd1797);
  writeJWD1797(jwd1797, port, 0b01100101);
  printCommandFlags(jwd1797);
  usleep(500000);
  // ReadSector - update SSO, no 15ms delay, 0 sector length, single record
  resetJWD1797(jwd1797);
  jwd1797->ready_pin = 1; // set drive to ready for TYPE II and III commands
  writeJWD1797(jwd1797, port, 0b10000010);
  printCommandFlags(jwd1797);
  usleep(500000);
  // ReadSector - no update SSO, 15ms delay, 1 sector length, multiple record
  resetJWD1797(jwd1797);
  jwd1797->ready_pin = 1;
  writeJWD1797(jwd1797, port, 0b10011100);
  printCommandFlags(jwd1797);
  usleep(500000);
  // WriteSector - DAM, no update SSO, 15ms delay, 1 sector length, multiple record
  resetJWD1797(jwd1797);
  jwd1797->ready_pin = 1;
  writeJWD1797(jwd1797, port, 0b10111100);
  printCommandFlags(jwd1797);
  usleep(500000);
  // WriteSector - deleted DAM, update SSO, no 15ms delay, 1 sector length, single record
  resetJWD1797(jwd1797);
  jwd1797->ready_pin = 1;
  writeJWD1797(jwd1797, port, 0b10101011);
  printCommandFlags(jwd1797);
  usleep(500000);
  // ReadAddress - update SSO, no 15ms delay
  resetJWD1797(jwd1797);
  jwd1797->ready_pin = 1;
  writeJWD1797(jwd1797, port, 0b11000010);
  printCommandFlags(jwd1797);
  usleep(500000);
  // ReadTrack - no update SSO, 15ms delay
  resetJWD1797(jwd1797);
  jwd1797->ready_pin = 1;
  writeJWD1797(jwd1797, port, 0b11100100);
  printCommandFlags(jwd1797);
  usleep(500000);
  // WriteTrack - update SSO, 15ms delay
  resetJWD1797(jwd1797);
  jwd1797->ready_pin = 1;
  writeJWD1797(jwd1797, port, 0b11110110);
  printCommandFlags(jwd1797);
  usleep(500000);

  // ForceInterrupt - no INTRQ/terminate current command
  resetJWD1797(jwd1797);
  writeJWD1797(jwd1797, port, 0b11010000);
  jwd1797->currentCommandType = 4;
  printCommandFlags(jwd1797);
  usleep(500000);
  // ForceInterrupt - INTRQ on NR to R
  resetJWD1797(jwd1797);
  writeJWD1797(jwd1797, port, 0b11010001);
  jwd1797->currentCommandType = 4;
  printCommandFlags(jwd1797);
  usleep(500000);
  // ForceInterrupt - INTRQ on R to NR
  resetJWD1797(jwd1797);
  writeJWD1797(jwd1797, port, 0b11010010);
  jwd1797->currentCommandType = 4;
  printCommandFlags(jwd1797);
  usleep(500000);
  // ForceInterrupt - INTRQ on INDEX PULSE
  resetJWD1797(jwd1797);
  writeJWD1797(jwd1797, port, 0b11010100);
  jwd1797->currentCommandType = 4;
  printCommandFlags(jwd1797);
  usleep(500000);
  // ForceInterrupt - immediate INTRQ/terminate current command
  resetJWD1797(jwd1797);
  writeJWD1797(jwd1797, port, 0b11011000);
  jwd1797->currentCommandType = 4;
  printCommandFlags(jwd1797);
  usleep(500000);
  // ForceInterrupt - INTRQ on NR to R/INTRQ on INDEX PULSE
  resetJWD1797(jwd1797);
  writeJWD1797(jwd1797, port, 0b11010101);
  jwd1797->currentCommandType = 4;
  printCommandFlags(jwd1797);
  usleep(500000);
}

/* this test confirms that the index hole is being registered by the status
  register every 200,000 microseconds (0.200 seconds). It also confirms that
  the index pulse is lasting 20 microseconds by repeatedly sampling the status
  register. (The status register will only report the index pulse when TYPE I
  commnands are running. For this reason, a RESTORE command is executed here. */
void indexPulseTest(JWD1797* jwd1797, double instr_times[]) {
  // index hole test
  printf("\n\n%s\n", "-------------- INDEX PULSE TEST --------------");
  printf("\n\n%s\n\n", "press <ENTER> key to continue...");
  while(getchar() != '\n') {};
  resetJWD1797(jwd1797);
  // set track to 5 to have RESTORE command do some work
  jwd1797->current_track = 5;
  /* send RESTORE command to jwd_controller to begin a TYPE I command - index
    pulses will only be valid in the statusRegister for TYPE I commands */
  writeJWD1797(jwd1797, 0xB0, 0b00000011);

  for(int i = 0; i < 150000; i++) {
    // simulate random instruction time by picking from instruction_times list
    double instr_t = instr_times[rand()%7];
    // printf("%f\n", instr_t);
    doJWD1797Cycle(jwd1797, instr_t); // pass instruction time elapsed to WD1797
    // **** insert IP interrupt here (0xD4) *****
    if(i == 6000) {
      writeJWD1797(jwd1797, 0xB0, 0xD4);
    }
    /* encounter index hole every ~200,000 microseconds - time for one
      full disk rotation at 300 RPM) */
    if(jwd1797->rotational_byte_pointer >= jwd1797->actual_num_track_bytes-2 ||
      jwd1797->rotational_byte_pointer <= 2) {
      usleep(500000); // delay loop iteration for observation
      printf(" -- \n%s", "Rot. Byte Ptr: ");
      printf("%lu\n", jwd1797->rotational_byte_pointer);
      printf("%s", "MASTER CLOCK: ");
      printf("%f\n", jwd1797->master_timer);
      printf("%s", "Track Start Signal: ");
      printf("%d\n", jwd1797->track_start_signal_);
      printf("%s", "INDEX PULSE TIMER: ");
      printf("%f\n", jwd1797->index_pulse_timer);
      printf("%s", "INDEX PULSE: ");
      printf("%d\n", jwd1797->index_pulse_pin);
      printf("%s", "TYPE I STATUS REGISTER: ");
      print_bin8_representation(jwd1797->statusRegister);
      printf("%s\n", "");
      printf("%d\n", i);
    }
  }
}

void restoreCommandTest(JWD1797* jwd1797, double instr_times[]) {
  printf("\n\n%s\n", "-------------- RESTORE COMMAND TEST --------------");
  printf("\n\n%s\n\n", "press <ENTER> key to continue...");
  while(getchar() != '\n') {};

  resetJWD1797(jwd1797);
  // set track to 3 to have RESTORE command do some work
  jwd1797->current_track = 3;
  jwd1797->trackRegister = 3;
  // issue RESTORE command - no headlaod, verify (30 ms), 30 ms step rate
  writeJWD1797(jwd1797, 0xB0, 0b00000111);
  printf("\n%s\n\n", "------- RESTORE from track 3: h=0, V=1 -------");
  printf("\n\n%s\n\n", "press <ENTER> key to continue...");
  while(getchar() != '\n') {};

  for(int i = 0; i < 500000; i++) {
    // printf("%s\n", "loop");
    // simulate random instruction time by picking from instruction_times list
    double instr_t = instr_times[rand()%7];
    // printf("%f\n", instr_t);
    doJWD1797Cycle(jwd1797, instr_t); // pass instruction time elapsed to WD1797

    if((jwd1797->master_timer >= 29990 && jwd1797->master_timer <= 30015) ||
      (jwd1797->master_timer >= 59990 && jwd1797->master_timer <= 60015) ||
      (jwd1797->master_timer >= 89990 && jwd1797->master_timer <= 90015) ||
      (jwd1797->master_timer >= 119990 && jwd1797->master_timer <= 120015) ||
      (jwd1797->master_timer >= 135000 && jwd1797->master_timer <= 150015)) {
        usleep(1000000); // delay loop iteration for observation
        restoreTestPrintHelper(jwd1797);
    }
    if(jwd1797->master_timer >= 135014.200000) {
      break;
    }
  }

  resetJWD1797(jwd1797);
  // set track to 3 to have RESTORE command do some work
  jwd1797->current_track = 3;
  jwd1797->trackRegister = 3;
  // issue RESTORE command - headlaod (60 ms), no verify (30 ms), 30 ms step rate
  writeJWD1797(jwd1797, 0xB0, 0b00001111);
  printf("\n%s\n\n", "------- RESTORE from track 3: h=1, V=1 -------");
  printf("\n\n%s\n\n", "press <ENTER> key to continue...");
  while(getchar() != '\n') {};

  for(int i = 0; i < 500000; i++) {
    // printf("%s\n", "loop");
    // simulate random instruction time by picking from instruction_times list
    double instr_t = instr_times[rand()%7];
    // printf("%f\n", instr_t);
    doJWD1797Cycle(jwd1797, instr_t); // pass instruction time elapsed to WD1797

    if((jwd1797->master_timer >= 29990 && jwd1797->master_timer <= 30015) ||
      (jwd1797->master_timer >= 59990 && jwd1797->master_timer <= 60015) ||
      (jwd1797->master_timer >= 89990 && jwd1797->master_timer <= 90015) ||
      (jwd1797->master_timer >= 119990 && jwd1797->master_timer <= 120015)) {
        usleep(1000000); // delay loop iteration for observation
        restoreTestPrintHelper(jwd1797);
    }
  }
}

void seekCommandTest(JWD1797* jwd1797, double instr_times[]) {
  /* SEEK command assumes the target track is in the data register. Also, the
    track register is updated automatically */
  printf("\n\n%s\n", "-------------- SEEK COMMAND TEST --------------");
  printf("\n\n%s\n\n", "press <ENTER> key to continue...");
  while(getchar() != '\n') {};

  printf("\n%s\n\n", "------- SEEK from track 7 -> 5: h=1, V=1 -------");
  printf("\n\n%s\n\n", "press <ENTER> key to continue...");
  while(getchar() != '\n') {};
  // try to seek track 0 from current track 3
  resetJWD1797(jwd1797);
  jwd1797->current_track = 7;
  writeJWD1797(jwd1797, 0xB1, 0b00000111);  // write 7 to data register
  writeJWD1797(jwd1797, 0xB3, 0b00000101);  // write 5 to data register
  // issue SEEK command - no headlaod (60 ms), no verify (30 ms), 30 ms step rate
  writeJWD1797(jwd1797, 0xB0, 0b00011111);

  for(int i = 0; i < 500000; i++) {
    // printf("%s\n", "loop");
    // simulate random instruction time by picking from instruction_times list
    double instr_t = instr_times[rand()%7];
    // printf("%f\n", instr_t);
    doJWD1797Cycle(jwd1797, instr_t); // pass instruction time elapsed to WD1797

    if((jwd1797->master_timer >= 29990 && jwd1797->master_timer <= 30015) ||
      (jwd1797->master_timer >= 59990 && jwd1797->master_timer <= 60015) ||
      (jwd1797->master_timer >= 89990 && jwd1797->master_timer <= 90015) ||
      (jwd1797->master_timer >= 119990 && jwd1797->master_timer <= 120015)) {
        printf("%d\n", i);
        usleep(1000000); // delay loop iteration for observation
        seekTestPrintHelper(jwd1797);
    }
    if(jwd1797->master_timer > 119992.400000) {break;}
  }
}

void stepCommandTest(JWD1797* jwd1797, double instr_times[]) {
  resetJWD1797(jwd1797);
  jwd1797->current_track = 6;
  jwd1797->direction_pin = 1;
  writeJWD1797(jwd1797, 0xB0, 0b00110111);
  printf("\n\n%s\n", "-------------- STEP COMMAND TEST --------------");
  printf("\n\n%s\n\n", "press <ENTER> key to continue...");
  while(getchar() != '\n') {};
  printf("\n%s\n\n", "------- STEP from track 6 -> 7: h=1, V=1 -------");
  printf("\n\n%s\n\n", "press <ENTER> key to continue...");
  while(getchar() != '\n') {};
  for(int i = 0; i < 500000; i++) {
    // printf("%s\n", "loop");
    // simulate random instruction time by picking from instruction_times list
    double instr_t = instr_times[rand()%7];
    // printf("%f\n", instr_t);
    doJWD1797Cycle(jwd1797, instr_t); // pass instruction time elapsed to WD1797

    if((jwd1797->master_timer >= 29990 && jwd1797->master_timer <= 30010) ||
      (jwd1797->master_timer >= 119990 && jwd1797->master_timer <= 119996)) {
        usleep(1000000); // delay loop iteration for observation
        seekTestPrintHelper(jwd1797);
    }
  }
}

void stepInCommandTest(JWD1797* jwd1797, double instr_times[]) {
  resetJWD1797(jwd1797);
  jwd1797->current_track = 5;
  writeJWD1797(jwd1797, 0xB0, 0b01011111);
  printf("\n\n%s\n", "-------------- STEP-IN COMMAND TEST --------------");
  printf("\n\n%s\n\n", "press <ENTER> key to continue...");
  while(getchar() != '\n') {};
  printf("\n%s\n\n", "------- STEP-IN from track 5 -> 6: h=1, V=1 -------");
  printf("\n\n%s\n\n", "press <ENTER> key to continue...");
  while(getchar() != '\n') {};

  for(int i = 0; i < 500000; i++) {
    // printf("%s\n", "loop");
    // simulate random instruction time by picking from instruction_times list
    double instr_t = instr_times[rand()%7];
    // printf("%f\n", instr_t);
    doJWD1797Cycle(jwd1797, instr_t); // pass instruction time elapsed to WD1797

    if((jwd1797->master_timer >= 29990 && jwd1797->master_timer <= 30010) ||
      (jwd1797->master_timer >= 119990 && jwd1797->master_timer <= 119996)) {
        sleep(1); // delay loop iteration for observation
        seekTestPrintHelper(jwd1797);
    }
  }
}

void stepOutCommandTest(JWD1797* jwd1797, double instr_times[]) {
  resetJWD1797(jwd1797);
  // jwd1797->HLD_pin = 1;
  // jwd1797->HLT_pin = 1;
  jwd1797->current_track = 5;
  writeJWD1797(jwd1797, 0xB0, 0b01111111);
  printf("\n\n%s\n", "-------------- STEP-OUT COMMAND TEST --------------");
  printf("\n\n%s\n\n", "press <ENTER> key to continue...");
  while(getchar() != '\n') {};
  printf("\n%s\n\n", "------- STEP-IN from track 5 -> 4: h=1, V=1 -------");
  printf("\n\n%s\n\n", "press <ENTER> key to continue...");
  while(getchar() != '\n') {};
  for(int i = 0; i < 500000; i++) {
    // printf("%s\n", "loop");
    // simulate random instruction time by picking from instruction_times list
    double instr_t = instr_times[rand()%7];
    // printf("%f\n", instr_t);
    doJWD1797Cycle(jwd1797, instr_t); // pass instruction time elapsed to WD1797

    if((jwd1797->master_timer >= 29990 && jwd1797->master_timer <= 30010) ||
      (jwd1797->master_timer >= 119990 && jwd1797->master_timer <= 119996)) {
        sleep(1); // delay loop iteration for observation
        seekTestPrintHelper(jwd1797);
    }
  }
}

void readSectorTest(JWD1797* jwd1797, double instr_times[]) {
  unsigned char target_sector_number;
  unsigned char sso_side;
  int test_byte_pointer;

  printf("\n\n%s\n", "-------------- READ SECTOR COMMAND TEST --------------");
  printf("\n\n%s\n\n", "press <ENTER> key to continue...");
  while(getchar() != '\n') {};
  resetJWD1797(jwd1797);

  printf("\n");
  // isssue restore command
  writeJWD1797(jwd1797, 0xB0, 0b00000000);
  for(int i = 0; i < 100000; i++) {
    // simulate random instruction time by picking from instruction_times list
    double instr_t = instr_times[rand()%7];
    // printf("%f\n", instr_t);
    doJWD1797Cycle(jwd1797, instr_t); // pass instruction time elapsed to WD1797
  }
  printf("%s", "RESTORE STATUS: ");
  print_bin8_representation(readJWD1797(jwd1797, 0xB0));
  printf("%s\n", "");
  printf("%s", "Track Reg: ");
  print_bin8_representation(readJWD1797(jwd1797, 0xB1));
  printf("%s\n", "");
  printf("\n");
  sleep(2);

  unsigned char target_cyl = 0x03;
  // load data register with 3
  writeJWD1797(jwd1797, 0xB3, 0b00000011);
  // issue seek track 3 command
  writeJWD1797(jwd1797, 0xB0, 0b00010011);
  for(int i = 0; i < 100000; i++) {
    // simulate random instruction time by picking from instruction_times list
    double instr_t = instr_times[rand()%7];
    // printf("%f\n", instr_t);
    doJWD1797Cycle(jwd1797, instr_t); // pass instruction time elapsed to WD1797
  }

  printf("%s", "SEEK STATUS: ");
  print_bin8_representation(readJWD1797(jwd1797, 0xB0));
  printf("%s\n", "");
  printf("%s", "Track Reg: ");
  print_bin8_representation(readJWD1797(jwd1797, 0xB1));
  printf("%s\n", "");
  sleep(2);

  // make array from disk image to test read sector bytes from formatted disk array
  unsigned char* payload_test_data = diskImageToCharArray("Z_DOS_ver1.bin", jwd1797);

  printf("\n%s\n", "--- READ SECTOR from track 7, sector 7->8 : m=1, E=1 ---");
  printf("\n%s\n\n", "--- (multi-record read/30ms delay) - WITH INTERRUPT mid-command ---");
  printf("\n\n%s\n\n", "press <ENTER> key to continue...");
  while(getchar() != '\n') {};

  // load the desired sector number into the SR
  target_sector_number = 0x07;
  writeJWD1797(jwd1797, 0xB2, target_sector_number);
  // issue READ SECTOR command - SSO = 1, 15ms delay, multiple record
  sso_side = 0x01;
  writeJWD1797(jwd1797, 0xB0, 0b10011110);

  // printf("%s%d\n", "num_sec/track * sec_length", jwd1797->sectors_per_track * jwd1797->sector_length);
  // get index for test byte array based on track and SSO bin
  test_byte_pointer = (target_cyl
    * (jwd1797->sectors_per_track * 2 * jwd1797->sector_length))
    + (sso_side * (jwd1797->sectors_per_track * jwd1797->sector_length))
    + (jwd1797->sector_length * (target_sector_number - 1));

  for(int i = 0; i < 300000; i++) {
    // printf("%d\n", i);
    // simulate random instruction time by picking from instruction_times list
    double instr_t = instr_times[rand()%7];
    // printf("%f\n", instr_t);
    doJWD1797Cycle(jwd1797, instr_t); // pass instruction time elapsed to WD1797
    if(jwd1797->new_byte_read_signal_ && jwd1797->id_field_data[2] >= 7 &&
      ((jwd1797->statusRegister)&1)) {
      // readSectorPrintHelper(jwd1797);
      usleep(50000); // delay loop iteration for observation
    }
    // is there a drq request? check status bit 1..
    if(((readJWD1797(jwd1797, 0xB0) >> 1) & 1) == 1) {
      // printf("%s%f\n", "MASTER CLOCK: ", jwd1797->master_timer);
      // read the data register to get the byte read from disk
      unsigned char r_byte = (unsigned char)(readJWD1797(jwd1797, 0xB3));
      // printf("%s%d\n", "Test payload data array index: ", test_byte_pointer);

      unsigned char r_test_byte = payload_test_data[test_byte_pointer];

      printf("%s%02X | %s%02X ", "Byte read: ", r_byte, "Test Byte: ", r_test_byte);
      // usleep(500000); // delay loop iteration for observation
      // compare read byte to payload data byte from disk image
      if(r_byte == r_test_byte) {
        printf("%s\n", " -- BYTE CONFIRMED");
      }
      else {
        printf("%s\n", " -- WRONG BYTE");
      }
      test_byte_pointer++;
    }

    // insert forced interrupt (0xD8) terminate command with INTRQ @ inst 150,000
    if(i == 27000) {
      writeJWD1797(jwd1797, 0xB0, 0xD8);
      break;
    }
  }
  printf("%s", "Verify Index count: ");
  printf("%d\n", jwd1797->verify_index_count);
  printf("\n\n");
  // printByteArray(memory, 512*4);
  printf("%s", "TYPE II STATUS: " );
  print_bin8_representation(readJWD1797(jwd1797, 0xB0));
  printf("\n\n");

  // ___________________________________________________________

  printf("\n%s\n", "--- READ SECTOR from track 7, sector 7->8 : m=1, E=1 ---");
  printf("%s\n", "(multi-record read/30ms delay) - NO INTERRUPT");
  printf("\n\n%s\n\n", "press <ENTER> key to continue...");
  while(getchar() != '\n') {};

  printf("\n\n%s\n\n", "clearing interrupt...");
  // write 0xD0 to clear immediate interrupt 0xD8
  writeJWD1797(jwd1797, 0xB0, 0xD0);

  for(int i = 0; i < 1000; i++) {
    double instr_t = instr_times[rand()%7];
    doJWD1797Cycle(jwd1797, instr_t);
  }
  // load the desired sector number into the SR
  target_sector_number = 0x07;
  writeJWD1797(jwd1797, 0xB2, target_sector_number);
  // issue READ SECTOR command - SSO = 0, 15ms delay, multiple record
  sso_side = 0x01;
  writeJWD1797(jwd1797, 0xB0, 0b10011110);

  printf("\n\n%s\n\n", "searching for sector... PLEASE WAIT...");

  // printf("%s%d\n", "num_sec/track * sec_length", jwd1797->sectors_per_track * jwd1797->sector_length);
  // get index for test byte array based on track and SSO bin
  test_byte_pointer = (target_cyl
    * (jwd1797->sectors_per_track * 2 * jwd1797->sector_length))
    + (sso_side * (jwd1797->sectors_per_track * jwd1797->sector_length))
    + (jwd1797->sector_length * (target_sector_number - 1));

  for(int i = 0; i < 300000; i++) {
    // printf("%d\n", i);
    // simulate random instruction time by picking from instruction_times list
    double instr_t = instr_times[rand()%7];
    // printf("%f\n", instr_t);
    doJWD1797Cycle(jwd1797, instr_t); // pass instruction time elapsed to WD1797
    if(jwd1797->new_byte_read_signal_ && jwd1797->id_field_data[2] >= 7 &&
      ((jwd1797->statusRegister)&1)) {
      // readSectorPrintHelper(jwd1797);
      usleep(5000); // delay loop iteration for observation
    }
    // is there a drq request? check status bit 1..
    if(((readJWD1797(jwd1797, 0xB0) >> 1) & 1) == 1) {
      usleep(50000);
      // printf("%s%f\n", "MASTER CLOCK: ", jwd1797->master_timer);
      // read the data register to get the byte read from disk
      unsigned char r_byte = (unsigned char)(readJWD1797(jwd1797, 0xB3));
      // printf("%s%d\n", "Test payload data array index: ", test_byte_pointer);

      unsigned char r_test_byte = payload_test_data[test_byte_pointer];

      printf("%s%02X | %s%02X ", "Byte read: ", r_byte, "Test Byte: ", r_test_byte);
      // usleep(500000); // delay loop iteration for observation
      // compare read byte to payload data byte from disk image
      if(r_byte == r_test_byte) {
        printf("%s\n", " -- BYTE CONFIRMED");
      }
      else {
        printf("%s\n", " -- WRONG BYTE");
      }
      test_byte_pointer++;
    }

    // insert forced interrupt (0xD8) terminate command with INTRQ @ inst 150,000
    if(i == 82700) {
      // writeJWD1797(jwd1797, 0xB0, 0xD8);
    }
  }
  printf("%s", "Verify Index count: ");
  printf("%d\n", jwd1797->verify_index_count);
  printf("\n\n");
  // printByteArray(memory, 512*4);
  printf("%s", "TYPE II STATUS: " );
  print_bin8_representation(readJWD1797(jwd1797, 0xB0));
  printf("\n\n");

  // ___________________________________________________________

  printf("\n%s\n", "--- READ SECTOR from track 6, sector 3 : m=0, E=1 ---");
  printf("%s\n", "(single record read/30ms delay) - NO INTERRUPT");
  printf("\n\n%s\n\n", "press <ENTER> key to continue...");
  while(getchar() != '\n') {};
  // write 0xD0 to clear immediate interrupt 0xD8
  // writeJWD1797(jwd1797, 0xB0, 0xD0);

  // load the desired sector number into the SR
  target_sector_number = 0x03;
  writeJWD1797(jwd1797, 0xB2, target_sector_number);
  // issue READ SECTOR command - SSO = 0, 15ms delay, multiple record
  sso_side = 0x00;
  writeJWD1797(jwd1797, 0xB0, 0b10001100);

  printf("\n\n%s\n\n", "searching for sector... PLEASE WAIT...");

  // printf("%s%d\n", "num_sec/track * sec_length", jwd1797->sectors_per_track * jwd1797->sector_length);
  // get index for test byte array based on track and SSO bin
  test_byte_pointer = (target_cyl
    * (jwd1797->sectors_per_track * 2 * jwd1797->sector_length))
    + (sso_side * (jwd1797->sectors_per_track * jwd1797->sector_length))
    + (jwd1797->sector_length * (target_sector_number - 1));

  for(int i = 0; i < 300000; i++) {
    // printf("%d\n", i);
    // simulate random instruction time by picking from instruction_times list
    double instr_t = instr_times[rand()%7];
    // printf("%f\n", instr_t);
    doJWD1797Cycle(jwd1797, instr_t); // pass instruction time elapsed to WD1797
    if(jwd1797->new_byte_read_signal_ && jwd1797->id_field_data[2] >= 7 &&
      ((jwd1797->statusRegister)&1)) {
      // readSectorPrintHelper(jwd1797);
      usleep(5000); // delay loop iteration for observation
    }
    // is there a drq request? check status bit 1..
    if(((readJWD1797(jwd1797, 0xB0) >> 1) & 1) == 1) {
      usleep(50000);
      // printf("%s%f\n", "MASTER CLOCK: ", jwd1797->master_timer);
      // read the data register to get the byte read from disk
      unsigned char r_byte = (unsigned char)(readJWD1797(jwd1797, 0xB3));
      // printf("%s%d\n", "Test payload data array index: ", test_byte_pointer);

      unsigned char r_test_byte = payload_test_data[test_byte_pointer];

      printf("%s%02X | %s%02X ", "Byte read: ", r_byte, "Test Byte: ", r_test_byte);
      // usleep(500000); // delay loop iteration for observation
      // compare read byte to payload data byte from disk image
      if(r_byte == r_test_byte) {
        printf("%s\n", " -- BYTE CONFIRMED");
      }
      else {
        printf("%s\n", " -- WRONG BYTE");
      }
      test_byte_pointer++;
    }

    // insert forced interrupt (0xD8) terminate command with INTRQ @ inst 150,000
    if(i == 82700) {
      // writeJWD1797(jwd1797, 0xB0, 0xD8);
    }
  }
  printf("%s", "Verify Index count: ");
  printf("%d\n", jwd1797->verify_index_count);
  printf("\n\n");
  // printByteArray(memory, 512*4);
  printf("%s", "TYPE II STATUS: " );
  print_bin8_representation(readJWD1797(jwd1797, 0xB0));
  printf("\n\n");

}


void readAddressTest(JWD1797* jwd1797, double instr_times[]) {
  printf("\n\n%s\n", "-------------- READ ADDRESS COMMAND TEST --------------");
  printf("\n\n%s\n\n", "press <ENTER> key to continue...");
  while(getchar() != '\n') {};

  resetJWD1797(jwd1797);


  printf("\n\n%s\n", "-- SEEK TRACK 2 --");
  printf("\n\n%s\n\n", "press <ENTER> key to continue...");
  while(getchar() != '\n') {};
  /*SEEK*/
  // set data register to target track 2
  writeJWD1797(jwd1797, 0xB3, 0b00000010);
  // isssue seek command to seek track 2
  writeJWD1797(jwd1797, 0xB0, 0b00011011);

  for(int i = 0; i < 500000; i++) {
    // simulate random instruction time by picking from instruction_times list
    double instr_t = instr_times[rand()%7];
    // printf("%f\n", instr_t);
    doJWD1797Cycle(jwd1797, instr_t); // pass instruction time elapsed to WD1797
  }
  printf("\n");
  seekTestPrintHelper(jwd1797);

  printf("\n\n%s\n", "-- ISSUE READ ADDRESS COMMAND - TRACK 2 --");
  printf("\n\n%s\n\n", "press <ENTER> key to continue...");
  while(getchar() != '\n') {};

  // now at track 2 - read next address using read address command
  // isssue read address command at current track
  writeJWD1797(jwd1797, 0xB0, 0b11000100);
  for(int i = 0; i < 200000; i++) {
    // simulate random instruction time by picking from instruction_times list
    double instr_t = instr_times[rand()%7];
    // printf("%f\n", instr_t);
    doJWD1797Cycle(jwd1797, instr_t); // pass instruction time elapsed to WD1797
    // is there a drq request? check status bit 1..
    if(jwd1797->e_delay_done && jwd1797->new_byte_read_signal_ &&
      jwd1797->zero_byte_counter >= 2) {
      readSectorPrintHelper(jwd1797);
      sleep(1); // delay loop iteration for observation
      // read
      if(((readJWD1797(jwd1797, 0xB0) >> 1) & 1) == 1) {
        // readSectorPrintHelper(jwd1797);
        // usleep(100000); // delay loop iteration for observation
        readJWD1797(jwd1797, 0xB3);
      }
    }
    if(jwd1797->id_field_data[5] == 0x01) {break;}
  }
  for(int i = 0; i < 500000; i++) {
    // simulate random instruction time by picking from instruction_times list
    double instr_t = instr_times[rand()%7];
    // printf("%f\n", instr_t);
    doJWD1797Cycle(jwd1797, instr_t); // pass instruction time elapsed to WD1797
  }

  resetJWD1797(jwd1797);

  // _____________________________________________________
  printf("\n\n%s\n", "-- SEEK TRACK 6 --");
  printf("\n\n%s\n\n", "press <ENTER> key to continue...");
  while(getchar() != '\n') {};

  // set data register to target track 6
  writeJWD1797(jwd1797, 0xB3, 0b00000110);
  // isssue seek command to seek track 6
  writeJWD1797(jwd1797, 0xB0, 0b00011011);

  for(int i = 0; i < 437000; i++) {
    // simulate random instruction time by picking from instruction_times list
    double instr_t = instr_times[rand()%7];
    // printf("%f\n", instr_t);
    doJWD1797Cycle(jwd1797, instr_t); // pass instruction time elapsed to WD1797
  }
  seekTestPrintHelper(jwd1797);

  printf("\n\n%s\n", "-- ISSUE READ ADDRESS COMMAND - TRACK 7 --");
  printf("\n\n%s\n\n", "press <ENTER> key to continue...");
  while(getchar() != '\n') {};

  for(int i = 0; i < 5000; i++) {
    // simulate random instruction time by picking from instruction_times list
    double instr_t = instr_times[rand()%7];
    // printf("%f\n", instr_t);
    doJWD1797Cycle(jwd1797, instr_t); // pass instruction time elapsed to WD1797
  }

  // now at track 6 - read next address using read address command
  // isssue read address command at current track
  writeJWD1797(jwd1797, 0xB0, 0b11000110);
  for(int i = 0; i < 200000; i++) {
    // simulate random instruction time by picking from instruction_times list
    double instr_t = instr_times[rand()%7];
    // printf("%f\n", instr_t);
    doJWD1797Cycle(jwd1797, instr_t); // pass instruction time elapsed to WD1797
    // is there a drq request? check status bit 1..
    if(jwd1797->e_delay_done && jwd1797->new_byte_read_signal_ &&
      jwd1797->zero_byte_counter >= 2) {
      readSectorPrintHelper(jwd1797);
      sleep(1); // delay loop iteration for observation
      // read
      if(((readJWD1797(jwd1797, 0xB0) >> 1) & 1) == 1) {
        // readSectorPrintHelper(jwd1797);
        // usleep(100000); // delay loop iteration for observation
        readJWD1797(jwd1797, 0xB3);
      }
    }
    if(jwd1797->id_field_data[5] == 0x01) {break;}
  }
  for(int i = 0; i < 500000; i++) {
    // simulate random instruction time by picking from instruction_times list
    double instr_t = instr_times[rand()%7];
    // printf("%f\n", instr_t);
    doJWD1797Cycle(jwd1797, instr_t); // pass instruction time elapsed to WD1797
  }

}

void readTrackTest(JWD1797* jwd1797, double instr_times[]) {
  printf("\n\n%s\n", "-------------- READ TRACK COMMAND TEST --------------");
  printf("\n\n%s\n\n", "press <ENTER> key to continue...");
  while(getchar() != '\n') {};

  resetJWD1797(jwd1797);

  printf("\n\n%s\n", "-------------- SEEK TRACK 10 --------------");
  printf("\n\n%s\n\n", "press <ENTER> key to continue...");
  while(getchar() != '\n') {};

  /*SEEK*/
  // set data register to target track 10
  unsigned char target_tr = 0x0A;
  writeJWD1797(jwd1797, 0xB3, target_tr);
  // isssue seek command to seek track 10 - load head
  writeJWD1797(jwd1797, 0xB0, 0b00011011);
  for(int i = 0; i < 5000000; i++) {
    // simulate random instruction time by picking from instruction_times list
    double instr_t = instr_times[rand()%7];
    // printf("%f\n", instr_t);
    doJWD1797Cycle(jwd1797, instr_t); // pass instruction time elapsed to WD1797
  }
  seekTestPrintHelper(jwd1797);

  printf("\n\n%s\n", "----------- ISSUE READ TRACK COMMAND - 20 -----------");
  printf("\n\n%s\n\n", "press <ENTER> key to continue...");
  while(getchar() != '\n') {};

  // isssue read track command
  unsigned char sso_side = 0x00;
  writeJWD1797(jwd1797, 0xB0, 0b11100100);

  // start pointer at formatted byte array to compare
  int formatted_test_array_pt = ((jwd1797->actual_num_track_bytes * 2) * target_tr)
    + (jwd1797->actual_num_track_bytes * sso_side);

  for(int i = 0; i < 5000000; i++) {
    // simulate random instruction time by picking from instruction_times list
    double instr_t = instr_times[rand()%7];
    // printf("%f\n", instr_t);
    doJWD1797Cycle(jwd1797, instr_t); // pass instruction time elapsed to WD1797

    // is there a drq request? check status bit 1..
    if(((readJWD1797(jwd1797, 0xB0) >> 1) & 1) == 1) {
      // usleep(10000);
      // printf("%s%f\n", "MASTER CLOCK: ", jwd1797->master_timer);
      // read the data register to get the byte read from disk
      unsigned char r_byte = (unsigned char)(readJWD1797(jwd1797, 0xB3));
      unsigned char r_test_byte =
      jwd1797->formattedDiskArray[formatted_test_array_pt];
      printf("%ld ", jwd1797->rotational_byte_pointer);
      printf("%s%02X | %s%02X ", "Byte read: ", r_byte, "Test Byte: ", r_test_byte);
      // usleep(500000); // delay loop iteration for observation
      // compare read byte to payload data byte from disk image
      if(r_byte == r_test_byte) {
        printf("%s\n", " -- BYTE CONFIRMED");
      }
      else {
        printf("%s\n", " -- WRONG BYTE - READ TRACK FAILED!!");
        printf("%s\n", " -- TEST ABORTED!!");
        break;
      }
      formatted_test_array_pt++;
    }
  }
  printf("\n");
  readTrackTestPrintHelper(jwd1797);

  // ______________________________________________________

  printf("\n\n%s\n\n", "press <ENTER> key to continue...");
  while(getchar() != '\n') {};

  resetJWD1797(jwd1797);

  printf("\n\n%s\n", "-------------- SEEK TRACK 31 --------------");
  printf("\n\n%s\n\n", "press <ENTER> key to continue...");
  while(getchar() != '\n') {};

  /*SEEK*/
  // set data register to target track 10
  target_tr = 0x1F;
  writeJWD1797(jwd1797, 0xB3, target_tr);
  // isssue seek command to seek track 10 - load head
  writeJWD1797(jwd1797, 0xB0, 0b00011011);
  for(int i = 0; i < 5000000; i++) {
    // simulate random instruction time by picking from instruction_times list
    double instr_t = instr_times[rand()%7];
    // printf("%f\n", instr_t);
    doJWD1797Cycle(jwd1797, instr_t); // pass instruction time elapsed to WD1797
  }
  seekTestPrintHelper(jwd1797);

  printf("\n\n%s\n", "----------- ISSUE READ TRACK COMMAND - 63 -----------");
  printf("\n\n%s\n\n", "press <ENTER> key to continue...");
  while(getchar() != '\n') {};

  // isssue read track command
  sso_side = 0x01;
  writeJWD1797(jwd1797, 0xB0, 0b11100110);

  // start pointer at formatted byte array to compare
  formatted_test_array_pt = ((jwd1797->actual_num_track_bytes * 2) * target_tr)
    + (jwd1797->actual_num_track_bytes * sso_side);

  for(int i = 0; i < 5000000; i++) {
    // simulate random instruction time by picking from instruction_times list
    double instr_t = instr_times[rand()%7];
    // printf("%f\n", instr_t);
    doJWD1797Cycle(jwd1797, instr_t); // pass instruction time elapsed to WD1797

    // is there a drq request? check status bit 1..
    if(((readJWD1797(jwd1797, 0xB0) >> 1) & 1) == 1) {
      // usleep(10000);
      // printf("%s%f\n", "MASTER CLOCK: ", jwd1797->master_timer);
      // read the data register to get the byte read from disk
      unsigned char r_byte = (unsigned char)(readJWD1797(jwd1797, 0xB3));
      unsigned char r_test_byte =
      jwd1797->formattedDiskArray[formatted_test_array_pt];
      printf("%ld ", jwd1797->rotational_byte_pointer);
      printf("%s%02X | %s%02X ", "Byte read: ", r_byte, "Test Byte: ", r_test_byte);
      // usleep(500000); // delay loop iteration for observation
      // compare read byte to payload data byte from disk image
      if(r_byte == r_test_byte) {
        printf("%s\n", " -- BYTE CONFIRMED");
      }
      else {
        printf("%s\n", " -- WRONG BYTE - READ TRACK FAILED!!");
        printf("%s\n", " -- TEST ABORTED!!");
        break;
      }
      formatted_test_array_pt++;
    }
  }
  printf("\n");
  readTrackTestPrintHelper(jwd1797);
}


/*
  ------------------------------------------------------------------------------
  ------------------ ** PRINT HELPER FUNCTIONS ** ------------------------------
  ------------------------------------------------------------------------------
*/

void restoreTestPrintHelper(JWD1797* jwd1797) {
  printf("%s", "MASTER CLOCK: ");
  printf("%f\n", jwd1797->master_timer);
  printf("%s", "byte read: ");
  printf("%02X\n", getFDiskByte(jwd1797));
  printf("%s", "V HEAD SETTLING TIMER: ");
  printf("%f\n", jwd1797->verify_head_settling_timer);
  printf("%s", "HLT TIMER: ");
  printf("%f\n", jwd1797->HLT_timer);
  printf("%s", "CURRENT TRACK: ");
  printf("%d\n", jwd1797->current_track);
  printf("%s", "head settling done: ");
  printf("%d\n", jwd1797->head_settling_done);
  printf("%s", "HLT pin: ");
  printf("%d\n", jwd1797->HLT_pin);
  typeIVerifyPrintHelper(jwd1797);
  printf("%s", "TRACK REGISTER: ");
  print_bin8_representation(readJWD1797(jwd1797, 0xB1));
  printf("%s\n", "");
  printf("%s", "TYPE I STATUS REGISTER: ");
  print_bin8_representation(readJWD1797(jwd1797, 0xB0));
  printf("%s\n", "");
  printf("%s\n", "");
}

void readSectorPrintHelper(JWD1797* jwd1797) {
  printf("%s%f\n", "MASTER CLOCK: ", jwd1797->master_timer);
  printf("%s%f\n", "E (15ms) DELAY TIMER: ", jwd1797->e_delay_timer);
  printf("%s%d\n", "E-Delay done: ", jwd1797->e_delay_done);
  printf("%s%f\n", "HLT TIMER: ", jwd1797->HLT_timer);
  printf("%s%d\n", "HLT_pin: ", jwd1797->HLT_pin);
  printf("%s", "verify_operation_active: ");
  printf("%d\n", jwd1797->verify_operation_active);
  printf("%s", "Verify Index count: ");
  printf("%d\n", jwd1797->verify_index_count);
  printf("%s%lu\n", "Rotational byte ptr: ", jwd1797->rotational_byte_pointer);
  if(jwd1797->new_byte_read_signal_) {
    printf("%s", "byte read: ");
    printf("%02X\n", getFDiskByte(jwd1797));
  }
  printf("%s", "0x00 count: ");
  printf("%d\n", jwd1797->zero_byte_counter);
  printf("%s", "Post 0x00 count search limit: ");
  printf("%d\n", jwd1797->address_mark_search_count);
  printf("%s", "0xA1 count: ");
  printf("%d\n", jwd1797->a1_byte_counter);
  printf("%s", "ID field Found: ");
  printf("%d\n", jwd1797->id_field_found);
  printf("%s", "ID Data Verified: ");
  printf("%d\n", jwd1797->ID_data_verified);

  printf("%s", "data a1 byte count: ");
  printf("%d\n", jwd1797->data_a1_byte_counter);
  printf("%s", "data AM search count: ");
  printf("%d\n", jwd1797->data_mark_search_count);
  printf("%s", "Data AM found: ");
  printf("%d\n", jwd1797->data_mark_found);
  printf("%s", "Sector length count: ");
  printf("%d\n", jwd1797->intSectorLength);

  printf("%s%d\n", "DRQ: ", jwd1797->drq);
  printf("%s%02X\n", "DATA REGISTER: ", jwd1797->dataRegister);
  printf("%s", "SECTOR REGISTER: ");
  print_bin8_representation(jwd1797->sectorRegister);
  printf("%s\n", "");
  printf("%s", "TYPE II STATUS REGISTER: ");
  print_bin8_representation(jwd1797->statusRegister);
  printf("%s\n", "");
  printByteArray(jwd1797->id_field_data, 6);
  // typeIVerifyPrintHelper(jwd1797);
  printf("%s\n", "");
  printf("%s\n", "");
}

void seekTestPrintHelper(JWD1797* jwd1797) {
  printf("%s", "MASTER CLOCK: ");
  printf("%f\n", jwd1797->master_timer);
  printf("%s", "V HEAD SETTLING TIMER: ");
  printf("%f\n", jwd1797->verify_head_settling_timer);
  printf("%s", "HLT TIMER: ");
  printf("%f\n", jwd1797->HLT_timer);
  printf("%s", "CURRENT TRACK: ");
  printf("%d\n", jwd1797->current_track);
  printf("%s", "Direction: ");
  printf("%d\n", jwd1797->direction_pin);
  printf("%s%lu\n", "Rotational byte ptr: ", jwd1797->rotational_byte_pointer);
  printf("%s", "TRACK REGISTER: ");
  print_bin8_representation(jwd1797->trackRegister);
  printf("%s\n", "");
  printf("%s", "DATA REGISTER: ");
  print_bin8_representation(jwd1797->dataRegister);
  printf("%s\n", "");
  printf("%s", "SECTOR REGISTER: ");
  print_bin8_representation(jwd1797->sectorRegister);
  printf("%s\n", "");
  printf("%s", "TYPE STATUS REGISTER: ");
  print_bin8_representation(jwd1797->statusRegister);
  typeIVerifyPrintHelper(jwd1797);
  printf("%s\n", "");
  printf("%s\n", "");
}

void typeIVerifyPrintHelper(JWD1797* jwd1797) {
  printf("\n%s", "0x00 count: ");
  printf("%d\n", jwd1797->zero_byte_counter);
  printf("%s", "Post 0x00 count search limit: ");
  printf("%d\n", jwd1797->address_mark_search_count);
  printf("%s", "0xA1 count: ");
  printf("%d\n", jwd1797->a1_byte_counter);
  printf("%s", "ID field Found: ");
  printf("%d\n", jwd1797->id_field_found);
  printf("%s", "ID field collected: ");
  printf("%d\n", jwd1797->id_field_data_collected);
  printf("%s", "verify_operation_active: ");
  printf("%d\n", jwd1797->verify_operation_active);
  printf("%s", "ID field data array: ");
  printByteArray(jwd1797->id_field_data, 6);
}


void readTrackTestPrintHelper(JWD1797* jwd1797) {
  printf("%s", "MASTER CLOCK: ");
  printf("%f\n", jwd1797->master_timer);
  printf("%s", "TYPE STATUS REGISTER: ");
  print_bin8_representation(jwd1797->statusRegister);
  printf("%s\n", "");
  printf("%s", "TRACK REGISTER: ");
  print_bin8_representation(jwd1797->trackRegister);
  printf("%s\n", "");
  printf("%s", "DATA REGISTER: ");
  print_bin8_representation(jwd1797->dataRegister);
  printf("%s\n", "");
  printf("%s", "SECTOR REGISTER: ");
  print_bin8_representation(jwd1797->sectorRegister);
  printf("%s\n", "");
  printf("%s%d\n", "Start track read = ", jwd1797->start_track_read_);
  if(jwd1797->new_byte_read_signal_) {
    printf("%s%lu\n", "Rotational byte ptr: ", jwd1797->rotational_byte_pointer);
    printf("%s", "byte read: ");
    printf("%02X\n", getFDiskByte(jwd1797));
  }
  printf("%s\n", "");
  printf("%s\n", "");
}
// end tests
