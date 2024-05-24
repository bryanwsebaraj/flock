#ifndef _KERN_PMM_MATOP_H_
#define _KERN_PMM_MATOP_H_

#ifdef _KERN_

unsigned int palloc(void);
void pfree(unsigned int pfree_index);

#endif  /* _KERN_ */

#endif  /* !_KERN_PMM_MATOP_H_ */
