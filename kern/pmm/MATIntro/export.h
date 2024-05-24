#ifndef _KERN_PMM_MATINTRO_H_
#define _KERN_PMM_MATINTRO_H_

#ifdef _KERN_

void mem_spinlock_init(void);
void mem_lock(void);
void mem_unlock(void);

unsigned int get_nps(void);
void set_nps(unsigned int page_index);

unsigned int at_is_norm(unsigned int page_index);
void at_set_perm(unsigned int page_index, unsigned int perm);

unsigned int at_is_allocated(unsigned int page_index);
void at_set_allocated(unsigned int page_index, unsigned int allocated);

#endif  /* _KERN_ */

#endif  /* !_KERN_PMM_MATINTRO_H_ */
