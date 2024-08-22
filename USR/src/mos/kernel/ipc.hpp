#ifndef _MOS_IPC_
#define _MOS_IPC_

#include "data_type/queue.hpp"
#include "task.hpp"

namespace MOS::Kernel::IPC
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
		using RawQueue_t     = Queue_t<T, N>;
		using EventList_t    = List_t;
		using NodePtr_t      = List_t::NodePtr_t;
		using ConstNodePtr_t = List_t::ConstNodePtr_t;

		MOS_INLINE inline bool
		full() const { return queue.full(); }

		MOS_INLINE inline bool
		empty() const { return queue.empty(); }

		enum Status : bool
		{
			Ok      = true,
			TimeOut = !Ok,
		};

		auto send(const T& msg, Tick_t timeout = 0)
		{
			if (queue.full() &&
			    wait_on(senders, timeout) == TimeOut) {
				return TimeOut;
			}

			{
				IntrGuard_t guard;
				queue.push(msg);
			}

			try_wake_up(receivers);
			return Ok;
		}

		auto recv(Tick_t timeout = 0)
		{
			RecvMsg_t res;

			if (queue.empty() &&
			    wait_on(receivers, timeout) == TimeOut) {
				return res;
			}

			{
				IntrGuard_t guard;
				res.status = Ok;
				res.msg    = queue.serve();
			}

			try_wake_up(senders);
			return res;
		}

	private:
		EventList_t senders, receivers;
		RawQueue_t queue;

		struct RecvMsg_t
		{
			Status status = TimeOut;
			T msg {};

			MOS_INLINE inline void
			map_or(auto&& fn, auto&& oops)
			{
				if (status == Ok)
					fn(msg);
				else
					oops();
			}
		};

		MOS_INLINE static constexpr inline auto
		into_tcb(ConstNodePtr_t event)
		{
			return container_of(event, TCB_t, event);
		}

		static Status
		check_for(EventList_t& src)
		{
			IntrGuard_t guard;
			auto cur = Task::current();
			if (cur->in_event()) {
				// Still in event-linked -> Timeout
				src.remove(cur->event);
				return TimeOut;
			}
			return Ok;
		}

		static Status
		wait_on(EventList_t& dest, Tick_t timeout)
		{
			// Priority & WakePoint Compare
			auto pri_wkpt_cmp =
			    [](const auto& _lhs, const auto& _rhs) {
				    auto lhs = into_tcb(&_lhs),
				         rhs = into_tcb(&_rhs);

				    // Compare pri first, then wkpt
				    return TCB_t::pri_cmp(lhs, rhs) &&
				           TCB_t::wkpt_cmp(lhs, rhs);
			    };

			if (timeout == 0)
				return TimeOut;

			{
				IntrGuard_t guard;
				dest.insert_in_order(
				    Task::current()->event,
				    pri_wkpt_cmp
				);
			}

			Task::delay(timeout); // Sleep until timeout

			// After being awakened, check for timeout
			return check_for(dest);
		}

		static void
		try_wake_up(EventList_t& src)
		{
			auto wake_up = [&](NodePtr_t event) {
				Task::wake_raw(into_tcb(event));
				src.remove(*event);
			};

			IntrGuard_t guard;
			if (!src.empty()) {
				wake_up(src.begin());
				if (Task::any_higher()) {
					return Task::yield();
				}
			}
		}
	};
}

#endif