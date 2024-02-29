#ifndef _MOS_ARCH_CORTEX_M4_
#define _MOS_ARCH_CORTEX_M4_

#include "stm32f4xx.h"
#include "core_cm4.h"

#define MOS_REBOOT()              NVIC_SystemReset()
#define MOS_TRIGGER_SVC_INTR()    asm volatile("SVC      0")
#define MOS_TRIGGER_PENDSV_INTR() SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk
#define MOS_SVC_HANDLER           SVC_Handler
#define MOS_PENDSV_HANDLER        PendSV_Handler
#define MOS_SYSTICK_HANDLER       SysTick_Handler
#define MOS_TEST_IRQ()            __get_PRIMASK() == 0
#define MOS_DISABLE_IRQ()         asm volatile("CPSID    I")
#define MOS_ENABLE_IRQ()          asm volatile("CPSIE    I")
#define MOS_NOP()                 asm volatile("nop")
#define MOS_DSB()                 __DSB()
#define MOS_ISB()                 __ISB()

// From FreeRTOS -> https://www.freertos.org
#define ARCH_INIT_ASM                                                        \
	"CPSID   I\n" /* Disable interrupts */                                   \
	"LDR     R3, =cur_tcb\n"                                                 \
	"LDR     R1, [R3]\n"                                                     \
	"LDR     R0, [R1,#8]\n"   /* R0 = cur_tcb.sp */                          \
	"LDMIA   R0!, {R4-R11}\n" /* Pop registers R4-R11(user saved context) */ \
	"MSR     PSP, R0\n"       /* PSP = cur_tcb.sp(new) */                    \
	"MOV     R0, #0\n"                                                       \
	"ORR     LR, #0xD\n" /* Enter Thread Mode: 0xFFFF'FFFD */                \
	"CPSIE   I\n"        /* Enable interrupts */                             \
	"BX      LR\n"

// From FreeRTOS -> https://www.freertos.org
#define ARCH_CONTEXT_SWITCH_ASM                          \
	"CPSID   I\n" /* Disable interrupts */               \
	"MRS     R0, PSP\n"                                  \
	"LDR     R3, =cur_tcb\n"                             \
	"LDR     R2, [R3]\n"                                 \
	"STMDB   R0!, {R4-R11}\n" /* Save core registers. */ \
	"STR     R0, [R2,#8]\n"   /* Get cur_tcb.sp */       \
	"STMDB   SP!, {R3,LR}\n"                             \
	"BL      next_tcb\n"                                 \
	"ldmia   SP!, {R3,LR}\n"                             \
	"LDR     R1, [R3]\n"                                 \
	"LDR     R0, [R1,#8]\n"   /* Get cur_tcb.sp(new) */  \
	"LDMIA   R0!, {R4-R11}\n" /* Pop core registers. */  \
	"MSR     PSP, R0\n"                                  \
	"CPSIE   I\n" /* Enable interrupts */                \
	"BX      LR\n"

#define ARCH_JUMP_TO_CONTEXT_SWITCH "B    context_switch"

#endif