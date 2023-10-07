#ifndef _MOS_MACRO_
#define _MOS_MACRO_

#include "config.h"
#include "stdint.h"

namespace MOS::Macro
{
	constexpr uint32_t MAX_TASK_NUM = MOS_CONF_MAX_TASK_NUM;
	constexpr uint32_t PAGE_SIZE    = MOS_CONF_PAGE_SIZE / 4;
	constexpr uint32_t SYSCLK       = MOS_CONF_SYSCLK;
}

#endif