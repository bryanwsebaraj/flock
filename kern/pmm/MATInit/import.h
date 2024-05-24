#ifndef _KERN_PMM_MATINIT_H_
#define _KERN_PMM_MATINIT_H_

#ifdef _KERN_

/**
 * Primitives that are already implemented in this lab.
 */
void mem_spinlock_init(void);
void mem_lock(void);
void mem_unlock(void);

// Sets the number of available pages.
void set_nps(unsigned int nps);
// Sets the permission of the physical page with given index.
void at_set_perm(unsigned int page_index, unsigned int perm);

/**
 * Getter and setter functions for the physical memory map table.
 *
 * They report which memory address ranges are usable and which are reserved for
 * use by the BIOS.
 * Each row of the table contains the start address of the range, the length of
 * the range, and a flag indicating whether that memory address range is usable
 * by the kernel.
 * E.g., A row (10000, 1000, 1) represents that the physical address range
 * [10000, 11000) is available to the kernel, while a row (11000, 300, 0) represents
 * that the range [11000, 11300) is reserved by the BIOS and is not available for the
 * kernel to use.
 */
unsigned int get_size(void);               // The number of rows in the table.
unsigned int get_mms(unsigned int idx);    // The start address of the range with given row index.
unsigned int get_mml(unsigned int idx);    // The length of the range with given row index.
unsigned int is_usable(unsigned int idx);  // Whether the range with given row index is usable by
                                           // the kernel. (0: reserved, 1: useable)

/**
 * Lower layer initialization function.
 * It initializes device drivers and interrupts.
 */
void devinit(unsigned int mbi_addr);

#endif  /* _KERN_ */

#endif  /* !_KERN_PMM_MATINIT_H_ */
