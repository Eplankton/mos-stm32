#ifndef _MOS_CONF_
#define _MOS_CONF_

// Platform
#define _DEVICE_STM32F4_

#if defined(_DEVICE_STM32F4_)
#include "stm32f4xx.h"
#define MOS_TRIGGER_SYSTICK() (SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk)
#define MOS_DISABLE_IRQ()     asm("CPSID I")
#define MOS_ENABLE_IRQ()      asm("CPSIE I")

#endif

#define MOS_CONF_ASSERT
#define MOS_CONF_DEBUG_INFO                 // Whether to use debug info
#define MOS_CONF_MAX_TASK_NUM     32        // Max task number
#define MOS_CONF_PAGE_SIZE        1024      // Page size for each task in bytes
#define MOS_CONF_SYSCLK           1000      // SystemFrequency / 1000    every 1ms
#define MOS_CONF_SCHEDULER_POLICY RoundRobin// Scheduler Policy: {RoundRobin, PreemptivePriority}

#endif