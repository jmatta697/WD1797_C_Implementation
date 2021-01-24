// test MAIN for jwd1797
// Joe Matta

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "jwd1797.h"
#include "testFunctions.h"


JWD1797* jwd1797;

int main(int argc, char* argv[]) {

  printf("\nstart WD1797 disk drive controller test MAIN...\n\n");

  jwd1797 = newJWD1797();
  resetJWD1797(jwd1797);
  // print pointer for new jwd1797 to verify creation
  printf("jwd1797 pointer: %p\n\n", jwd1797);
  /* simulate various instruction timings by picking randomly from this list
    these timings are in microseconds */
  double instruction_times[7] = {0.8, 1.6, 1.0, 1.2, 2.6, 2.8, 4.0};
  // seed random number generator with Epoch time
  srand(time(NULL));

  // assembleFormattedDiskArray(jwd1797, "z-dos-1.img");

  /* tests that the WD1797 is receiving and processing incoming instruction
    timings by using a master clock only for testing */
  // masterClockTest(jwd1797, instruction_times);
  // test that the WD1797 is properly recognizing commands
  // commandWriteTests(jwd1797);
  /* tests that the index hole pulse is being properly registered by the status
    register when a TYPE I command is being executed */
  // indexPulseTest(jwd1797, instruction_times);
  // restore command test
  // restoreCommandTest(jwd1797, instruction_times);
  // SEEK command test
  // seekCommandTest(jwd1797, instruction_times);
  // STEP command test
  // stepCommandTest(jwd1797, instruction_times);
  // STEP-IN command test
  // stepInCommandTest(jwd1797, instruction_times);
  // STEP-OUT command test
  // stepOutCommandTest(jwd1797, instruction_times);
  // READ SECTOR command test
  // readSectorTest(jwd1797, instruction_times);

  return 0;
}
