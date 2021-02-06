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

/* ------------- TEST FUNCTIONS ---------------- */

/* test the WD1797 master clock - this test makes sure the incoming instruction
  times are being registered by the JWD1797's master DEBUG clock */
void masterClockTest(JWD1797* jwd1797, double instr_times[]) {
  printf("\n\n%s\n\n", "-------------- MASTER CLOCK TEST --------------");
  resetJWD1797(jwd1797);
  for(int i = 0; i < 10; i++) {
    sleep(1);
    // simulate random instruction time by picking from instruction_times list
    // pass instruction time elapsed to WD1797
    double instr_t = instr_times[rand()%7];
    printf("%f\n", instr_t);
    doJWD1797Cycle(jwd1797, instr_t);
    printf("%s%f\n","Master CLOCK: ", jwd1797->master_timer);
  }
}

/* tests that incoming commands affect the correct flags and are received as
  expected */
void commandWriteTests(JWD1797* jwd1797) {
  printf("\n\n%s\n", "-------------- COMMAND INTAKE TEST --------------");
  /* command write tests... */
  int port = 0xb0;
  // Restore - rate-6, verify, load head
  resetJWD1797(jwd1797);
  writeJWD1797(jwd1797, port, 0b00001100);
  printCommandFlags(jwd1797);
  sleep(1);
  // Restore - rate-20, no verify, load head
  resetJWD1797(jwd1797);
  writeJWD1797(jwd1797, port, 0b00001010);
  printCommandFlags(jwd1797);
  sleep(1);
  // Seek - rate-20, no verify, load head
  resetJWD1797(jwd1797);
  writeJWD1797(jwd1797, port, 0b00011010);
  printCommandFlags(jwd1797);
  sleep(1);
  // Seek - rate-12, verify, unload head
  resetJWD1797(jwd1797);
  writeJWD1797(jwd1797, port, 0b00010101);
  printCommandFlags(jwd1797);
  sleep(1);
  // Step - rate-20, no verify, load head, update track reg
  resetJWD1797(jwd1797);
  writeJWD1797(jwd1797, port, 0b00111010);
  printCommandFlags(jwd1797);
  sleep(1);
  // Step - rate-30, verify, unload head, no track reg update
  resetJWD1797(jwd1797);
  writeJWD1797(jwd1797, port, 0b00100111);
  printCommandFlags(jwd1797);
  sleep(1);
  // StepIn - rate-30, no verify, load head, update track reg
  resetJWD1797(jwd1797);
  writeJWD1797(jwd1797, port, 0b01011011);
  printCommandFlags(jwd1797);
  sleep(1);
  // StepOut - rate-12, verify, unload head, no track reg update
  resetJWD1797(jwd1797);
  writeJWD1797(jwd1797, port, 0b01100101);
  printCommandFlags(jwd1797);
  sleep(1);
  // ReadSector - update SSO, no 15ms delay, 0 sector length, single record
  resetJWD1797(jwd1797);
  jwd1797->ready_pin = 1; // set drive to ready for TYPE II and III commands
  writeJWD1797(jwd1797, port, 0b10000010);
  printCommandFlags(jwd1797);
  sleep(1);
  // ReadSector - no update SSO, 15ms delay, 1 sector length, multiple record
  resetJWD1797(jwd1797);
  jwd1797->ready_pin = 1;
  writeJWD1797(jwd1797, port, 0b10011100);
  printCommandFlags(jwd1797);
  sleep(1);
  // WriteSector - DAM, no update SSO, 15ms delay, 1 sector length, multiple record
  resetJWD1797(jwd1797);
  jwd1797->ready_pin = 1;
  writeJWD1797(jwd1797, port, 0b10111100);
  printCommandFlags(jwd1797);
  sleep(1);
  // WriteSector - deleted DAM, update SSO, no 15ms delay, 1 sector length, single record
  resetJWD1797(jwd1797);
  jwd1797->ready_pin = 1;
  writeJWD1797(jwd1797, port, 0b10101011);
  printCommandFlags(jwd1797);
  sleep(1);
  // ReadAddress - update SSO, no 15ms delay
  resetJWD1797(jwd1797);
  jwd1797->ready_pin = 1;
  writeJWD1797(jwd1797, port, 0b11000010);
  printCommandFlags(jwd1797);
  sleep(1);
  // ReadTrack - no update SSO, 15ms delay
  resetJWD1797(jwd1797);
  jwd1797->ready_pin = 1;
  writeJWD1797(jwd1797, port, 0b11100100);
  printCommandFlags(jwd1797);
  sleep(1);
  // WriteTrack - update SSO, 15ms delay
  resetJWD1797(jwd1797);
  jwd1797->ready_pin = 1;
  writeJWD1797(jwd1797, port, 0b11110110);
  printCommandFlags(jwd1797);
  sleep(1);
  // ForceInterrupt - no INTRQ/terminate current command
  resetJWD1797(jwd1797);
  writeJWD1797(jwd1797, port, 0b11010000);
  printCommandFlags(jwd1797);
  sleep(1);
  // ForceInterrupt - INTRQ on NR to R
  resetJWD1797(jwd1797);
  writeJWD1797(jwd1797, port, 0b11010001);
  printCommandFlags(jwd1797);
  sleep(1);
  // ForceInterrupt - INTRQ on R to NR
  resetJWD1797(jwd1797);
  writeJWD1797(jwd1797, port, 0b11010010);
  printCommandFlags(jwd1797);
  sleep(1);
  // ForceInterrupt - INTRQ on INDEX PULSE
  resetJWD1797(jwd1797);
  writeJWD1797(jwd1797, port, 0b11010100);
  printCommandFlags(jwd1797);
  sleep(1);
  // ForceInterrupt - immediate INTRQ/terminate current command
  resetJWD1797(jwd1797);
  writeJWD1797(jwd1797, port, 0b11011000);
  printCommandFlags(jwd1797);
  sleep(1);
  // ForceInterrupt - INTRQ on NR to R/INTRQ on INDEX PULSE
  resetJWD1797(jwd1797);
  writeJWD1797(jwd1797, port, 0b11010101);
  printCommandFlags(jwd1797);
  sleep(1);
}

