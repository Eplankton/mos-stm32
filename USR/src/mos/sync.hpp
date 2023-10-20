#ifndef _MOS_SYNC_
#define _MOS_SYNC_

#include "config.h"
#include "task.hpp"

namespace MOS::Sync
{
	using namespace Task;
	using namespace Util;

	struct Semaphore_t
	{
		using Atomic_i32_t = volatile int32_t;

		Atomic_i32_t cnt;
		List_t waiting_list;

		// Must set a original value
		Semaphore_t() = delete;
		Semaphore_t(int32_t val): cnt(val) {}

		void up()// V
		{
			// Assert if irq disabled
			MOS_ASSERT(test_irq(), "Disabled Interrupt");
			MOS_DISABLE_IRQ();
			if (cnt < 0) {
				auto& tcb = (TCB_t&) *waiting_list.begin();
				tcb.set_status(Status_t::READY);
				waiting_list.remove(tcb.node);
				ready_list.insert_in_order(tcb.node, &TCB_t::priority_cmp);
			}
			cnt += 1;
			MOS_ENABLE_IRQ();
			if (curTCB != (TcbPtr_t) ready_list.begin()) {
				yield();
			}
		}

		void down()// P
		{
			// Assert if irq disabled
			MOS_ASSERT(test_irq(), "Disabled Interrupt");
			MOS_DISABLE_IRQ();
			cnt -= 1;
			if (cnt < 0) {
				curTCB->set_status(Status_t::BLOCKED);
				ready_list.remove(curTCB->node);
				waiting_list.add(curTCB->node);
				MOS_ENABLE_IRQ();
				yield();
			}
			MOS_ENABLE_IRQ();
		}
	};

	struct Lock_t
	{
		TcbPtr_t holder;
		Semaphore_t sema;

		Lock_t(): holder(nullptr), sema(1) {}

		__attribute__((always_inline)) inline void
		acquire()
		{
			MOS_ASSERT(holder != current_task(), "Non-recursive lock");
			holder = current_task();
			sema.down();
		}

		__attribute__((always_inline)) inline void
		release()
		{
			MOS_ASSERT(holder == current_task(), "Lock can only be released by holder");
			sema.up();
			holder = nullptr;
		}
	};
}

#endif