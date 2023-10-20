#ifndef _MOS_SCHEDULER_
#define _MOS_SCHEDULER_

#include "drivers/systick.hpp"
#include "config.h"
#include "task.hpp"

namespace MOS::Scheduler
{
	using namespace Task;

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

		asm("POP     {R0}");

		// Enable irq to leave critical section
		MOS_ENABLE_IRQ();

		asm("BX      LR");
	}

	__attribute__((naked, used)) extern "C" inline void
	ContextSwitch(void)
	{
		// 步骤1 - 保存当前任务的上下文
		// 中断时，处理器已经将 xPSR, PC, LR, R12, R3, R2, R1, R0 这8个寄存器压入处理器堆栈。
		// 所以这里需要压入剩下的8个寄存器 {R4-R11} 以保存当前任务的上下文。

		// 禁用中断
		MOS_DISABLE_IRQ();

		// 压入寄存器 R4-R7
		asm("PUSH    {R4-R7}");

		// 压入寄存器 R8-R11
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

		// 将堆栈指针的值（复制到R4）存储到 curTCB.sp
		asm("STR     R4, [R1,#8]");// TCB.sp偏移量 = 8

		// 保存当前任务上下文的流程到此结束

		// 步骤2：更新 curTCB，从其堆栈加载新任务上下文到 CPU 寄存器
		asm("PUSH    {R0,LR}");// 保存上下文 R0，LR 的值
		asm("BL      nextTCB");// 自定义调度函数，curTCB = next
		asm("POP     {R0,LR}");// 恢复 R0，LR 的值
		asm("LDR     R1, [R0]");

		// 将下一个任务的堆栈指针的内容加载到 curTCB，相当于将 curTCB 指向新任务的 TCB
		// 使用 R4 将新任务的 sp 加载到 SP
		asm("LDR     R4, [R1,#8]");// TCB.sp offset = 8
		asm("MOV     SP, R4");

		// 弹出寄存器 R8-R11
		asm("POP     {R4-R7}");
		asm("MOV     R8, R4");
		asm("MOV     R9, R5");
		asm("MOV     R10, R6");
		asm("MOV     R11, R7");

		// 弹出寄存器 R4-R7
		asm("POP     {R4-R7}");

		// 使能中断
		MOS_ENABLE_IRQ();

		// 从中断返回
		asm("BX      LR");
	}

	__attribute__((naked, used)) extern "C" void
	PendSV_Handler(void)
	{
		asm("B     ContextSwitch");
	}

	__attribute__((used)) extern "C" void
	SysTick_Handler(void)
	{
		// Trigger PendSV
		MOS_DISABLE_IRQ();
		MOS_TRIGGER_PENDSV_INTR();
		MOS_ENABLE_IRQ();
	}

	// Called only once
	inline void launch()
	{
		Driver::SysTick_t::config(Macro::SYSTICK);
		curTCB = (TcbPtr_t) ready_list.begin();
		curTCB->set_status(Status_t::RUNNING);
		init();
	}

	// Custom Scheduler Policy, return TCB* as the next task to run
	template <Policy policy = Policy::RoundRobin>
	inline void next_tcb()
	{
		static auto switch_to = [](TcbPtr_t tcb) {
			tcb->set_status(Status_t::RUNNING);
			curTCB = tcb;
		};

		auto idle = (TcbPtr_t) ready_list.end()->prev,
		     st   = (TcbPtr_t) ready_list.begin(),
		     ed   = (TcbPtr_t) ready_list.end();

		if (curTCB->is_status(Status_t::TERMINATED) ||
		    curTCB->is_status(Status_t::BLOCKED)) {
			// curTCB has been removed from ready_list
			return switch_to(st);
		}

		if constexpr (policy == Policy::RoundRobin) {
			if (--curTCB->ticks <= 0) {
				auto next     = curTCB->next();
				curTCB->ticks = Macro::TICKS;
				curTCB->set_status(Status_t::READY);
				return switch_to((next == ed) ? st : next);
			}
		}

		if constexpr (policy == Policy::PreemptivePriority) {
			curTCB->set_status(Status_t::READY);
			return switch_to(st);
		}
	}

	__attribute__((used, always_inline)) extern "C" inline void
	nextTCB()// Don't change this name which used in asm("")
	{
		next_tcb<Policy::MOS_CONF_SCHEDULER_POLICY>();
	}

	__attribute__((always_inline)) inline constexpr auto
	policy_name()
	{
		switch (Policy::MOS_CONF_SCHEDULER_POLICY) {
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