#ifndef _MOS_TASK_
#define _MOS_TASK_

#include "concepts.hpp"
#include "utils.hpp"
#include "global.hpp"
#include "alloc.hpp"

namespace MOS::Task
{
	using namespace KernelGlobal;
	using namespace Utils;
	using namespace Alloc;

	using Fn_t      = TCB_t::Fn_t;
	using Argv_t    = TCB_t::Argv_t;
	using Pri_t     = TCB_t::Prior_t;
	using Node_t    = TCB_t::Node_t;
	using Name_t    = TCB_t::Name_t;
	using Tid_t     = TCB_t::Tid_t;
	using Tick_t    = TCB_t::Tick_t;
	using TcbPtr_t  = TCB_t::TcbPtr_t;
	using Status    = TCB_t::Status;

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
	dec_tmslc()
	{
		// Avoid underflow
		if (current()->time_slice <= TIME_SLICE)
			current()->time_slice -= 1;
		else
			current()->time_slice = 0;
	}

	MOS_INLINE inline void
	nop_and_yield()
	{
		dec_tmslc();
		yield();
	}

	MOS_INLINE inline Tick_t
	inc_ticks()
	{
		os_ticks += 1;
		return os_ticks;
	}

	MOS_INLINE inline Tid_t
	tid_alloc()
	{
		auto tid = tids.first_zero();
		tids.set(tid);
		return tid;
	}

	// Used in idle task
	inline void recycle()
	{
		DisIntrGuard_t guard;
		while (!zombie_list.empty()) {
			auto tcb = zombie_list.begin();
			// For debug only
			debug_tcbs.remove(tcb);

			// Delete tcb from zombie_list
			zombie_list.remove(tcb);

			// Reset tcb to default
			tcb->deinit();
		}
		return yield();
	}

	// For debug only
	MOS_INLINE inline uint32_t
	num() { return debug_tcbs.size(); }

	inline void terminate_raw(TcbPtr_t tcb)
	{
		// Remove the task from list
		if (tcb->is_status(Status::RUNNING) ||
		    tcb->is_status(Status::READY)) {
			ready_list.remove(tcb);
		}
		else if (tcb->is_sleeping()) {
			sleeping_list.remove(tcb);
		}
		else {
			blocked_list.remove(tcb);
		}

		// Mark tcb as TERMINATED and add to zombie_list
		tcb->set_status(Status::TERMINATED);

		// Clear the bit in tids
		tids.reset(tcb->get_tid());

		// Only DYNAMIC pages need delayed recycling
		if (tcb->page.is_policy(PagePolicy::DYNAMIC)) {
			zombie_list.add(tcb);
		}
		else { // Otherwise POOL or STATIC, just remove and deinit
			debug_tcbs.remove(tcb);
			tcb->deinit();
		}
	}

	inline void terminate(TcbPtr_t tcb = current())
	{
		if (tcb == nullptr || tcb->is_status(Status::TERMINATED))
			return;

		// Assert if irq disabled
		MOS_ASSERT(test_irq(), "Disabled Interrupt");
		DisIntrGuard_t guard;
		terminate_raw(tcb);
		if (tcb == current()) {
			return yield();
		}
	}

	MOS_INLINE static inline void
	exit() { terminate(current()); }

