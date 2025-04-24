#include "types.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "defs.h"
#include "pageswap.h"
#include "fs.h"
#include "param.h"
#include "buf.h"
#include "proc.h"      // struct proc 
// #include "mmu.h"       // For V2P, PTE flags, etc.
#include "memlayout.h"
#include "x86.h"       // For rcr2(), lcr3(), CR3, etc.


struct swap_slot swap_table[NSLOTS];

void
swapinit(void) {
  for (int i = 0; i < NSLOTS; i++) {
    swap_table[i].is_free = 1;
    swap_table[i].page_perm = 0;
  }
  cprintf("Swap table initialized with %d slots\n", NSLOTS);
}

int
find_free_swap_slot(void) {
  for (int i = 0; i < NSLOTS; i++) {
    if (swap_table[i].is_free)
      return i;
  }
  return -1;
}

int
write_page_to_swap(char *page, int perm, int slot_index) {
  if (slot_index < 0 || slot_index >= NSLOTS || !swap_table[slot_index].is_free)
    return -1;

  for (int i = 0; i < 8; i++) {
    struct buf *b = bread(ROOTDEV, SWAPSTART + slot_index * 8 + i);
    memmove(b->data, page + i * 512, 512);
    bwrite(b);
    brelse(b);
  }

  swap_table[slot_index].page_perm = perm;
  swap_table[slot_index].is_free = 0;
  return 0;
}

int
read_page_from_swap(char *page, int slot_index) {
  if (slot_index < 0 || slot_index >= NSLOTS || swap_table[slot_index].is_free)
    return -1;

  for (int i = 0; i < 8; i++) {
    struct buf *b = bread(ROOTDEV, SWAPSTART + slot_index * 8 + i);
    memmove(page + i * 512, b->data, 512);
    brelse(b);
  }

  return swap_table[slot_index].page_perm;
}

void
free_swap_slot(int slot_index) {
  if (slot_index >= 0 && slot_index < NSLOTS) {
    swap_table[slot_index].is_free = 1;
    swap_table[slot_index].page_perm = 0;
  }
}


// trying to handle on demand page loads, not sure if this is req. 
// still not able to properly handle - memset is a pain to handle
// int page_fault_handler(void) {
//   uint va = rcr2(); // Faulting virtual address
//   struct proc *curproc = myproc();

//   // Only handle user addresses within process size
//   if (va >= curproc->sz)
//       return -1;

//   // Locate the PTE for the faulting address
//   pte_t *pte = walkpgdir(curproc->pgdir, (void*)va, 0);
//   if (!pte)
//       return -1;

//   // Case 1: Swapped-out page
//   if (*pte & SWAPPED_FLAG) {
//       int slot = (*pte >> 12) & 0xFFFFF;  // Extract swap slot index
//       char *mem = kalloc();
//       if (!mem) {
//           // Try to free memory by swapping out another page
//           if (swapout_one_page() < 0)
//               return -1;
//           mem = kalloc();
//           if (!mem)
//               return -1;
//       }
//       // Read the page back from swap
//       int perm = read_page_from_swap(mem, slot);
//       if (perm < 0) {
//           kfree(mem);
//           return -1;
//       }
//       // Map the page back in
//       *pte = V2P(mem) | (perm & ~SWAPPED_FLAG) | PTE_P;
//       lcr3(V2P(curproc->pgdir)); // Flush TLB
//       free_swap_slot(slot);
//       curproc->rss++;
//       return 0;
//   }

//   // Case 2: Demand-zero allocation (page never allocated)
//   if (!(*pte & PTE_P)) {
//     char *mem = kalloc();
//     if (!mem) {
//         if (swapout_one_page() < 0)
//             return -1;
//         mem = kalloc();
//         if (!mem)
//             return -1;
//     }
//     // Zero out the page
//     for (int i = 0; i < PGSIZE; i++)
//         mem[i] = 0;
//     // Map the page: set PTE to physical address + permissions
//     *pte = V2P(mem) | PTE_W | PTE_U | PTE_P;
//     curproc->rss++;
//     lcr3(V2P(curproc->pgdir)); // Flush TLB
//     return 0;
//   }
//   // if (!(*pte & PTE_P)) {
//   //     char *mem = kalloc();
//   //     if (!mem) {
//   //         // Try to free memory by swapping out another page
//   //         if (swapout_one_page() < 0)
//   //             return -1;
//   //         mem = kalloc();
//   //         if (!mem)
//   //             return -1;
//   //     }
//   //     memset(mem, 0, PGSIZE);
//   //     // Map the new page with user and write permissions
//   //     if (mappages(curproc->pgdir, (void*)PGROUNDDOWN(va), PGSIZE, V2P(mem), PTE_W | PTE_U) < 0) {
//   //         kfree(mem);
//   //         return -1;
//   //     }
//   //     curproc->rss++;
//   //     lcr3(V2P(curproc->pgdir)); // Flush TLB
//   //     return 0;
//   // }

//   // If the page is present or some other error, return failure
//   return -1;
// }

// does not worry about demand allocation - but isnt this wrong logically?
int page_fault_handler(void) {
  uint va = rcr2();
  struct proc *curproc = myproc();
  pte_t *pte = walkpgdir(curproc->pgdir, (void*)va, 0);

  if (!pte)
      return -1;

  // Only handle faults on swapped-out pages
  if (!(*pte & SWAPPED_FLAG))
      return -1;

  // int slot = (*pte >> 12) & 0xFFFFF;  // Extract slot index (adjust mask as needed)
  int slot = (*pte >> 12);  // Extract slot index (adjust mask as needed)
  char *mem = kalloc();

  if (!mem) {
      // Not enough free memory, try to swap out another page
      if (swapout_one_page() < 0)
          return -1;
      mem = kalloc();
      if (!mem)
          return -1;
  }

  // if (mem==0) return -1;

  // Read page from swap slot into memory
  int perm = read_page_from_swap(mem, slot);
  if (perm < 0) {
      kfree(mem);
      return -1;
  }

  // Restore the PTE: physical address, original permissions, present, and clear SWAPPED_FLAG
  *pte = V2P(mem) | (perm & ~SWAPPED_FLAG) | PTE_P;

  lcr3(V2P(curproc->pgdir));  // Flush TLB

  free_swap_slot(slot);
  curproc->rss++;
  return 0;
}
