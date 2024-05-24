#ifndef _KERN_DEV_CONSOLE_H_
#define _KERN_DEV_CONSOLE_H_

#ifdef _KERN_

#define CONSOLE_BUFFER_SIZE 512

void cons_init(void);
void cons_enable_kbd(void);
void cons_putc(char c);
void cons_intr(int (*proc)(void));
char *readline(const char *prompt);

#endif  /* _KERN_ */

#endif  /* !_KERN_DEV_CONSOLE_H_ */