	MOS_INLINE static inline TcbPtr_t
	setup_context(TcbPtr_t tcb)
	{
		// A descending stack consists of 16 registers as context.
		// high -> low, descending stack
		// | xPSR | PC | LR | R12 | R3 | R2 | R1 | R0 | R11 | R10 | R9 | R8 | R7 | R6 | R5 | R4 |
		tcb->set_SP(&tcb->page.from_bottom(16));

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

	inline TcbPtr_t
	create_raw(Fn_t fn, Argv_t argv, Pri_t pri, Name_t name, Page_t page)
	{
		MOS_ASSERT(fn != nullptr, "fn can't be null");

		if (debug_tcbs.size() >= MAX_TASK_NUM) {
			MOS_MSG("Max tasks!");
			return nullptr;
		}

		if (page.get_raw() == nullptr) {
			MOS_MSG("Page Alloc Failed!");
			return nullptr;
		}

		DisIntrGuard_t guard;
		TcbPtr_t cur = current(), tcb = nullptr;

		// Construct a tcb at the head of a page
		tcb = TCB_t::build(fn, argv, pri, name, page);

		// Load empty context
		setup_context(tcb);

		// Give Tid
		tcb->set_tid(tid_alloc());

		// Set Time Stamp
		tcb->set_stamp(os_ticks);

		// Set parent
		tcb->set_parent(cur);

		// Set tcb to be READY
		tcb->set_status(Status::READY);

		// Add to ready_list
		ready_list.insert_in_order(tcb, TCB_t::pri_cmp);

		// For debug only
		debug_tcbs.add(tcb);

		return tcb;
	}

	inline TcbPtr_t
	create_impl(Fn_t fn, Argv_t argv, Pri_t pri, Name_t name, Page_t page)
	{
		auto tcb = create_raw(fn, argv, pri, name, page);

		// If a new task has higher priority, switch at once
		if (TCB_t::pri_cmp(tcb, current())) {
			yield();
		}

		return tcb;
	}

	// Create task from static memory
	MOS_INLINE inline TcbPtr_t
	create(Fn_t fn, Argv_t argv, Pri_t pri, Name_t name, Page_t page)
	{
		return create_impl(fn, argv, pri, name, page);
	}

	MOS_INLINE inline Page_t
	page_alloc(PagePolicy policy, PgSz_t pg_sz)
	{
		return Page_t {
		        .policy = policy,
		        .raw    = palloc_raw(policy, pg_sz),
		        .size   = pg_sz,
		};
	}

	// Create task from pre-allocated `page_pool`
	MOS_INLINE inline TcbPtr_t
	create(Fn_t fn, Argv_t argv, Pri_t pri, Name_t name)
	{
		auto page = page_alloc(PagePolicy::POOL, PAGE_SIZE);
		return create_impl(fn, argv, pri, name, page);
	}

	// Create task from dynamic allocated memory
	MOS_INLINE inline TcbPtr_t
	create(Fn_t fn, Argv_t argv, Pri_t pri, Name_t name, PgSz_t pg_sz)
	{
		auto page = page_alloc(PagePolicy::DYNAMIC, pg_sz);
		return create_impl(fn, argv, pri, name, page);
	}

	// Not recommended to use
	MOS_INLINE inline TcbPtr_t
	create_from_isr(Fn_t fn, Argv_t argv, Pri_t pri, Name_t name)
	{
		auto page = page_alloc(PagePolicy::POOL, PAGE_SIZE);
		return create_raw(fn, argv, pri, name, page);
	}

	static inline void
	block_to_raw(TcbPtr_t tcb, TcbList_t& dest = blocked_list)
	{
		tcb->set_status(Status::BLOCKED);
		ready_list.send_to(tcb, dest);
	}

	static inline void
	block_to_in_order_raw(
	        TcbPtr_t tcb,
	        TcbList_t& dest,
	        TcbCmpFn auto&& cmp)
	{
		tcb->set_status(Status::BLOCKED);
		ready_list.send_to_in_order(tcb, dest, cmp);
	}

	inline void block_to(TcbPtr_t tcb, TcbList_t& dest)
	{
		if (tcb == nullptr || tcb->is_status(Status::BLOCKED))
			return;

		// Assert if irq disabled
		MOS_ASSERT(test_irq(), "Disabled Interrupt");
		DisIntrGuard_t guard;
		block_to_raw(tcb, dest);
		if (tcb == current()) {
			return yield();
		}
	}

	inline void
	block_to_in_order(
	        TcbPtr_t tcb,
	        TcbList_t& dest,
	        TcbCmpFn auto&& cmp)
	{
		if (tcb == nullptr || tcb->is_status(Status::BLOCKED))
			return;

		// Assert if irq disabled
		MOS_ASSERT(test_irq(), "Disabled Interrupt");
		DisIntrGuard_t guard;
		block_to_in_order_raw(tcb, dest, cmp);
		if (tcb == current()) {
			return yield();
		}
	}

	MOS_INLINE inline void
	block(TcbPtr_t tcb = current())
	{
		block_to(tcb, blocked_list);
	}

	static inline void
	resume_raw(TcbPtr_t tcb, TcbList_t& src = blocked_list)
	{
		tcb->set_status(Status::READY);
		src.send_to_in_order(
		        tcb,
		        ready_list,
		        TCB_t::pri_cmp);
	}

	inline void resume(TcbPtr_t tcb)
	{
		if (tcb == nullptr || !tcb->is_status(Status::BLOCKED))
			return;

		// Assert if irq disabled
		MOS_ASSERT(test_irq(), "Disabled Interrupt");
		DisIntrGuard_t guard;
		resume_raw(tcb, blocked_list);

		// If tasks with higher priority exist
		if (higher_exists()) {
			return yield();
		}
	}

	// Not recommended
	inline void resume_from_isr(TcbPtr_t tcb)
	{
		if (tcb == nullptr || !tcb->is_status(Status::BLOCKED))
			return;

		DisIntrGuard_t guard;
		resume_raw(tcb, blocked_list);
	}

	inline void
	change_pri(TcbPtr_t tcb, Pri_t pri)
	{
		// Assert if irq disabled
		MOS_ASSERT(test_irq(), "Disabled Interrupt");
		DisIntrGuard_t guard;
		tcb->set_pri(pri);
		ready_list.re_insert(tcb, TCB_t::pri_cmp);

		// If tasks with higher priority exist
		if (higher_exists()) {
			return yield();
		}
	}

	inline TcbPtr_t
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

	inline void
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
		debug_tcbs.iter([](TcbPtr_t tcb) { print_info(tcb); });
		kprintf("-----------------------------------\n");
	}

