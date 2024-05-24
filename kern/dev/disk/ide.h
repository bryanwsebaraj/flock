/**
 * IDE interface driver
 */

#ifndef _KERN_DEV_DISK_IDE_H_
#define _KERN_DEV_DISK_IDE_H_

#ifdef _KERN_

#define IDE_BSY  0x80
#define IDE_DRDY 0x40
#define IDE_DF   0x20
#define IDE_ERR  0x01

#define IDE_CMD_READ  0x20
#define IDE_CMD_WRITE 0x30

void ide_init(void);

// Interrupt handler.
void ide_intr(void);

// Sync buf with disk.
// If B_DIRTY is set, write buf to disk, clear B_DIRTY, set B_VALID.
// Else if B_VALID is not set, read buf from disk, set B_VALID.
void ide_rw(struct buf *b);

void picenable(int32_t irq);
void ioapicenable(int32_t irq, int cpunum);
uint32_t pcpu_ncpu(void);

#endif  /* _KERN_ */

#endif  /* !_KERN_DEV_DISK_IDE_H_ */
