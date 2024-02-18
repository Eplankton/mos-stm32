#ifndef _MOS_KERNEL_GLOBAL_
#define _MOS_KERNEL_GLOBAL_

#include "macro.hpp"
#include "data_type/tcb.hpp"
#include "data_type/bitmap.hpp"

#if (MOS_CONF_DEBUG_INFO)
#define MOS_DEBUG_INFO extern "C" __attribute__((used)) volatile
#else
#define MOS_DEBUG_INFO
#endif

namespace MOS::KernelGlobal
{
	using namespace Macro;
	using namespace DataType;

	using PagePool_t = Page_t::Word_t[POOL_NUM][PAGE_SIZE];
	using Tids_t     = BitMap_t<MAX_TASK_NUM>;
	using Tick_t     = TCB_t::Tick_t;
	using TcbPtr_t   = TCB_t::TcbPtr_t;

	PagePool_t page_pool;
	Tids_t tids;

	TcbList_t
	    ready_list,    // Tasks sorted by `Prior_t` that are `READY` to be scheduled.
	    blocked_list,  // Tasks that are `BLOCKED` and waiting for a certain condition.
	    sleeping_list, // Tasks sorted by `delay_ticks` that are sleeping `BLOCKED` for a certain amount of time.
	    zombie_list;   // Tasks that have been `TERMINATED` but their resources are not recycled yet.

	// Put it in `extern "C"` because the name is referred in `asm("")` and don't change it.
	// At anytime, `cur_tcb` should point to the task running currently.
	MOS_DEBUG_INFO
	TcbPtr_t cur_tcb = nullptr;

	MOS_DEBUG_INFO
	Tick_t os_ticks = 0;

	// For debug only
	MOS_DEBUG_INFO
	DebugTcbs_t debug_tcbs {};
}

#endif