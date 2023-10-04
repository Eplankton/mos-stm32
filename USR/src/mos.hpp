#ifndef _MOS_
#define _MOS_

#include "main.h"

#ifdef _MOS_DEBUG_INFO_
#define debug_info(format, ...) printf(format, ##__VA_ARGS__)
#else
#define debug_info(format, ...) ((void) 0)
#endif

// placement new
inline void*
operator new(size_t, void* addr) noexcept { return addr; }

namespace MOS
{
	namespace Macro
	{
		constexpr uint32_t MAX_TASK_NUM = 16;
		constexpr uint32_t PAGE_SIZE    = 256;
	}

	namespace DataType
	{
		struct list_node_t
		{
			using SelfPtr_t = list_node_t*;
			SelfPtr_t prev, next;
			list_node_t(): prev(this), next(this) {}
		};

		struct list_t
		{
			using Node_t = list_node_t;

			Node_t head;
			uint32_t len = 0;

			__attribute__((always_inline)) inline uint32_t size() const { return len; }
			__attribute__((always_inline)) inline bool empty() const { return size() == 0; }
			__attribute__((always_inline)) inline Node_t* begin() const { return head.next; }
			__attribute__((always_inline)) inline const Node_t* end() const { return &head; }

			void add(Node_t& node)
			{
				Node_t* last = head.prev;

				node.next = &head;
				node.prev = last;

				head.prev  = &node;
				last->next = &node;

				len++;
			}

			void remove(Node_t& node)
			{
				Node_t* prevNode = node.prev;
				Node_t* nextNode = node.next;

				prevNode->next = nextNode;
				nextNode->prev = prevNode;

				node.next = &node;
				node.prev = &node;

				len--;
			}

			__attribute__((always_inline)) inline void
			iter(auto&& fn) const
			{
				for (auto it = begin(); it != end(); it = it->next) {
					fn(*it);
				}
			}
		};

		struct Page_t
		{
			bool is_used                   = false;
			uint32_t raw[Macro::PAGE_SIZE] = {0};
		};

		struct __attribute__((packed)) TCB_t
		{
			using Self_t        = TCB_t;
			using SelfPtr_t     = TCB_t*;
			using ParentPtr_t   = TCB_t*;
			using StackPtr_t    = uint32_t*;
			using Node_t        = list_node_t;
			using PagePtr_t     = Page_t*;
			using SubTaskList_t = list_t;
			using Ret_t         = void;
			using Argv_t        = void*;
			using Fn_t          = Ret_t (*)(Argv_t);
			using Prior_t       = uint8_t;// L:15 ~ HIGH:0
			using Name_t        = const char*;

			using Status_t = enum {
				READY,
				RUNNING,
				BLOCKED,
				TERMINATED,
			};

			// For TCBList
			Node_t node;

			// Don't change the sp offset, it's important
			StackPtr_t sp = nullptr;

			// Add more members here
			Fn_t func        = nullptr;
			Argv_t argv      = nullptr;
			Prior_t priority = 15;
			PagePtr_t page   = nullptr;
			Status_t status  = TERMINATED;
			Name_t name      = "";

			ParentPtr_t parent = nullptr;

			TCB_t() = default;
			TCB_t(Fn_t fn,
			      Argv_t argv      = nullptr,
			      Prior_t pr       = 15,
			      const char* name = ""): func(fn), argv(argv),
			                              priority(pr), name(name) {}

			__attribute__((always_inline)) inline SelfPtr_t
			next() volatile const
			{
				return (SelfPtr_t) node.next;
			}

			__attribute__((always_inline)) inline void
			deinit() volatile
			{
				new ((void*) this) TCB_t {};
			}

			__attribute__((always_inline)) inline void
			set_parent(ParentPtr_t parent_ptr) volatile
			{
				parent = parent_ptr;
			}

			__attribute__((always_inline)) inline ParentPtr_t
			get_parent() volatile const
			{
				return parent;
			}

			__attribute__((always_inline)) inline void
			set_status(Status_t new_status) volatile
			{
				status = new_status;
			}

			__attribute__((always_inline)) inline Status_t
			get_status() volatile const
			{
				return status;
			}

