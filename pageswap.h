#ifndef XV6_PAGESWAP_H
#define XV6_PAGESWAP_H

#define NSLOTS 800
#define BLOCKS_PER_SLOT 8

struct swap_slot {
  int is_free;  // 1 if slot is available
  int page_perm;     // saved page permissions (e.g. PTE_U | PTE_W)
};

void swapinit(void);  // initialize swap table

#endif
