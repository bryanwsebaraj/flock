#ifndef _KERN_TRAP_TTRAPHANDLER_H_
#define _KERN_TRAP_TTRAPHANDLER_H_

#ifdef _KERN_

unsigned int get_curid(void);
unsigned int alloc_page(unsigned int proc_index, unsigned int vaddr,
                        unsigned int perm);
unsigned int syscall_get_arg1(void);
void set_pdir_base(unsigned int index);
void proc_start_user(void);

#endif  /* _KERN_ */

#endif  /* !_KERN_TRAP_TTRAPHANDLER_H_ */
