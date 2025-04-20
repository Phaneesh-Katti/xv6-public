#include "types.h"
#include "defs.h"
#include "pageswap.h"

struct swap_slot swap_table[NSLOTS];

void
swapinit(void) {
  for (int i = 0; i < NSLOTS; i++) {
    swap_table[i].is_free = 1;
    swap_table[i].page_perm = 0;
  }
  cprintf("Swap table initialized with %d slots\n", NSLOTS);
}
