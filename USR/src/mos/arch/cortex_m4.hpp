#ifndef _MOS_CPU_CORTEX_M4_
#define _MOS_CPU_CORTEX_M4_

#include "stm32f4xx.h"
#include "core_cm4.h"

#define MOS_REBOOT()               NVIC_SystemReset()
#define MOS_TRIGGER_SYSTICK_INTR() SCB->ICSR |= SCB_ICSR_PENDSTSET_Msk
#define MOS_TRIGGER_PENDSV_INTR()  SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk
#define MOS_TEST_IRQ()             __get_PRIMASK() == 0
#define MOS_DISABLE_IRQ()          asm volatile("CPSID I");
#define MOS_ENABLE_IRQ()           asm volatile("CPSIE I");

#define ARCH_INIT_ASM                                                                                 \
	"CPSID   I\n"           /* Disable irq to enter critical section */                               \
	"LDR     R0, =curTCB\n" /* R0 contains the address of curTCB */                                   \
	"LDR     R2, [R0]\n"    /* R2 contains the address in curTCB */                                   \
	"LDR     R4, [R2,#8]\n" /* Load the SP reg with the stacked SP value */                           \
	"MOV     SP, R4\n"      /* Move SP to R4 */                                                       \
	"POP     {R4-R7}\n"     /* Pop registers R8-R11(user saved context) */                            \
	"MOV     R8, R4\n"      /* Move R4-R7 to R8-R11 */                                                \
	"MOV     R9, R5\n"                                                                                \
	"MOV     R10, R6\n"                                                                               \
	"MOV     R11, R7\n"                                                                               \
	"POP     {R4-R7}\n" /* Pop registers R4-R7(user saved context) */                                 \
	"POP     {R0-R3}\n" /* Start poping the stacked exception frame. */                               \
	"POP     {R4}\n"                                                                                  \
	"MOV     R12, R4\n" /* Move R4 to R12 */                                                          \
	"POP     {LR}\n"    /* Popthe saved LR */                                                         \
	"POP     {R4}\n"    /* Pop the saved PC into LR via R4, We do this to jump into the first task */ \
	"ADD     SP,SP,#4\n"                                                                              \
	"CPSIE   I\n"  /* Enable irq to leave critical section */                                         \
	"BX      R4\n" /* Branch instruction to exit this routine. */

#define ARCH_CONTEXT_SWITCH_ASM                                                                                        \
	"CPSID   I\n"       /* Disable interrupts */                                                                       \
	"PUSH    {R4-R7}\n" /* Push registers R4-R7 */                                                                     \
	"MOV     R4, R8\n"  /* Push registers R8-R11 */                                                                    \
	"MOV     R5, R9\n"                                                                                                 \
	"MOV     R6, R10\n"                                                                                                \
	"MOV     R7, R11\n"                                                                                                \
	"PUSH    {R4-R7}\n"                                                                                                \
	"LDR     R0, =curTCB\n" /* R0 = &curTCB */                                                                         \
	"LDR     R1, [R0]\n"    /* R1 = *R0 = curTCB */                                                                    \
	"MOV     R4, SP\n"      /* R4 = SP */                                                                              \
	"STR     R4, [R1,#8]\n" /* Store the value of the stack pointer (copied to R4) to curTCB.sp */                     \
	""                      /* The process of saving the context of the current task ends here */                      \
	""                      /* Step 2: Update curTCB, load the new task context from its stack to the CPU registers */ \
	"PUSH    {R0,LR}\n"     /* Save the context values of R0, LR */                                                    \
	"BL      nextTCB\n"     /* Custom scheduling function, curTCB = next */                                            \
	"POP     {R0,LR}\n"     /* Restore the values of R0, LR */                                                         \
	"LDR     R1, [R0]\n"                                                                                               \
	"LDR     R4, [R1,#8]\n" /* Load the content of the next task's stack pointer */                                    \
	"MOV     SP, R4\n"      /* Use R4 to load the new task's sp to SP */                                               \
	"POP     {R4-R7}\n"     /* Pop registers R8-R11 */                                                                 \
	"MOV     R8, R4\n"                                                                                                 \
	"MOV     R9, R5\n"                                                                                                 \
	"MOV     R10, R6\n"                                                                                                \
	"MOV     R11, R7\n"                                                                                                \
	"POP     {R4-R7}\n" /* Pop registers R4-R7 */                                                                      \
	"CPSIE   I\n"       /* Enable interrupts */                                                                        \
	"BX      LR"        /* Return from interrupt */

#endif