#ifndef _MOS_KERNEL_GLOBAL_
#define _MOS_KERNEL_GLOBAL_

#include "macro.hpp"
#include "data_type.hpp"

#ifdef MOS_CONF_DEBUG_INFO
#define MOS_DEBUG_INFO extern "C" __attribute__((used)) volatile
#else
#define MOS_DEBUG_INFO
#endif

namespace MOS::KernelGlobal
{
	using namespace Macro;
	using namespace DataType;

	Page_t page_pool[MAX_TASK_NUM];
	List_t ready_list, blocked_list, sleep_list;
	TCB_t::Tid_t tids = 0;
	RxBuffer<32> rx_buf;

	// Put it in extern "C" because the name is referred in asm("") and don't change it.
	// At anytime, the curTCB points to task running currently.
	MOS_DEBUG_INFO TCB_t::TcbPtr_t curTCB = nullptr;
	MOS_DEBUG_INFO TCB_t::Tick_t os_ticks = 0;

	// For debug only
	MOS_DEBUG_INFO DebugTasks debug_tcbs {};
}

#endif