			__attribute__((always_inline)) inline bool
			is_status(Status_t expected) volatile const
			{
				return get_status() == expected;
			}

			__attribute__((always_inline)) inline Name_t
			get_name() volatile const
			{
				return name;
			}

			__attribute__((always_inline)) inline void
			set_priority(Prior_t pr) volatile
			{
				priority = pr;
			}

			__attribute__((always_inline)) inline Prior_t
			get_priority() volatile const
			{
				return priority;
			}

			__attribute__((always_inline)) inline void
			set_SP(StackPtr_t sp_val) volatile
			{
				sp = sp_val;
			}

			__attribute__((always_inline)) inline void
			set_xPSR(uint32_t xpsr_val) volatile
			{
				page->raw[Macro::PAGE_SIZE - 1] = xpsr_val;
			}

			__attribute__((always_inline)) inline void
			set_PC(uint32_t pc_val) volatile
			{
				page->raw[Macro::PAGE_SIZE - 2] = pc_val;
			}

			__attribute__((always_inline)) inline void
			attach_page(PagePtr_t page_ptr) volatile
			{
				page          = page_ptr;
				page->is_used = true;
			}

			__attribute__((always_inline)) inline void
			release_page() volatile
			{
				page          = nullptr;
				page->is_used = false;
			}

			__attribute__((always_inline)) inline bool
			empty() volatile const
			{
				return func == nullptr;
			}

			__attribute__((always_inline)) inline StackPtr_t
			stack_top() volatile const
			{
				return &page->raw[Macro::PAGE_SIZE - 16];
			}
		};
	}

	namespace GlobalRes
	{
		using namespace Macro;
		using namespace DataType;

		Page_t PAGES[MAX_TASK_NUM];
		list_t ready_list, blocked_list;

		extern "C" {
		// Put it in extern "C" because the name is referred in asm("") and don't change it.
		// Anytime when a task is running, the curTCB points to its function.
		__attribute__((used)) volatile TCB_t* curTCB = nullptr;
		}
	}

	namespace Task
	{
		using namespace GlobalRes;

		inline TCB_t* create(TCB_t::Fn_t&& fn   = nullptr,
		                     TCB_t::Argv_t argv = nullptr,
		                     TCB_t::Prior_t pr  = 15,
		                     TCB_t::Name_t name = "")
		{
			// Disable interrupt to enter critical section
			DISABLE_IRQ();

			TCB_t::PagePtr_t p = nullptr;

			if (ready_list.size() >= MAX_TASK_NUM) {
				// Failed, no enough page
				return nullptr;
			}

			for (auto& page: PAGES) {
				if (!page.is_used) {
					p = &page;
					break;
				}
			}

			// Construct a TCB inside the page
			auto& tcb = *new (p->raw) TCB_t {fn, argv, pr, name};

			// Set task page
			tcb.attach_page(p);

			// Setup the stack to hold task context.
			// Remember it is a descending stack and a context consists of 16 registers.
			tcb.set_SP(tcb.stack_top());

			// Set the 'T' bit in stacked xPSR to '1' to notify processor on exception return about the thumb state.
			// V6-m and V7-m cores can only support thumb state hence it should always be set to '1'.
			tcb.set_xPSR((uint32_t) 0x01000000);

			// Set the stacked PC to point to the task
			tcb.set_PC((uint32_t) tcb.func);

			// Set TCB to running
			tcb.set_status(TCB_t::READY);

			// Add parent
			tcb.set_parent((TCB_t*) curTCB);

			// Add to TCBs list
			ready_list.add(tcb.node);

			// Enable interrupt, leave critical section
			ENABLE_IRQ();

			return &tcb;
		}

		__attribute__((always_inline)) inline void yield()
		{
			// Trigger SysTick Interrupt -> SysTick_Handler
			SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
		}

		inline void block()
		{
			// Disable interrupt to enter critical section
			DISABLE_IRQ();

			auto& cur_tcb = (TCB_t&) *curTCB;

			// Remove the task from the task list and kids list
			cur_tcb.set_status(TCB_t::BLOCKED);
			ready_list.remove(cur_tcb.node);
			blocked_list.add(cur_tcb.node);

			// Enable interrupt, leave critical section
			ENABLE_IRQ();

			yield();
		}

