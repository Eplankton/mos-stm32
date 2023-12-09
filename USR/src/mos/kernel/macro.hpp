#ifndef _MOS_MACRO_
#define _MOS_MACRO_

#include <stdint.h>
#include "../config.h"

namespace MOS::Macro
{
	constexpr uint32_t MAX_TASK_NUM = MOS_CONF_MAX_TASK_NUM;
	constexpr uint32_t PAGE_SIZE    = MOS_CONF_PAGE_SIZE / sizeof(uint32_t);
	constexpr uint32_t SYSTICK      = MOS_CONF_SYSTICK;
	constexpr int8_t PRI_MAX        = MOS_CONF_PRI_MAX;
	constexpr int8_t PRI_MIN        = MOS_CONF_PRI_MIN;
	constexpr uint16_t TIME_SLICE   = MOS_CONF_TIME_SLICE;
}

#endif