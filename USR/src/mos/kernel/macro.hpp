#ifndef _MOS_MACRO_
#define _MOS_MACRO_

#include "../config.h"
#include "type.hpp"

namespace MOS::Macro
{
	constexpr uint32_t MAX_TASK_NUM = MOS_CONF_MAX_TASK_NUM;
	constexpr uint32_t POOL_NUM     = MOS_CONF_POOL_NUM;
	constexpr uint32_t PAGE_SIZE    = MOS_CONF_PAGE_SIZE / sizeof(uint32_t);
	constexpr uint16_t TIME_SLICE   = MOS_CONF_TIME_SLICE;
	constexpr uint32_t SYSTICK      = MOS_CONF_SYSTICK;
	constexpr int8_t PRI_NONE       = MOS_CONF_PRI_NONE;
	constexpr int8_t PRI_MAX        = MOS_CONF_PRI_MAX;
	constexpr int8_t PRI_MIN        = MOS_CONF_PRI_MIN;
	constexpr uint32_t RX_BUF_SIZE  = MOS_CONF_RX_BUF_SIZE;
}

#endif