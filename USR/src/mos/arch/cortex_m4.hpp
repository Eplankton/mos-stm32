#ifndef _MOS_ARCH_CORTEX_M4_
#define _MOS_ARCH_CORTEX_M4_

#include "stm32f4xx.h"
#include "core_cm4.h"

#define MOS_REBOOT()              NVIC_SystemReset()
#define MOS_TRIGGER_SVC_INTR()    asm volatile("svc    0")
#define MOS_TRIGGER_PENDSV_INTR() SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk
#define MOS_SVC_HANDLER           SVC_Handler
#define MOS_PENDSV_HANDLER        PendSV_Handler
#define MOS_SYSTICK_HANDLER       SysTick_Handler
#define MOS_TEST_IRQ()            __get_PRIMASK() == 0
#define MOS_DISABLE_IRQ()         asm volatile("cpsid i" : : : "memory")
#define MOS_ENABLE_IRQ()          asm volatile("cpsie i" : : : "memory")
#define MOS_NOP()                 asm volatile("nop")
#define MOS_DSB()                 __DSB()
#define MOS_ISB()                 __ISB()

// From FreeRTOS -> https://www.freertos.org
#define ARCH_INIT_ASM                                                        \
	"cpsid   i\n" /* Disable interrupts */                                   \
	"ldr     r3, =cur_tcb\n"                                                 \
	"ldr     r1, [r3]\n"                                                     \
	"ldr     r0, [r1,#8]\n"   /* r0 = cur_tcb.sp */                          \
	"ldmia   r0!, {r4-r11}\n" /* Pop registers R4-R11(user saved context) */ \
	"msr     psp, r0\n"       /* PSP = cur_tcb.sp(new) */                    \
	"mov     r0, #0\n"                                                       \
	"orr     lr, #0xD\n" /* Enter Thread Mode: 0xFFFF'FFFD */                \
	"cpsie   i\n"        /* Enable interrupts */                             \
	"bx      lr\n"

// From FreeRTOS -> https://www.freertos.org
#define ARCH_CONTEXT_SWITCH_ASM                          \
	"cpsid   i\n" /* Disable interrupts */               \
	"mrs     r0, psp\n"                                  \
	"ldr     r3, =cur_tcb\n"                             \
	"ldr     r2, [r3]\n"                                 \
	"stmdb   r0!, {r4-r11}\n" /* Save core registers. */ \
	"str     r0, [r2,#8]\n"   /* Get cur_tcb.sp */       \
	"stmdb   sp!, {r3,lr}\n"                             \
	"bl      next_tcb\n"                                 \
	"ldmia   sp!, {r3,lr}\n"                             \
	"ldr     r1, [r3]\n"                                 \
	"ldr     r0, [r1,#8]\n"   /* Get cur_tcb.sp(new) */  \
	"ldmia   r0!, {r4-r11}\n" /* Pop core registers. */  \
	"msr     psp, r0\n"                                  \
	"cpsie   i\n" /* Enable interrupts */                \
	"bx      lr\n"

#define ARCH_JUMP_TO_CONTEXT_SWITCH "B    context_switch"

#endif