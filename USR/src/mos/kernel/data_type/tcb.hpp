#ifndef _MOS_TCB_
#define _MOS_TCB_

#include "../macro.hpp"
#include "list.hpp"
#include "page.hpp"

namespace MOS::DataType
{
	using namespace Macro;

	// Task Control Block
	struct __attribute__((packed)) TCB_t
	{
		using Self_t        = TCB_t;
		using SelfPtr_t     = TCB_t*;
		using TcbPtr_t      = SelfPtr_t;
		using ConstTcbPtr_t = const TCB_t*;
		using StackPtr_t    = uint32_t*;
		using Node_t        = ListNode_t;
		using Tid_t         = int16_t;
		using Prior_t       = int8_t;
		using Tick_t        = uint32_t;
		using Ret_t         = void;
		using Argv_t        = void*;
		using Fn_t          = Ret_t (*)(Argv_t);
		using Name_t        = const char*;

		enum class Status : int8_t
		{
			TERMINATED,
			READY,
			RUNNING,
			BLOCKED,
		};

		using enum Status;
		using enum Page_t::Policy;

		// Don't change the offset of node and sp
		Node_t node;
		StackPtr_t sp = nullptr;

		// Add more members here
		Tid_t tid         = -1;
		TcbPtr_t parent   = nullptr;
		Page_t page       = {ERROR, nullptr, 0};
		Prior_t pri       = PRI_MIN;
		Status status     = TERMINATED;
		Tick_t time_slice = TIME_SLICE,
		       wake_point = -1,
		       stamp      = -1;

		// For events like send/recv/...
		Node_t event;

		// Only for debug
		Fn_t fn     = nullptr;
		Argv_t argv = nullptr;
		Name_t name = "";

		TCB_t() = default;
		TCB_t(
		    Fn_t fn, Argv_t argv, Prior_t pri,
		    Name_t name, Page_t page
		): fn(fn), argv(argv), pri(pri),
		   name(name), page(page) {}

		MOS_INLINE inline TcbPtr_t
		next() const volatile
		{
			return (TcbPtr_t) node.next;
		}

		MOS_INLINE inline TcbPtr_t
		prev() const volatile
		{
			return (TcbPtr_t) node.prev;
		}

		void deinit() volatile
		{
			Page_t inactive {
			    .policy = page.get_policy(),
			    .raw    = page.get_raw(),
			    // Ignore the size
			};

			// Use inplace new
			new ((void*) this) TCB_t {};
			inactive.recycle();
		}

		MOS_INLINE inline void
		set_parent(TcbPtr_t parent) volatile
		{
			this->parent = parent;
		}

		MOS_INLINE inline TcbPtr_t
		get_parent() const volatile
		{
			return parent;
		}

		MOS_INLINE inline void
		set_tid(Tid_t _tid) volatile
		{
			tid = _tid;
		}

		MOS_INLINE inline Tid_t
		get_tid() const volatile
		{
			return tid;
		}

		MOS_INLINE inline void
		set_status(Status _status) volatile
		{
			status = _status;
		}

		MOS_INLINE inline Status
		get_status() const volatile
		{
			return status;
		}

		MOS_INLINE inline bool
		is_status(Status expected) const volatile
		{
			return get_status() == expected;
		}

		MOS_INLINE inline bool
		is_sleeping() const volatile
		{
			return wake_point != 0 && is_status(BLOCKED);
		}

		MOS_INLINE inline Name_t
		get_name() const volatile
		{
			return name;
		}

		MOS_INLINE inline void
		set_pri(Prior_t _pri) volatile
		{
			pri = _pri;
		}

		MOS_INLINE inline Prior_t
		get_pri() const volatile
		{
			return pri;
		}

		MOS_INLINE inline void
		set_sp(const uint32_t _sp) volatile
		{
			sp = (StackPtr_t) _sp;
		}

		MOS_INLINE inline void
		set_xpsr(const uint32_t _xpsr) volatile
		{
			page.from_bottom(1) = _xpsr;
		}

		MOS_INLINE inline void
		set_pc(const uint32_t _pc) volatile
		{
			page.from_bottom(2) = _pc;
		}

		MOS_INLINE inline void
		set_lr(const uint32_t _lr) volatile
		{
			page.from_bottom(3) = _lr;
		}

		MOS_INLINE inline void
		set_argv(const uint32_t _argv) volatile
		{
			page.from_bottom(8) = _argv;
		}

		// Set Wake Point
		MOS_INLINE inline void
		set_wkpt(const Tick_t ticks) volatile
		{
			wake_point = ticks;
		}

		// Get Wake Point
		MOS_INLINE inline Tick_t
		get_wkpt() const volatile
		{
			return wake_point;
		}

		MOS_INLINE inline void
		set_stamp(const Tick_t _stamp) volatile
		{
			stamp = _stamp;
		}

		MOS_INLINE inline Tick_t
		get_stamp() const volatile
		{
			return stamp;
		}

		MOS_INLINE inline uint32_t
		page_usage() const volatile
		{
			const uint32_t stk_top = (uint32_t) &page.from_bottom();
			const uint32_t atu     = (stk_top - (uint32_t) sp + sizeof(TCB_t));
			return atu * 25 / page.get_size();
		}

