#ifndef _MOS_TASK_
#define _MOS_TASK_

#include "concepts.hpp"
#include "utils.hpp"
#include "global.hpp"
#include "alloc.hpp"

namespace MOS::Task
{
	using namespace Utils;
	using namespace KernelGlobal;

	using Tcb_t     = DataType::Tcb_t;
	using TcbList_t = DataType::TcbList_t;
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

	// Whether any task with higher priority exists
	MOS_INLINE inline bool
	higher_exists(TcbPtr_t tcb = current())
	{
		return tcb != ready_list.begin();
	}

	MOS_INLINE inline void
	nop_and_yield()
	{
		current()->time_slice -= 1;
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
				ready_list.remove(tcb);
			}
			else if (tcb->is_sleeping()) {
				sleep_list.remove(tcb);
			}
			else {
				blocked_list.remove(tcb);
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
		tcb->set_SP(tcb->page->first_stk_top());

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
	        Prior_t pri = Macro::PRI_MIN,
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
			tcb = Tcb_t::build(page, fn, argv, pri, name);

			// Load empty context
			load_context(tcb);

			// Give TID
			tcb->set_tid(inc_tids());

			// Set parent
			tcb->set_parent(cur);

			// Set TCB to be READY
			tcb->set_status(Status::READY);

			// Add to TCBs list
			ready_list.insert_in_order(tcb, Tcb_t::pri_cmp);

			// For debug only
			debug_tcbs.add(tcb);
		} // Enable interrupt, leave critical section

		// If a new task has higher priority, switch at once
		if (Tcb_t::pri_cmp(tcb, cur)) {
			yield();
		}

		return tcb;
	}

	// Experimental
	MOS_INLINE inline TcbPtr_t
	create(Fn_t fn,
	       auto& argv  = nullptr,
	       Prior_t pri = Macro::PRI_MIN,
	       Name_t name = "")
	{
		return create(fn, (Argv_t) &argv, pri, name);
	}

	// Not recommended
	inline TcbPtr_t create_fromISR(
	        Fn_t fn,
	        Argv_t argv = nullptr,
	        Prior_t pri = Macro::PRI_MIN,
	        Name_t name = "")
	{
		MOS_ASSERT(fn != nullptr, "fn can't be null");

		if (num() >= Macro::MAX_TASK_NUM) {
			return nullptr;
		}

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
			tcb = Tcb_t::build(page, fn, argv, pri, name);

			// Load empty context
			load_context(tcb);

			// Give TID
			tcb->set_tid(inc_tids());

			// Set parent
			tcb->set_parent(cur);

			// Set TCB to be READY
			tcb->set_status(Status::READY);

			// Add to ready_list
			ready_list.insert_in_order(tcb, Tcb_t::pri_cmp);

			// For debug only
			debug_tcbs.add(tcb);
		}

		return tcb;
	}

	inline void block_to(TcbPtr_t tcb, TcbList_t& dest)
	{
		if (tcb == nullptr || tcb->is_status(Status::BLOCKED))
			return;

		// Assert if irq disabled
		MOS_ASSERT(test_irq(), "Disabled Interrupt");

		DisIntrGuard_t guard;
		tcb->set_status(Status::BLOCKED);
		ready_list.send_to(tcb, dest);

		if (tcb == current()) {
			return yield();
		}
	}

	inline void block_to_in_order(
	        TcbPtr_t tcb,
	        TcbList_t& dest,
	        auto&& cmp)
	{
		if (tcb == nullptr || tcb->is_status(Status::BLOCKED))
			return;

		// Assert if irq disabled
		MOS_ASSERT(test_irq(), "Disabled Interrupt");

		DisIntrGuard_t guard;
		tcb->set_status(Status::BLOCKED);
		ready_list.send_to_in_order(tcb, dest, cmp);

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
		        tcb,
		        ready_list,
		        Tcb_t::pri_cmp);

		// If tasks with higher priority exist
		if (higher_exists()) {
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
		        tcb,
		        ready_list,
		        Tcb_t::pri_cmp);
	}

	MOS_INLINE inline void
	change_priority(TcbPtr_t tcb, Prior_t pri)
	{
		MOS_ASSERT(test_irq(), "Disabled Interrupt");

		DisIntrGuard_t guard;
		tcb->set_pri(pri);
		ready_list.re_insert(tcb, Tcb_t::pri_cmp);

		// If tasks with higher priority exist
		if (higher_exists()) {
			return yield();
		}
	}

	MOS_INLINE inline TcbPtr_t
	find(auto info)
	{
		DisIntrGuard_t guard;

		auto fetch = [info](TcbPtr_t tcb) {
			using Concepts::Same;

			if constexpr (Same<decltype(info), Tid_t>) {
				return tcb->get_tid() == info;
			}

			if constexpr (Same<decltype(info), Name_t>) {
				return strcmp(tcb->get_name(), info) == 0;
			}
		};

		return debug_tcbs.iter_until(fetch);
	}

	MOS_INLINE inline void
	print_name()
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

	MOS_INLINE inline void
	print_info(TcbPtr_t tcb, const char* format = " #%-2d %-9s %-5d %-9s %2d%%\n")
	{
		kprintf(format,
		        tcb->get_tid(),
		        tcb->get_name(),
		        tcb->get_pri(),
		        status_name(tcb->get_status()),
		        tcb->stack_usage());
	};

	// For debug only
	inline void print_all()
	{
		DisIntrGuard_t guard;
		kprintf("-----------------------------------\n");
		debug_tcbs.iter_mut([](TcbPtr_t tcb) { print_info(tcb); });
		kprintf("-----------------------------------\n");
	}

	inline void delay(const Tick_t ticks)
	{
		static auto delay_cmp = [](TcbPtr_t lhs, TcbPtr_t rhs) {
			return lhs->delay_ticks < rhs->delay_ticks;
		};

		auto cur = current();
		cur->set_delay(os_ticks + ticks);
		block_to_in_order(cur, sleep_list, delay_cmp);
	}
}

#endif