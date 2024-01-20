#ifndef _MOS_DATA_TYPE_
#define _MOS_DATA_TYPE_

#include "concepts.hpp"
#include "utils.hpp"
#include "macro.hpp"

namespace MOS::DataType
{
	template <size_t N>
	struct RxBuffer_t
	{
		using Raw_t = char[N];
		using Cnt_t = volatile int32_t;

		Raw_t raw;
		Cnt_t index = 0;

		MOS_INLINE inline auto
		c_str() const { return raw; }

		MOS_INLINE inline bool
		full() const volatile { return index >= N; }

		MOS_INLINE inline bool
		empty() const volatile { return index == 0; }

		MOS_INLINE inline void
		add(char ch) volatile { raw[index++] = ch; }

		MOS_INLINE inline char
		back() const volatile { return empty() ? '\0' : raw[index - 1]; }

		MOS_INLINE inline void
		pop() volatile
		{
			if (!empty()) {
				raw[--index] = '\0';
			}
		}

		MOS_INLINE inline void
		clear() volatile
		{
			Utils::memset((void*) raw, 0, sizeof(raw));
			index = 0;
		}
	};

	class QueueImpl_t
	{
	protected:
		uint32_t m_len = 0, m_head = 0, m_tail = 0;

		MOS_INLINE inline void*
		front(void* src, const uint32_t size) volatile
		{
			return (void*) ((uint32_t) (src) + m_head * size);
		}

		MOS_INLINE inline void*
		back(void* src, const uint32_t size) volatile
		{
			return (void*) ((uint32_t) (src) + (m_len - 1) * size);
		}

		inline void
		push(void* dest, const void* src, const u32 size, const u32 N) volatile
		{
			if (!full()) {
				Utils::memcpy(dest, src, size);
				m_tail = (m_tail + 1) % N;
				m_len += 1;
			}
		}

		MOS_INLINE inline void
		pop(uint32_t N) volatile
		{
			if (!empty()) {
				m_head = (m_head + 1) % N;
				m_len -= 1;
			}
		}

	public:
		MOS_INLINE inline uint32_t size() const volatile { return m_len; }
		MOS_INLINE inline bool full() const volatile { return m_len != 0 && m_head == m_tail; }
		MOS_INLINE inline bool empty() const volatile { return m_len == 0; }
		MOS_INLINE inline void clear() volatile { m_head = m_tail = m_len = 0; }
	};

	template <typename T, uint32_t N, typename Base = QueueImpl_t>
	class Queue_t : public Base
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

		Queue_t()  = default;
		~Queue_t() = default;

		static inline constexpr uint32_t capacity() { return N; }

		MOS_INLINE inline auto
		data() const volatile { return m_data; }

		MOS_INLINE inline value_ref
		front() volatile
		{
			return *(value_ptr) front((void*) m_data, sizeof(T));
		}

		MOS_INLINE inline value_ref
		back() volatile
		{
			return *(value_ptr) back((void*) m_data, sizeof(T));
		}

		MOS_INLINE inline void pop() volatile { pop(N); }

		MOS_INLINE inline void
		push(const T& val) volatile
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
		using Raw_t = uint32_t[(N + 31) / 32];

		Raw_t data = {0};

		MOS_INLINE inline BitMap_t() = default;

		MOS_INLINE inline void
		set(uint32_t pos)
		{
			uint32_t index = pos / 32;
			uint32_t bit   = pos % 32;
			data[index] |= (1 << bit);
		}

		MOS_INLINE inline void
		reset(uint32_t pos)
		{
			uint32_t index = pos / 32;
			uint32_t bit   = pos % 32;
			data[index] &= ~(1 << bit);
		}

