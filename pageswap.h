#ifndef XV6_PAGESWAP_H
#define XV6_PAGESWAP_H

#define NSLOTS 800
#define BLOCKS_PER_SLOT 8
#define SWAPPED_FLAG 0x200  // custom bit for swapped-out pages 
#define PTE_A 0x020  // Accessed bit
#define SWAPSTART 2  // Accessed bit


struct swap_slot {
  int is_free;  // 1 if slot is available
  int page_perm;     // saved page permissions (e.g. PTE_U | PTE_W)
};

void swapinit(void);  // initialize swap table
int find_free_swap_slot(void);
int write_page_to_swap(char *page, int perm, int slot_index);
int read_page_from_swap(char *page, int slot_index);
void free_swap_slot(int slot_index);
int page_fault_handler(void);


#endif
