// JWD1797 TEST FUNCTIONS

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "jwd1797.h"
#include "utility_functions.h"

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
  // Restore - rate-6, verify, unload head
  resetJWD1797(jwd1797);
  writeJWD1797(jwd1797, port, 0b00001100);
  printCommandFlags(jwd1797);
  sleep(1);
  // Restore - rate-20, no verify, unload head
  resetJWD1797(jwd1797);
  writeJWD1797(jwd1797, port, 0b00001010);
  printCommandFlags(jwd1797);
  sleep(1);
  // Seek - rate-20, no verify, unload head
  resetJWD1797(jwd1797);
  writeJWD1797(jwd1797, port, 0b00011010);
  printCommandFlags(jwd1797);
  sleep(1);
  // Seek - rate-12, verify, load head
  resetJWD1797(jwd1797);
  writeJWD1797(jwd1797, port, 0b00010101);
  printCommandFlags(jwd1797);
  sleep(1);
  // Step - rate-20, no verify, unload head, update track reg
  resetJWD1797(jwd1797);
  writeJWD1797(jwd1797, port, 0b00111010);
  printCommandFlags(jwd1797);
  sleep(1);
  // Step - rate-30, verify, load head, no track reg update
  resetJWD1797(jwd1797);
  writeJWD1797(jwd1797, port, 0b00100111);
  printCommandFlags(jwd1797);
  sleep(1);
  // StepIn - rate-30, no verify, unload head, update track reg
  resetJWD1797(jwd1797);
  writeJWD1797(jwd1797, port, 0b01011011);
  printCommandFlags(jwd1797);
  sleep(1);
  // StepOut - rate-12, verify, load head, no track reg update
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

  for(int i; i < 1500000; i++) {
    // simulate random instruction time by picking from instruction_times list
    double instr_t = instr_times[rand()%7];
    // printf("%f\n", instr_t);
    doJWD1797Cycle(jwd1797, instr_t); // pass instruction time elapsed to WD1797

    /* encounter index hole every ~200,000 microseconds - time for one
      full disk rotation at 300 RPM) */
    if((jwd1797->master_timer >= 199990 && jwd1797->master_timer <= 200030) ||
      (jwd1797->master_timer >= 399990 && jwd1797->master_timer <= 400030) ||
      (jwd1797->master_timer >= 599990 && jwd1797->master_timer <= 600030)) {
      sleep(1); // delay loop iteration for observation
      printf("%s", "MASTER CLOCK: ");
      printf("%f\n", jwd1797->master_timer);
      printf("%s", "TYPE I STATUS REGISTER: ");
      print_bin8_representation(jwd1797->statusRegister);
      printf("%s\n", "");
    }
  }
}
