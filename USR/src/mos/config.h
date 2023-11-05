#ifndef _MOS_CONF_
#define _MOS_CONF_

#define MOS_VERSION "0.0.2"

// Platform
#define MOS_DEVICE "STM32F429"

#if defined(MOS_DEVICE)
#include "stm32f4xx.h"
#include "core_cm4.h"

#define MOS_REBOOT() \
	NVIC_SystemReset()

#define MOS_TRIGGER_SYSTICK_INTR() \
	SCB->ICSR |= SCB_ICSR_PENDSTSET_Msk

#define MOS_TRIGGER_PENDSV_INTR() \
	SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk

#define MOS_TEST_IRQ() \
	__get_PRIMASK() == 0

#define MOS_DISABLE_IRQ() asm volatile("CPSID I");
#define MOS_ENABLE_IRQ()  asm volatile("CPSIE I");

#endif

#define MOS_CONF_ASSERT                          // Whether to use full assert
#define MOS_CONF_PRINTF                          // Whether to use printf
#define MOS_CONF_DEBUG_INFO                      // Whether to use debug info
#define MOS_CONF_MAX_TASK_NUM 16                 // Max task number
#define MOS_CONF_PAGE_SIZE    1024               // Page size for each task in bytes
#define MOS_CONF_SYSTICK      1000               // SystemFrequency / 1000 = every 1ms
#define MOS_CONF_PRI_MAX      0                  // Max priority
#define MOS_CONF_PRI_MIN      15                 // Min priority
#define MOS_CONF_TIME_SLICE   50                 // Time slice
#define MOS_CONF_POLICY       PreemptivePriority // Scheduler Policy: RoundRobin, PreemptivePriority

#endif