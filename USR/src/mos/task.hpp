#ifndef _MOS_TASK_
#define _MOS_TASK_

#include "nuts/concepts.hpp"
#include "util.hpp"
#include "global_res.hpp"

namespace MOS::Task
{
	using namespace GlobalRes;

	using Status_t = TCB_t::Status_t;
	using Node_t   = TCB_t::Node_t;
	using Name_t   = TCB_t::Name_t;
	using Tid_t    = TCB_t::Tid_t;

	__attribute__((always_inline)) inline uint32_t
	num() { return ready_list.size() + blocked_list.size(); }

	inline TCB_t* create(TCB_t::Fn_t&& fn   = nullptr,
	                     TCB_t::Argv_t argv = nullptr,
	                     TCB_t::Prior_t pr  = 15,
	                     TCB_t::Name_t name = "")
	{
		// Disable interrupt to enter critical section
		MOS_DISABLE_IRQ();

		TCB_t::PagePtr_t p = nullptr;

		MOS_ASSERT(fn != nullptr, "fn == null");

		if (num() >= MAX_TASK_NUM) {
			// Failed, no enough page
			return nullptr;
		}

		for (auto& page: pages) {
			if (!page.is_used) {
				p = &page;
				break;
			}
		}

		// Construct TCB at head of a page
		auto& tcb = *new (p->raw) TCB_t {fn, argv, pr, name};

		// Give TID
		tcb.set_tid(tids++);

		// Set task page
		tcb.attach_page(p);

		// Setup the stack to hold task context.
		// Remember it is a descending stack and a context consists of 16 registers.
		tcb.set_SP(&p->raw[Macro::PAGE_SIZE - 16]);

		// Set the 'T' bit in stacked xPSR to '1' to notify processor on exception return about the thumb state.
		// V6-m and V7-m cores can only support thumb state hence it should always be set to '1'.
		tcb.set_xPSR((uint32_t) 0x01000000);

		// Set the stacked PC to point to the task
		tcb.set_PC((uint32_t) tcb.fn);

		// Set TCB to ready
		tcb.set_status(Status_t::READY);

		// Add parent
		tcb.set_parent((TCB_t*) curTCB);

		// Add to TCBs list
		ready_list.add(tcb.node);

		// Enable interrupt, leave critical section
		MOS_ENABLE_IRQ();

		return &tcb;
	}

	__attribute__((always_inline)) inline void yield()
	{
		// Trigger SysTick Interrupt -> SysTick_Handler
		MOS_TRIGGER_SYSTICK();
	}

	inline void block()
	{
		// Disable interrupt to enter critical section
		MOS_DISABLE_IRQ();

		auto& cur_tcb = (TCB_t&) *curTCB;

		// Remove the task from the task list and kids list
		cur_tcb.set_status(Status_t::BLOCKED);
		ready_list.remove(cur_tcb.node);
		blocked_list.add(cur_tcb.node);

		// Enable interrupt, leave critical section
		MOS_ENABLE_IRQ();

		// Give out the CPU
		yield();
	}

	inline void resume(TCB_t* blocked)
	{
		if (blocked == nullptr || !blocked->is_status(Status_t::BLOCKED))
			return;
		MOS_DISABLE_IRQ();
		blocked_list.remove(blocked->node);
		ready_list.add(blocked->node);
		blocked->set_status(Status_t::READY);
		MOS_ENABLE_IRQ();
		yield();
	}

	inline void terminate()
	{
		// Disable interrupt to enter critical section
		MOS_DISABLE_IRQ();

		auto& cur_tcb = (TCB_t&) *curTCB;

		// Remove the task from the task list and kids list
		ready_list.remove(cur_tcb.node);

		// Clear the task's page
		// todo!()

		// Mark the page as unused
		cur_tcb.release_page();

		// Reset the TCB to default
		cur_tcb.deinit();

		// Enable interrupt, leave critical section
		MOS_ENABLE_IRQ();

		while (true) {
			// Never goes to scheduling again...
		}
	}

	__attribute__((always_inline)) inline void
	for_all_tasks(auto&& fn)
	    requires nuts::Invocable<decltype(fn), const Node_t&>
	{
		ready_list.iter(fn);
		blocked_list.iter(fn);
	}

	inline TCB_t* find_by_id(Tid_t id)
	{
		TCB_t* res = nullptr;

		auto fetch = [id, &res](const Node_t& node) {
			auto& tcb = (TCB_t&) node;
			if (tcb.get_tid() == id) {
				res = &tcb;
				return;
			}
		};

		for_all_tasks(fetch);
		return res;
	}

	inline TCB_t* find_by_name(Name_t name)
	{
		TCB_t* res = nullptr;

		auto fetch = [name, &res](const Node_t& node) {
			auto& tcb = (TCB_t&) node;
			if (tcb.get_name() == name) {
				res = &tcb;
				return;
			}
		};

		for_all_tasks(fetch);
		return res;
	}

	inline TCB_t* find_by(auto info)
	{
		using nuts::Same;

		if constexpr (Same<decltype(info), Tid_t>) {
			return find_by_id(info);
		}

		if constexpr (Same<decltype(info), Name_t>) {
			return find_by_name(info);
		}
	}

	inline void print_name()
	{
		MOS_DISABLE_IRQ();
		printf("%s\n", curTCB->get_name());
		MOS_ENABLE_IRQ();
	}

	inline void print_all_tasks()
	{
		if (Task::num() <= 1)// If only idle exists, just return.
			return;

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
			}
		};

		// Print to screen
		static auto prts = [](const Node_t& node) {
			auto& tcb = (TCB_t&) node;
			printf("#%-2d %-10s %-5d %-9s %2d%%\n",
			       tcb.get_tid(),
			       tcb.get_name(),
			       (int8_t) tcb.get_priority(),
			       stos(tcb.get_status()),
			       tcb.page_usage());
		};

		MOS_DISABLE_IRQ();
		printf("=====================================\n");
		for_all_tasks(prts);
		printf("=====================================\n");
		MOS_ENABLE_IRQ();
	}

	__attribute__((always_inline)) inline void
	delay_ms(const uint32_t n, const uint32_t unit = 1000)
	{
		Util::delay(n, unit);
	}
}

#endif