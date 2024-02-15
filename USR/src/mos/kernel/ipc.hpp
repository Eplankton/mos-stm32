#ifndef _MOS_IPC_
#define _MOS_IPC_

#include "data_type/queue.hpp"
#include "task.hpp"
#include "global.hpp"

namespace MOS::IPC
{
	using namespace Macro;
	using namespace Utils;
	using namespace DataType;
	using KernelGlobal::os_ticks;

	template <typename T, size_t N>
	struct MsgQueue_t
	{
		using Tick_t   = TCB_t::Tick_t;
		using TcbPtr_t = TCB_t::TcbPtr_t;
		using Raw_t    = Queue_t<T, N>;

		TcbList_t senders, receivers;
		Raw_t raw;

		void block_to(TcbList_t& dest, Tick_t timeout)
		{
			static auto pri_delay_cmp = [](TcbPtr_t lhs, TcbPtr_t rhs) {
				return lhs->get_pri() < rhs->get_pri() &&
				       lhs->get_delay() < rhs->get_delay();
			};

			if (timeout != 0) {
				DisIntrGuard_t guard;
				auto cur = Task::current();
				cur->set_delay(os_ticks + timeout);
				Task::block_to_in_order_raw(cur, dest, pri_delay_cmp);
			}
			Task::yield();
		}

		void wake_up(TcbList_t& src)
		{
			auto wake_opr = [&](TcbPtr_t tcb) {
				if (tcb->get_delay() > os_ticks)
					tcb->set_delay(0);
				Task::resume_raw(tcb, src);
			};

			if (!src.empty()) {
				wake_opr(src.begin());
				auto it = src.begin();
				while (it != src.end()) {
					auto nx = it->next();
					if (it->get_delay() <= os_ticks) {
						wake_opr(it);
					}
					it = nx;
				}
			}
		}

		MOS_INLINE static inline bool
		is_timeout()
		{
			return Task::current()->get_delay() != 0;
		}

		bool send(const T& msg, Tick_t timeout)
		{
			if (raw.full()) {
				block_to(senders, timeout);
				if (is_timeout())
					return false;
			}

			{
				DisIntrGuard_t guard;
				raw.push(msg);
				wake_up(receivers);
			}

			if (Task::higher_exists()) {
				Task::yield();
			}

			return true;
		}

		bool recv(T& buf, Tick_t timeout)
		{
			if (raw.empty()) {
				block_to(receivers, timeout);
				if (is_timeout())
					return false;
			}

			{
				DisIntrGuard_t guard;
				buf = raw.serve();
				wake_up(senders);
			}

			if (Task::higher_exists()) {
				Task::yield();
			}

			return true;
		}
	};
}

#endif