#ifndef _MOS_DATA_TYPE_
#define _MOS_DATA_TYPE_

#include "concepts.hpp"
#include "util.hpp"
#include "macro.hpp"

namespace MOS::DataType
{
	template <size_t N>
	struct RxBuffer
	{
		char raw[N];
		volatile int32_t index = 0;

		__attribute__((always_inline)) inline auto
		c_str() const { return raw; }

		__attribute__((always_inline)) inline bool
		full() const volatile { return index >= N; }

		__attribute__((always_inline)) inline bool
		empty() const volatile { return index == 0; }

		__attribute__((always_inline)) inline void
		add(char ch) volatile { raw[index++] = ch; }

		// If now empty
		__attribute__((always_inline)) inline char
		back() const volatile { return empty() ? '\0' : raw[index - 1]; }

		__attribute__((always_inline)) inline void
		pop() volatile
		{
			if (!empty()) {
				raw[--index] = '\0';
			}
		}

		__attribute__((always_inline)) inline void
		clear()
		{
			while (--index >= 0) {
				raw[index] = '\0';
			}
			index = 0;
		}
	};

	class QueueBase
	{
	protected:
		uint32_t m_len = 0, m_head = 0, m_tail = 0;

		inline void* front(void* src, const uint32_t size) volatile
		{
			return (void*) ((uint32_t) (src) + m_head * size);
		}

		inline void* back(void* src, const uint32_t size) volatile
		{
			return (void*) ((uint32_t) (src) + (m_len - 1) * size);
		}

		inline void
		push(void* dest, const void* src,
		     const u32 size, const u32 N) volatile
		{
			if (!full()) {
				Util::memcpy(dest, src, size);
				m_tail = (m_tail + 1) % N;
				m_len++;
			}
		}

		inline void pop(uint32_t N) volatile
		{
			if (!empty()) {
				m_head = (m_head + 1) % N;
				m_len--;
			}
		}

	public:
		inline uint32_t size() const volatile { return m_len; }
		inline bool full() const volatile { return m_len != 0 && m_head == m_tail; }
		inline bool empty() const volatile { return m_len == 0; }
		inline void clear() volatile { m_head = m_tail = m_len = 0; }
	};

	template <typename T, uint32_t N, typename Base = QueueBase>
	class Queue : public Base
	{
	protected:
		using Base::front;
		using Base::back;
		using Base::push;
		using Base::pop;
		using Base::m_tail;
		using Base::m_head;

		using value_type = T;
		using value_ptr  = T*;
		using value_ref  = T&;

		value_type m_data[N];

	public:
		using Base::size;
		using Base::full;
		using Base::empty;
		using Base::clear;

		Queue()  = default;
		~Queue() = default;

		static inline constexpr uint32_t capacity() { return N; }

		inline auto data() const volatile { return m_data; }

		inline value_ref front() volatile
		{
			return *(value_ptr) front((void*) m_data, sizeof(T));
		}

		inline value_ref back() volatile
		{
			return *(value_ptr) back((void*) m_data, sizeof(T));
		}

		inline void pop() volatile { pop(N); }

		inline void push(const T& val) volatile
		{
			push((void*) (m_data + m_tail), (void*) &val, sizeof(T), N);
		}

		inline value_ref serve() volatile
		{
			auto& tmp = front();
			pop();
			return tmp;
		}

		inline void iter(auto&& fn) volatile
		{
			if (empty()) return;
			else {
				auto i = m_head;
				do {
					fn(m_data[i]);
					i = (i + 1) % N;
				} while (i != m_tail);
			}
		}

		inline void iter(auto&& fn) const volatile
		{
			if (empty()) return;
			else {
				auto i = m_head;
				do {
					fn(m_data[i]);
					i = (i + 1) % N;
				} while (i != m_tail);
			}
		}
	};

	template <size_t N>
	struct BitMap_t
	{
		static constexpr uint32_t NUM = (N + 31) / 32;
		uint32_t data[NUM]            = {0};

		BitMap_t() = default;

		inline void set(uint32_t pos)
		{
			uint32_t index = pos / 32;
			uint32_t bit   = pos % 32;
			data[index] |= (1 << bit);
		}

		inline void reset(uint32_t pos)
		{
			uint32_t index = pos / 32;
			uint32_t bit   = pos % 32;
			data[index] &= ~(1 << bit);
		}

