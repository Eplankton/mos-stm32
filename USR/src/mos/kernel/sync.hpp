#ifndef _MOS_SYNC_
#define _MOS_SYNC_

#include "task.hpp"

namespace MOS::Sync
{
	using KernelGlobal::ready_list;
	using KernelGlobal::blocked_list;
	using KernelGlobal::cur_tcb;
	using Util::DisIntrGuard;
	using Util::test_irq;

	using Tcb_t       = DataType::Tcb_t;
	using TcbPtr_t    = Tcb_t::TcbPtr_t;
	using Prior_t     = Tcb_t::Prior_t;
	using List_t      = DataType::List_t;
	using AtomicCnt_t = volatile int32_t;
	using Status      = Tcb_t::Status;

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
				cur_tcb->set_status(Status::BLOCKED);
				ready_list.send_to(cur_tcb->node, waiting_list);
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
				auto tcb = (TcbPtr_t) waiting_list.begin();
				tcb->set_status(Status::READY);
				waiting_list.send_to_in_order(tcb->node, ready_list, Tcb_t::priority_cmp);
			}
			cnt += 1;
			if (cur_tcb != (TcbPtr_t) ready_list.begin()) {
				return Task::yield();
			}
		}
	};

	struct Lock_t
	{
		Semaphore_t sema;
		TcbPtr_t owner;

		Lock_t(): owner(nullptr), sema(1) {}

		MOS_INLINE inline void
		acquire()
		{
			MOS_ASSERT(owner != Task::current(), "Non-recursive lock");
			owner = Task::current();
			sema.down();
		}

		MOS_INLINE inline void
		release()
		{
			MOS_ASSERT(owner == Task::current(),
			           "Lock can only be released by holder");
			sema.up();
			owner = nullptr;
		}
	};

	struct MutexImpl_t
	{
		Semaphore_t sema;
		TcbPtr_t owner;
		Prior_t old_pr, ceiling;
		AtomicCnt_t recursive_cnt;

		MutexImpl_t(Prior_t ceiling = Macro::PRI_MAX)
		    : sema(1), owner(nullptr), old_pr(-1), recursive_cnt(0), ceiling(ceiling) {}

		void lock() // P-opr
		{
			MOS_ASSERT(test_irq(), "Disabled Interrupt");

			DisIntrGuard guard;

			if (owner == cur_tcb) {
				// If the current task already owns the lock, just increment the lock count
				recursive_cnt += 1;
				return;
			}

			// Raise the priority of the current task to the ceiling of the mutex
			old_pr = cur_tcb->get_priority();
			if (ceiling < cur_tcb->get_priority()) {
				cur_tcb->set_priority(ceiling);
			}

			sema.cnt -= 1;

			if (sema.cnt < 0) {
				cur_tcb->set_status(Status::BLOCKED);
				ready_list.send_to_in_order(cur_tcb->node, sema.waiting_list, Tcb_t::priority_cmp);
				return Task::yield();
			}
			else {
				owner = cur_tcb;
				recursive_cnt += 1;
			}
		}

		void unlock() // V-opr
		{
			MOS_ASSERT(test_irq(), "Disabled Interrupt");
			MOS_ASSERT(owner == cur_tcb, "Lock can only be released by holder");

			DisIntrGuard guard;
			recursive_cnt -= 1;

			if (recursive_cnt > 0) {
				// If the lock is still held by the current task, just return
				return;
			}

			if (!sema.waiting_list.empty()) {

				auto ed = sema.waiting_list.end();

				// Starvation Prevention
				auto tcb = (TcbPtr_t) sema.waiting_list.iter_until(
				        [&](const auto& node) {
					        return node.next == ed ||
					               !Tcb_t::priority_equal(node, *node.next);
				        });

				tcb->set_status(Status::READY);

				sema.waiting_list.send_to_in_order(tcb->node, ready_list, Tcb_t::priority_cmp);

				// Transfer ownership to the last highest priority task in queue
				owner = tcb;
				sema.cnt += 1;

				if (cur_tcb != (TcbPtr_t) ready_list.begin()) {
					return Task::yield();
				}
			}
			else {
				// Restore the original priority of the owner
				if (recursive_cnt == 0 && old_pr != -1) {
					owner->set_priority(old_pr);
					old_pr = -1;
				}

				// No owner if no tasks are waiting
				owner = nullptr;
				sema.cnt += 1;

				return;
			}
		}
	};

	template <typename T = void>
	struct Mutex_t : private MutexImpl_t
	{
		using Raw_t    = T;
		using RawRef_t = Raw_t&;

		struct MutexGuard
		{
			// Unlock when scope ends
			MOS_INLINE inline ~MutexGuard() { mutex.unlock(); }
			MOS_INLINE inline MutexGuard(Mutex_t<T>& mutex)
			    : mutex(mutex) { mutex.MutexImpl_t::lock(); }

			// Raw Accessor
			MOS_INLINE inline RawRef_t get() { return mutex.raw; }
			MOS_INLINE inline RawRef_t operator*() { return get(); }

		private:
			Mutex_t<Raw_t>& mutex;
		};

		Mutex_t(T raw, Prior_t ceiling)
		    : MutexImpl_t(ceiling), raw(raw) {}

		MOS_INLINE inline MutexGuard
		lock() { return MutexGuard {*this}; }

		MOS_INLINE inline auto
		exec(auto&& section) // To safely execute
		{
			// MutexGuard scope begins
			auto guard = lock();
			return section();
			// MutexGuard scope ends
		}

	private:
		Raw_t raw;
	};

	template <>
	struct Mutex_t<> : private MutexImpl_t
	{
		struct MutexGuard // No Raw Accessor for T=void
		{
			// Unlock when scope ends
			MOS_INLINE inline ~MutexGuard() { mutex.unlock(); }
			MutexGuard(Mutex_t& mutex)
			    : mutex(mutex) { mutex.MutexImpl_t::lock(); }

		private:
			Mutex_t& mutex;
		};

		MOS_INLINE inline Mutex_t(Prior_t ceiling = Macro::PRI_MAX)
		    : MutexImpl_t(ceiling) {}

		MOS_INLINE inline MutexGuard
		lock() { return MutexGuard {*this}; }

		MOS_INLINE inline auto
		exec(auto&& section) // To safely execute
		{
			// MutexGuard scope begins
			auto guard = lock();
			return section();
			// MutexGuard scope ends
		}
	};

	// Template deduction where T = void
	Mutex_t() -> Mutex_t<void>;
	Mutex_t(Task::Prior_t) -> Mutex_t<void>;

	template <typename T>
	Mutex_t(T&&, Task::Prior_t) -> Mutex_t<T>;

	template <typename T>
	Mutex_t(T&, Task::Prior_t) -> Mutex_t<T&>;
}

#endif