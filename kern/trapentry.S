/* See COPYRIGHT for copyright information. */

#include <inc/mmu.h>
#include <inc/memlayout.h>
#include <inc/trap.h>
#include <kern/macro.h>

#include <kern/picirq.h>


###################################################################
# exceptions/interrupts
###################################################################

/* TRAPHANDLER defines a globally-visible function for handling a trap.
 * It pushes a trap number onto the stack, then jumps to _alltraps.
 * Use TRAPHANDLER for traps where the CPU automatically pushes an error code.
 *
 * You shouldn't call a TRAPHANDLER function from C, but you may
 * need to _declare_ one in C (for instance, to get a function pointer
 * during IDT setup).  You can declare the function with
 *   void NAME();
 * where NAME is the argument passed to TRAPHANDLER.
 */
#define TRAPHANDLER(name, num)						\
	.globl name;		/* define global symbol for 'name' */	\
	.type name, @function;	/* symbol type is function */		\
	.align 2;		/* align function definition */		\
	name:			/* function starts here */		\
	pushq $(num);							\
	jmp _alltraps

/* Use TRAPHANDLER_NOEC for traps where the CPU doesn't push an error code.
 * It pushes a 0 in place of the error code, so the trap frame has the same
 * format in either case.
 */
#define TRAPHANDLER_NOEC(name, num)					\
	.globl name;							\
	.type name, @function;						\
	.align 2;							\
	name:								\
	pushq $0;							\
	pushq $(num);							\
	jmp _alltraps

.text

/*
 * Lab 3: Your code here for generating entry points for the different traps.
   Find the PUSHA,POPA macros in kern/macro.h. Since amd64 doesn't support
   pusha,popa so you will find these macros handy.
 */



/*
 * Lab 3: Your code here for _alltraps
 *
 * Hint: Be sure to review the x64 calling convention from lab1
 *       for how to pass a pointer to the trapframe.
 */

TRAPHANDLER_NOEC(HANDLE_DIVIDE,T_DIVIDE)
TRAPHANDLER_NOEC(HANDLE_DEBUG,T_DEBUG)
TRAPHANDLER_NOEC(HANDLE_NMI,T_NMI)
TRAPHANDLER_NOEC(HANDLE_BRKPT,T_BRKPT)
TRAPHANDLER_NOEC(HANDLE_OFLOW,T_OFLOW)
TRAPHANDLER_NOEC(HANDLE_BOUND,T_BOUND)
TRAPHANDLER_NOEC(HANDLE_ILLOP,T_ILLOP)
TRAPHANDLER_NOEC(HANDLE_DEVICE,T_DEVICE)
TRAPHANDLER(HANDLE_DBFLT,T_DBLFLT)
//TRAPHANDLER(HANDLE_CORPROC,T_CORPROC)
TRAPHANDLER(HANDLE_TSS,T_TSS)
TRAPHANDLER(HANDLE_SEGNP,T_SEGNP)
TRAPHANDLER(HANDLE_STACK,T_STACK)
TRAPHANDLER(HANDLE_GPFLT,T_GPFLT)
TRAPHANDLER(HANDLE_PGFLT,T_PGFLT)
//TRAPHANDLER(HANDLE_RES,T_RES)
TRAPHANDLER_NOEC(HANDLE_FPERR,T_FPERR)
TRAPHANDLER(HANDLE_ALIGN,T_ALIGN)
TRAPHANDLER_NOEC(HANDLE_MCHK,T_MCHK)
TRAPHANDLER_NOEC(HANDLE_SIMDERR,T_SIMDERR)
TRAPHANDLER_NOEC(HANDLE_SYSCALL,T_SYSCALL)
TRAPHANDLER_NOEC(HANDLE_DEFAULT,T_DEFAULT)
TRAPHANDLER_NOEC(HANDLE_IRQ_TIMER,IRQ_OFFSET + IRQ_TIMER)
TRAPHANDLER_NOEC(HANDLE_IRQ_KBD,IRQ_OFFSET + IRQ_KBD)
TRAPHANDLER_NOEC(HANDLE_IRQ_SERIAL,IRQ_OFFSET + IRQ_SERIAL)
TRAPHANDLER_NOEC(HANDLE_IRQ_SPURIOUS,IRQ_OFFSET + IRQ_SPURIOUS)
TRAPHANDLER_NOEC(HANDLE_IRQ_IDE,IRQ_OFFSET + IRQ_IDE)
TRAPHANDLER_NOEC(HANDLE_IRQ_ERROR,IRQ_OFFSET + IRQ_ERROR)
/*
 * Lab 3: Your code here for _alltraps
 */
 .globl _alltraps
_alltraps:
	subq $8,%rsp 
	movw %ds,(%rsp)
	subq $8,%rsp
	movw %es,(%rsp)
	PUSHA
	movq $GD_KD,%rax
	movw %rax,%ds
	movw %rax,%es
	movq %rsp,%rdi
	/*movq $0, %rbp*/
	call trap
	POPA_
	add $8, %rsp
	movw (%rsp),%es
	add $8, %rsp
	movw (%rsp),%ds
	iret