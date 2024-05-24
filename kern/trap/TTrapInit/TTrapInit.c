#include <lib/trap.h>
#include <lib/debug.h>
#include <lib/string.h>
#include <dev/intr.h>
#include "import.h"

#define KERN_INFO_CPU(str, idx) \
    if (idx == 0) KERN_INFO("[BSP KERN] " str); \
    else KERN_INFO("[AP%d KERN] " str, idx);

int inited = FALSE;

trap_cb_t TRAP_HANDLER[NUM_CPUS][256];

void trap_init_array(void)
{
    KERN_ASSERT(inited == FALSE);
    memzero(&TRAP_HANDLER, sizeof(trap_cb_t) * 8 * 256);
    inited = TRUE;
}

void trap_handler_register(int cpu_idx, int trapno, trap_cb_t cb)
{
    KERN_ASSERT(0 <= cpu_idx && cpu_idx < 8);
    KERN_ASSERT(0 <= trapno && trapno < 256);
    KERN_ASSERT(cb != NULL);

    TRAP_HANDLER[cpu_idx][trapno] = cb;
}

void trap_init(unsigned int cpu_idx)
{
    int trapno;

    if (cpu_idx == 0) {
        trap_init_array();
    }

    KERN_INFO_CPU("Register trap handlers...\n", cpu_idx);

    for (trapno = 0; trapno < 256; trapno++) {
        // Exceptions
        if ((T_DIVIDE <= trapno && trapno <= T_SIMD) || trapno == T_SECEV) {
            trap_handler_register(cpu_idx, trapno, exception_handler);
        }
        // Interrupts
        else if ((T_IRQ0 + IRQ_TIMER <= trapno && trapno <= T_IRQ0 + IRQ_RTC)
                 || (T_IRQ0 + IRQ_MOUSE <= trapno && trapno <= T_IRQ0 + IRQ_IDE2)
                 || (trapno == T_IRQ0 + IRQ_ERROR) || (trapno == T_IRQ0 + IRQ_EHCI_2)
                 || (T_LTIMER <= trapno && trapno <= T_PERFCTR)) {
            trap_handler_register(cpu_idx, trapno, interrupt_handler);
        }
        // Syscall
        else if (trapno == T_SYSCALL) {
            trap_handler_register(cpu_idx, trapno, syscall_dispatch);
        }
    }

    KERN_INFO_CPU("Done.\n", cpu_idx);
    KERN_INFO_CPU("Enabling interrupts...\n", cpu_idx);

    /* enable interrupts */
    intr_enable(IRQ_TIMER, cpu_idx);
    intr_enable(IRQ_KBD, cpu_idx);
    intr_enable(IRQ_SERIAL13, cpu_idx);
    intr_enable(IRQ_IDE1, cpu_idx);

    KERN_INFO_CPU("Done.\n", cpu_idx);
}
