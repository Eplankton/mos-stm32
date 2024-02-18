#ifndef _MOS_SCHEDULER_
#define _MOS_SCHEDULER_

#include "../arch/cpu.hpp"
#include "task.hpp"

namespace MOS::Scheduler
{
	enum class SchedStatus : bool
	{
		OK  = true,
		ERR = !OK,
	} static sched_status = SchedStatus::ERR;

	enum class Policy : int8_t
	{
		RoundRobin,

		// In this policy, a task with a higher priority can preempt currently running task,
		// `TCB_t::pri_cmp(st, cr)` compares the priority of the new task `st` with the currently running task `cr`.
		// If `st` has higher priority, the current task `cr` will be set to `READY` status and switched to `st`.
		// If the `time slice` of the current task `cr` is exhausted (i.e., `cr->time_slice <= 0`),
		// the `time_slice` will be reset to `TIME_SLICE`, and switched to the next.
		// If there are other tasks with the same priority as `cr` that are ready
		// to run (i.e., `nx != ed && TCB_t::pri_equal(nx, cr)`),
		// the scheduler will perform `RoundRobin` scheduling among these tasks (so called a `PriGroup`).
		// Otherwise, the scheduler switches back to `st` with the highest priority.
		PreemptPri,
	};

	using namespace Macro;
	using namespace KernelGlobal;

	using DataType::TCB_t;
	using TcbPtr_t = TCB_t::TcbPtr_t;
	using Fn_t     = TCB_t::Fn_t;

	using enum TCB_t::Status;
	using enum SchedStatus;
	using enum Policy;

	MOS_INLINE inline bool
	is_ok() { return sched_status == SchedStatus::OK; }

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

	// Start scheduling
	static inline void
	launch(Fn_t hook = nullptr)
	{
		// Default idle can be replaced by user-defined hook
		auto idle = [](void* argv) {
			while (true) {
				Task::recycle();
			}
		};

		// Create idle task with hook
		Task::create(
		    hook ? hook : idle,
		    nullptr,
		    PRI_MIN,
		    "idle"
		);

		MOS_ASSERT(!ready_list.empty(), "Launch Failed!");

		cur_tcb = ready_list.begin();
		cur_tcb->set_status(RUNNING);

		debug_tcbs.mark(cur_tcb); // For debug only
		sched_status = SchedStatus::OK;

		init();
	}

	static inline void
	try_wake_up()
	{
		// sleeping_list is sorted
		auto tcb = sleeping_list.begin();
		while (tcb != sleeping_list.end()) {
			if (tcb->get_wkpt() > os_ticks)
				return;
			Task::wake_raw(tcb);
			tcb = sleeping_list.begin(); // check next
		}
	}

	template <Policy policy>
	static inline void next_tcb()
	{
		auto switch_to = [](TcbPtr_t tcb) {
			tcb->set_status(RUNNING);
			cur_tcb = tcb;
			debug_tcbs.mark(cur_tcb); // For debug only
		};

		try_wake_up();

		auto st = ready_list.begin(),
		     ed = ready_list.end(),
		     cr = Task::current(),
		     nx = cr->next();

		if (cr->is_status(TERMINATED) ||
		    cr->is_status(BLOCKED)) {
			return switch_to(st);
		}

		if constexpr (policy == RoundRobin) {
			if (cr->time_slice <= 0) {
				cr->time_slice = TIME_SLICE;
				cr->set_status(READY);
				return switch_to(nx == ed ? st : nx);
			}
		}

		if constexpr (policy == PreemptPri) {
			// If there's a task with higher priority
			if (TCB_t::pri_cmp(st, cr)) {
				cr->set_status(READY);
				return switch_to(st);
			}

			if (cr->time_slice <= 0) {
				cr->time_slice = TIME_SLICE;
				cr->set_status(READY);
				// RoundRobin under same priority
				if (nx != ed && TCB_t::pri_equal(nx, cr)) {
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
		next_tcb<Policy::MOS_CONF_SCHED_POLICY>();
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
		if (Scheduler::is_ok()) {
			return Task::nop_and_yield();
		}
	}
}

#endif