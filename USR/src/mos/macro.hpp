#ifndef _MOS_MACRO_
#define _MOS_MACRO_

#include "config.h"
#include "stdint.h"

namespace MOS::Macro
{
	constexpr uint32_t MAX_TASK_NUM = MOS_CONF_MAX_TASK_NUM;
	constexpr uint32_t PAGE_SIZE    = MOS_CONF_PAGE_SIZE / 4;
	constexpr uint32_t SYSTICK      = MOS_CONF_SYSTICK;
	constexpr int8_t PRI_MAX        = MOS_CONF_PRI_MAX;
	constexpr int8_t PRI_MIN        = MOS_CONF_PRI_MIN;
	constexpr uint16_t TICKS        = MOS_CONF_TICKS;
}

#endif