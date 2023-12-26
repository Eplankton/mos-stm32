#ifndef _MOS_TASK_
#define _MOS_TASK_

#include "concepts.hpp"
#include "util.hpp"
#include "global.hpp"
#include "alloc.hpp"

namespace MOS::Task
{
	using namespace Util;
	using namespace KernelGlobal;

	using TCB_t     = DataType::TCB_t;
	using List_t    = DataType::List_t;
	using Fn_t      = TCB_t::Fn_t;
	using Argv_t    = TCB_t::Argv_t;
	using Prior_t   = TCB_t::Prior_t;
	using Node_t    = TCB_t::Node_t;
	using Status_t  = TCB_t::Status_t;
	using Name_t    = TCB_t::Name_t;
	using Tid_t     = TCB_t::Tid_t;
	using TcbPtr_t  = TCB_t::TcbPtr_t;
	using PagePtr_t = TCB_t::PagePtr_t;

	__attribute__((always_inline)) inline auto
	current_task() { return curTCB; }

	__attribute__((always_inline)) inline void
	yield() { MOS_TRIGGER_PENDSV_INTR(); }

	__attribute__((always_inline)) inline void
	nop_and_yield()
	{
		curTCB->time_slice -= 1;
		yield();
	}

	__attribute__((always_inline)) inline void
	inc_ticks() { os_ticks += 1; }

	__attribute__((always_inline)) inline auto
	inc_tids()
	{
		tids += 1;
		return tids;
	}

	__attribute__((always_inline)) inline uint32_t
	num() { return debug_tcbs.size(); }

	inline void terminate(TcbPtr_t tcb = current_task())
	{
		if (tcb == nullptr || tcb->is_status(Status_t::TERMINATED))
			return;

		// Assert if irq disabled
		MOS_ASSERT(test_irq(), "Disabled Interrupt");

		// Disable interrupt to enter critical section
		{
			DisIntrGuard guard;

			// Remove the task from list
			if (tcb->is_status(Status_t::RUNNING) || tcb->is_status(Status_t::READY)) {
				ready_list.remove(tcb->node);
			}
			else if (tcb->delay_ticks == 0) {
				blocked_list.remove(tcb->node);
			}
			else {
				sleep_list.remove(tcb->node);
			}

			// Mark the page as unused and deinit the page
			tcb->release_page();

			// Reset the TCB to default
			tcb->deinit();

			// For debug only
			debug_tcbs.remove(tcb);
		}
		// Enable interrupt, leave critical section

		if (tcb == current_task()) {
			yield();
		}
	}

	__attribute__((always_inline)) static inline void
	exit() { terminate(current_task()); }

	__attribute__((always_inline)) static inline TcbPtr_t
	load_context(TcbPtr_t tcb)
	{
		// A descending stack consists of 16 registers as context.
		// high -> low, descending stack
		// | xPSR | PC | LR | R12 | R3 | R2 | R1 | R0 | R11 | R10 | R9 | R8 | R7 | R6 | R5 | R4 |
		tcb->set_SP(&tcb->page->raw[Macro::PAGE_SIZE - 16]);

		// Set the 'T' bit in stacked xPSR to '1' to notify processor on exception return about the Thumb state.
		// V6-m and V7-m cores can only support Thumb state so it should always be set to '1'.
		tcb->set_xPSR((uint32_t) 0x01000000);

		// Set the stacked PC to point to the task
		tcb->set_PC((uint32_t) tcb->fn);

		// Call terminate(current_task()) automatically
		tcb->set_LR((uint32_t) exit);

		// Set arguments
		tcb->set_param((uint32_t) tcb->argv);

		return tcb;
	}

	inline TcbPtr_t create(Fn_t fn, Argv_t argv = nullptr, Prior_t pr = 15, Name_t name = "")
	{
		if (num() >= Macro::MAX_TASK_NUM) {
			return nullptr;
		}

		// Assert if irq disabled
		MOS_ASSERT(test_irq(), "Disabled Interrupt");
		MOS_ASSERT(fn != nullptr, "fn ptr can't be null");

		TcbPtr_t tcb = nullptr, cur = current_task();

		// Disable interrupt to enter critical section
		{
			DisIntrGuard guard;

			// Page Alloc
			auto page = Alloc::palloc();

			// Construct a TCB at the head of a page block
			tcb = TCB_t::build(page, fn, argv, pr, name);

			// Load empty context
			load_context(tcb);

			// Give TID
			tcb->set_tid(inc_tids());

			// Set parent
			tcb->set_parent(cur);

			// Set TCB to be READY
			tcb->set_status(Status_t::READY);

			// Add to TCBs list
			ready_list.insert_in_order(tcb->node, TCB_t::priority_cmp);

			// For debug only
			debug_tcbs.add(tcb);
		}
		// Enable interrupt, leave critical section

		// If the new task's priority is higher, switch at once
		if (TCB_t::priority_cmp(tcb->node, cur->node)) {
			yield();
		}

		return tcb;
	}

