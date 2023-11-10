#ifndef _MOS_SCHEDULER_
#define _MOS_SCHEDULER_

#include "config.h"
#include "task.hpp"

namespace MOS::Scheduler
{
	using namespace Task;
	using namespace KernelGlobal;
	using Status_t = TCB_t::Status_t;

	enum class Policy
	{
		RoundRobin,
		PreemptivePriority,
	};

	__attribute__((naked)) inline void init()
	{
		// Disable irq to enter critical section
		MOS_DISABLE_IRQ();

		// R0 contains the address of curTCB
		asm("LDR     R0, =curTCB");

		// R2 contains the address in curTCB
		asm("LDR     R2, [R0]");

		// Load the SP reg with the stacked SP value
		asm("LDR     R4, [R2,#8]");// sp offest = 8
		asm("MOV     SP, R4");

		// Pop registers R8-R11(user saved context)
		asm("POP     {R4-R7}");
		asm("MOV     R8, R4");
		asm("MOV     R9, R5");
		asm("MOV     R10, R6");
		asm("MOV     R11, R7");

		// Pop registers R4-R7(user saved context)
		asm("POP     {R4-R7}");

		// Start poping the stacked exception frame.
		asm("POP     {R0-R3}");
		asm("POP     {R4}");
		asm("MOV     R12, R4");

		// Skip the saved LR
		asm("ADD     SP,SP,#4");

		// POP the saved PC into LR via R4, We do this to jump into the first task
		// when we execute the branch instruction to exit this routine.
		asm("POP     {R4}");
		asm("MOV     LR, R4");
		asm("ADD     SP,SP,#4");

		// Enable irq to leave critical section
		MOS_ENABLE_IRQ();

		asm("BX      LR");
	}

	__attribute__((naked, used)) extern "C" inline void
	ContextSwitch(void)
	{
		// Step 1 - Save the context of the current task
		// Processor has already pushed the xPSR, PC, LR, R12, R3, R2, R1, R0 registers to the processor stack.
		// Therefore, we need to push the remaining 8 registers {R4-R11} to save the context of the current task.

		// Disable interrupts
		MOS_DISABLE_IRQ();

		// Push registers R4-R7
		asm("PUSH    {R4-R7}");

		// Push registers R8-R11
		asm("MOV     R4, R8");
		asm("MOV     R5, R9");
		asm("MOV     R6, R10");
		asm("MOV     R7, R11");
		asm("PUSH    {R4-R7}");

		// R0 = &curTCB
		asm("LDR     R0, =curTCB");

		// R1 = *R0 = curTCB
		asm("LDR     R1, [R0]");

		// R4 = SP
		asm("MOV     R4, SP");

		// Store the value of the stack pointer (copied to R4) to curTCB.sp
		asm("STR     R4, [R1,#8]");// TCB.sp offset = 8

		// The process of saving the context of the current task ends here

		// Step 2: Update curTCB, load the new task context from its stack to the CPU registers
		asm("PUSH    {R0,LR}");// Save the context values of R0, LR
		asm("BL      nextTCB");// Custom scheduling function, curTCB = next
		asm("POP     {R0,LR}");// Restore the values of R0, LR
		asm("LDR     R1, [R0]");

		// Load the content of the next task's stack pointer
		// Use R4 to load the new task's sp to SP
		asm("LDR     R4, [R1,#8]");// TCB.sp offset = 8
		asm("MOV     SP, R4");

		// Pop registers R8-R11
		asm("POP     {R4-R7}");
		asm("MOV     R8, R4");
		asm("MOV     R9, R5");
		asm("MOV     R10, R6");
		asm("MOV     R11, R7");

		// Pop registers R4-R7
		asm("POP     {R4-R7}");

		// Enable interrupts
		MOS_ENABLE_IRQ();

		// Return from interrupt
		asm("BX      LR");
	}

	// Called only once
	static inline void launch()
	{
		curTCB = (TcbPtr_t) ready_list.begin();
		curTCB->set_status(Status_t::RUNNING);
		init();
	}

	static inline void wake_up()
	{
		TcbPtr_t to_wake = nullptr;

		blocked_list.iter([&to_wake](const Node_t& node) {
			auto& tcb = (TCB_t&) node;
			if (tcb.delay_ticks == os_ticks) {
				tcb.delay_ticks = 0;
				to_wake         = &tcb;
			}
		});

		if (to_wake != nullptr) {
			to_wake->set_status(Status_t::READY);
			blocked_list.send_to_in_order(to_wake->node, ready_list, TCB_t::priority_cmp);
		}
	}

	// Custom Scheduler Policy
	template <Policy policy>
	static inline void next_tcb()
	{
		wake_up();

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
			if (--curTCB->time_slice <= 0) {
				curTCB->set_status(Status_t::READY);
				curTCB->time_slice = Macro::TIME_SLICE;
				return switch_to((nx == ed) ? st : nx);
			}
		}

		if constexpr (policy == Policy::PreemptivePriority) {
			if (TCB_t::priority_cmp(st->node, curTCB->node)) {
				curTCB->set_status(Status_t::READY);
				return switch_to(st);
			}

			if (--curTCB->time_slice <= 0) {
				curTCB->set_status(Status_t::READY);
				curTCB->time_slice = Macro::TIME_SLICE;
				if (nx != ed && TCB_t::priority_equal(nx->node, curTCB->node)) {
					return switch_to(nx);
				}
				else {
					return switch_to(st);
				}
			}
		}
	}

	__attribute__((used, always_inline)) extern "C" inline void
	nextTCB()// Don't change this name which used in asm("")
	{
		next_tcb<Policy::MOS_CONF_POLICY>();
	}

	__attribute__((always_inline)) inline constexpr auto
	policy_name()
	{
		switch (Policy::MOS_CONF_POLICY) {
			case Policy::RoundRobin:
				return "RoundRobin";
			case Policy::PreemptivePriority:
				return "PreemptivePriority";
			default:
				return "INVALID";
		}
	}
}

#endif