#ifndef _MOS_SCHEDULER_
#define _MOS_SCHEDULER_

#include "../arch/cpu.hpp"
#include "task.hpp"

namespace MOS::Kernel::Scheduler
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

	using namespace Global;

	using DataType::TCB_t;
	using TcbPtr_t = TCB_t::TcbPtr_t;
	using Fn_t     = TCB_t::Fn_t;

	using enum TCB_t::Status;
	using enum SchedStatus;
	using enum Policy;

	MOS_INLINE inline bool
	is_ready() { return sched_status == SchedStatus::OK; }

	// This will execute only once for the first task
	MOS_NAKED inline void
	init() { MOS_TRIGGER_SVC_INTR(); }

	// Don't change this name which used in asm("")
	extern "C" MOS_USED MOS_NAKED inline void
	context_switch(void)
	{
		asm volatile(ARCH_CONTEXT_SWITCH_ASM);

		// asm volatile("CPSID   I"); /* Disable interrupts */

		// asm volatile("MRS     R0, PSP");
		// asm volatile("LDR     R3, =cur_tcb");
		// asm volatile("LDR     R2, [R3]");

		// // asm volatile("tst r14, #0x10                  \n" /* Is using FPU context? */
		// //              "it eq                           \n"
		// //              "vstmdbeq r0!, {s16-s31}         \n");

		// asm volatile("STMDB   R0!, {R4-R11}"); /* Save core registers. */
		// asm volatile("STR     R0, [R2,#8]");   /* Get tcb.sp */

		// asm volatile("STMDB   SP!, {R3,LR}");
		// asm volatile("BL      next_tcb");
		// asm volatile("ldmia   SP!, {R3,LR}");

		// asm volatile("LDR     R1, [R3]");
		// asm volatile("LDR     R0, [R1,#8]");   /* Get tcb.sp */
		// asm volatile("LDMIA   R0!, {R4-R11}"); /* Pop core registers. */

		// // asm volatile("tst r14, #0x10         \n" /* Is using FPU context? */
		// //              "it eq                  \n"
		// //              "vldmiaeq r0!, {s16-s31}");

		// asm volatile("MSR     PSP, R0");

		// asm volatile("CPSIE   I"); /* Enable interrupts */
		// asm volatile("BX      LR");
	}

	// Start scheduling
	static inline void
	launch(Fn_t hook = nullptr)
	{
		static uint32_t idle_page_block[PAGE_SIZE / 2];

		Page_t idle_page {
		    .policy = Page_t::Policy::STATIC,
		    .raw    = idle_page_block,
		    .size   = sizeof(idle_page_block) / sizeof(uint32_t),
		};

		// Default idle can be replaced by user-defined hook
		auto idle = [](void* argv) {
			while (true) {
				Task::recycle();
			}
		};

		// Create idle task with hook
		Task::create(
		    hook ? hook : idle,
		    nullptr, PRI_MIN,
		    "idle", idle_page
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
	extern "C" MOS_USED MOS_INLINE inline void
	next_tcb()
	{
		next_tcb<Policy::MOS_CONF_SCHED_POLICY>();
	}
}

namespace MOS::ISR
{
	extern "C" {
		MOS_NAKED void
		MOS_SVC_HANDLER()
		{
			asm volatile(ARCH_INIT_ASM);
		};

		MOS_NAKED void
		MOS_PENDSV_HANDLER()
		{
			asm volatile(ARCH_JUMP_TO_CONTEXT_SWITCH);
		}

		void MOS_SYSTICK_HANDLER()
		{
			using namespace Kernel;
			using Utils::IntrGuard_t;

			IntrGuard_t guard;
			Task::inc_ticks();
			if (Scheduler::is_ready()) {
				Task::dec_tmslc();
				return Task::yield();
			}
		}
	}
}

#endif