#ifndef _MOS_GLOBAL_
#define _MOS_GLOBAL_

#include "macro.hpp"
#include "data_type.hpp"

namespace MOS::KernelGlobal
{
	using namespace Macro;
	using namespace DataType;

	Page_t pages[MAX_TASK_NUM];
	List_t ready_list, blocked_list;
	TCB_t::Tid_t tids = 0;
	RxBuffer<32> rx_buf;

	// Put it in extern "C" because the name is referred in asm("") and don't change it.
	// At anytime, the curTCB points to task running currently.
	extern "C" __attribute__((used)) volatile TCB_t::TcbPtr_t curTCB = nullptr;
}

#endif