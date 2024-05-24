#ifndef _KERN_VMM_MPTINIT_H_
#define _KERN_VMM_MPTINIT_H_

#ifdef _KERN_

void paging_init(unsigned int mbi_addr);
void paging_init_ap(void);

#endif  /* _KERN_ */

#endif  /* !_KERN_VMM_MPTINIT_H_ */
