#ifndef _MOS_IPC_
#define _MOS_IPC_

#include "data_type/queue.hpp"
#include "task.hpp"
#include "global.hpp"

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
		using EventList_t = List_t;
		using Tick_t      = TCB_t::Tick_t;
		using TcbPtr_t    = TCB_t::TcbPtr_t;
		using Raw_t       = Queue_t<T, N>;

		bool send(const T& msg, Tick_t timeout = 0)
		{
			if (raw.full()) {
				if (timeout == 0)
					return false;
				block_to(senders, timeout);
				DisIntrGuard_t guard; // Resumed
				auto cur = Task::current();
				if (cur->in_event()) {
					senders.remove(cur->event);
					return false;
				}
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
				DisIntrGuard_t guard; // Resumed
				auto cur = Task::current();
				if (cur->in_event()) {
					receivers.remove(cur->event);
					return false;
				}
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

		void block_to(EventList_t& dest, Tick_t timeout)
		{
			// Pri & Delay Compare
			auto pd_cmp = [](const auto& _lhs, const auto& _rhs) {
				auto lhs = (const TCB_t&) _lhs,
				     rhs = (const TCB_t&) _rhs;
				return lhs.get_pri() < rhs.get_pri() &&
				       lhs.get_delay() < rhs.get_delay();
			};

			auto cur = Task::current();
			dest.insert_in_order(cur->event, pd_cmp);
			Task::delay(timeout);
		}

		void try_wake_up(EventList_t& src)
		{
			auto wake_opr = [&](TcbPtr_t tcb) {
				src.remove(tcb->event);
				Task::wake_raw(tcb);
			};

			if (!src.empty()) {
				auto tcb = container_of(src.begin(), TCB_t, event);
				wake_opr(tcb);
			}
		}
	};
}

#endif