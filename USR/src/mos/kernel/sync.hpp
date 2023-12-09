#ifndef _MOS_SYNC_
#define _MOS_SYNC_

#include "task.hpp"

namespace MOS::Sync
{
	using KernelGlobal::ready_list;
	using KernelGlobal::blocked_list;
	using KernelGlobal::curTCB;
	using Util::DisIntrGuard;
	using Util::test_irq;

	using TCB_t       = DataType::TCB_t;
	using TcbPtr_t    = TCB_t::TcbPtr_t;
	using AtomicCnt_t = volatile int32_t;
	using List_t      = DataType::List_t;
	using Status_t    = TCB_t::Status_t;

	struct Semaphore_t
	{
		AtomicCnt_t cnt;
		List_t waiting_list;

		// Must set a original value
		Semaphore_t() = delete;
		Semaphore_t(int32_t val): cnt(val) {}

		// P
		void down()
		{
			// Assert if irq disabled
			MOS_ASSERT(test_irq(), "Disabled Interrupt");
			DisIntrGuard guard;
			cnt -= 1;
			if (cnt < 0) {
				curTCB->set_status(Status_t::BLOCKED);
				ready_list.send_to(curTCB->node, waiting_list);
				return Task::yield();
			}
		}

		// V
		void up()
		{
			// Assert if irq disabled
			MOS_ASSERT(test_irq(), "Disabled Interrupt");
			DisIntrGuard guard;
			if (cnt < 0) {
				auto& tcb = (TCB_t&) *waiting_list.begin();
				tcb.set_status(Status_t::READY);
				waiting_list.send_to_in_order(tcb.node, ready_list, TCB_t::priority_cmp);
			}
			cnt += 1;
			if (curTCB != (TcbPtr_t) ready_list.begin()) {
				return Task::yield();
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
			MOS_ASSERT(owner != Task::current_task(), "Non-recursive lock");
			owner = Task::current_task();
			sema.down();
		}

		__attribute__((always_inline)) inline void
		release()
		{
			MOS_ASSERT(owner == Task::current_task(),
			           "Lock can only be released by holder");
			sema.up();
			owner = nullptr;
		}
	};

	struct MutexImpl_t
	{
		using Prior_t = TCB_t::Prior_t;

		Semaphore_t sema;
		TcbPtr_t owner;
		Prior_t old_pr;
		Prior_t ceiling;
		AtomicCnt_t recursive_cnt;

		MutexImpl_t(Prior_t ceiling = Macro::PRI_MAX)
		    : sema(1), owner(nullptr), old_pr(-1), recursive_cnt(0), ceiling(ceiling) {}

		void lock()// P
		{
			MOS_ASSERT(test_irq(), "Disabled Interrupt");

			DisIntrGuard guard;

			if (owner == curTCB) {
				// If the current task already owns the lock, just increment the lock count
				recursive_cnt += 1;
				return;
			}

			// Raise the priority of the current task to the ceiling of the mutex
			old_pr = curTCB->get_priority();
			if (ceiling < curTCB->get_priority()) {
				curTCB->set_priority(ceiling);
			}

			sema.cnt -= 1;

			if (sema.cnt < 0) {
				curTCB->set_status(Status_t::BLOCKED);
				ready_list.send_to_in_order(curTCB->node, sema.waiting_list, TCB_t::priority_cmp);
				return Task::yield();
			}
			else {
				owner = curTCB;
				recursive_cnt += 1;
			}
		}

		void unlock()// V
		{
			MOS_ASSERT(test_irq(), "Disabled Interrupt");
			MOS_ASSERT(owner == curTCB, "Lock can only be released by holder");

			DisIntrGuard guard;
			recursive_cnt -= 1;

			if (recursive_cnt > 0) {
				// If the lock is still held by the current task, just return
				return;
			}

			if (!sema.waiting_list.empty()) {

				auto st = sema.waiting_list.begin(),
				     ed = sema.waiting_list.end();

				// Starvation Prevention
				for (; st != ed; st = st->next) {
					if (st->next == ed || !TCB_t::priority_equal(*st, *st->next)) {
						break;
					}
				}

				auto tcb = (TcbPtr_t) st;
				tcb->set_status(Status_t::READY);

				sema.waiting_list.send_to_in_order(tcb->node, ready_list, TCB_t::priority_cmp);

				// Transfer ownership to the last highest priority task in queue
				owner = tcb;
				sema.cnt += 1;

				if (curTCB != (TcbPtr_t) ready_list.begin()) {
					return Task::yield();
				}
			}
			else {
				// No owner if no tasks are waiting
				owner = nullptr;
				sema.cnt += 1;
			}

			// Restore the original priority of the owner
			if (recursive_cnt == 0 && old_pr != -1) {
				owner->set_priority(old_pr);
				old_pr = -1;
			}
		}
	};

	// template <typename T = void>
	// struct Mutex_t : public MutexImpl_t
	// {
	// 	T raw;

	// 	T& get()
	// 	{
	// 		lock();
	// 		unlock();
	// 		return raw;
	// 	}

	// 	const T& get() const
	// 	{
	// 		lock();
	// 		unlock();
	// 		return raw;
	// 	}
	// };

	// template <>
	// struct Mutex_t<void> : public MutexImpl_t
	// {
	// };
}

#endif