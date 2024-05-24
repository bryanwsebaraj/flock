#ifndef _KERN_PROC_PPROC_H_
#define _KERN_PROC_PPROC_H_

#ifdef _KERN_

unsigned int proc_create(void *elf_addr, unsigned int quota);
void proc_start_user(void);

#endif  /* _KERN_ */

#endif  /* !_KERN_PROC_PPROC_H_ */
