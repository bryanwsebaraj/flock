#ifndef _KERN_BBUFFER_BOUNDED_BUFFER_H
#define _KERN_BBUFFER_BOUNDED_BUFFER_H

#ifdef _KERN_

void bbuffer_init(void);
void bbuffer_insert_item(int item);
int bbuffer_remove_item(void);

#endif /* _KERN_ */

#endif /* _KERN_BBUFFER_BOUNDED_BUFFER_H */