#ifndef _MOS_CONF_
#define _MOS_CONF_

#define MOS_DISABLE_IRQ() asm("CPSID I")
#define MOS_ENABLE_IRQ()  asm("CPSIE I")

#define MOS_CONF_DEBUG_INFO       // Whether to use debug info
#define MOS_CONF_MAX_TASK_NUM 32  // Max task number
#define MOS_CONF_PAGE_SIZE    1024// Page size for each task in bytes

// SystemFrequency / 1000    every 1ms
// SystemFrequency / 10000   every 100us
// SystemFrequency / 100000  every 10us
// SystemFrequency / 1000000 every 1us
#define MOS_CONF_SYSCLK 1000

#define MOS_CONF_SCHEDULER_POLICY RoundRobin// Scheduler Policy: {RoundRobin, PreemptivePriority}

#endif