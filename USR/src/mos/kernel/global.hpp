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

	using Tid_t    = Tcb_t::Tid_t;
	using Tick_t   = Tcb_t::Tick_t;
	using TcbPtr_t = Tcb_t::TcbPtr_t;

	Tid_t tids = -1;
	Page_t page_pool[MAX_TASK_NUM];
	TcbList_t ready_list, blocked_list, sleep_list;

	// Put it in `extern "C"` because the name is referred in `asm("")` and don't change it.
	// At anytime, the `cur_tcb` should point to the task running currently.
	MOS_DEBUG_INFO TcbPtr_t cur_tcb = nullptr;
	MOS_DEBUG_INFO Tick_t os_ticks  = 0;

	// For debug only
	MOS_DEBUG_INFO DebugTcbs_t debug_tcbs {};
}

#endif