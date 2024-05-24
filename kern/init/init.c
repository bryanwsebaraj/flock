#include <lib/debug.h>
#include <lib/types.h>
#include <lib/kstack.h>
#include <lib/thread.h>
#include <lib/x86.h>
#include <dev/devinit.h>
#include <pcpu/PCPUIntro/export.h>
#include <proc/PProc/export.h>
#include <thread/PCurID/export.h>
#include <thread/PKCtxIntro/export.h>
#include <thread/PTCBIntro/export.h>
#include <thread/PTQueueInit/export.h>
#include <thread/PThread/export.h>

extern uint32_t pcpu_ncpu(void);

static volatile int cpu_booted = 0;
static volatile int all_ready = FALSE;

extern uint8_t _binary___obj_user_idle_idle_start[];

static void kern_main(void)
{
    KERN_INFO("[BSP KERN] In kernel main.\n\n");

    KERN_INFO("[BSP KERN] Number of CPUs in this system: %d. \n", pcpu_ncpu());

    int cpu_idx = get_pcpu_idx();
    unsigned int pid;

    pid = proc_create(_binary___obj_user_idle_idle_start, 10000);
    // pid = proc_create(_binary___obj_user_idle_idle_start, 10000); 
    KERN_INFO("CPU%d: process idle %d is created.\n", cpu_idx, pid);
    tqueue_remove(NUM_IDS, pid);
    tcb_set_state(pid, TSTATE_RUN);
    set_curid(pid);
    kctx_switch(0, pid);

    KERN_PANIC("kern_main() should never reach here.\n");
}

static void kern_main_ap(void)
{
    int cpu_idx = get_pcpu_idx();

    set_pcpu_boot_info(cpu_idx, TRUE);

    while (all_ready == FALSE);

    KERN_INFO("[AP%d KERN] kernel_main_ap\n", cpu_idx);

    cpu_booted++;
}

void kern_init(uintptr_t mbi_addr)
{
    thread_init(mbi_addr);
    KERN_INFO("[BSP KERN] Kernel initialized.\n");
    kern_main();
}

void kern_init_ap(void (*f)(void))
{
    devinit_ap();
    f();
}