/* this test confirms that the index hole is being registered by the status
  register every 200,000 microseconds (0.200 seconds). It also confirms that
  the index pulse is lasting 20 microseconds by repeatedly sampling the status
  register. (The status register will only report the index pulse when TYPE I
  commnands are running. For this reason, a RESTORE command is executed here. */
void indexPulseTest(JWD1797* jwd1797, double instr_times[]) {
  // index hole test
  printf("\n\n%s\n", "-------------- INDEX PULSE TEST --------------");
  resetJWD1797(jwd1797);
  // set track to 5 to have RESTORE command do some work
  jwd1797->current_track = 5;
  /* send RESTORE command to jwd_controller to begin a TYPE I command - index
    pulses will only be valid in the statusRegister for TYPE I commands */
  writeJWD1797(jwd1797, 0xB0, 0b00000011);

  for(int i = 0; i < 1500000; i++) {
    // simulate random instruction time by picking from instruction_times list
    double instr_t = instr_times[rand()%7];
    // printf("%f\n", instr_t);
    doJWD1797Cycle(jwd1797, instr_t); // pass instruction time elapsed to WD1797

    /* encounter index hole every ~200,000 microseconds - time for one
      full disk rotation at 300 RPM) */
    if(jwd1797->rotational_byte_pointer >= 6394 ||
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
    }

    // sleep(1); // delay loop iteration for observation
    // printf("%s", "MASTER CLOCK: ");
    // printf("%f\n", jwd1797->master_timer);
    // printf("%s", "TYPE I STATUS REGISTER: ");
    // print_bin8_representation(jwd1797->statusRegister);
    // printf("%s\n", "");
  }
}

