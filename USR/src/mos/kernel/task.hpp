#ifndef _MOS_TASK_
#define _MOS_TASK_

#include "concepts.hpp"
#include "utils.hpp"
#include "global.hpp"
#include "alloc.hpp"

namespace MOS::Task
{
	using namespace Util;
	using namespace KernelGlobal;

	using Tcb_t     = DataType::Tcb_t;
	using List_t    = DataType::List_t;
	using Fn_t      = Tcb_t::Fn_t;
	using Argv_t    = Tcb_t::Argv_t;
	using Prior_t   = Tcb_t::Prior_t;
	using Node_t    = Tcb_t::Node_t;
	using Name_t    = Tcb_t::Name_t;
	using Tid_t     = Tcb_t::Tid_t;
	using Tick_t    = Tcb_t::Tick_t;
	using TcbPtr_t  = Tcb_t::TcbPtr_t;
	using PagePtr_t = Tcb_t::PagePtr_t;
	using Status    = Tcb_t::Status;

	MOS_INLINE inline TcbPtr_t
	current() { return cur_tcb; }

	MOS_INLINE inline void
	yield() { MOS_TRIGGER_PENDSV_INTR(); }

	MOS_INLINE inline void
	nop_and_yield() // This will consume time_slice and yield
	{
		cur_tcb->time_slice -= 1;
		yield();
	}

	MOS_INLINE inline Tick_t
	inc_ticks()
	{
		os_ticks += 1;
		return os_ticks;
	}

	MOS_INLINE inline Tid_t
	inc_tids()
	{
		tids += 1;
		return tids;
	}

	// For debug only
	MOS_INLINE inline uint32_t
	num() { return debug_tcbs.size(); }

	inline void terminate(TcbPtr_t tcb = current())
	{
		if (tcb == nullptr || tcb->is_status(Status::TERMINATED))
			return;

		// Assert if irq disabled
		MOS_ASSERT(test_irq(), "Disabled Interrupt");

		{ // Disable interrupt to enter critical section
			DisIntrGuard_t guard;

			// Remove the task from list
			if (tcb->is_status(Status::RUNNING) ||
			    tcb->is_status(Status::READY)) {
				ready_list.remove(tcb->node);
			}
			else if (tcb->is_sleeping()) {
				sleep_list.remove(tcb->node);
			}
			else {
				blocked_list.remove(tcb->node);
			}

			// Mark the page as unused and deinit the page
			tcb->release_page();

			// Reset the TCB to default
			tcb->deinit();

			// For debug only
			debug_tcbs.remove(tcb);
		} // Enable interrupt, leave critical section

		if (tcb == current()) {
			yield();
		}
	}

	MOS_INLINE static inline void
	exit() { terminate(current()); }

	MOS_INLINE static inline TcbPtr_t
	load_context(TcbPtr_t tcb)
	{
		// A descending stack consists of 16 registers as context.
		// high -> low, descending stack
		// | xPSR | PC | LR | R12 | R3 | R2 | R1 | R0 | R11 | R10 | R9 | R8 | R7 | R6 | R5 | R4 |
		tcb->set_SP(&tcb->page->raw[Macro::PAGE_SIZE - 16]);

		// Set the 'T' bit in stacked xPSR to '1' to notify processor on exception return about the Thumb state.
		// V6-m and V7-m cores can only support Thumb state so it should always be set to '1'.
		tcb->set_xPSR((uint32_t) 0x01000000);

		// Set the stacked PC
		tcb->set_PC((uint32_t) tcb->fn);

		// Call terminate(current()) automatically
		tcb->set_LR((uint32_t) exit);

		// Set arguments
		tcb->set_argv((uint32_t) tcb->argv);

		return tcb;
	}

	inline TcbPtr_t create(
	        Fn_t fn,
	        Argv_t argv = nullptr,
	        Prior_t pr  = 15,
	        Name_t name = "")
	{
		if (num() >= Macro::MAX_TASK_NUM) {
			return nullptr;
		}

		// Assert if irq disabled
		MOS_ASSERT(test_irq(), "Disabled Interrupt");
		MOS_ASSERT(fn != nullptr, "fn ptr can't be null");

		TcbPtr_t cur = current(), tcb = nullptr;

		{ // Disable interrupt to enter critical section
			DisIntrGuard_t guard;

			// Page Alloc
			auto page = Alloc::palloc();

			if (page == nullptr) {
				MOS_MSG("Page Alloc Failed!");
				return nullptr;
			}

			// Construct a TCB at the head of a page block
			tcb = Tcb_t::build(page, fn, argv, pr, name);

			// Load empty context
			load_context(tcb);

			// Give TID
			tcb->set_tid(inc_tids());

			// Set parent
			tcb->set_parent(cur);

			// Set TCB to be READY
			tcb->set_status(Status::READY);

			// Add to TCBs list
			ready_list.insert_in_order(tcb->node, Tcb_t::priority_cmp);

			// For debug only
			debug_tcbs.add(tcb);
		} // Enable interrupt, leave critical section

		// If the new task's priority is higher, switch at once
		if (Tcb_t::priority_cmp(tcb->node, cur->node)) {
			yield();
		}

		return tcb;
	}

	// Experimental
	MOS_INLINE inline TcbPtr_t
	create(Fn_t fn, auto& argv = nullptr, Prior_t pr = 15, Name_t name = "")
	{
		return create(fn, (Argv_t) &argv, pr, name);
	}

