#ifndef _KERN_VMM_MPTCOMM_H_
#define _KERN_VMM_MPTCOMM_H_

#ifdef _KERN_

unsigned int container_alloc(unsigned int id);
void container_free(unsigned int id, unsigned int page_index);
void idptbl_init(unsigned int mbi_addr);
void set_pdir_entry_identity(unsigned int proc_index, unsigned int pde_index);
void rmv_pdir_entry(unsigned int proc_index, unsigned int pde_index);
void rmv_ptbl_entry(unsigned int proc_index, unsigned int pde_index,
                    unsigned int pte_index);
unsigned int get_pdir_entry_by_va(unsigned int proc_index, unsigned int vaddr);
void rmv_pdir_entry_by_va(unsigned int proc_index, unsigned int vaddr);
void set_pdir_entry_by_va(unsigned int proc_index, unsigned int vaddr,
                          unsigned int page_index);

#endif  /* _KERN_ */

#endif  /* !_KERN_MM_MPTCOMM_H_ */
