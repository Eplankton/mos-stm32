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

		// P
		void down()
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

		// V
		void up()
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
	};

	struct Lock_t
	{
		Semaphore_t sema;
		TcbPtr_t owner;

		Lock_t(): owner(nullptr), sema(1) {}

		__attribute__((always_inline)) inline void
		acquire()
		{
			MOS_ASSERT(owner != current_task(), "Non-recursive lock");
			owner = current_task();
			sema.down();
		}

		__attribute__((always_inline)) inline void
		release()
		{
			MOS_ASSERT(owner == current_task(), "Lock can only be released by holder");
			sema.up();
			owner = nullptr;
		}
	};

	// struct MutexImpl_t
	// {
	// 	using Atomic_i32_t = volatile int32_t;

	// 	Semaphore_t sema;
	// 	TcbPtr_t owner;

	// 	// Must set a original value
	// 	MutexImpl_t(): sema(1), owner(nullptr) {}

	// 	void lock()// P
	// 	{
	// 		// Assert if irq disabled
	// 		MOS_ASSERT(test_irq(), "Disabled Interrupt");

	// 		MOS_DISABLE_IRQ();
	// 		sema.cnt -= 1;
	// 		if (sema.cnt < 0) {
	// 			if (owner != nullptr && TCB_t::priority_cmp(curTCB->node, owner->node)) {
	// 				owner->set_priority(curTCB->get_priority());// Priority inheritance
	// 			}
	// 			curTCB->set_status(Status_t::BLOCKED);
	// 			sema.waiting_list.insert_in_order(curTCB->node, &TCB_t::priority_cmp);
	// 			MOS_ENABLE_IRQ();
	// 			yield();
	// 		}
	// 		else {
	// 			owner = curTCB;
	// 		}
	// 		MOS_ENABLE_IRQ();
	// 	}

	// 	void unlock()// V
	// 	{
	// 		// Assert if irq disabled
	// 		MOS_ASSERT(test_irq(), "Disabled Interrupt");
	// 		MOS_DISABLE_IRQ();
	// 		if (sema.cnt < 0) {
	// 			auto& tcb = *(TcbPtr_t) sema.waiting_list.begin();
	// 			sema.waiting_list.remove(tcb.node);
	// 			ready_list.insert_in_order(tcb.node, &TCB_t::priority_cmp);
	// 			tcb.set_status(Status_t::READY);
	// 			if (owner == curTCB) {
	// 				if (!sema.waiting_list.empty()) {
	// 					// Transfer ownership to highest priority task in queue
	// 					owner = (TcbPtr_t) sema.waiting_list.begin();
	// 				}
	// 				else {
	// 					// No owner if no tasks are waiting
	// 					owner = nullptr;
	// 				}
	// 			}
	// 			sema.cnt += 1;
	// 			MOS_ENABLE_IRQ();
	// 			if (curTCB != (TcbPtr_t) sema.waiting_list.begin()) {
	// 				yield();
	// 			}
	// 		}
	// 		sema.cnt += 1;
	// 		MOS_ENABLE_IRQ();
	// 	}
	// };

	// template <typename T = void>
	// struct Mutex_t : public MutexImpl_t
	// {
	// 	T raw;
	// };

	// template <>
	// struct Mutex_t<void> : public MutexImpl_t
	// {
	// };
}

#endif