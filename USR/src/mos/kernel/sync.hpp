#ifndef _MOS_SYNC_
#define _MOS_SYNC_

#include "task.hpp"

namespace MOS::Sync
{
	using KernelGlobal::ready_list;
	using KernelGlobal::blocked_list;
	using KernelGlobal::cur_tcb;
	using Utils::DisIntrGuard_t;
	using Utils::test_irq;

	using Tcb_t     = DataType::Tcb_t;
	using TcbList_t = DataType::TcbList_t;
	using TcbPtr_t  = Tcb_t::TcbPtr_t;
	using Prior_t   = Tcb_t::Prior_t;
	using Cnt_t     = volatile int32_t;
	using Status    = Tcb_t::Status;

	struct Semaphore_t
	{
		Cnt_t cnt;
		TcbList_t waiting_list;

		// Must set a original value
		Semaphore_t() = delete;
		Semaphore_t(int32_t val): cnt(val) {}

		// P
		void down()
		{
			// Assert if irq disabled
			MOS_ASSERT(test_irq(), "Disabled Interrupt");
			DisIntrGuard_t guard;
			cnt -= 1;
			if (cnt < 0) {
				cur_tcb->set_status(Status::BLOCKED);
				ready_list.send_to(Task::current(), waiting_list);
				return Task::yield();
			}
		}

		// V
		void up()
		{
			// Assert if irq disabled
			MOS_ASSERT(test_irq(), "Disabled Interrupt");
			DisIntrGuard_t guard;
			if (cnt < 0) {
				auto tcb = waiting_list.begin();
				tcb->set_status(Status::READY);
				waiting_list.send_to_in_order(
				        tcb, ready_list, Tcb_t::pri_cmp);
			}
			cnt += 1;
			if (Task::higher_exists()) {
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
		Semaphore_t sema = 1;
		Cnt_t recr_cnt   = 0;
		TcbPtr_t owner   = nullptr;
		Prior_t ceiling  = Macro::PRI_MIN;

		MOS_INLINE inline void
		raise_all_pri()
		{
			sema.waiting_list.iter([&](Tcb_t& tcb) {
				tcb.set_pri(ceiling);
			});
		}

		MOS_INLINE inline void
		find_new_ceiling()
		{
			sema.waiting_list.iter([&](Tcb_t& tcb) {
				if (tcb.old_pr < ceiling &&
				    tcb.old_pr != Macro::PRI_NONE) {
					ceiling = tcb.old_pr;
				}
			});
		}

		void lock() // P-opr
		{
			MOS_ASSERT(test_irq(), "Disabled Interrupt");

			DisIntrGuard_t guard;

			auto cur = Task::current();

			if (owner == cur) {
				// If the current task already owns the lock, just increment the lock count
				recr_cnt += 1;
				return;
			}

			// Compare priority with ceiling
			if (ceiling < cur->get_pri()) {
				// Temporarily raise the priority
				cur->old_pr = cur->get_pri();
				cur->set_pri(ceiling);
			}
			else {
				// If current priority is higher, set it as the new ceiling
				ceiling = cur->get_pri();
				// Raise the priority of all waiting tasks
				raise_all_pri();
			}

			sema.cnt -= 1;

			if (sema.cnt < 0) {
				cur->set_status(Status::BLOCKED);
				ready_list.send_to(cur, sema.waiting_list);
				return Task::yield();
			}
			else {
				owner = cur_tcb;
				recr_cnt += 1;
			}
		}

		void unlock() // V-opr
		{
			MOS_ASSERT(test_irq(), "Disabled Interrupt");
			MOS_ASSERT(owner == cur_tcb, "Lock can only be released by holder");

			DisIntrGuard_t guard;
			recr_cnt -= 1;

			if (recr_cnt > 0) {
				// If the lock is still held by the owner
				return;
			}

			if (!sema.waiting_list.empty()) {
				// Starvation Prevention
				auto tcb = sema.waiting_list.begin();
				tcb->set_status(Status::READY);
				sema.waiting_list.send_to_in_order(
				        tcb,
				        ready_list,
				        Tcb_t::pri_cmp);

				// Transfer ownership to the last highest priority task in queue
				owner = tcb;
				sema.cnt += 1;

				find_new_ceiling();

				if (Task::higher_exists()) {
					return Task::yield();
				}
			}
			else {
				// Restore the original priority of the owner
				if (recr_cnt == 0 && owner->old_pr != Macro::PRI_NONE) {
					// Restore the original priority
					owner->set_pri(owner->old_pr);
					owner->old_pr = Macro::PRI_NONE;
				}

				// No owner if no tasks are waiting
				owner = nullptr;
				sema.cnt += 1;

				// Reset the ceiling to the lowest priority
				ceiling = Macro::PRI_MIN;

				return;
			}
		}
	};

	template <typename T = void>
	struct Mutex_t : private MutexImpl_t
	{
		using Raw_t    = T;
		using RawRef_t = Raw_t&;

		struct MutexGuard_t
		{
			// Unlock when scope ends
			MOS_INLINE inline ~MutexGuard_t() { mutex.unlock(); }
			MOS_INLINE inline MutexGuard_t(Mutex_t<T>& mutex)
			    : mutex(mutex) { mutex.MutexImpl_t::lock(); }

			// Raw Accessor
			MOS_INLINE inline RawRef_t get() { return mutex.raw; }
			MOS_INLINE inline RawRef_t operator*() { return get(); }

		private:
			Mutex_t<Raw_t>& mutex;
		};

		MOS_INLINE inline Mutex_t(T raw): raw(raw) {}

		MOS_INLINE inline MutexGuard_t
		lock() { return MutexGuard_t {*this}; }

		MOS_INLINE inline auto
		exec(auto&& section) // To safely execute
		{
			auto guard = lock(); // scope begins
			return section();    // scope ends
		}

	private:
		Raw_t raw;
	};

	template <>
	struct Mutex_t<> : private MutexImpl_t
	{
		struct MutexGuard_t // No Raw Accessor for T=void
		{
			// Unlock when scope ends
			MOS_INLINE inline ~MutexGuard_t() { mutex.unlock(); }
			MOS_INLINE inline MutexGuard_t(Mutex_t& mutex)
			    : mutex(mutex) { mutex.MutexImpl_t::lock(); }

		private:
			Mutex_t& mutex;
		};

		MOS_INLINE inline Mutex_t() = default;
		MOS_INLINE inline MutexGuard_t
		lock() { return MutexGuard_t {*this}; }

		MOS_INLINE inline auto
		exec(auto&& section) // To safely execute
		{
			auto guard = lock(); // scope begins
			return section();    // scope ends
		}
	};

	// Template deduction where T = void
	Mutex_t() -> Mutex_t<void>;

	template <typename T>
	Mutex_t(T&&) -> Mutex_t<T>;

	template <typename T>
	Mutex_t(T&) -> Mutex_t<T&>;
}

#endif