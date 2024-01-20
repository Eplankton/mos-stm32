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

	enum class SchedStatus
	{
		READY,
		ERROR,
	} static os_status = SchedStatus::ERROR;

	MOS_INLINE inline bool
	is_ready() { return os_status == SchedStatus::READY; }

	// This will execute only once for the first task
	__attribute__((naked)) inline void
	init()
	{
		asm volatile(ARCH_INIT_ASM);
	}

	// Don't change this name which used in asm("")
	extern "C" __attribute__((naked, used)) inline void
	context_switch(void)
	{
		asm volatile(ARCH_CONTEXT_SWITCH_ASM);
	}

	// Called only once to start scheduling
	static inline void
	launch(Fn_t hook = nullptr)
	{
		// Default idle can be replaced by user-defined hook
		auto idle = [](void* argv) {
			while (true) {
				asm volatile("");
			}
		};

		// Create idle task with hook
		Task::create(
		        hook == nullptr ? idle : hook,
		        nullptr,
		        Macro::PRI_MIN,
		        "idle");

		cur_tcb = ready_list.begin();
		cur_tcb->set_status(Status::RUNNING);
		os_status = SchedStatus::READY;
		init();
	}

	static inline void try_wake_up()
	{
		// Only have to check the first one since they're sorted by delay_ticks
		auto to_wake = sleep_list.begin();
		if (to_wake->delay_ticks <= os_ticks) {
			to_wake->set_delay(0);
			to_wake->set_status(Status::READY);
			sleep_list.send_to_in_order(
			        to_wake,
			        ready_list,
			        Tcb_t::pri_cmp);
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

		auto st = ready_list.begin(),
		     ed = ready_list.end(),
		     cr = Task::current(),
		     nx = cr->next();

		if (cr->is_status(Status::TERMINATED) ||
		    cr->is_status(Status::BLOCKED))
		{
			// cur_tcb has been removed from ready_list
			return switch_to(st);
		}

		if constexpr (policy == Policy::RoundRobin) {
			if (cr->time_slice <= 0) {
				cr->time_slice = Macro::TIME_SLICE;
				cr->set_status(Status::READY);
				return switch_to((nx == ed) ? st : nx);
			}
		}

		if constexpr (policy == Policy::PreemptivePriority) {
			// If there's a task with higher priority
			if (Tcb_t::pri_cmp(st, cr)) {
				cr->set_status(Status::READY);
				return switch_to(st);
			}

			if (cr->time_slice <= 0) {
				cr->time_slice = Macro::TIME_SLICE;
				cr->set_status(Status::READY);
				// RoundRobin under same priority
				if (nx != ed && Tcb_t::pri_equal(nx, cr)) {
					return switch_to(nx);
				}
				else {
					return switch_to(st);
				}
			}
		}
	}

	// Don't change this function name used in asm("")
	extern "C" __attribute__((used, always_inline)) inline void
	next_tcb()
	{
		next_tcb<Policy::MOS_CONF_POLICY>();
	}
}

namespace MOS::ISR
{
	using Utils::DisIntrGuard_t;

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
		if (Scheduler::is_ready()) {
			Task::nop_and_yield();
		}
	}
}

#endif