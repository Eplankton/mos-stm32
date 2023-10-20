#ifndef _MOS_TASK_
#define _MOS_TASK_

#include "concepts.hpp"
#include "util.hpp"
#include "global_res.hpp"

namespace MOS::Task
{
	using namespace GlobalRes;
	using namespace Util;

	using Status_t  = TCB_t::Status_t;
	using Node_t    = TCB_t::Node_t;
	using Name_t    = TCB_t::Name_t;
	using Tid_t     = TCB_t::Tid_t;
	using TcbPtr_t  = TCB_t::TcbPtr_t;
	using PagePtr_t = TCB_t::PagePtr_t;

	__attribute__((always_inline)) inline constexpr auto&
	current_task() { return (TcbPtr_t&) curTCB; }

	__attribute__((always_inline)) inline void
	yield() { MOS_TRIGGER_SYSTICK_INTR(); }

	__attribute__((always_inline)) inline uint32_t
	num() { return ready_list.size() + blocked_list.size(); }

	inline void terminate(TcbPtr_t tcb = current_task())
	{
		if (tcb == nullptr || tcb->is_status(Status_t::TERMINATED))
			return;

		// Assert if irq disabled
		MOS_ASSERT(test_irq(), "Disabled Interrupt");

		// Disable interrupt to enter critical section
		MOS_DISABLE_IRQ();

		// Remove the task from list
		if (tcb->is_status(Status_t::RUNNING) || tcb->is_status(Status_t::READY))
			ready_list.remove(tcb->node);
		else
			blocked_list.remove(tcb->node);

		// Mark the page as unused and deinit the page
		tcb->release_page();

		// Reset the TCB to default
		tcb->deinit();

		// Enable interrupt, leave critical section
		MOS_ENABLE_IRQ();

		if (tcb == curTCB) {
			yield();
		}
	}

	__attribute__((always_inline)) inline void
	ending() { terminate(current_task()); }

	inline TcbPtr_t create(TCB_t::Fn_t&& fn, TCB_t::Argv_t argv = nullptr,
	                       TCB_t::Prior_t pr = 15, TCB_t::Name_t name = "")
	{
		if (num() >= MAX_TASK_NUM) {
			return nullptr;
		}

		// Assert if irq disabled
		MOS_ASSERT(test_irq(), "Disabled Interrupt");
		MOS_ASSERT(fn != nullptr, "fn ptr can't be null");

		// Disable interrupt to enter critical section
		MOS_DISABLE_IRQ();

		PagePtr_t p = nullptr;

		for (auto& page: pages) {
			if (!page.is_used()) {
				p = &page;
				break;
			}
		}

		MOS_ASSERT(p != nullptr, "Page error");

		// Construct a TCB at the head of a page block
		auto& tcb = *new (p->raw) TCB_t {fn, argv, pr, name};

		// Give TID
		tcb.set_tid(tids++);

		// Set task page
		tcb.attach_page(p);

		// Setup the stack to hold task context.
		// Remember it is a descending stack and a context consists of 16 registers.
		// high -> low, descending
		// | xPSR | PC | LR | R12 | R3 | R2 | R1 | R0 | R11 | R10 | R9 | R8 | R7 | R6 | R5 | R4 |
		tcb.set_SP(&p->raw[Macro::PAGE_SIZE - 16]);

		// Set the 'T' bit in stacked xPSR to '1' to notify processor on exception return about the thumb state.
		// V6-m and V7-m cores can only support thumb state so it should always be set to '1'.
		tcb.set_xPSR((uint32_t) 0x01000000);

		// Set the stacked PC to point to the task
		tcb.set_PC((uint32_t) fn);

		// Call terminate() automatically
		tcb.set_LR((uint32_t) ending);

		// Set arguments
		tcb.set_param((uint32_t) argv);

		// Set TCB to be READY
		tcb.set_status(Status_t::READY);

		// Add parent
		tcb.set_parent(curTCB);

		// Add to TCBs list
		ready_list.insert_in_order(tcb.node, &TCB_t::priority_cmp);

		// Enable interrupt, leave critical section
		MOS_ENABLE_IRQ();

		// If the new task's priority is higher, switch at once
		if (TCB_t::priority_cmp(tcb.node, curTCB->node)) {
			yield();
		}

		return &tcb;
	}