		inline bool test(uint32_t pos) const
		{
			uint32_t index = pos / 32;
			uint32_t bit   = pos % 32;
			return (data[index] & (1 << bit)) != 0;
		}
	};

	struct ListNode_t
	{
		using SelfPtr_t = ListNode_t*;

		SelfPtr_t prev, next;
		// Make it self-linked
		ListNode_t(): prev(this), next(this) {}
	};

	struct List_t
	{
		using Node_t    = ListNode_t;
		using NodePtr_t = Node_t::SelfPtr_t;

		Node_t head;
		uint32_t len = 0;

		__attribute__((always_inline)) inline uint32_t
		size() const { return len; }

		__attribute__((always_inline)) inline bool
		empty() const { return size() == 0; }

		__attribute__((always_inline)) inline NodePtr_t
		begin() const { return head.next; }

		__attribute__((always_inline)) inline NodePtr_t
		end() const { return (NodePtr_t) &head; }

		__attribute__((always_inline)) inline void
		iter(auto&& fn) const
		    requires Invocable<decltype(fn), const Node_t&>
		{
			for (auto it = begin(); it != end(); it = it->next) {
				fn(*it);
			}
		}

		__attribute__((always_inline)) inline void
		iter_until(auto&& fn) const
		    requires Invocable<decltype(fn), const Node_t&>
		{
			for (auto it = begin(); it != end(); it = it->next) {
				if (fn(*it)) return;
			}
		}

		void insert(Node_t& node, NodePtr_t pos)
		{
			if (pos == nullptr)
				return;
			node.next       = pos;
			node.prev       = pos->prev;
			pos->prev->next = &node;
			pos->prev       = &node;
			len += 1;
		}

		void insert_in_order(Node_t& node, auto&& cmp)
		    requires Invocable<decltype(cmp), const Node_t&, const Node_t&>
		{
			auto st = begin();
			while (st != end() && cmp(*st, node)) {
				st = st->next;
			}
			insert(node, st);
		}

		__attribute__((always_inline)) inline void
		add(Node_t& node)
		{
			insert(node, &head);
		}

		void remove(Node_t& node)
		{
			NodePtr_t prev = node.prev;
			NodePtr_t next = node.next;
			prev->next     = next;
			next->prev     = prev;
			node.next      = &node;
			node.prev      = &node;
			len -= 1;
		}

		__attribute__((always_inline)) inline void
		send_to(Node_t& node, List_t& dest)
		{
			remove(node);
			dest.add(node);
		}

		__attribute__((always_inline)) inline void
		send_to_in_order(Node_t& node, List_t& dest, auto&& cmp)
		{
			remove(node);
			dest.insert_in_order(node, cmp);
		}

		__attribute__((always_inline)) inline void
		re_insert(Node_t& node, auto&& cmp)
		{
			send_to_in_order(node, *this, cmp);
		}

		__attribute__((always_inline)) inline bool
		contains(const Node_t& node)
		{
			bool flag = false;
			iter([&node, &flag](const Node_t& x) {
				if (&node == &x) {
					flag = true;
					return;
				}
			});
			return flag;
		}
	};

	struct Page_t
	{
		using PagePtr_t = Page_t*;

		volatile bool used             = false;
		uint32_t raw[Macro::PAGE_SIZE] = {0};

		__attribute__((always_inline)) inline bool
		is_used() const { return used; }
	};

	struct __attribute__((packed)) TCB_t
	{
		using Tid_t       = int16_t;
		using Self_t      = TCB_t;
		using SelfPtr_t   = TCB_t*;
		using TcbPtr_t    = SelfPtr_t;
		using ParentPtr_t = TcbPtr_t;
		using StackPtr_t  = uint32_t*;
		using Node_t      = ListNode_t;
		using PagePtr_t   = Page_t::PagePtr_t;
		using Tick_t      = uint32_t;
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

		using enum Status_t;

		// Don't change the offset of node and sp, it's important for context switch routine
		Node_t node;
		StackPtr_t sp = nullptr;

		// Add more members here
		Tid_t tid = -1;

		// Only for debug
		Fn_t fn     = nullptr;
		Argv_t argv = nullptr;
		Name_t name = "";

