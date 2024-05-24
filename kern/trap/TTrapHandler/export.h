#ifndef _KERN_TRAP_TTRAPHANDLER_H_
#define _KERN_TRAP_TTRAPHANDLER_H_

#ifdef _KERN_

#include <lib/trap.h>

void trap(tf_t *tf);
void exception_handler(tf_t *tf);
void interrupt_handler(tf_t *tf);

#endif  /* _KERN_ */

#endif  /* !_KERN_TRAP_TTRAPHANDLER_H_ */
