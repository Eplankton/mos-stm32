#ifndef _MOS_DATATYPE_
#define _MOS_DATATYPE_

#include "util.hpp"
#include "macro.hpp"

namespace MOS::DataType
{
	template <uint32_t SIZE>
	struct BitMap_t
	{
		static constexpr uint32_t NUM = (SIZE + 31) / 32;
		uint32_t data[NUM]            = {0};

		BitMap_t() = default;

		void set(uint32_t pos)
		{
			uint32_t index = pos / 32;
			uint32_t bit   = pos % 32;
			data[index] |= (1 << bit);
		}

		void reset(uint32_t pos)
		{
			uint32_t index = pos / 32;
			uint32_t bit   = pos % 32;
			data[index] &= ~(1 << bit);
		}

		bool test(uint32_t pos) const
		{
			uint32_t index = pos / 32;
			uint32_t bit   = pos % 32;
			return (data[index] & (1 << bit)) != 0;
		}
	};

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

		__attribute__((always_inline)) inline uint32_t
		size() const { return len; }

		__attribute__((always_inline)) inline bool
		empty() const { return size() == 0; }

		__attribute__((always_inline)) inline const Node_t*
		end() const { return &head; }

		__attribute__((always_inline)) inline Node_t*
		begin() const { return head.next; }

		void add(Node_t& node)
		{
			Node_t* last = head.prev;
			node.next    = &head;
			node.prev    = last;
			head.prev    = &node;
			last->next   = &node;
			len++;
		}

		void remove(Node_t& node)
		{
			Node_t* prevNode = node.prev;
			Node_t* nextNode = node.next;
			prevNode->next   = nextNode;
			nextNode->prev   = prevNode;
			node.next        = &node;
			node.prev        = &node;
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
		volatile bool is_used          = false;
		uint32_t raw[Macro::PAGE_SIZE] = {0};
	};

	struct __attribute__((packed)) TCB_t
	{
		using Tid_t       = int16_t;
		using Self_t      = TCB_t;
		using SelfPtr_t   = TCB_t*;
		using ParentPtr_t = TCB_t*;
		using StackPtr_t  = uint32_t*;
		using Node_t      = list_node_t;
		using PagePtr_t   = Page_t*;
		using Ret_t       = void;
		using Argv_t      = void*;
		using Fn_t        = Ret_t (*)(Argv_t);
		using Prior_t     = int8_t;
		using Name_t      = const char*;

		enum class Status_t
		{
			READY,
			RUNNING,
			BLOCKED,
			TERMINATED,
		};

		// Don't change the offset of node and sp, it's important for context switch routine
		Node_t node;
		StackPtr_t sp = nullptr;

		// Add more members here
		Tid_t tid          = -1;
		Fn_t fn            = nullptr;
		Argv_t argv        = nullptr;
		Prior_t priority   = 15;// Low-High = 15-0
		PagePtr_t page     = nullptr;
		ParentPtr_t parent = nullptr;
		Status_t status    = Status_t::TERMINATED;
		Name_t name        = "";

		TCB_t() = default;
		TCB_t(Fn_t fn,
		      Argv_t argv      = nullptr,
		      Prior_t pr       = 15,
		      const char* name = ""): fn(fn), argv(argv),
		                              priority(pr), name(name) {}

		__attribute__((always_inline)) inline void
		set_tid(Tid_t id) volatile
		{
			tid = id;
		}

		__attribute__((always_inline)) inline Tid_t
		get_tid() volatile const
		{
			return tid;
		}

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
			page->is_used = false;
			page          = nullptr;
		}

		__attribute__((always_inline)) inline bool
		empty() volatile const
		{
			return fn == nullptr;
		}

		__attribute__((always_inline)) inline uint32_t
		page_usage() volatile const
		{
			const uint32_t atu = ((uint32_t) &page->raw[Macro::PAGE_SIZE] - (uint32_t) sp + sizeof(TCB_t)) / 4;
			return atu * 100 / Macro::PAGE_SIZE;
		}
	};
}
#endif