void restoreCommandTest(JWD1797* jwd1797, double instr_times[]) {
  printf("\n\n%s\n", "-------------- RESTORE COMMAND TEST --------------");

  // resetJWD1797(jwd1797);
  // // set track to 3 to have RESTORE command do some work
  // jwd1797->current_track = 3;
  // jwd1797->trackRegister = 3;
  // // issue RESTORE command - no headlaod, no verify, 30 ms step rate
  // writeJWD1797(jwd1797, 0xB0, 0b00000011);
  // printf("%s\n\n", "------- RESTORE from track 3: h=0, V=0 -------");
  //
  // for(int i = 0; i < 500000; i++) {
  //   // printf("%s\n", "loop");
  //   // simulate random instruction time by picking from instruction_times list
  //   double instr_t = instr_times[rand()%7];
  //   // printf("%f\n", instr_t);
  //   doJWD1797Cycle(jwd1797, instr_t); // pass instruction time elapsed to WD1797
  //
  //   if((jwd1797->master_timer >= 29990 && jwd1797->master_timer <= 30015) ||
  //     (jwd1797->master_timer >= 59990 && jwd1797->master_timer <= 60015) ||
  //     (jwd1797->master_timer >= 89990 && jwd1797->master_timer <= 90015)) {
  //       sleep(1); // delay loop iteration for observation
  //       restoreTestPrintHelper(jwd1797);
  //   }
  // }

  // resetJWD1797(jwd1797);
  // // set track to 3 to have RESTORE command do some work
  // jwd1797->current_track = 3;
  // jwd1797->trackRegister = 3;
  // // issue RESTORE command - headlaod (60 ms), no verify, 30 ms step rate
  // writeJWD1797(jwd1797, 0xB0, 0b00001011);
  // printf("%s\n\n", "------- RESTORE from track 3: h=1, V=0 -------");
  //
  // for(int i = 0; i < 500000; i++) {
  //   // printf("%s\n", "loop");
  //   // simulate random instruction time by picking from instruction_times list
  //   double instr_t = instr_times[rand()%7];
  //   // printf("%f\n", instr_t);
  //   doJWD1797Cycle(jwd1797, instr_t); // pass instruction time elapsed to WD1797
  //
  //   if((jwd1797->master_timer >= 29990 && jwd1797->master_timer <= 30015) ||
  //     (jwd1797->master_timer >= 59990 && jwd1797->master_timer <= 60015) ||
  //     (jwd1797->master_timer >= 89990 && jwd1797->master_timer <= 90015)) {
  //       sleep(1); // delay loop iteration for observation
  //       restoreTestPrintHelper(jwd1797);
  //   }
  // }

  resetJWD1797(jwd1797);
  // set track to 3 to have RESTORE command do some work
  jwd1797->current_track = 3;
  jwd1797->trackRegister = 3;
  // issue RESTORE command - no headlaod, verify (30 ms), 30 ms step rate
  writeJWD1797(jwd1797, 0xB0, 0b00000111);
  printf("%s\n\n", "------- RESTORE from track 3: h=0, V=1 -------");

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
        usleep(500000); // delay loop iteration for observation
        restoreTestPrintHelper(jwd1797);
    }
  }

  resetJWD1797(jwd1797);
  // set track to 3 to have RESTORE command do some work
  jwd1797->current_track = 3;
  jwd1797->trackRegister = 3;
  // issue RESTORE command - headlaod (60 ms), verify (30 ms), 30 ms step rate
  writeJWD1797(jwd1797, 0xB0, 0b00001111);
  printf("%s\n\n", "------- RESTORE from track 3: h=1, V=1 -------");

  for(int i = 0; i < 500000; i++) {
    // printf("%s\n", "loop");
    // simulate random instruction time by picking from instruction_times list
    double instr_t = instr_times[rand()%7];
    // printf("%f\n", instr_t);
    doJWD1797Cycle(jwd1797, instr_t); // pass instruction time elapsed to WD1797

    if(jwd1797->master_timer >= 123000) {
        usleep(100000); // delay loop iteration for observation
        restoreTestPrintHelper(jwd1797);
    }
  }

  resetJWD1797(jwd1797);
  // set track to 3 to have RESTORE command do some work
  //jwd1797->current_track = 3;
  //jwd1797->trackRegister = 3;
  // issue RESTORE command - headlaod (60 ms), no verify (30 ms), 30 ms step rate
  writeJWD1797(jwd1797, 0xB0, 0b00001111);
  printf("%s\n\n", "------- RESTORE from track 0: h=1, V=1 -------");

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
        sleep(1); // delay loop iteration for observation
        restoreTestPrintHelper(jwd1797);
    }
  }
}

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