		inline void resume(TCB_t* blocked_tcb)
		{
			DISABLE_IRQ();
			if (!blocked_tcb->is_status(TCB_t::BLOCKED))
				return;
			blocked_list.remove(blocked_tcb->node);
			ready_list.add(blocked_tcb->node);
			blocked_tcb->set_status(TCB_t::READY);
			ENABLE_IRQ();
		}

		inline void terminate()
		{
			// Disable interrupt to enter critical section
			DISABLE_IRQ();

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
			ENABLE_IRQ();

			while (true) {
				// Never goes to scheduling again...
			}
		}

		__attribute__((always_inline)) inline void
		print_name()
		{
			DISABLE_IRQ();
			printf("%s\n", curTCB->name);
			ENABLE_IRQ();
		}

		inline void print_all_tasks()
		{
			// Status to String
			static auto stos = [](const TCB_t::Status_t s) constexpr {
				switch (s) {
					case TCB_t::READY:
						return "READY";
					case TCB_t::RUNNING:
						return "RUNNING";
					case TCB_t::BLOCKED:
						return "BLOCKED";
					case TCB_t::TERMINATED:
						return "TERMINATED";
				}
			};

			static auto prts = [](TCB_t::Node_t& node) {
				auto& tcb = (TCB_t&) node;
				printf(" %s\t P: %d\t S: %s\n",
				       tcb.get_name(),
				       (int8_t) tcb.get_priority(),
				       stos(tcb.get_status()));
			};

			DISABLE_IRQ();
			printf("===================================\n");
			ready_list.iter(prts);
			blocked_list.iter(prts);
			printf("===================================\n");
			ENABLE_IRQ();
		}

		__attribute__((always_inline)) inline void
		delay_ms(const uint32_t n, const uint32_t unit = 1000)
		{
			nuts::delay(n, unit);
		}
	}

	namespace Scheduler
	{
		using namespace DataType;
		using namespace GlobalRes;

		enum Policy
		{
			RoundRobin,
			PreemptivePriority,
		};

		inline __attribute__((naked)) void init()
		{
			DISABLE_IRQ();

			// R0 contains the address of currentPt
			asm("LDR     R0, =curTCB");

			// R2 contains the address in currentPt(value of currentPt)
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

			// Enable interrupts
			ENABLE_IRQ();

			asm("BX      LR");
		}

		inline void launch()
		{
			curTCB = reinterpret_cast<TCB_t*>(ready_list.begin());
			curTCB->set_status(TCB_t::RUNNING);
			SysTick_t::config(1000);
			init();
		}

		// Custom Scheduler Policy, return TCB* as the next task to run
		template <Policy policy = RoundRobin>
		inline TCB_t* next_tcb()
		{
			static auto switch_to_next = [](void* x) {
				auto tmp = reinterpret_cast<TCB_t*>(x);
				tmp->set_status(TCB_t::RUNNING);
				return tmp;
			};

			if constexpr (policy == RoundRobin) {
				if (curTCB->is_status(TCB_t::BLOCKED) || curTCB->empty()) {
					return switch_to_next(ready_list.begin());
				}
				else {
					// curTCB -> READY
					curTCB->set_status(TCB_t::READY);

					// Return the next task of curTCB
					if (curTCB->next() != (TCB_t*) ready_list.end()) {
						// The next task is not the head, return it
						return switch_to_next(curTCB->next());
					}
					else {
						// The next task is the head, return the first task in the list
						return switch_to_next(ready_list.begin());
					}
				}
			}

			if constexpr (policy == PreemptivePriority) {
				// Find the task with the highest priority
				auto res = reinterpret_cast<TCB_t*>(ready_list.begin());

				ready_list.iter([&](TCB_t::Node_t& node) {
					auto& tmp = (TCB_t&) (node);
					if (tmp.get_priority() < res->get_priority()) {
						res = &tmp;
					}
				});

				return switch_to_next(res);
			}
		}

		__attribute__((used, always_inline)) extern "C" inline TCB_t*
		nextTCB()// Don't change this name which used in asm
		{
			return next_tcb<Policy::RoundRobin>();
		}
	}
}

#endif
