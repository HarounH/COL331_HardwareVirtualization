#include <inc/assert.h>

#include <kern/env.h>
#include <kern/pmap.h>
#include <kern/monitor.h>

#include <vmm/vmx.h>

// Why is schedhalt missing?
// NOTE: took Sched_halt from some online source.
#include <inc/assert.h>
#include <inc/x86.h>
#include <kern/spinlock.h>
#include <kern/env.h>
#include <kern/pmap.h>
#include <kern/monitor.h>

void sched_halt();

static int
vmxon() {
    int r;
    if(!thiscpu->is_vmx_root) {
        r = vmx_init_vmxon();
        if(r < 0) {
            cprintf("Error executing VMXON: %e\n", r);
            return r;
        }
        cprintf("VMXON\n");
    }
    return 0;
}

// Choose a user environment to run and run it.
    void
sched_yield(void)
{
    struct Env *idle;
    int i;

    // Implement simple round-robin scheduling.
    //
    // Search through 'envs' for an ENV_RUNNABLE environment in
    // circular fashion starting just after the env this CPU was
    // last running.  Switch to the first such environment found.
    //
    // If no envs are runnable, but the environment previously
    // running on this CPU is still ENV_RUNNING, it's okay to
    // choose that environment.
    //
    // Never choose an environment that's currently running on
    // another CPU (env_status == ENV_RUNNING) and never choose an
    // idle environment (env_type == ENV_TYPE_IDLE).  If there are
    // no runnable environments, simply drop through to the code
    // below to switch to this CPU's idle environment.

    // LAB 4: Your code here.

    // For debugging and testing purposes, if there are no
    // runnable environments other than the idle environments,
    // drop into the kernel monitor.
    
    int trythis=0;
    idle = thiscpu->cpu_env;
    if(idle!=NULL) {
        trythis = ENVX(idle->env_id);
    }
    
    
    for (i = 0; i < NENV; i++) {
        trythis = (trythis+1)%NENV;
        if (envs[trythis].env_type != ENV_TYPE_IDLE &&
                (envs[trythis].env_status == ENV_RUNNABLE ||
                 envs[trythis].env_status == ENV_RUNNING)) {
            
            //
            env_run(&envs[trythis]);
        }
    }
    // Get here only if no other environments were found.
    if(idle && ((idle->env_status==ENV_RUNNING) || (idle->env_status==ENV_RUNNABLE))) {
        env_run(idle);
    }
    // Nothing was runnable bro.
    sched_halt();
    // if (i == NENV) {
    //     cprintf("No more runnable environments!\n");
    //     while (1)
    //         monitor(NULL);
    // }

    // // Run this CPU's idle environment when nothing else is runnable.
    // idle = &envs[cpunum()];
    // if (!(idle->env_status == ENV_RUNNABLE || idle->env_status == ENV_RUNNING))
    //     panic("CPU %d: No idle environment!", cpunum());
    // env_run(idle);
}


void
sched_halt(void)
{
    int i;

    // For debugging and testing purposes, if there are no runnable
    // environments in the system, then drop into the kernel monitor.
    for (i = 0; i < NENV; i++) {
        if ((envs[i].env_status == ENV_RUNNABLE ||
             envs[i].env_status == ENV_RUNNING ||
             envs[i].env_status == ENV_DYING))
            break;
    }
    if (i == NENV) {
        cprintf("No runnable environments in the system!\n");
        while (1)
            monitor(NULL);
    }

    // Mark that no environment is running on this CPU
    curenv = NULL;
    lcr3(PADDR(boot_pml4e));

    // Mark that this CPU is in the HALT state, so that when
    // timer interupts come in, we know we should re-acquire the
    // big kernel lock
    xchg(&thiscpu->cpu_status, CPU_HALTED);

    // Release the big kernel lock as if we were "leaving" the kernel
    unlock_kernel();

    // Reset stack pointer, enable interrupts and then halt.
    asm volatile (
        "movq $0, %%rbp\n"
        "movq %0, %%rsp\n"
        "pushq $0\n"
        "pushq $0\n"
        "sti\n"
        "hlt\n"
        : : "a" (thiscpu->cpu_ts.ts_esp0));
}
