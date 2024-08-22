#ifndef _MOS_TASK_
#define _MOS_TASK_

#include "concepts.hpp"
#include "utils.hpp"
#include "global.hpp"
#include "alloc.hpp"

namespace MOS::Kernel::Task
{
	using namespace Global;
	using namespace Concepts;
	using namespace Utils;
	using namespace Alloc;

	using Node_t   = TCB_t::Node_t;
	using Fn_t     = TCB_t::Fn_t;
	using Argv_t   = TCB_t::Argv_t;
	using Prior_t  = TCB_t::Prior_t;
	using Name_t   = TCB_t::Name_t;
	using Tid_t    = TCB_t::Tid_t;
	using Tick_t   = TCB_t::Tick_t;
	using TcbPtr_t = TCB_t::TcbPtr_t;

	using enum TCB_t::Status;

	MOS_INLINE inline TcbPtr_t
	current() { return cur_tcb; }

	MOS_INLINE inline void
	yield() { MOS_TRIGGER_PENDSV_INTR(); }

	// Whether task with higher priority than tcb exists
	MOS_INLINE inline bool
	any_higher(TcbPtr_t tcb = current())
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
	inc_ticks() { os_ticks += 1; }

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
		IntrGuard_t guard;
		while (!zombie_list.empty()) {
			auto tcb = zombie_list.begin();
			zombie_list.remove(tcb); // Remove from zombie_list
			tcb->release();          // Release resources
		}
		return yield();
	}

	inline void
	terminate_raw(TcbPtr_t tcb)
	{
		// Remove the task from where it belongs to
		if (tcb->is_status(RUNNING) || tcb->is_status(READY)) {
			ready_list.remove(tcb);
		}
		else if (tcb->is_sleeping()) {
			sleeping_list.remove(tcb);
		}
		else {
			blocked_list.remove(tcb);
		}

		tcb->set_status(TERMINATED); // Mark tcb as TERMINATED
		tids.reset(tcb->get_tid());  // Clear the bit in tids
		debug_tcbs.remove(tcb);      // For debug only

		// Only `DYNAMIC` pages need delayed recycling
		if (tcb->page.is_policy(DYNAMIC)) {
			zombie_list.add(tcb); // Add to zombie_list
		}
		else { // Otherwise for `POOL` or `STATIC` just release immediately
			tcb->release();
		}
	}

	inline void
	terminate(TcbPtr_t tcb = current())
	{
		MOS_ASSERT(test_irq(), "Disabled Interrupt");
		IntrGuard_t guard;
		if (tcb == nullptr || tcb->is_status(TERMINATED))
			return;
		terminate_raw(tcb);
		if (tcb == current()) {
			return yield();
		}
	}

	MOS_INLINE static inline void
	exit() { terminate(current()); }

	MOS_INLINE static inline void
	load_context(TcbPtr_t tcb)
	{
		// A descending stack consists of 16 registers as context.
		// high -> low, descending stack
		// | xPSR | PC | LR | R12 | R3 | R2 | R1 | R0 | R11 | R10 | R9 | R8 | R7 | R6 | R5 | R4 |
		tcb->set_sp((uint32_t) &tcb->page.from_bottom(16));

		// Set the 'T' bit in stacked xPSR to '1' to notify processor on exception return about the Thumb state.
		// V6-m and V7-m cores can only support Thumb state so it should always be set to '1'.
		tcb->set_xpsr((uint32_t) 0x0100'0000);
		tcb->set_pc((uint32_t) tcb->fn);     // Set the stacked PC as entry
		tcb->set_lr((uint32_t) exit);        // Call terminate() automatically
		tcb->set_argv((uint32_t) tcb->argv); // Set arguments to R0
	}

	namespace // private checker concepts
	{
		template <typename Fn, typename ArgvRef>
		concept LambdaArgvCheck =
		    Same<
		        decltype(&Fn::operator()),
		        void (Fn::*)(ArgvRef) const> ||
		    Same<
		        decltype(&Fn::operator()),
		        void (Fn::*)(const_ref_t<ArgvRef>) const>;

		// clang-format off
		template <typename Fn, typename Argv>
		concept LambdaInvoke =
		(LambdaArgvCheck<Fn, Argv> &&
			requires(Fn fn, Argv argv) {
				{ fn.operator()(argv) } -> Same<void>;
		}) ||
		requires(Fn fn) {
			{ fn.operator()() } -> Same<void>;
		};
		// clang-format on

		template <typename Fn, typename Argv>
		concept AsLambda =
		    (Pointer<Argv> &&
		     (LambdaInvoke<Fn, deref_t<Argv>*> ||
		      LambdaInvoke<Fn, deref_t<Argv>&>) ) ||
		    (!Pointer<Argv> && LambdaInvoke<Fn, Argv>);

		template <typename Fn, typename Argv>
		concept AsFnPtr =
		    (Pointer<Argv> &&
		     (Invocable<Fn, void, deref_t<Argv>*> ||
		      Invocable<Fn, void, deref_t<Argv>&>) ) ||
		    (!Pointer<Argv> &&
		     (Invocable<Fn, void> || Invocable<Fn, void, Argv>) );

		MOS_INLINE static inline constexpr auto
		type_check(auto fn, auto argv)
		{
			using RawFn_t   = decltype(fn);
			using RawArgv_t = decltype(argv);

			if constexpr (AsLambda<RawFn_t, RawArgv_t>) {
				return (Fn_t) ((uint32_t) + fn);
			}
			else {
				static_assert(
				    AsFnPtr<RawFn_t, RawArgv_t>,
				    "Invalid Invoke Type!"
				);
				return (Fn_t) ((uint32_t) fn);
			}
		}
	}

	inline TcbPtr_t // No yield() inside
	create_raw(
	    auto fn, auto argv, Prior_t pri,
	    Name_t name, Page_t page
	)
	{
		MOS_ASSERT(fn != nullptr, "fn can't be null");

		if (page.get_raw() == nullptr) {
			MOS_MSG("Page Alloc Failed!");
			return nullptr;
		}

		if (debug_tcbs.size() >= MAX_TASK_NUM) {
			MOS_MSG("Max tasks!");
			return nullptr;
		}

		// Construct a tcb at the head of a page
		auto cur = current(),
		     tcb = TCB_t::build(
		         type_check(fn, argv), (Argv_t) argv,
		         pri, name, page
		     );

		load_context(tcb);         // Load Context
		tcb->set_tid(tid_alloc()); // Set Tid
		tcb->set_stamp(os_ticks);  // Set Timestamp
		tcb->set_parent(cur);      // Set Parent
		tcb->set_status(READY);    // Set Status into READY

		ready_list.insert_in_order( // Add to ready_list
		    tcb, TCB_t::pri_cmp
		);

		debug_tcbs.add(tcb); // For debug only
		return tcb;
	}

	MOS_INLINE inline Page_t
	page_alloc(Page_t::Policy policy, PgSz_t pg_sz)
	{
		return Page_t {
		    .policy = policy,
		    .raw    = palloc(policy, pg_sz),
		    .size   = pg_sz,
		};
	}

	inline TcbPtr_t
	create_impl(
	    auto fn, auto argv, Prior_t pri,
	    Name_t name, Page_t page
	)
	{
		MOS_ASSERT(test_irq(), "Disabled Interrupt");
		TcbPtr_t tcb = nullptr;

		{
			IntrGuard_t guard;
			tcb = create_raw(fn, argv, pri, name, page);
		}

		if (TCB_t::pri_cmp(tcb, current())) {
			yield();
		}

		return tcb;
	}

	// Create task from static memory
	MOS_INLINE inline TcbPtr_t
	create(
	    auto fn, auto argv, Prior_t pri,
	    Name_t name, Page_t page
	)
	{
		return create_impl(fn, argv, pri, name, page);
	}

	// Create task from pre-allocated `page_pool`
	MOS_INLINE inline TcbPtr_t
	create(
	    auto fn, auto argv, Prior_t pri, Name_t name
	)
	{
		auto page = page_alloc(POOL, PAGE_SIZE);
		return create(fn, argv, pri, name, page);
	}

	// Create task from dynamic memory
	MOS_INLINE inline TcbPtr_t
	create(
	    auto fn, auto argv, Prior_t pri,
	    Name_t name, PgSz_t pg_sz
	)
	{
		auto page = page_alloc(DYNAMIC, pg_sz);
		return create(fn, argv, pri, name, page);
	}

	// Not recommended to use
	MOS_INLINE inline TcbPtr_t
	create_from_isr(
	    auto fn, auto argv, Prior_t pri, Name_t name
	)
	{
		auto page = page_alloc(POOL, PAGE_SIZE);
		return create_raw(fn, argv, pri, name, page);
	}

	static inline void
	block_to_raw(
	    TcbPtr_t tcb,
	    TcbList_t& dest = blocked_list
	)
	{
		tcb->set_status(BLOCKED);
		ready_list.send_to(tcb, dest);
	}

	static inline void
	block_to_in_order_raw(
	    TcbPtr_t tcb,
	    TcbList_t& dest,
	    TcbCmpFn auto&& cmp
	)
	{
		tcb->set_status(BLOCKED);
		ready_list.send_to_in_order(tcb, dest, cmp);
	}

	inline void
	block_to(TcbPtr_t tcb, TcbList_t& dest)
	{
		MOS_ASSERT(test_irq(), "Disabled Interrupt");
		IntrGuard_t guard;
		if (tcb == nullptr || tcb->is_status(BLOCKED))
			return;
		block_to_raw(tcb, dest);
		if (tcb == current()) {
			return yield();
		}
	}

	inline void
	block_to_in_order(
	    TcbPtr_t tcb,
	    TcbList_t& dest,
	    TcbCmpFn auto&& cmp
	)
	{
		MOS_ASSERT(test_irq(), "Disabled Interrupt");
		IntrGuard_t guard;
		if (tcb == nullptr || tcb->is_status(BLOCKED))
			return;
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
	resume_raw(
	    TcbPtr_t tcb,
	    TcbList_t& src = blocked_list
	)
	{
		tcb->set_status(READY);
		src.send_to_in_order(tcb, ready_list, TCB_t::pri_cmp);
	}

	inline void
	resume(
	    TcbPtr_t tcb,
	    TcbList_t& src = blocked_list
	)
	{
		MOS_ASSERT(test_irq(), "Disabled Interrupt");
		IntrGuard_t guard;
		if (tcb == nullptr || !tcb->is_status(BLOCKED))
			return;
		resume_raw(tcb, src);
		if (any_higher()) {
			return yield();
		}
	}

	inline void
	resume_from_isr(
	    TcbPtr_t tcb,
	    TcbList_t& src = blocked_list
	)
	{
		MOS_ASSERT(test_irq(), "Disabled Interrupt");
		IntrGuard_t guard;
		if (tcb == nullptr || !tcb->is_status(BLOCKED))
			return;
		resume_raw(tcb, src);
	}

	inline void
	change_pri(TcbPtr_t tcb, Prior_t pri)
	{
		MOS_ASSERT(test_irq(), "Disabled Interrupt");
		IntrGuard_t guard;
		tcb->set_pri(pri);
		ready_list.re_insert(tcb, TCB_t::pri_cmp);
		if (any_higher()) {
			return yield();
		}
	}

	inline TcbPtr_t
	find(auto info)
	{
		IntrGuard_t guard;

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

	MOS_INLINE inline void
	print_name()
	{
		IntrGuard_t guard;
		kprintf("%s\n", current()->get_name());
	}

	MOS_INLINE inline constexpr auto
	status_name(const TCB_t::Status status)
	{
		switch (status) {
			case READY: return "READY";
			case RUNNING: return "RUNNING";
			case BLOCKED: return "BLOCKED";
			case TERMINATED: return "TERMINATED";
			default: return "INVALID";
		}
	};

	inline void
	print_info(
	    TcbPtr_t tcb,
	    const char* format = " #%-2d %-10s %-5d %-9s %3d%%\n"
	)
	{
		kprintf(
		    format,
		    tcb->get_tid(),
		    tcb->get_name(),
		    tcb->get_pri(),
		    status_name(tcb->get_status()),
		    tcb->stack_usage()
		);
	};

	// For debug only
	inline void print_all()
	{
		IntrGuard_t guard;
		kprintf("-------------------------------------\n");
		debug_tcbs.iter([](TcbPtr_t tcb) { print_info(tcb); });
		kprintf("-------------------------------------\n");
	}

	void delay(const Tick_t ticks)
	{
		auto cur = current();
		cur->set_wkpt(os_ticks + ticks);
		block_to_in_order(cur, sleeping_list, TCB_t::wkpt_cmp);
	}

	inline void
	wake_raw(TcbPtr_t tcb)
	{
		tcb->set_wkpt(0xFFFF'FFFF); // Set wakepoint as invalid
		Task::resume_raw(tcb, sleeping_list);
	}
}

#endif