		MOS_INLINE inline bool
		test(uint32_t pos) const
		{
			uint32_t index = pos / 32;
			uint32_t bit   = pos % 32;
			return (data[index] & (1 << bit)) != 0;
		}
	};

	struct ListNode_t
	{
		using Self_t    = ListNode_t;
		using SelfPtr_t = Self_t*;

		// Self-linked as default
		SelfPtr_t prev = this, next = this;
	};

	template <typename Fn, typename Ret = void>
	concept ListIterFn = Concepts::Invocable<Fn, Ret, const ListNode_t&>;

	template <typename Fn>
	concept NodeCmpFn = Concepts::Invocable<Fn, bool, const ListNode_t&, const ListNode_t&>;

	struct ListImpl_t
	{
		using Node_t    = ListNode_t;
		using NodePtr_t = Node_t::SelfPtr_t;

		Node_t head;
		uint32_t len = 0;

		MOS_INLINE inline uint32_t
		size() const { return len; }

		MOS_INLINE inline bool
		empty() const { return size() == 0; }

		MOS_INLINE inline NodePtr_t
		begin() const { return head.next; }

		MOS_INLINE inline NodePtr_t
		end() const { return (NodePtr_t) &head; }

		MOS_INLINE inline void
		iter(ListIterFn auto&& fn) const
		{
			for (auto it = begin();
			     it != end();
			     it = it->next) {
				fn(*it);
			}
		}

		MOS_INLINE inline void
		iter_mut(auto&& fn)
		{
			for (auto it = begin();
			     it != end();
			     it = it->next) {
				fn(*it);
			}
		}

		MOS_INLINE inline NodePtr_t
		iter_until(ListIterFn<bool> auto&& fn) const
		{
			for (auto it = begin();
			     it != end();
			     it = it->next) {
				if (fn(*it))
					return it;
			}
			return nullptr;
		}

		MOS_NO_INLINE void // Never inline because of instruction reordering
		insert(Node_t& node, NodePtr_t pos)
		{
			if (pos == nullptr)
				return;
			node.next       = pos;
			node.prev       = pos->prev;
			pos->prev->next = &node;
			pos->prev       = &node;
			len += 1;
		}

		MOS_NO_INLINE void // Never inline because of instruction reordering
		remove(Node_t& node)
		{
			NodePtr_t prev = node.prev, next = node.next;
			prev->next = next;
			next->prev = prev;
			node.next  = &node;
			node.prev  = &node;
			len -= 1;
		}

		MOS_INLINE inline void
		insert_in_order(Node_t& node, NodeCmpFn auto&& cmp)
		{
			auto st = begin();
			while (st != end() && cmp(*st, node)) {
				st = st->next;
			}
			insert(node, st);
		}

		MOS_INLINE inline void
		add(Node_t& node) { insert(node, end()); }

		MOS_INLINE inline void
		send_to(Node_t& node, ListImpl_t& dest)
		{
			remove(node);
			dest.add(node);
		}

		MOS_INLINE inline void
		send_to_in_order(Node_t& node, ListImpl_t& dest, NodeCmpFn auto&& cmp)
		{
			remove(node);
			dest.insert_in_order(node, cmp);
		}

		MOS_INLINE inline void
		re_insert(Node_t& node, NodeCmpFn auto&& cmp)
		{
			send_to_in_order(node, *this, cmp);
		}

		MOS_INLINE inline bool
		contains(const Node_t& node)
		{
			return iter_until([&node](const Node_t& x) {
				       return &node == &x;
			       }) != nullptr;
		}
	};

	struct Page_t
	{
		using Self_t    = Page_t;
		using SelfPtr_t = Self_t*;
		using PagePtr_t = SelfPtr_t;
		using Mark_t    = volatile bool;
		using Raw_t     = uint32_t[Macro::PAGE_SIZE];

		Mark_t used = false;
		Raw_t raw   = {0};

		MOS_INLINE inline bool
		is_used() const { return used; }

		MOS_INLINE inline auto
		first_stk_top() const
		{
			return &raw[Macro::PAGE_SIZE - 16];
		}
	};

	struct __attribute__((packed)) Tcb_t
	{
		using Self_t        = Tcb_t;
		using SelfPtr_t     = Tcb_t*;
		using TcbPtr_t      = SelfPtr_t;
		using ConstTcbPtr_t = const Tcb_t*;
		using ParentPtr_t   = TcbPtr_t;
		using StackPtr_t    = uint32_t*;
		using Node_t        = ListNode_t;
		using PagePtr_t     = Page_t::PagePtr_t;
		using Tid_t         = int16_t;
		using Tick_t        = uint32_t;
		using Ret_t         = void;
		using Argv_t        = void*;
		using Fn_t          = Ret_t (*)(Argv_t);
		using Prior_t       = int8_t;
		using Name_t        = const char*;

		enum class Status
		{
			READY,
			RUNNING,
			BLOCKED,
			TERMINATED,
		};

		// Don't change the offset of node and sp
		Node_t node;
		StackPtr_t sp = nullptr;

		// Add more members here
		Tid_t tid = -1;

		// Only for debug
		Fn_t fn     = nullptr;
		Argv_t argv = nullptr;
		Name_t name = "";

		Prior_t priority = Macro::PRI_MIN, // Low -> High: 15->0, -1 -> Invalid
		        old_pr   = Macro::PRI_NONE;

		PagePtr_t page     = nullptr;
		Status status      = Status::TERMINATED;
		Tick_t time_slice  = Macro::TIME_SLICE;
		Tick_t delay_ticks = 0;
		ParentPtr_t parent = nullptr;

		Tcb_t() = default;
		Tcb_t(Fn_t fn,
		      Argv_t argv = nullptr,
		      Prior_t pri = Macro::PRI_MIN,
		      Name_t name = "")
		    : fn(fn),
		      argv(argv),
		      priority(pri),
		      name(name) {}

		MOS_INLINE inline void
		set_tid(Tid_t tid_val) volatile
		{
			tid = tid_val;
		}

		MOS_INLINE inline Tid_t
		get_tid() volatile const
		{
			return tid;
		}

		MOS_INLINE inline TcbPtr_t
		next() volatile const
		{
			return (TcbPtr_t) node.next;
		}

		MOS_INLINE inline TcbPtr_t
		prev() volatile const
		{
			return (TcbPtr_t) node.prev;
		}

		MOS_INLINE inline void
		deinit() volatile
		{
			new ((void*) this) Tcb_t {};
		}

		MOS_INLINE inline void
		set_parent(ParentPtr_t parent_ptr) volatile
		{
			parent = parent_ptr;
		}

		MOS_INLINE inline ParentPtr_t
		get_parent() volatile const
		{
			return parent;
		}

		MOS_INLINE inline void
		set_status(Status new_status) volatile
		{
			status = new_status;
		}

		MOS_INLINE inline Status
		get_status() volatile const
		{
			return status;
		}

		MOS_INLINE inline bool
		is_status(Status expected) volatile const
		{
			return get_status() == expected;
		}

		MOS_INLINE inline bool
		is_sleeping() volatile const
		{
			return is_status(Status::BLOCKED) &&
			       delay_ticks != 0;
		}

		MOS_INLINE inline Name_t
		get_name() volatile const
		{
			return name;
		}

		MOS_INLINE inline void
		set_pri(Prior_t pri) volatile
		{
			priority = pri;
		}

		MOS_INLINE inline Prior_t
		get_pri() volatile const
		{
			return priority;
		}

		MOS_INLINE inline void
		set_SP(const uint32_t* sp_val) volatile
		{
			sp = (StackPtr_t) sp_val;
		}

		MOS_INLINE inline void
		set_xPSR(const uint32_t xpsr_val) volatile
		{
			page->raw[Macro::PAGE_SIZE - 1U] = xpsr_val;
		}

		MOS_INLINE inline void
		set_PC(const uint32_t pc_val) volatile
		{
			page->raw[Macro::PAGE_SIZE - 2U] = pc_val;
		}

		MOS_INLINE inline void
		set_LR(const uint32_t lr_val) volatile
		{
			page->raw[Macro::PAGE_SIZE - 3U] = lr_val;
		}

		MOS_INLINE inline void
		set_argv(const uint32_t argv_ref) volatile
		{
			page->raw[Macro::PAGE_SIZE - 8U] = argv_ref;
		}

		MOS_INLINE inline void
		set_delay(const Tick_t ticks) volatile
		{
			delay_ticks = ticks;
		}

		MOS_INLINE inline void
		attach_page(const PagePtr_t page_ptr) volatile
		{
			page       = page_ptr;
			page->used = true;
		}

		MOS_INLINE inline void
		release_page() volatile
		{
			page->used = false;
			page       = nullptr;
		}

		MOS_INLINE inline uint32_t
		page_usage() volatile const
		{
			const uint32_t stk_top = (uint32_t) &page->raw[Macro::PAGE_SIZE];
			const uint32_t atu     = (stk_top - (uint32_t) sp + sizeof(Tcb_t));
			return atu * 25 / Macro::PAGE_SIZE;
		}

		MOS_INLINE inline uint32_t
		stack_usage() volatile const
		{
			const uint32_t stk_top = (uint32_t) &page->raw[Macro::PAGE_SIZE];
			const uint32_t atu     = (stk_top - (uint32_t) sp);
			return atu * 25 / (Macro::PAGE_SIZE - sizeof(Tcb_t) / sizeof(void*));
		}

		MOS_INLINE static inline bool
		pri_cmp(ConstTcbPtr_t lhs, ConstTcbPtr_t rhs)
		{
			return lhs->get_pri() < rhs->get_pri();
		}

		MOS_INLINE static inline bool
		pri_equal(ConstTcbPtr_t lhs, ConstTcbPtr_t rhs)
		{
			return lhs->get_pri() == rhs->get_pri();
		}

		MOS_INLINE static inline TcbPtr_t
		build(PagePtr_t page_ptr,
		      Fn_t fn,
		      Argv_t argv = nullptr,
		      Prior_t pri = Macro::PRI_MIN,
		      Name_t name = "")
		{
			// In-placement new
			auto tcb = new (page_ptr->raw) Tcb_t {fn, argv, pri, name};
			tcb->attach_page(page_ptr);
			return tcb;
		}
	};

	template <typename Fn, typename Ret = void>
	concept TcbListIterFn = Concepts::Invocable<Fn, Ret, const Tcb_t&>;

	template <typename Fn>
	concept TcbCmpFn = Concepts::Invocable<
	        Fn,
	        bool,
	        Tcb_t::TcbPtr_t,
	        Tcb_t::TcbPtr_t>;

	// A wrapper of ListImpl_t for Tcb_t
	struct TcbList_t : private ListImpl_t
	{
		using TcbPtr_t = Tcb_t::TcbPtr_t;
		using ListImpl_t::size;
		using ListImpl_t::empty;

		MOS_INLINE inline TcbPtr_t
		begin() const { return (TcbPtr_t) ListImpl_t::begin(); }

		MOS_INLINE inline TcbPtr_t
		end() const { return (TcbPtr_t) ListImpl_t::end(); }

		MOS_INLINE inline void
		iter(TcbListIterFn auto&& fn) const
		{
			auto wrapper = [](auto&& fn) {
				return [&](const Node_t& node) {
					fn((const Tcb_t&) node);
				};
			};

			ListImpl_t::iter_mut(wrapper(fn));
		}

		MOS_INLINE inline void
		iter_mut(auto&& fn)
		{
			auto wrapper = [](auto&& fn) {
				return [&](Node_t& node) {
					fn((Tcb_t&) node);
				};
			};

			ListImpl_t::iter_mut(wrapper(fn));
		}

		MOS_INLINE inline TcbPtr_t
		iter_until(TcbListIterFn<bool> auto&& fn) const
		{
			for (auto it = begin();
			     it != end();
			     it = it->next()) {
				if (fn(*it)) return it;
			}
			return nullptr;
		}

		void insert(TcbPtr_t tcb, TcbPtr_t pos)
		{
			ListImpl_t::insert(tcb->node, (NodePtr_t) pos);
		}

		void insert_in_order(TcbPtr_t tcb, TcbCmpFn auto&& cmp)
		{
			auto wrapper = [](auto&& cmp) {
				return [&](const Node_t& lhs, const Node_t& rhs) {
					return cmp((TcbPtr_t) &lhs, (TcbPtr_t) &rhs);
				};
			};

			ListImpl_t::insert_in_order(tcb->node, wrapper(cmp));
		}

		MOS_INLINE inline void
		add(TcbPtr_t tcb) { ListImpl_t::add(tcb->node); }

		void remove(TcbPtr_t tcb)
		{
			ListImpl_t::remove(tcb->node);
		}

		MOS_INLINE inline void
		send_to(TcbPtr_t tcb, TcbList_t& dest)
		{
			remove(tcb);
			dest.add(tcb);
		}

		MOS_INLINE inline void
		send_to_in_order(
		        TcbPtr_t tcb,
		        TcbList_t& dest,
		        TcbCmpFn auto&& cmp)
		{
			remove(tcb);
			dest.insert_in_order(tcb, cmp);
		}

		MOS_INLINE inline void
		re_insert(TcbPtr_t tcb, TcbCmpFn auto&& cmp)
		{
			send_to_in_order(tcb, *this, cmp);
		}

		MOS_INLINE inline bool
		contains(TcbPtr_t tcb)
		{
			return iter_until([tcb](const Tcb_t& x) {
				       return &x == tcb;
			       }) != nullptr;
		}
	};

	struct DebugTcbs_t
	{
		using TcbPtr_t = volatile Tcb_t::TcbPtr_t;
		using Raw_t    = volatile TcbPtr_t[Macro::MAX_TASK_NUM];
		using Len_t    = volatile uint32_t;

		Raw_t raw = {nullptr};
		Len_t len = 0;

		MOS_INLINE inline auto
		size() const volatile { return len; }

		MOS_INLINE inline void
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

		MOS_INLINE inline void
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

		MOS_INLINE inline void
		iter(auto&& fn) const volatile
		    requires Concepts::Invocable<decltype(fn), void, const TcbPtr_t&>
		{
			for (auto& pt: raw) {
				if (pt != nullptr) {
					fn(pt);
				}
			}
		}

		MOS_INLINE inline void
		iter_mut(auto&& fn) volatile
		    requires Concepts::Invocable<decltype(fn), void, TcbPtr_t&>
		{
			for (auto& pt: raw) {
				if (pt != nullptr) {
					fn(pt);
				}
			}
		}

		MOS_INLINE inline TcbPtr_t
		iter_until(auto&& fn) volatile const
		    requires Concepts::Invocable<decltype(fn), bool, TcbPtr_t&>
		{
			for (auto& pt: raw) {
				if (pt != nullptr) {
					if (fn(pt)) return pt;
				}
			}
			return nullptr;
		}
	};
}

#endif