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

		Atomic_i32_t cnt = 0;
		List_t waiting_list;

		Semaphore_t() = default;
		Semaphore_t(int32_t val): cnt(val) {}

		__attribute__((always_inline)) inline void
		up()// V
		{
			// Assert if irq disabled
			MOS_ASSERT(test_irq(), "");
			MOS_DISABLE_IRQ();
			if (cnt < 0) {
				auto& t = (TCB_t&) *waiting_list.begin();
				t.set_status(Status_t::READY);
				waiting_list.remove(t.node);
				ready_list.insert_in_order(t.node, &TCB_t::priority_cmp);
			}
			cnt += 1;
			MOS_ENABLE_IRQ();
		}

		void down()// P
		{
			// Assert if irq disabled
			MOS_ASSERT(test_irq(), "");
			MOS_DISABLE_IRQ();
			cnt -= 1;
			if (cnt < 0) {
				curTCB->set_status(Status_t::BLOCKED);
				ready_list.remove((Node_t&) curTCB->node);
				waiting_list.add((Node_t&) curTCB->node);
				MOS_ENABLE_IRQ();
				yield();
			}
			MOS_ENABLE_IRQ();
		}
	};

	struct Lock_t
	{
		TCB_t* holder;
		Semaphore_t sema;

		Lock_t(): holder((TCB_t*) curTCB), sema(1) {}

		__attribute__((always_inline)) inline void
		acquire() { sema.down(); };

		__attribute__((always_inline)) inline void
		release() { sema.up(); };
	};
}

#endif