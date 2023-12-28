#ifndef _MOS_SCHEDULER_
#define _MOS_SCHEDULER_

#include "../arch/cpu.hpp"
#include "task.hpp"

namespace MOS::Scheduler
{
	using namespace KernelGlobal;

	using Status_t = TCB_t::Status_t;
	using TcbPtr_t = TCB_t::TcbPtr_t;
	using Node_t   = TCB_t::Node_t;
	using Fn_t     = TCB_t::Fn_t;

	enum class Policy
	{
		RoundRobin,
		PreemptivePriority,
	};

	// Execute only once
	__attribute__((naked)) inline void init()
	{
		asm volatile(ARCH_INIT_ASM);
	}

	extern "C" __attribute__((naked, used)) inline void
	ContextSwitch(void)
	{
		asm volatile(ARCH_CONTEXT_SWITCH_ASM);
	}

	// Called only once
	static inline void launch(Fn_t hook = nullptr)
	{
		// Default idle can be replaced by hook
		auto idle = [](void* argv) {
			while (true) {
				asm volatile("");
			}
		};

		// Create idle task with hook
		Task::create(hook == nullptr ? idle : hook, nullptr, Macro::PRI_MIN, "idle");
		curTCB = (TcbPtr_t) ready_list.begin();
		curTCB->set_status(Status_t::RUNNING);
		init();
	}

	static inline void try_wake_up()
	{
		auto fetch = [](const Node_t& node) {
			return ((const TCB_t&) node).delay_ticks <= os_ticks;
		};

		if (auto to_wake = (TcbPtr_t) sleep_list.iter_until(fetch)) {
			to_wake->set_delay_ticks(0);
			to_wake->set_status(Status_t::READY);
			sleep_list.send_to_in_order(to_wake->node, ready_list, TCB_t::priority_cmp);
		}
	}

	// Custom Scheduler Policy
	template <Policy policy>
	static inline void next_tcb()
	{
		if (!sleep_list.empty())
			try_wake_up();

		static auto switch_to = [](TcbPtr_t tcb) {
			tcb->set_status(Status_t::RUNNING);
			curTCB = tcb;
		};

		auto st = (TcbPtr_t) ready_list.begin(),
		     ed = (TcbPtr_t) ready_list.end(),
		     nx = (TcbPtr_t) curTCB->next();

		if (curTCB->is_status(Status_t::TERMINATED) ||
		    curTCB->is_status(Status_t::BLOCKED)) {
			// curTCB has been removed from ready_list
			return switch_to(st);
		}

		if constexpr (policy == Policy::RoundRobin) {
			if (curTCB->time_slice <= 0) {
				curTCB->set_status(Status_t::READY);
				curTCB->time_slice = Macro::TIME_SLICE;
				return switch_to((nx == ed) ? st : nx);
			}
		}

		if constexpr (policy == Policy::PreemptivePriority) {
			// If there's a task with higher priority
			if (TCB_t::priority_cmp(st->node, curTCB->node)) {
				curTCB->set_status(Status_t::READY);
				return switch_to(st);
			}

			if (curTCB->time_slice <= 0) {
				curTCB->set_status(Status_t::READY);
				curTCB->time_slice = Macro::TIME_SLICE;
				// RoundRobin in same group
				if (nx != ed && TCB_t::priority_equal(nx->node, curTCB->node)) {
					return switch_to(nx);
				}
				else {
					return switch_to(st);
				}
			}
		}
	}

	// Don't change this name which used in asm("")
	extern "C" __attribute__((used)) MOS_INLINE inline void
	nextTCB()
	{
		next_tcb<Policy::MOS_CONF_POLICY>();
	}
}

namespace MOS::ISR
{
	using Util::DisIntrGuard;

	extern "C" __attribute__((naked)) void
	PendSV_Handler()
	{
		asm volatile(ARCH_JUMP_TO_CONTEXT_SWITCH);
	}

	extern "C" void SysTick_Handler()
	{
		DisIntrGuard guard;
		Task::inc_ticks();
		if (Task::current_task() != nullptr) {
			Task::nop_and_yield();
		}
	}
}

#endif