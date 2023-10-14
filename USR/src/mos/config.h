#ifndef _MOS_CONF_
#define _MOS_CONF_

// Platform
#define _DEVICE_STM32F429_

#if defined(_DEVICE_STM32F429_)
#include "stm32f4xx.h"

#define MOS_TRIGGER_SYSTICK_INTR() \
	SysTick->VAL = 0;              \
	SCB->ICSR |= SCB_ICSR_PENDSTSET_Msk;

#define MOS_TRIGGER_PENDSV_INTR() \
	SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;

#include "core_cm4.h"  // Change this to your specific core_cmX.h file

#define MOS_TEST_IRQ() \
	__get_PRIMASK() == 0

#define MOS_DISABLE_IRQ() asm volatile ("cpsid i" : : : "memory");
#define MOS_ENABLE_IRQ()  asm volatile ("cpsie i" : : : "memory");

#endif

#define MOS_CONF_ASSERT                      // Whether to use full assert
#define MOS_CONF_DEBUG_INFO                  // Whether to use debug info
#define MOS_CONF_MAX_TASK_NUM     32         // Max task number
#define MOS_CONF_PAGE_SIZE        1024       // Page size for each task in bytes
#define MOS_CONF_SYSTICK          1000       // SystemFrequency / 1000    every 1ms
#define MOS_CONF_SCHEDULER_POLICY PreemptivePriority // Scheduler Policy: RoundRobin, PreemptivePriority

#endif