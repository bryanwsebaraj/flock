#ifndef _KERN_LIB_MONITOR_H_
#define _KERN_LIB_MONITOR_H_

#ifdef _KERN_

struct Trapframe;

// Activate the kernel monitor,
// optionally providing a trap frame indicating the current state
// (NULL if none).
void monitor(struct Trapframe *tf);

// Functions implementing monitor commands.
int mon_help(int argc, char **argv, struct Trapframe *tf);
int mon_kerninfo(int argc, char **argv, struct Trapframe *tf);

#endif  /* _KERN_ */

#endif  /* !_KERN_LIB_MONITOR_H_ */