	// Experimental
	__attribute__((always_inline)) inline TcbPtr_t
	create(Fn_t fn, auto& argv = nullptr, Prior_t pr = 15, Name_t name = "")
	{
		return create(fn, (Argv_t) &argv, pr, name);
	}

	inline void block_to(TcbPtr_t tcb, List_t& dest)
	{
		if (tcb == nullptr || tcb->is_status(Status_t::BLOCKED))
			return;

		// Assert if irq disabled
		MOS_ASSERT(test_irq(), "Disabled Interrupt");

		DisIntrGuard guard;
		tcb->set_status(Status_t::BLOCKED);
		ready_list.send_to(tcb->node, dest);

		if (tcb == current_task()) {
			return yield();
		}
	}

	__attribute__((always_inline)) inline void
	block(TcbPtr_t tcb = current_task())
	{
		block_to(tcb, blocked_list);
	}

	inline void resume(TcbPtr_t tcb)
	{
		if (tcb == nullptr || !tcb->is_status(Status_t::BLOCKED))
			return;

		// Assert if irq disabled
		MOS_ASSERT(test_irq(), "Disabled Interrupt");

		DisIntrGuard guard;
		tcb->set_status(Status_t::READY);
		blocked_list.send_to_in_order(tcb->node, ready_list, TCB_t::priority_cmp);

		if (current_task() != (TcbPtr_t) ready_list.begin()) {
			// if curTCB isn't the highest priority
			return yield();
		}
	}

	__attribute__((always_inline)) inline void
	change_priority(TcbPtr_t tcb, TCB_t::Prior_t pr)
	{
		MOS_ASSERT(test_irq(), "Disabled Interrupt");

		DisIntrGuard guard;
		tcb->set_priority(pr);
		ready_list.re_insert(tcb->node, TCB_t::priority_cmp);

		if (current_task() != (TcbPtr_t) ready_list.begin()) {
			// if curTCB isn't the highest priority
			return yield();
		}
	}

	// For debug only
	__attribute__((always_inline)) inline void
	for_all_tasks(auto&& fn)
	    requires Invocable<decltype(fn), TcbPtr_t>
	{
		debug_tcbs.iter(fn);
	}

	__attribute__((always_inline)) inline TcbPtr_t
	find(auto info)
	{
		DisIntrGuard guard;

		auto fetch = [info](TcbPtr_t tcb) {
			if constexpr (Same<decltype(info), Tid_t>) {
				return tcb->get_tid() == info;
			}

			if constexpr (Same<decltype(info), Name_t>) {
				return strcmp(tcb->get_name(), info) == 0;
			}
		};

		return debug_tcbs.iter_until(fetch);
	}

	inline void print_name()
	{
		DisIntrGuard guard;
		MOS_MSG("%s\n", current_task()->get_name());
	}

	__attribute__((always_inline)) inline constexpr auto
	status_name(const Status_t status)
	{
		switch (status) {
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

	inline void
	print_info(const Node_t& node,
	           const char* format = "#%-2d %-10s %-5d %-9s %3d%%\n")
	{
		auto& tcb = (TCB_t&) node;
		MOS_MSG(format,
		        tcb.get_tid(),
		        tcb.get_name(),
		        tcb.get_priority(),
		        status_name(tcb.get_status()),
		        tcb.stack_usage());
	};

	// For debug only
	inline void print_all()
	{
		DisIntrGuard guard;
		MOS_MSG("------------------------------------\n");
		debug_tcbs.iter([](TcbPtr_t tcb) {
			print_info(tcb->node);
		});
		MOS_MSG("------------------------------------\n");
	}

	__attribute__((always_inline)) inline void
	delay(const uint32_t ticks)
	{
		auto cur = current_task();
		cur->set_delay_ticks(os_ticks + ticks);
		block_to(cur, sleep_list);
	}
}

#endif