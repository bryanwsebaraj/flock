#include <lib/x86.h>

#include "import.h"

#define PDE_ADDR(x) (x >> 22)
#define PTE_ADDR(x) ((x >> 12) & 0x3ff)

#define PAGESIZE      4096
#define PDIRSIZE      (PAGESIZE * 1024)
#define VM_USERLO     0x40000000
#define VM_USERHI     0xF0000000
#define VM_USERLO_PDE (VM_USERLO / PDIRSIZE)
#define VM_USERHI_PDE (VM_USERHI / PDIRSIZE)

/**
 * For each process from id 0 to NUM_IDS - 1,
 * set up the page directory entries so that the kernel portion of the map is
 * the identity map, and the rest of the page directories are unmapped.
 */
void pdir_init(unsigned int mbi_addr)
{
    unsigned int proc_index, pde_index;
    idptbl_init(mbi_addr);

    for (proc_index = 0; proc_index < NUM_IDS; proc_index++) {
        for (pde_index = 0; pde_index < 1024; pde_index++) {
            if ((pde_index < VM_USERLO_PDE) || (VM_USERHI_PDE <= pde_index)) {
                set_pdir_entry_identity(proc_index, pde_index);
            } else {
                rmv_pdir_entry(proc_index, pde_index);
            }
        }
    }
}

/**
 * Allocates a page (with container_alloc) for the page table,
 * and registers it in the page directory for the given virtual address,
 * and clears (set to 0) all page table entries for this newly mapped page table.
 * It returns the page index of the newly allocated physical page.
 * In the case when there's no physical page available, it returns 0.
 */
unsigned int alloc_ptbl(unsigned int proc_index, unsigned int vaddr)
{
    unsigned int page_index = container_alloc(proc_index);
    unsigned int pde_index = PDE_ADDR(vaddr);
    unsigned int pte_index;

    if (page_index == 0) {
        return 0;
    } else {
        set_pdir_entry_by_va(proc_index, vaddr, page_index);
        for (pte_index = 0; pte_index < 1024; pte_index++) {
            rmv_ptbl_entry(proc_index, pde_index, pte_index);
        }

        return page_index;
    }
}

// Reverse operation of alloc_ptbl.
// Removes corresponding the page directory entry,
// and frees the page for the page table entries (with container_free).
void free_ptbl(unsigned int proc_index, unsigned int vaddr)
{
    unsigned int page_index = get_pdir_entry_by_va(proc_index, vaddr) >> 12;

    rmv_pdir_entry(proc_index, PDE_ADDR(vaddr));
    container_free(proc_index, page_index);
}
