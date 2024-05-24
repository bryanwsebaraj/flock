#include <proc.h>
#include <syscall.h>
#include <types.h>

pid_t spawn(uintptr_t exec, unsigned int quota)
{
    return sys_spawn(exec, quota);
}

void yield(void)
{
    sys_yield();
}

void produce(int item)
{
    sys_produce(item);
}

int consume(void)
{
    return sys_consume();
}