	// Not recommended
	inline TcbPtr_t create_fromISR(
	        Fn_t fn,
	        Argv_t argv = nullptr,
	        Prior_t pr  = 15,
	        Name_t name = "")
	{
		if (num() >= Macro::MAX_TASK_NUM) {
			return nullptr;
		}

		MOS_ASSERT(fn != nullptr, "fn can't be null");

		TcbPtr_t cur = current(), tcb = nullptr;

		{
			DisIntrGuard_t guard;

			// Page Alloc
			auto page = Alloc::palloc();

			if (page == nullptr) {
				MOS_MSG("Page Alloc Failed!");
				return nullptr;
			}

			// Construct a TCB at the head of a page block
			tcb = Tcb_t::build(page, fn, argv, pr, name);

			// Load empty context
			load_context(tcb);

			// Give TID
			tcb->set_tid(inc_tids());

			// Set parent
			tcb->set_parent(cur);

			// Set TCB to be READY
			tcb->set_status(Status::READY);

			// Add to ready_list
			ready_list.insert_in_order(tcb->node, Tcb_t::priority_cmp);

			// For debug only
			debug_tcbs.add(tcb);
		}

		return tcb;
	}

	inline void block_to(TcbPtr_t tcb, List_t& dest)
	{
		if (tcb == nullptr || tcb->is_status(Status::BLOCKED))
			return;

		// Assert if irq disabled
		MOS_ASSERT(test_irq(), "Disabled Interrupt");

		DisIntrGuard_t guard;
		tcb->set_status(Status::BLOCKED);
		ready_list.send_to(tcb->node, dest);

		if (tcb == current()) {
			return yield();
		}
	}

	inline void block_to_in_order(TcbPtr_t tcb, List_t& dest, NodeCmpFn auto&& cmp)
	{
		if (tcb == nullptr || tcb->is_status(Status::BLOCKED))
			return;

		// Assert if irq disabled
		MOS_ASSERT(test_irq(), "Disabled Interrupt");

		DisIntrGuard_t guard;
		tcb->set_status(Status::BLOCKED);
		ready_list.send_to_in_order(tcb->node, dest, cmp);

		if (tcb == current()) {
			return yield();
		}
	}

	MOS_INLINE inline void
	block(TcbPtr_t tcb = current())
	{
		block_to(tcb, blocked_list);
	}

	inline void resume(TcbPtr_t tcb)
	{
		if (tcb == nullptr || !tcb->is_status(Status::BLOCKED))
			return;

		// Assert if irq disabled
		MOS_ASSERT(test_irq(), "Disabled Interrupt");

		DisIntrGuard_t guard;
		tcb->set_status(Status::READY);
		blocked_list.send_to_in_order(
		        tcb->node,
		        ready_list,
		        Tcb_t::priority_cmp);

		if (current() != (TcbPtr_t) ready_list.begin()) {
			// if cur_tcb isn't the highest priority
			return yield();
		}
	}

	// Not recommended
	inline void resume_fromISR(TcbPtr_t tcb)
	{
		if (tcb == nullptr || !tcb->is_status(Status::BLOCKED))
			return;

		DisIntrGuard_t guard;
		tcb->set_status(Status::READY);
		blocked_list.send_to_in_order(
		        tcb->node,
		        ready_list,
		        Tcb_t::priority_cmp);
	}

	MOS_INLINE inline void
	change_priority(TcbPtr_t tcb, Tcb_t::Prior_t pr)
	{
		MOS_ASSERT(test_irq(), "Disabled Interrupt");

		DisIntrGuard_t guard;
		tcb->set_priority(pr);
		ready_list.re_insert(tcb->node, Tcb_t::priority_cmp);

		if (current() != (TcbPtr_t) ready_list.begin()) {
			// if cur_tcb isn't the highest priority
			return yield();
		}
	}

	MOS_INLINE inline TcbPtr_t
	find(auto info)
	{
		DisIntrGuard_t guard;

		auto fetch = [info](TcbPtr_t tcb) {
			if constexpr (Concept::Same<decltype(info), Tid_t>) {
				return tcb->get_tid() == info;
			}

			if constexpr (Concept::Same<decltype(info), Name_t>) {
				return strcmp(tcb->get_name(), info) == 0;
			}
		};

		return debug_tcbs.iter_until(fetch);
	}

	inline void print_name()
	{
		DisIntrGuard_t guard;
		kprintf("%s\n", current()->get_name());
	}

	MOS_INLINE inline constexpr auto
	status_name(const Status status)
	{
		switch (status) {
			case Status::READY:
				return "READY";
			case Status::RUNNING:
				return "RUNNING";
			case Status::BLOCKED:
				return "BLOCKED";
			case Status::TERMINATED:
				return "TERMINATED";
			default:
				return "INVALID";
		}
	};

	inline void
	print_info(TcbPtr_t tcb, const char* format = " #%-2d %-9s %-5d %-9s %3d%%\n")
	{
		kprintf(format,
		        tcb->get_tid(),
		        tcb->get_name(),
		        tcb->get_priority(),
		        status_name(tcb->get_status()),
		        tcb->stack_usage());
	};

	// For debug only
	inline void print_all()
	{
		DisIntrGuard_t guard;
		kprintf("------------------------------------\n");
		debug_tcbs.iter([](TcbPtr_t tcb) { print_info(tcb); });
		kprintf("------------------------------------\n");
	}

	MOS_INLINE inline void
	delay(const Tick_t ticks)
	{
		static auto delay_ticks_cmp = [](const auto& lhs, const auto& rhs) {
			return ((const Tcb_t&) lhs).delay_ticks <
			       ((const Tcb_t&) rhs).delay_ticks;
		};

		auto cur = current();
		cur->set_delay_ticks(os_ticks + ticks);
		block_to_in_order(cur, sleep_list, delay_ticks_cmp);
	}
}

#endif