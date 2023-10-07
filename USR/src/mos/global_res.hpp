#ifndef _MOS_GLOBALRES_
#define _MOS_GLOBALRES_

#include "macro.hpp"
#include "datatype.hpp"

namespace MOS::GlobalRes
{
	using namespace Macro;
	using namespace DataType;

	Page_t pages[MAX_TASK_NUM];
	list_t ready_list, blocked_list;
	TCB_t::Tid_t tids = 0;

	// Put it in extern "C" because the name is referred in asm("") and don't change it.
	// Anytime when a task is running, the curTCB points to its function.
	extern "C" __attribute__((used)) volatile TCB_t* curTCB = nullptr;
}

#endif