void seekCommandTest(JWD1797* jwd1797, double instr_times[]) {
  /* SEEK command assumes the target track is in the data register. Also, the
    track register is updated automatically */
  printf("\n\n%s\n", "-------------- SEEK COMMAND TEST --------------");

  // printf("%s\n\n", "------- SEEK from track 5 -> 2: h=0, V=0 -------");
  // // try to seek track 2 from current track 5
  // resetJWD1797(jwd1797);
  // jwd1797->current_track = 5;
  // writeJWD1797(jwd1797, 0xB1, 0b00000101); // write track 5 to track register
  // writeJWD1797(jwd1797, 0xB3, 0b00000010);  // write 2 to data register
  // // issue SEEK command - no headlaod (60 ms), no verify (30 ms), 30 ms step rate
  // writeJWD1797(jwd1797, 0xB0, 0b00010011);
  //
  // for(int i = 0; i < 500000; i++) {
  //   // printf("%s\n", "loop");
  //   // simulate random instruction time by picking from instruction_times list
  //   double instr_t = instr_times[rand()%7];
  //   // printf("%f\n", instr_t);
  //   doJWD1797Cycle(jwd1797, instr_t); // pass instruction time elapsed to WD1797
  //
  //   if((jwd1797->master_timer >= 29990 && jwd1797->master_timer <= 30015) ||
  //     (jwd1797->master_timer >= 59990 && jwd1797->master_timer <= 60015) ||
  //     (jwd1797->master_timer >= 89990 && jwd1797->master_timer <= 90015)) {
  //       sleep(1); // delay loop iteration for observation
  //       seekTestPrintHelper(jwd1797);
  //   }
  // }
  //
  // printf("%s\n\n", "------- SEEK from track 2 -> 5: h=0, V=0 -------");
  // // try to seek track 5 from current track 2
  // resetJWD1797(jwd1797);
  // jwd1797->current_track = 2;
  // writeJWD1797(jwd1797, 0xB3, 0b00000101);  // write 5 to data register
  // // issue SEEK command - no headlaod (60 ms), no verify (30 ms), 30 ms step rate
  // writeJWD1797(jwd1797, 0xB0, 0b00010011);
  //
  // for(int i = 0; i < 500000; i++) {
  //   // printf("%s\n", "loop");
  //   // simulate random instruction time by picking from instruction_times list
  //   double instr_t = instr_times[rand()%7];
  //   // printf("%f\n", instr_t);
  //   doJWD1797Cycle(jwd1797, instr_t); // pass instruction time elapsed to WD1797
  //
  //   if((jwd1797->master_timer >= 29990 && jwd1797->master_timer <= 30015) ||
  //     (jwd1797->master_timer >= 59990 && jwd1797->master_timer <= 60015) ||
  //     (jwd1797->master_timer >= 89990 && jwd1797->master_timer <= 90015)) {
  //       sleep(1); // delay loop iteration for observation
  //       seekTestPrintHelper(jwd1797);
  //   }
  // }
  //
  // printf("%s\n\n", "------- SEEK from track 3 -> 0: h=0, V=0 -------");
  // // try to seek track 0 from current track 3
  // resetJWD1797(jwd1797);
  // jwd1797->current_track = 3;
  // writeJWD1797(jwd1797, 0xB3, 0b00000000);  // write 0 to data register
  // // issue SEEK command - no headlaod (60 ms), no verify (30 ms), 30 ms step rate
  // writeJWD1797(jwd1797, 0xB0, 0b00010011);
  //
  // for(int i = 0; i < 500000; i++) {
  //   // printf("%s\n", "loop");
  //   // simulate random instruction time by picking from instruction_times list
  //   double instr_t = instr_times[rand()%7];
  //   // printf("%f\n", instr_t);
  //   doJWD1797Cycle(jwd1797, instr_t); // pass instruction time elapsed to WD1797
  //
  //   if((jwd1797->master_timer >= 29990 && jwd1797->master_timer <= 30015) ||
  //     (jwd1797->master_timer >= 59990 && jwd1797->master_timer <= 60015) ||
  //     (jwd1797->master_timer >= 89990 && jwd1797->master_timer <= 90015)) {
  //       sleep(1); // delay loop iteration for observation
  //       seekTestPrintHelper(jwd1797);
  //   }
  // }

  printf("%s\n\n", "------- SEEK from track 7 -> 5: h=1, V=1 -------");
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
        sleep(1); // delay loop iteration for observation
        seekTestPrintHelper(jwd1797);
    }
  }
}

void stepCommandTest(JWD1797* jwd1797, double instr_times[]) {
  resetJWD1797(jwd1797);
  jwd1797->current_track = 6;
  jwd1797->direction_pin = 1;
  writeJWD1797(jwd1797, 0xB0, 0b00110111);
  printf("\n\n%s\n", "-------------- STEP COMMAND TEST --------------");
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
        sleep(1); // delay loop iteration for observation
        seekTestPrintHelper(jwd1797);
    }
  }
}

