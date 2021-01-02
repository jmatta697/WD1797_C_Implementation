// test MAIN for jwd1797
// Joe Matta

#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include "jwd1797.h"

JWD1797* jwd1797;

int main(int argc, char* argv[]) {

  printf("start test main...\n\n");
  jwd1797 = newJWD1797();
  resetJWD1797(jwd1797);
  // print pointer for new jwd1797 to verify creation
  printf("jwd1797 pointer: %p\n\n", jwd1797);

  /* command write tests... */
  int port = 0xb0;
  // Restore - rate-6, verify, unload head
  writeJWD1797(jwd1797, port, 0b00001100);
  printCommandFlags(jwd1797);
  // Restore - rate-20, no verify, unload head
  writeJWD1797(jwd1797, port, 0b00001010);
  printCommandFlags(jwd1797);

  // Seek - rate-20, no verify, unload head
  writeJWD1797(jwd1797, port, 0b00011010);
  printCommandFlags(jwd1797);
  // Seek - rate-12, verify, load head
  writeJWD1797(jwd1797, port, 0b00010101);
  printCommandFlags(jwd1797);

  // Step - rate-20, no verify, unload head, update track reg
  writeJWD1797(jwd1797, port, 0b00111010);
  printCommandFlags(jwd1797);
  // Step - rate-30, verify, load head, no track reg update
  writeJWD1797(jwd1797, port, 0b00100111);
  printCommandFlags(jwd1797);

  // StepIn - rate-30, no verify, unload head, update track reg
  writeJWD1797(jwd1797, port, 0b01011011);
  printCommandFlags(jwd1797);
  // StepOut - rate-12, verify, load head, no track reg update
  writeJWD1797(jwd1797, port, 0b01100101);
  printCommandFlags(jwd1797);

  // ReadSector - update SSO, no 15ms delay, 0 sector length, single record
  writeJWD1797(jwd1797, port, 0b10000010);
  printCommandFlags(jwd1797);
  // ReadSector - no update SSO, 15ms delay, 1 sector length, multiple record
  writeJWD1797(jwd1797, port, 0b10011100);
  printCommandFlags(jwd1797);

  // WriteSector - DAM, no update SSO, 15ms delay, 1 sector length, multiple record
  writeJWD1797(jwd1797, port, 0b10111100);
  printCommandFlags(jwd1797);
  // WriteSector - deleted DAM, update SSO, no 15ms delay, 1 sector length, single record
  writeJWD1797(jwd1797, port, 0b10101011);
  printCommandFlags(jwd1797);

  // ReadAddress - update SSO, no 15ms delay
  writeJWD1797(jwd1797, port, 0b11000010);
  printCommandFlags(jwd1797);
  // ReadTrack - no update SSO, 15ms delay
  writeJWD1797(jwd1797, port, 0b11100100);
  printCommandFlags(jwd1797);
  // WriteTrack - update SSO, 15ms delay
  writeJWD1797(jwd1797, port, 0b11110110);
  printCommandFlags(jwd1797);

  // ForceInterrupt - no INTRQ/terminate current command
  writeJWD1797(jwd1797, port, 0b11010000);
  printCommandFlags(jwd1797);
  // ForceInterrupt - INTRQ on NR to R
  writeJWD1797(jwd1797, port, 0b11010001);
  printCommandFlags(jwd1797);
  // ForceInterrupt - INTRQ on R to NR
  writeJWD1797(jwd1797, port, 0b11010010);
  printCommandFlags(jwd1797);
  // ForceInterrupt - INTRQ on INDEX PULSE
  writeJWD1797(jwd1797, port, 0b11010100);
  printCommandFlags(jwd1797);
  // ForceInterrupt - immediate INTRQ/terminate current command
  writeJWD1797(jwd1797, port, 0b11011000);
  printCommandFlags(jwd1797);
  // ForceInterrupt - INTRQ on NR to R/INTRQ on INDEX PULSE
  writeJWD1797(jwd1797, port, 0b11010101);
  printCommandFlags(jwd1797);

  // various instruction timings
  double instruction_times[6] = {0.8, 1.0, 1.2, 2.6, 2.8, 4.0};
  // seed rand
  srand(time(NULL));

  for(int i = 0; i < 10; i++) {
    sleep(1);
    // simulate random instruction time by picking from instruction_times list
    // pass instruction time elapsed to WD1797
    double instr_t = instruction_times[rand()%6];
    printf("%f\n", instr_t);
    doJWD1797Cycle(jwd1797, instr_t);
    printf("%s%f\n","Master CLOCK: ", jwd1797->master_clock);
  }

  return 0;
}
