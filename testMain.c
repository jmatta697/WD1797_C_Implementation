// test MAIN for jwd1797
// Joe Matta

#include <stdio.h>
#include "jwd1797.h"

JWD1797* jwd1797;

int main(int argc, char* argv[]) {

  printf("start test main...\n\n");
  jwd1797 = newJWD1797();
  // print pointer for new jwd1797 yo verify creation
  printf("jwd1797 pointer: %p\n", jwd1797);

  return 0;
}
