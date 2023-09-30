#ifndef _MOS_
#define _MOS_

#include "main.h"

// placement new
inline void*
operator new(size_t, void* addr) noexcept { return addr; }

extern "C" __attribute__((naked, used)) void ContextSwitch(void);

namespace MOS
{
	namespace Macro
	{
		constexpr uint32_t MAX_TASK_NUM = 16;
		constexpr uint32_t PAGE_SIZE    = 64;// 64x4 = 256 bytes
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

			inline uint32_t size() const { return len; }
			inline bool empty() const { return size() == 0; }
			inline Node_t* begin() const { return head.next; }

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
			using StackPtr_t    = uint32_t*;// 32-bit address
			using Node_t        = list_node_t;
			using PagePtr_t     = Page_t*;
			using SubTaskList_t = list_t;
			using Ret_t         = void;
			using Argv_t        = void*;
			using Fn_t          = Ret_t (*)(Argv_t);
			using Prior_t       = uint8_t;// 0~15
			using Name_t        = const char*;

			using State_t = enum {
				READY,
				RUNNING,
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
			State_t state    = TERMINATED;
			Name_t name      = "";

			SelfPtr_t parent = nullptr;

			TCB_t() = default;
			TCB_t(Fn_t fn,
			      Argv_t argv      = nullptr,
			      Prior_t pr       = 15,
			      const char* name = ""): func(fn), argv(argv),
			                              priority(pr), name(name) {}

			inline void set_state(State_t new_state) volatile
			{
				state = new_state;
			}

			inline bool empty() volatile const
			{
				return func == nullptr;
			}
		};
	}

	namespace GlobalRes
	{
		using namespace DataType;
		using namespace Macro;

		Page_t PAGES[MAX_TASK_NUM];
		list_t TCBs;

		extern "C" {
		// Put it in extern "C" because the name is referred in asm("") for name demangling.
		// Anytime when a task is running, the curTCB points to its function.
		__attribute__((used)) volatile TCB_t* curTCB = nullptr;
		}
	}

	namespace Task
	{
		using namespace Macro;
		using namespace DataType;
		using namespace GlobalRes;

		inline TCB_t* create(TCB_t::Fn_t&& fn, TCB_t::Argv_t argv = nullptr,
		                     TCB_t::Prior_t pr = 15, TCB_t::Name_t name = "")
		{
			// Disable interrupt to enter critical section
			DISABLE_IRQ();

			TCB_t::PagePtr_t p = nullptr;

			if (TCBs.size() >= MAX_TASK_NUM) {
				// Failed
				return nullptr;
			}

			// Until find an unused page
			for (auto& page: PAGES) {
				if (!page.is_used) {
					p = &page;
					break;
				}
			}

			// Construct a TCB inside the page
			TCB_t& tcb = *new (p->raw) TCB_t {fn, argv, pr, name};
			tcb.page   = p;
			p->is_used = true;

			// Setup the stack to hold task context.
			// Remember it is a descending stack and a context consists of 16 registers.
			tcb.sp = &p->raw[PAGE_SIZE - 16];

			// Set the 'T' bit in stacked xPSR to '1' to notify processor on exception return about the thumb state.
			// V6-m and V7-m cores can only support thumb state hence this should be always set to '1'.
			tcb.page->raw[PAGE_SIZE - 1] = 0x01000000;

			// Set the stacked PC to point to the task
			tcb.page->raw[PAGE_SIZE - 2] = (uint32_t) tcb.func;

			// Set TCB to running
			tcb.set_state(TCB_t::RUNNING);

			// Add to TCBs
			TCBs.add(tcb.node);

			// Add to sub-task list
			tcb.parent = (TCB_t*) curTCB;

			// Enable interrupt, leave critical section
			ENABLE_IRQ();

			return &tcb;
		}

		// __attribute__((naked, used)) inline void yield()
		// {
		// 	asm("B     ContextSwitch");
		// }

		inline void terminate()
		{
			// Disable interrupt to enter critical section
			DISABLE_IRQ();

			auto& cur_tcb = (TCB_t&) *curTCB;

			// Remove the task from the task list and kids list
			TCBs.remove(cur_tcb.node);

			// Mark the page as not used
			cur_tcb.page->is_used = false;

			// Clear the task's page
			///

			// Reset the TCB to default
			new (&cur_tcb) TCB_t {};

			// Enable interrupt, leave critical section
			ENABLE_IRQ();

			while (true) {
				// Never
			}
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
			// Make curTCB points to task0
			curTCB = reinterpret_cast<TCB_t*>(TCBs.begin());
			SysTick_t::config(1000);
			init();
		}

		// Custom Scheduler Policy, return TCB*
		template <Policy policy = RoundRobin>
		inline TCB_t* next()
		{
			if constexpr (policy == Policy::RoundRobin) {
				if (curTCB->empty()) {
					return reinterpret_cast<TCB_t*>(TCBs.begin());
				}

				// Return the next task of curTCB
				if (curTCB->node.next != &TCBs.head) {
					// The next task is not the head, return it
					return reinterpret_cast<TCB_t*>(curTCB->node.next);
				}
				else {
					// The next task is the head, return the first task in the list
					return reinterpret_cast<TCB_t*>(TCBs.head.next);
				}
			}

			if constexpr (policy == Policy::PreemptivePriority) {
				// Find the task with the highest priority
				TCB_t* tmp = reinterpret_cast<TCB_t*>(TCBs.begin());

				for (auto st = (TCB_t*) tmp->node.next;
				     st != (TCB_t*) &TCBs.head;
				     st = (TCB_t*) st->node.next) {
					if (st->priority < tmp->priority) {
						tmp = st;
					}
				}

				return tmp;
			}
		}

		__attribute__((used)) extern "C" inline TCB_t*
		nextTCB()// Don't change this name which used in asm
		{
			return next<Policy::RoundRobin>();
		}
	}
}

#endif
