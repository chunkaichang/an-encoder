#include <stdlib.h>

#define MAXRANDINT  100

void mysrand(unsigned int s) { srand(s); }
int myrand(void) { return rand() % MAXRANDINT; }