		Prior_t priority   = 15;// Low-High = 15-0
		PagePtr_t page     = nullptr;
		ParentPtr_t parent = nullptr;
		Status_t status    = TERMINATED;
		Tick_t time_slice  = Macro::TIME_SLICE;
		Tick_t delay_ticks = 0;

		TCB_t() = default;
		TCB_t(Fn_t fn, Argv_t argv = nullptr,
		      Prior_t pr = 15, Name_t name = "")
		    : fn(fn), argv(argv), priority(pr), name(name) {}

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
			page->raw[Macro::PAGE_SIZE - 1U] = xpsr_val;
		}

		__attribute__((always_inline)) inline void
		set_PC(uint32_t pc_val) volatile
		{
			page->raw[Macro::PAGE_SIZE - 2U] = pc_val;
		}

		__attribute__((always_inline)) inline void
		set_LR(uint32_t lr_val) volatile
		{
			page->raw[Macro::PAGE_SIZE - 3U] = lr_val;
		}

		__attribute__((always_inline)) inline void
		set_param(uint32_t param) volatile
		{
			page->raw[Macro::PAGE_SIZE - 8U] = param;
		}

		__attribute__((always_inline)) inline void
		set_delay_ticks(Tick_t ticks) volatile
		{
			delay_ticks = ticks;
		}

		__attribute__((always_inline)) inline void
		attach_page(PagePtr_t page_ptr) volatile
		{
			page       = page_ptr;
			page->used = true;
		}

		__attribute__((always_inline)) inline void
		release_page() volatile
		{
			page->used = false;
			page       = nullptr;
		}

		__attribute__((always_inline)) inline uint32_t
		page_usage() volatile const
		{
			const uint32_t stk_top = (uint32_t) &page->raw[Macro::PAGE_SIZE];
			const uint32_t atu     = (stk_top - (uint32_t) sp + sizeof(TCB_t));
			return atu * 25 / Macro::PAGE_SIZE;
		}

		__attribute__((always_inline)) inline uint32_t
		stack_usage() volatile const
		{
			const uint32_t stk_top = (uint32_t) &page->raw[Macro::PAGE_SIZE];
			const uint32_t atu     = (stk_top - (uint32_t) sp);
			return atu * 25 / (Macro::PAGE_SIZE - sizeof(TCB_t) / 4);
		}

		__attribute__((always_inline)) static inline bool
		priority_cmp(const Node_t& lhs, const Node_t& rhs)
		{
			return ((TCB_t&) lhs).get_priority() < ((TCB_t&) rhs).get_priority();
		}

		__attribute__((always_inline)) static inline bool
		priority_equal(const Node_t& lhs, const Node_t& rhs)
		{
			return ((TCB_t&) lhs).get_priority() == ((TCB_t&) rhs).get_priority();
		}

		__attribute__((always_inline)) static inline TcbPtr_t
		build(PagePtr_t page_ptr, Fn_t fn, Argv_t argv = nullptr,
		      Prior_t pr = 15, Name_t name = "")
		{
			auto tcb = new (page_ptr->raw) TCB_t {fn, argv, pr, name};
			tcb->attach_page(page_ptr);
			return tcb;
		}
	};

	struct DebugTasks
	{
		using TcbPtr_t = TCB_t::TcbPtr_t;

		volatile TcbPtr_t raw[Macro::MAX_TASK_NUM] = {nullptr};
		volatile uint32_t len                      = 0;

		__attribute__((always_inline)) inline auto
		size() const volatile { return len; }

		__attribute__((always_inline)) inline void
		add(TcbPtr_t tcb) volatile
		{
			for (auto& pt: raw) {
				if (pt == nullptr) {
					pt = tcb;
					len += 1;
					return;
				}
			}
		}

		__attribute__((always_inline)) inline void
		remove(TcbPtr_t tcb) volatile
		{
			for (auto& pt: raw) {
				if (pt == tcb) {
					pt = nullptr;
					len -= 1;
					return;
				}
			}
		}

		__attribute__((always_inline)) inline void
		iter(auto&& fn) volatile
		{
			for (auto& pt: raw) {
				if (pt != nullptr) {
					fn(pt);
				}
			}
		}

		__attribute__((always_inline)) inline void
		iter_until(auto&& fn) volatile
		{
			for (auto& pt: raw) {
				if (pt != nullptr) {
					if (fn(pt)) return;
				}
			}
		}
	};
}

#endif