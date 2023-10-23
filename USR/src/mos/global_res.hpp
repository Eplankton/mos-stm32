#ifndef _MOS_GLOBALRES_
#define _MOS_GLOBALRES_

#include "macro.hpp"
#include "data_type.hpp"

namespace MOS::GlobalRes
{
	using namespace Macro;
	using namespace DataType;
	using TcbPtr_t = TCB_t::TcbPtr_t;

	Page_t pages[MAX_TASK_NUM];
	List_t ready_list, blocked_list;
	TCB_t::Tid_t tids = 0;

	// Put it in extern "C" because the name is referred in asm("") and don't change it.
	// At anytime, the curTCB points to task running currently.
	extern "C" __attribute__((used)) volatile TcbPtr_t curTCB = nullptr;
}

#endif