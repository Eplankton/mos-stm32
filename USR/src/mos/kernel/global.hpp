#ifndef _MOS_KERNEL_GLOBAL_
#define _MOS_KERNEL_GLOBAL_

#include "data_type/tcb.hpp"
#include "data_type/bitmap.hpp"

#if (MOS_CONF_DEBUG_INFO)
#define MOS_DEBUG_INFO extern "C" MOS_USED volatile
#else
#define MOS_DEBUG_INFO
#endif

namespace MOS::Kernel::Global
{
	using namespace Macro;
	using namespace DataType;

	using Pool_t   = Page_t::Word_t[POOL_NUM][PAGE_SIZE];
	using Tids_t   = BitMap_t<MAX_TASK_NUM>;
	using Tick_t   = TCB_t::Tick_t;
	using TcbPtr_t = TCB_t::TcbPtr_t;

	Pool_t page_pool;
	Tids_t tids;

	TcbList_t
	    ready_list,    // Contains tasks sorted by `Prior_t` that are `READY` to be scheduled.
	    blocked_list,  // Contains tasks that are `BLOCKED` and waiting for a certain condition.
	    sleeping_list, // Contains tasks sorted by `delay_ticks` that are sleeping `BLOCKED` for a certain amount of time.
	    zombie_list;   // Contains tasks that have been `TERMINATED` but resources are not yet recycled.

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