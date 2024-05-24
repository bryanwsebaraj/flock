#ifndef _KERN_VMM_MPTINTRO_H_
#define _KERN_VMM_MPTINTRO_H_

#ifdef _KERN_

void set_pdir_base(unsigned int index);
unsigned int get_pdir_entry(unsigned int proc_index, unsigned int pde_index);
void set_pdir_entry(unsigned int proc_index, unsigned int pde_index,
                    unsigned int page_index);
void set_pdir_entry_identity(unsigned int proc_index, unsigned int pde_index);
void rmv_pdir_entry(unsigned int proc_index, unsigned int pde_index);
unsigned int get_ptbl_entry(unsigned int proc_index, unsigned int pde_index,
                            unsigned int pte_index);
void set_ptbl_entry(unsigned int proc_index, unsigned int pde_index,
                    unsigned int pte_index, unsigned int page_index,
                    unsigned int perm);
void set_ptbl_entry_identity(unsigned int pde_index, unsigned int pte_index,
                             unsigned int perm);
void rmv_ptbl_entry(unsigned int proc_index, unsigned int pde_index,
                    unsigned int pte_index);

#endif  /* _KERN_ */

#endif  /* !_KERN_VMM_MPTINTRO_H_ */
