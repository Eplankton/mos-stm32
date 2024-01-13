#ifndef _MOS_SCHEDULER_
#define _MOS_SCHEDULER_

#include "../arch/cpu.hpp"
#include "task.hpp"

namespace MOS::Scheduler
{
	using namespace KernelGlobal;

	using TcbPtr_t = Tcb_t::TcbPtr_t;
	using Node_t   = Tcb_t::Node_t;
	using Fn_t     = Tcb_t::Fn_t;
	using Status   = Tcb_t::Status;

	enum class Policy
	{
		RoundRobin,
		PreemptivePriority,
	};

	__attribute__((naked)) inline void
	init() // This will execute only once for the first task
	{
		asm volatile(ARCH_INIT_ASM);
	}

	extern "C" __attribute__((naked, used)) inline void
	context_switch(void) // Don't change this name which used in asm("")
	{
		asm volatile(ARCH_CONTEXT_SWITCH_ASM);
	}

	// Called only once to start scheduling
	static inline void launch(Fn_t hook = nullptr)
	{
		// Default idle can be replaced by user-defined hook
		auto idle = [](void* argv) {
			while (true) {
				asm volatile("");
			}
		};

		// Create idle task with hook
		Task::create((hook == nullptr ? idle : hook), nullptr, Macro::PRI_MIN, "idle");
		cur_tcb = (TcbPtr_t) ready_list.begin();
		cur_tcb->set_status(Status::RUNNING);
		init();
	}

	static inline void try_wake_up()
	{
		auto to_wake = (TcbPtr_t) sleep_list.begin();
		if (to_wake->delay_ticks <= os_ticks) {
			to_wake->set_delay_ticks(0);
			to_wake->set_status(Status::READY);
			sleep_list.send_to_in_order(to_wake->node, ready_list, Tcb_t::priority_cmp);
		}
	}

	template <Policy policy>
	static inline void next_tcb()
	{
		static auto switch_to = [](TcbPtr_t tcb) {
			tcb->set_status(Status::RUNNING);
			cur_tcb = tcb;
		};

		if (!sleep_list.empty()) {
			try_wake_up();
		}

		auto st = (TcbPtr_t) ready_list.begin(),
		     ed = (TcbPtr_t) ready_list.end(),
		     nx = (TcbPtr_t) cur_tcb->next();

		if (cur_tcb->is_status(Status::TERMINATED) ||
		    cur_tcb->is_status(Status::BLOCKED)) {
			// cur_tcb has been removed from ready_list
			return switch_to(st);
		}

		if constexpr (policy == Policy::RoundRobin) {
			if (cur_tcb->time_slice <= 0) {
				cur_tcb->set_status(Status::READY);
				cur_tcb->time_slice = Macro::TIME_SLICE;
				return switch_to((nx == ed) ? st : nx);
			}
		}

		if constexpr (policy == Policy::PreemptivePriority) {
			// If there's a task with higher priority
			if (Tcb_t::priority_cmp(st->node, cur_tcb->node)) {
				cur_tcb->set_status(Status::READY);
				return switch_to(st);
			}

			if (cur_tcb->time_slice <= 0) {
				cur_tcb->time_slice = Macro::TIME_SLICE;
				cur_tcb->set_status(Status::READY);
				// RoundRobin in same group
				if (nx != ed && Tcb_t::priority_equal(nx->node, cur_tcb->node)) {
					return switch_to(nx);
				}
				else {
					return switch_to(st);
				}
			}
		}
	}

	extern "C" __attribute__((used, always_inline)) inline void
	next_tcb() // Don't change this name which used in asm("")
	{
		next_tcb<Policy::MOS_CONF_POLICY>();
	}
}

namespace MOS::ISR
{
	using Util::DisIntrGuard_t;

	extern "C" __attribute__((naked)) void
	MOS_PENDSV_HANDLER()
	{
		asm volatile(ARCH_JUMP_TO_CONTEXT_SWITCH);
	}

	extern "C" void
	MOS_SYSTICK_HANDLER()
	{
		DisIntrGuard_t guard;
		Task::inc_ticks();
		if (Task::current() != nullptr) {
			Task::nop_and_yield();
		}
	}
}

#endif