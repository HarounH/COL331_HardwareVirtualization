// implement fork from user space

#include <inc/string.h>
#include <inc/lib.h>
#include <inc/memlayout.h>
#include <inc/mmu.h>

// PTE_COW marks copy-on-write page table entries.
// It is one of the bits explicitly allocated to user processes (PTE_AVAIL).
#define PTE_COW		0x800

//
// Custom page fault handler - if faulting page is copy-on-write,
// map in our own private writable copy.
//
static void
pgfault(struct UTrapframe *utf)
{
    void *addr = (void *) utf->utf_fault_va;
    uint32_t err = utf->utf_err;
    int r;

    // Check that the faulting access was (1) a write, and (2) to a
    // copy-on-write page.  If not, panic.
    // Hint:
    //   Use the read-only page table mappings at vpt
    //   (see <inc/memlayout.h>).

    // LAB 4: Your code here.

    // IMPORTANT: Skipping all error checks man.
    pte_t pte = vpt[VPN(addr)];
    if( (err & FEC_WR) && (pte & PTE_COW) ) {
        r = sys_page_alloc(0, PFTEMP, PTE_U|PTE_P|PTE_W);
        if(r<0) ;//return r;
        memcpy(PFTEMP, ROUNDDOWN(addr,PGSIZE), PGSIZE);
        r = sys_page_map(0, PFTEMP, 0, ROUNDDOWN(addr, PGSIZE), PTE_P|PTE_U|PTE_W);
        if(r<0) ;//return r;
        r = sys_page_unmap(0,PFTEMP);
        if(r<0) ;//return r;
    }

    // Allocate a new page, map it at a temporary location (PFTEMP),
    // copy the data from the old page to the new page, then move the new
    // page to the old page's address.
    // Hint:
    //   You should make three system calls.
    //   No need to explicitly delete the old page's mapping.

    // LAB 4: Your code here.

    panic("pgfault not implemented");
}

//
// Map our virtual page pn (address pn*PGSIZE) into the target envid
// at the same virtual address.  If the page is writable or copy-on-write,
// the new mapping must be created copy-on-write, and then our mapping must be
// marked copy-on-write as well.  (Exercise: Why do we need to mark ours
// copy-on-write again if it was already copy-on-write at the beginning of
// this function?)
//
// Returns: 0 on success, < 0 on error.
// It is also OK to panic on error.
//
    static int
duppage(envid_t envid, unsigned pn)
{
    int r;

    // LAB 4: Your code here.
    void* addr = (void*) ((uintptr_t)pn * PGSIZE);
    pte_t pte = vpt[pn]; // Takes a page number. woohoo.
    int perm = pte & PTE_SYSCALL;
    if( (perm & PTE_W) || (pte & PTE_COW) ) {
        perm &= ~PTE_W;
        perm |= PTE_COW;
        r = sys_page_map(0, addr, 0, addr, perm);
        if(r<0) return r;
    }
    r = sys_page_map(0, addr, envid, addr, perm);
    if(r<0) return r;


    // panic("duppage not implemented");
    return 0;
}

//
// User-level fork with copy-on-write.
// Set up our page fault handler appropriately.
// Create a child.
// Copy our address space and page fault handler setup to the child.
// Then mark the child as runnable and return.
//
// Returns: child's envid to the parent, 0 to the child, < 0 on error.
// It is also OK to panic on error.
//
// Hint:
//   Use vpd, vpt, and duppage. ....  BAHAHAHAHAHAHAHAHAH what a joke!
//   Remember to fix "thisenv" in the child process.
//   Neither user exception stack should ever be marked copy-on-write,
//   so you must allocate a new page for the child's user exception stack.
//
envid_t
fork(void)
{
    // LAB 4: Your code here.
    set_pgfault_handler(pgfault); // Reflects in child too.
    envid_t childid = sys_exofork();
    if(childid<0) return childid; // Error.

    if(childid == 0) { // I am the child.
        thisenv = &envs[ENVX(sys_getenvid())];  // Why can't the parent do this, haroun?
        return childid; 
    }

    int r = sys_page_alloc(childid, (void*)(UXSTACKTOP-PGSIZE), PTE_P|PTE_W|PTE_U); // UXSTACK for child.
    if(r<0) return r; // What error can I throw even?
    // Copy every page over. This sucks.

    uint64_t vaddr;
    for(vaddr = 0; vaddr < USTACKTOP; vaddr += PGSIZE) {
        // If conditions are met, duppage it.
        if( // Gotta use the virtual types apparently.
            (vpml4e[VPML4E(vaddr)] & PTE_P) // Mota mota level is present.
            && ((vpde[VPDPE(vaddr)] & PTE_U) && (vpde[VPDPE(vaddr)] & PTE_P)) // Slighlt less mota level is also present.
            && ((vpd[VPD(vaddr)] & PTE_U) && (vpd[VPD(vaddr)] & PTE_P))
            &&  ((vpt[VPN(vaddr)] & PTE_U) && (vpt[VPN(vaddr)] & PTE_P))
            )

            r = duppage(childid, PGNUM(vaddr));
            if(r<0) {
                cprintf("WARN: Your environments are now throughly done for man.\n");
                return -1;
            }
    }    
    extern void _pgfault_upcall(void);
    r = sys_env_set_pgfault_upcall(childid, _pgfault_upcall);
    r = sys_env_set_status(childid, ENV_RUNNABLE);
    return childid;
    // panic("fork not implemented");
}

// Challenge!
    int
sfork(void)
{
    panic("sfork not implemented");
    return -E_INVAL;
}
