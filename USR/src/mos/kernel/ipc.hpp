#ifndef _MOS_IPC_
#define _MOS_IPC_

#include "data_type/queue.hpp"
#include "task.hpp"

namespace MOS::IPC
{
	using namespace Macro;
	using namespace Utils;

	using DataType::TCB_t;
	using DataType::List_t;
	using DataType::Queue_t;

	template <typename T, size_t N>
	struct MsgQueue_t
	{
		using Tick_t         = TCB_t::Tick_t;
		using Raw_t          = Queue_t<T, N>;
		using EventList_t    = List_t;
		using NodePtr_t      = List_t::NodePtr_t;
		using ConstNodePtr_t = List_t::ConstNodePtr_t;

		MOS_INLINE inline bool
		full() const { return raw.full(); }

		MOS_INLINE inline bool
		empty() const { return raw.empty(); }

		bool send(const T& msg, Tick_t timeout = 0)
		{
			if (raw.full()) {
				if (timeout == 0)
					return false;
				block_to(senders, timeout);
				if (!check_on(senders)) // Resumed
					return false;
			}

			{
				DisIntrGuard_t guard;
				raw.push(msg);
				try_wake_up(receivers);
			}

			if (Task::higher_exists()) {
				Task::yield();
			}

			return true;
		}

		bool recv(T& buf, Tick_t timeout = 0)
		{
			if (raw.empty()) {
				if (timeout == 0)
					return false;
				block_to(receivers, timeout);
				if (!check_on(receivers)) // Resumed
					return false;
			}

			{
				DisIntrGuard_t guard;
				buf = raw.serve();
				try_wake_up(senders);
			}

			if (Task::higher_exists()) {
				Task::yield();
			}

			return true;
		}

	private:
		EventList_t senders, receivers;
		Raw_t raw;

		MOS_INLINE static inline auto
		into_tcb(ConstNodePtr_t _event)
		{
			return container_of(_event, TCB_t, event);
		}

		void block_to(EventList_t& dest, Tick_t timeout)
		{
			// Pri & Delay Compare
			auto pd_cmp = [](const auto& _lhs, const auto& _rhs) {
				auto lhs = into_tcb(&_lhs),
				     rhs = into_tcb(&_rhs);
				return lhs->get_pri() < rhs->get_pri() &&
				       lhs->get_delay() < rhs->get_delay();
			};

			auto cur = Task::current();
			dest.insert_in_order(cur->event, pd_cmp);
			Task::delay(timeout);
		}

		void try_wake_up(EventList_t& src)
		{
			auto wake_up = [&](NodePtr_t event) {
				src.remove(*event);
				Task::wake_raw(into_tcb(event));
			};

			if (!src.empty()) {
				wake_up(src.begin());
			}
		}

		bool check_on(EventList_t& src)
		{
			DisIntrGuard_t guard;
			auto cur = Task::current();
			if (cur->in_event()) {
				// If still event-linked -> Timeout(Awakened by Scheduler)
				src.remove(cur->event);
				return false;
			}
			return true;
		}
	};
}

#endif