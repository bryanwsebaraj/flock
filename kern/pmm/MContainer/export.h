#ifndef _KERN_PMM_MCONTAINER_H_
#define _KERN_PMM_MCONTAINER_H_

#ifdef _KERN_

void container_init(unsigned int mbi_addr);
unsigned int container_get_parent(unsigned int id);
unsigned int container_get_nchildren(unsigned int id);
unsigned int container_get_quota(unsigned int id);
unsigned int container_get_usage(unsigned int id);
unsigned int container_can_consume(unsigned int id, unsigned int n);
unsigned int container_split(unsigned int id, unsigned int quota);
unsigned int container_alloc(unsigned int id);
void container_free(unsigned int id, unsigned int page_index);

#endif  /* _KERN_ */

#endif  /* !_KERN_PMM_MCONTAINER_H_ */
