#ifndef _MOS_CONFIG_
#define _MOS_CONFIG_

// Info Configuration
#define MOS_VERSION        "0.0.3(beta)"
#define MOS_ARCH_CORTEX_M4 "Cortex-M4F"
#define MOS_ARCH           MOS_ARCH_CORTEX_M4
#define MOS_MCU            "STM32F429ZIT6"

// MOS Settings
#define MOS_CONF_ASSERT       true       // Whether to use full assert
#define MOS_CONF_PRINTF       true       // Whether to use printf
#define MOS_CONF_DEBUG_INFO   true       // Whether to use debug info
#define MOS_CONF_MAX_TASK_NUM 16         // Maximum number of tasks
#define MOS_CONF_POOL_NUM     16         // Number of pre-allocated pages
#define MOS_CONF_PAGE_SIZE    1024       // Page size in bytes
#define MOS_CONF_SYSTICK      1000       // SystemFrequency / 1000 = every 1ms
#define MOS_CONF_PRI_NONE     -1         // None priority
#define MOS_CONF_PRI_MAX      0          // Max priority
#define MOS_CONF_PRI_MIN      15         // Min priority
#define MOS_CONF_TIME_SLICE   50         // Time slice width
#define MOS_CONF_SCHED_POLICY PreemptPri // Scheduler Policy: RoundRobin, PreemptPri
#define MOS_CONF_RX_BUF_SIZE  16         // UART RX Buffer Size

#endif