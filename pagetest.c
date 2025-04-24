#include "types.h"
#include "stat.h"
#include "user.h"

#define PAGESIZE 4096
#define NUM_PAGES_SIMPLE 2
#define NUM_PAGES_EVICTION 1500  // Should exceed your physical memory limit

void simple_fault_test() {
  printf(1, "[TEST 1] Simple page fault without eviction\n");
  char *buf[NUM_PAGES_SIMPLE];

  for (int i = 0; i < NUM_PAGES_SIMPLE; i++) {
    buf[i] = sbrk(PAGESIZE);    // Allocate new page
    if (buf[i] == (char*)-1) {
      printf(1, "sbrk failed at page %d\n", i);
      exit();
    }
    buf[i][0] = 'A';  // Trigger a page fault
    printf(1, "Touched page %d: %c\n", i, buf[i][0]);
  }
  printf(1, "[TEST 1] Done\n");
}

void eviction_test() {
  printf(1, "[TEST 2] Forcing eviction by allocating many pages\n");
  char *buf[NUM_PAGES_EVICTION];

//   for (int i = 0; i < NUM_PAGES_EVICTION; i++) {
//     printf(1, "%s", buf[i]);}
  for (int i = 0; i < NUM_PAGES_EVICTION; i++) {
    buf[i] = sbrk(PAGESIZE);
    if (buf[i] == (char*)-1) {
      printf(1, "sbrk failed at page %d\n", i);
      exit();
    }
    buf[i][0] = 'A' + (i % 26);  // Touch to force actual fault
    if (i % 10 == 0)
      printf(1, "Allocated and touched page %d\n", i);
  }

  // Access some early pages again (may trigger swap-in if they were evicted)
  for (int i = 0; i < NUM_PAGES_EVICTION; i += 20) {
    printf(1, "Re-access page %d: %c\n", i, buf[i][0]);
  }

  printf(1, "[TEST 2] Done\n");
}

int main(int argc, char *argv[]) {
  simple_fault_test();
  eviction_test();
  exit();
}