		MOS_INLINE inline uint32_t
		stack_usage() const volatile
		{
			const uint32_t stk_top = (uint32_t) &page.from_bottom();
			const uint32_t atu     = (stk_top - (uint32_t) sp);
			return atu * 25 / (page.get_size() - sizeof(TCB_t) / sizeof(void*));
		}

		MOS_INLINE inline bool
		in_event() const volatile
		{
			return event.prev != &event;
		}

		MOS_INLINE static inline bool
		pri_cmp(ConstTcbPtr_t lhs, ConstTcbPtr_t rhs)
		{
			return lhs->get_pri() < rhs->get_pri();
		}

		MOS_INLINE static inline bool
		wkpt_cmp(ConstTcbPtr_t lhs, ConstTcbPtr_t rhs)
		{
			return lhs->get_wkpt() < rhs->get_wkpt();
		}

		MOS_INLINE static inline bool
		pri_equal(ConstTcbPtr_t lhs, ConstTcbPtr_t rhs)
		{
			return lhs->get_pri() == rhs->get_pri();
		}

		static inline TcbPtr_t
		build(
		    Fn_t fn, Argv_t argv, Prior_t pri,
		    Name_t name, Page_t page
		)
		{
			// Use inplace new
			return new (page.get_raw()) TCB_t {
			    fn,
			    argv,
			    pri,
			    name,
			    page,
			};
		}
	};

	template <typename Fn, typename Ret = void>
	concept TcbListIterFn = Invocable<Fn, Ret, const TCB_t&>;

	template <typename Fn, typename Ret = void>
	concept TcbListIterMutFn = Invocable<Fn, Ret, TCB_t&>;

	template <typename Fn>
	concept TcbCmpFn = Invocable<Fn, bool, TCB_t*, TCB_t*>;

	// A wrapper of List_t for TCB_t
	struct TcbList_t : private List_t
	{
		using TcbPtr_t = TCB_t::TcbPtr_t;
		using List_t::size;
		using List_t::empty;

		MOS_INLINE inline TcbPtr_t
		begin() const { return (TcbPtr_t) List_t::begin(); }

		MOS_INLINE inline TcbPtr_t
		end() const { return (TcbPtr_t) List_t::end(); }

		MOS_INLINE inline void
		iter(TcbListIterFn auto&& fn) const
		{
			auto wrap = [](auto&& fn) {
				return [&](const Node_t& node) {
					fn((const TCB_t&) node);
				};
			};

			List_t::iter(wrap(fn));
		}

		MOS_INLINE inline void
		iter_mut(TcbListIterMutFn auto&& fn)
		{
			auto wrap = [](auto&& fn) {
				return [&](Node_t& node) {
					fn((TCB_t&) node);
				};
			};

			List_t::iter_mut(wrap(fn));
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

		MOS_INLINE inline void
		insert(TcbPtr_t tcb, TcbPtr_t pos)
		{
			List_t::insert(tcb->node, (NodePtr_t) pos);
		}

		inline void
		insert_in_order(TcbPtr_t tcb, TcbCmpFn auto&& cmp)
		{
			auto wrap = [](auto&& cmp) {
				return [&](const auto& lhs, const auto& rhs) {
					return cmp((TcbPtr_t) &lhs, (TcbPtr_t) &rhs);
				};
			};

			List_t::insert_in_order(tcb->node, wrap(cmp));
		}

		MOS_INLINE inline void
		add(TcbPtr_t tcb) { List_t::add(tcb->node); }

		MOS_INLINE inline void
		remove(TcbPtr_t tcb)
		{
			List_t::remove(tcb->node);
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
		    TcbCmpFn auto&& cmp
		)
		{
			remove(tcb);
			dest.insert_in_order(tcb, cmp);
		}

		MOS_INLINE inline void
		re_insert(TcbPtr_t tcb, TcbCmpFn auto&& cmp)
		{
			send_to_in_order(tcb, *this, cmp);
		}
	};

	struct DebugTcbs_t
	{
		using TcbPtr_t = volatile TCB_t::TcbPtr_t;
		using Raw_t    = volatile TcbPtr_t[MAX_TASK_NUM];
		using Len_t    = volatile uint32_t;
		using Tid_t    = volatile TCB_t::Tid_t;

		Raw_t raw = {nullptr};
		Len_t len = 0;
		Tid_t tid = -1;

		MOS_INLINE inline void
		mark(TcbPtr_t tcb) volatile
		{
			tid = tcb->get_tid();
		}

		MOS_INLINE inline Len_t
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
		    requires Invocable<decltype(fn), void, TcbPtr_t>
		{
			for (auto& pt: raw) {
				if (pt != nullptr) {
					fn(pt);
				}
			}
		}

		MOS_INLINE inline void
		iter_mut(auto&& fn) volatile
		    requires Invocable<decltype(fn), void, TcbPtr_t>
		{
			for (auto& pt: raw) {
				if (pt != nullptr) {
					fn(pt);
				}
			}
		}

		MOS_INLINE inline TcbPtr_t
		iter_until(auto&& fn) const volatile
		    requires Invocable<decltype(fn), bool, TcbPtr_t>
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