void stepInCommandTest(JWD1797* jwd1797, double instr_times[]) {
  resetJWD1797(jwd1797);
  jwd1797->current_track = 5;
  writeJWD1797(jwd1797, 0xB0, 0b01011111);
  printf("\n\n%s\n", "-------------- STEP-IN COMMAND TEST --------------");
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
        sleep(1); // delay loop iteration for observation
        seekTestPrintHelper(jwd1797);
    }
  }
}

void stepOutCommandTest(JWD1797* jwd1797, double instr_times[]) {
  resetJWD1797(jwd1797);
  // jwd1797->HLD_pin = 1;
  // jwd1797->HLT_pin = 1;
  jwd1797->current_track = 0;
  writeJWD1797(jwd1797, 0xB0, 0b01111111);
  printf("\n\n%s\n", "-------------- STEP-OUT COMMAND TEST --------------");
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
        sleep(1); // delay loop iteration for observation
        seekTestPrintHelper(jwd1797);
    }
  }
}

void readSectorTest(JWD1797* jwd1797, double instr_times[]) {
  printf("\n\n%s\n", "-------------- READ SECTOR COMMAND TEST --------------");
  resetJWD1797(jwd1797);
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
  sleep(2);

  // load data register with 3
  writeJWD1797(jwd1797, 0xB3, 0b00000011);
  // isssue seek track 3 command
  writeJWD1797(jwd1797, 0xB0, 0b00010011);
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
  sleep(2);
  // // wait until not busy
  // while((readJWD1797(jwd1797, 0xB0) & 1) == 1) {
  //   printf("%s", "RESTORE STATUS: ");
  //   print_bin8_representation(readJWD1797(jwd1797, 0xB0));
  //   printf("\n");
  //   usleep(100000); // delay loop iteration for observation
  // }

  /* create an array to hold up to 512 X 3 (1536) bytes to simulate reading
  into memory */
  unsigned char memory[512*4];
  // initialize to all 0x00
  for(int i = 0; i < 512*4; i++) {
    memory[i] = 0x00;
  }
  // memory index pointer
  int memory_index_ptr = 0;

  // load the desired sector number into the SR
  writeJWD1797(jwd1797, 0xB2, 0b00000111);
  // issue READ SECTOR command - SSO = 0, 15ms delay, single record
  writeJWD1797(jwd1797, 0xB0, 0b10011110);
  for(int i = 0; i < 300000; i++) {
    // simulate random instruction time by picking from instruction_times list
    double instr_t = instr_times[rand()%7];
    // printf("%f\n", instr_t);
    doJWD1797Cycle(jwd1797, instr_t); // pass instruction time elapsed to WD1797
    if(jwd1797->new_byte_read_signal_ && jwd1797->id_field_data[2] >= 7 &&
      ((jwd1797->statusRegister)&1)) {
      readSectorPrintHelper(jwd1797);
      usleep(100000); // delay loop iteration for observation
    }

    // is there a drq request? check status bit 1..
    if(((readJWD1797(jwd1797, 0xB0) >> 1) & 1) == 1) {

      // printf("%s%f\n", "MASTER CLOCK: ", jwd1797->master_timer);
      // read the data register to get the byte read from disk
      unsigned char r_byte = (unsigned char)(readJWD1797(jwd1797, 0xB3));
      memory[memory_index_ptr] = r_byte;
      memory_index_ptr++;
      // usleep(500000); // delay loop iteration for observation
    }
  }
  printf("%s", "Verify Index count: ");
  printf("%d\n", jwd1797->verify_index_count);
  printf("\n\n");
  printByteArray(memory, 512*4);
  printf("%s", "TYPE II STATUS: " );
  print_bin8_representation(readJWD1797(jwd1797, 0xB0));
}

void readAddressCommandTest(JWD1797* jwd1797) {

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
  printf("%s", "TRACK REGISTER: ");
  print_bin8_representation(jwd1797->trackRegister);
  printf("%s\n", "");
  printf("%s", "DATA REGISTER: ");
  print_bin8_representation(jwd1797->dataRegister);
  printf("%s\n", "");
  printf("%s", "TYPE I STATUS REGISTER: ");
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

void getFByteTest(JWD1797* jwd1797, double instr_times[]) {
  // adjust these for testing different conditions
  jwd1797->current_track = 2;
  jwd1797->sso_pin = 1;

  for(int i = 0; i < 200000; i++) {
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
      printf("%02X\n", getFDiskByte(jwd1797));
      usleep(50000);
    }
  }
}