	inline void block(TcbPtr_t tcb = current_task())
	{
		if (tcb == nullptr || tcb->is_status(Status_t::BLOCKED))
			return;

		// Assert if irq disabled
		MOS_ASSERT(test_irq(), "Disabled Interrupt");
		MOS_DISABLE_IRQ();
		tcb->set_status(Status_t::BLOCKED);
		ready_list.remove(tcb->node);
		blocked_list.add(tcb->node);
		MOS_ENABLE_IRQ();
		if (tcb == curTCB) {
			// Give out the CPU
			yield();
		}
	}

	inline void resume(TcbPtr_t tcb)
	{
		if (tcb == nullptr || !tcb->is_status(Status_t::BLOCKED))
			return;

		// Assert if irq disabled
		MOS_ASSERT(test_irq(), "Disabled Interrupt");
		MOS_DISABLE_IRQ();
		blocked_list.remove(tcb->node);
		ready_list.insert_in_order(tcb->node, &TCB_t::priority_cmp);
		tcb->set_status(Status_t::READY);
		MOS_ENABLE_IRQ();
		if (curTCB != (TcbPtr_t) ready_list.begin()) {
			// if curTCB isn't the highest priority
			yield();
		}
	}

	__attribute__((always_inline)) inline void
	change_priority(TcbPtr_t tcb, TCB_t::Prior_t pr)
	{
		MOS_ASSERT(test_irq(), "Disabled Interrupt");
		MOS_DISABLE_IRQ();
		tcb->set_priority(pr);
		ready_list.remove(tcb->node);// Re-insert and sort
		ready_list.insert_in_order(tcb->node, &TCB_t::priority_cmp);
		MOS_ENABLE_IRQ();
		if (curTCB != (TcbPtr_t) ready_list.begin()) {
			// if curTCB isn't the highest priority
			yield();
		}
	}

	__attribute__((always_inline)) inline void
	for_all_tasks(auto&& fn)
	    requires Invocable<decltype(fn), const Node_t&>
	{
		ready_list.iter(fn);
		blocked_list.iter(fn);
	}

	__attribute__((always_inline)) inline TcbPtr_t
	find(auto info)
	{
		TcbPtr_t res = nullptr;

		auto fetch = [info, &res](const Node_t& node) {
			auto& tcb = (TCB_t&) node;

			if constexpr (Same<decltype(info), Tid_t>) {
				if (tcb.get_tid() == info) {
					res = &tcb;
					return;
				}
			}

			if constexpr (Same<decltype(info), Name_t>) {
				if (tcb.get_name() == info) {
					res = &tcb;
					return;
				}
			}
		};

		MOS_DISABLE_IRQ();
		for_all_tasks(fetch);
		MOS_ENABLE_IRQ();
		return res;
	}

	inline void print_name()
	{
		MOS_DISABLE_IRQ();
		MOS_MSG("%s\n", curTCB->get_name());
		MOS_ENABLE_IRQ();
	}

	inline void print_all_tasks()
	{
		// Status to String
		static auto stos = [](const Status_t s) constexpr {
			switch (s) {
				case Status_t::READY:
					return "READY";
				case Status_t::RUNNING:
					return "RUNNING";
				case Status_t::BLOCKED:
					return "BLOCKED";
				case Status_t::TERMINATED:
					return "TERMINATED";
				default:
					return "INVALID";
			}
		};

		// Print to screen
		static auto prts = [](const Node_t& node) {
			auto& tcb = (TCB_t&) node;
			MOS_MSG("#%-2d %-10s %-5d %-9s %2d%%\n",
			        tcb.get_tid(),
			        tcb.get_name(),
			        tcb.get_priority(),
			        stos(tcb.get_status()),
			        tcb.page_usage());
		};

		MOS_DISABLE_IRQ();
		MOS_MSG("=====================================\n");
		for_all_tasks(prts);
		MOS_MSG("=====================================\n");
		MOS_ENABLE_IRQ();
	}

	__attribute__((always_inline)) inline void
	delay_ms(const uint32_t n, const uint32_t unit = 1000)
	{
		Util::delay(n, unit);
	}
}

#endif