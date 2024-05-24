#ifndef _USER_PROC_H_
#define _USER_PROC_H_

#include <types.h>

pid_t spawn(unsigned int elf_id, unsigned int quota);
void yield(void);

void produce(int item);
int consume(void);

#endif  /* !_USER_PROC_H_ */
