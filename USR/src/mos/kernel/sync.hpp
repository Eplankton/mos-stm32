#ifndef _MOS_SYNC_
#define _MOS_SYNC_

#include "task.hpp"

namespace MOS::Sync
{
	using namespace Macro;
	using namespace Utils;

	using DataType::TCB_t;
	using DataType::TcbList_t;
	using Concepts::Invocable;

	using TcbPtr_t = TCB_t::TcbPtr_t;
	using Prior_t  = TCB_t::Prior_t;
	using Cnt_t    = volatile int32_t;
	using Status   = TCB_t::Status;

	struct Semaphore_t
	{
		TcbList_t waiting_list;
		Cnt_t cnt;

		// Must set an original value
		MOS_INLINE
		inline Semaphore_t() = delete;

		MOS_INLINE
		inline Semaphore_t(int32_t _cnt): cnt(_cnt) {}

		// P
		MOS_NO_INLINE void
		down()
		{
			// Assert if irq disabled
			MOS_ASSERT(test_irq(), "Disabled Interrupt");
			DisIntrGuard_t guard;
			cnt -= 1;
			if (cnt < 0) {
				Task::block_to_raw(Task::current(), waiting_list);
				return Task::yield();
			}
		}

		// `V-opr`
		void up()
		{
			// Assert if irq disabled
			MOS_ASSERT(test_irq(), "Disabled Interrupt");
			DisIntrGuard_t guard;
			up_raw();
			if (Task::higher_exists()) {
				return Task::yield();
			}
		}

		// `V-opr` from ISR
		MOS_INLINE inline void
		up_from_isr() { up_raw(); }

	private:
		MOS_INLINE inline void
		up_raw()
		{
			if (cnt < 0) {
				Task::resume_raw(waiting_list.begin(), waiting_list);
			}
			cnt += 1;
		}
	};

	struct Lock_t
	{
		Semaphore_t sema;
		TcbPtr_t owner;

		MOS_INLINE
		inline Lock_t(): owner(nullptr), sema(1) {}

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
		void lock() // P operation
		{
			MOS_ASSERT(test_irq(), "Disabled Interrupt");

			DisIntrGuard_t guard;

			auto cur = Task::current();

			if (owner == cur) {
				// If task already owns the lock, just increment the recursive count
				recursive += 1;
				return;
			}

			// Compare priority with ceiling
			if (ceiling < cur->get_pri()) {
				// Temporarily raise the priority
				cur->old_pri = cur->get_pri();
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
				Task::block_to_raw(cur, sema.waiting_list);
				return Task::yield();
			}
			else {
				owner = Task::current();
				recursive += 1;
			}
		}

		void unlock() // V operation
		{
			MOS_ASSERT(test_irq(), "Disabled Interrupt");
			MOS_ASSERT(owner == Task::current(),
			           "Lock can only be released by holder");

			DisIntrGuard_t guard;
			recursive -= 1;

			if (recursive > 0) {
				// If the lock is still held by the owner
				return;
			}

			// If has waiting tasks
			if (!sema.waiting_list.empty()) {
				// Starvation Prevention by choosing first one
				auto tcb = sema.waiting_list.begin();
				Task::resume_raw(tcb, sema.waiting_list);

				// Transfer ownership to the last highest priority task in queue
				owner = tcb;
				sema.cnt += 1;

				set_new_ceiling();

				if (Task::higher_exists()) {
					return Task::yield();
				}
			}
			else {
				// Restore the original priority of the owner
				if (recursive == 0 && owner->old_pri != PRI_NONE) {
					// Restore the original priority
					owner->set_pri(owner->old_pri);
					owner->old_pri = PRI_NONE;
				}

				// No owner if no tasks are waiting
				owner = nullptr;
				sema.cnt += 1;

				// Reset the ceiling to the lowest priority
				ceiling = PRI_MIN;

				return;
			}
		}

		MOS_INLINE inline auto
		exec(auto&& section) // To protect certain routine
		{
			lock();
			section();
			unlock();
		}

	private:
		Semaphore_t sema = 1;
		Cnt_t recursive  = 0;
		TcbPtr_t owner   = nullptr;
		Prior_t ceiling  = PRI_MIN;