	inline void delay(const Tick_t ticks)
	{
		static auto delay_cmp = [](TcbPtr_t lhs, TcbPtr_t rhs) {
			return lhs->delay_ticks < rhs->delay_ticks;
		};

		auto cur = current();
		cur->set_delay(os_ticks + ticks);
		block_to_in_order(cur, sleeping_list, delay_cmp);
	}

	inline namespace Async
	{
		struct Future_t
		{
			struct Stamp_t
			{
				Tick_t stmp = 0;
				Tid_t tid   = -1;

				MOS_INLINE Stamp_t(TcbPtr_t tcb)
				    : tid(tcb->get_tid()),
				      stmp(tcb->get_stamp()) {}

				// A distinguishable task can be marked by tid and stamp(os_ticks)
				MOS_INLINE inline bool
				is_valid(TcbPtr_t tcb) const
				{
					return tid == tcb->get_tid() &&
					       stmp == tcb->get_stamp();
				}
			} stamp;

			TcbPtr_t tcb;

			MOS_INLINE
			inline Future_t(TcbPtr_t tcb)
			    : tcb(tcb), stamp(tcb) {}

			MOS_INLINE
			inline ~Future_t() { await(); }

			MOS_INLINE inline bool
			is_ready() const
			{
				// If tcb is invalid -> task has been deinited -> DONE (mostly)
				// If tcb is valid, check status, TERMINATED -> DONE (rarely)
				DisIntrGuard_t guard;
				return !stamp.is_valid(tcb) ||
				       tcb->is_status(Status::TERMINATED);
			}

			MOS_INLINE inline void
			await()
			{
				// Spin Waiting
				while (!is_ready()) {
					Task::delay(1);
				}
				tcb = nullptr;
			}
		};

		MOS_INLINE inline auto
		async_raw(Fn_t fn, Argv_t argv, Name_t name, Page_t page)
		{
			DisIntrGuard_t guard;
			auto tcb = create_raw(
			        fn,
			        argv,
			        current()->get_pri(),
			        name,
			        page);
			return Future_t {tcb};
		}

		MOS_INLINE inline auto
		async(Fn_t fn, Argv_t argv, Name_t name, Page_t page)
		{
			return async_raw(fn, argv, name, page);
		}

		MOS_INLINE inline auto
		async(Fn_t fn, Argv_t argv, Name_t name)
		{
			auto page = page_alloc(PagePolicy::POOL, PAGE_SIZE);
			return async_raw(fn, argv, name, page);
		}

		MOS_INLINE inline auto
		async(Fn_t fn, Argv_t argv, Name_t name, PgSz_t pg_sz)
		{
			auto page = page_alloc(PagePolicy::DYNAMIC, pg_sz);
			return async_raw(fn, argv, name, page);
		}
	}
}

#endif