		MOS_INLINE inline void
		raise_all_pri()
		{
			sema.waiting_list.iter_mut([&](TCB_t& tcb) {
				tcb.set_pri(ceiling);
			});
		}

		MOS_INLINE inline void
		set_new_ceiling()
		{
			sema.waiting_list.iter([&](const TCB_t& tcb) {
				const auto old_pr = tcb.old_pri;
				if (old_pr != PRI_NONE && old_pr < ceiling) {
					ceiling = old_pr;
				}
			});
		}
	};

	template <typename T = void>
	struct Mutex_t : private MutexImpl_t
	{
		using MutexImpl_t::exec;
		using Raw_t    = T;
		using RawRef_t = Raw_t&;

		struct MutexGuard_t
		{
			// Unlock when scope ends
			MOS_INLINE
			inline ~MutexGuard_t() { mutex.unlock(); }

			MOS_INLINE
			inline MutexGuard_t(Mutex_t<Raw_t>& mutex)
			    : mutex(mutex) { mutex.MutexImpl_t::lock(); }

			// Raw Accessor
			MOS_INLINE inline RawRef_t
			get() { return mutex.raw; }

			MOS_INLINE inline RawRef_t
			operator*() { return get(); }

		private:
			Mutex_t<Raw_t>& mutex;
		};

		MOS_INLINE
		inline Mutex_t(Raw_t raw): raw(raw) {}

		MOS_INLINE inline auto
		lock() { return MutexGuard_t {*this}; }

	private:
		Raw_t raw;
	};

	template <>
	struct Mutex_t<> : private MutexImpl_t
	{
		using MutexImpl_t::exec;

		struct MutexGuard_t // No Raw Accessor for T=void
		{
			// Unlock when scope ends
			MOS_INLINE
			inline ~MutexGuard_t() { mutex.unlock(); }

			MOS_INLINE
			inline MutexGuard_t(Mutex_t& mutex)
			    : mutex(mutex) { mutex.MutexImpl_t::lock(); }

		private:
			Mutex_t& mutex;
		};

		MOS_INLINE inline Mutex_t() = default;

		MOS_INLINE inline auto
		lock() { return MutexGuard_t {*this}; }
	};

	// Template Deduction for T = void
	Mutex_t() -> Mutex_t<void>;

	template <typename T>
	Mutex_t(T&&) -> Mutex_t<T>;

	template <typename T>
	Mutex_t(T&) -> Mutex_t<T&>;

	struct CondVar_t
	{
		MOS_INLINE inline bool
		has_waiters() const
		{
			return !waiting_list.empty();
		}

		inline void
		wait(MutexImpl_t& mtx,
		     Invocable<bool> auto&& pred)
		{
			mtx.unlock();
			while (!pred()) {
				block_current();
			}
			mtx.lock();
		}

		inline void signal() // Notify one
		{
			DisIntrGuard_t guard;
			if (has_waiters()) {
				wake_up_one();
			}
			return Task::yield();
		}

		inline void broadcast() // Notify all
		{
			DisIntrGuard_t guard;
			while (has_waiters()) {
				wake_up_one();
			}
			return Task::yield();
		}

	private:
		TcbList_t waiting_list;

		MOS_INLINE inline void
		block_current()
		{
			DisIntrGuard_t guard;
			Task::block_to_raw(Task::current(), waiting_list);
			return Task::yield();
		}

		MOS_INLINE inline void
		wake_up_one()
		{
			DisIntrGuard_t guard;
			Task::resume_raw(waiting_list.begin(), waiting_list);
		}
	};

	struct Barrier_t
	{
		MutexImpl_t mtx;
		CondVar_t cond;
		Cnt_t total, cnt = 0;

		MOS_INLINE
		inline Barrier_t(Cnt_t total): total(total) {}

		inline void wait()
		{
			mtx.exec([&] {
				cnt += 1;
				cond.wait(mtx, [&] { return cnt == total; });
				if (!cond.has_waiters()) {
					cnt = 0;
				}
			});
			cond.broadcast();
		}
	};
}

#endif