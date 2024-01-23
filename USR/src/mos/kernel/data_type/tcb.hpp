#ifndef _MOS_TCB_
#define _MOS_TCB_

#include "../macro.hpp"
#include "list.hpp"
#include "page.hpp"

namespace MOS::DataType
{
	using namespace Macro;

	struct __attribute__((packed)) Tcb_t
	{
		using Self_t        = Tcb_t;
		using SelfPtr_t     = Tcb_t*;
		using TcbPtr_t      = SelfPtr_t;
		using ConstTcbPtr_t = const Tcb_t*;
		using StackPtr_t    = uint32_t*;
		using Node_t        = ListNode_t;
		using Tid_t         = int16_t;
		using Prior_t       = int8_t;
		using Tick_t        = uint32_t;
		using Ret_t         = void;
		using Argv_t        = void*;
		using Fn_t          = Ret_t (*)(Argv_t);
		using Name_t        = const char*;

		enum class Status
		{
			READY,
			RUNNING,
			BLOCKED,
			TERMINATED,
		};

		using enum Status;
		using enum Page_t::Policy;

		// Don't change the offset of node and sp
		Node_t node;
		StackPtr_t sp = nullptr;

		// Add more members here
		Tid_t tid = -1;

		// Only for debug
		Fn_t fn     = nullptr;
		Argv_t argv = nullptr;
		Name_t name = "";

		// Low -> High: 15->0, -1 -> Invalid
		Prior_t priority = PRI_MIN,
		        old_pr   = PRI_NONE;

		Page_t page        = {0, nullptr, ERROR};
		Status status      = TERMINATED;
		Tick_t time_slice  = TIME_SLICE;
		Tick_t delay_ticks = 0;
		TcbPtr_t parent    = nullptr;

		MOS_INLINE Tcb_t() = default;
		MOS_INLINE Tcb_t(
		        Fn_t fn,
		        Argv_t argv,
		        Prior_t pri,
		        Name_t name,
		        Page_t page)
		    : fn(fn), argv(argv), priority(pri),
		      name(name), page(page) {}

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
			Page_t inactive {
			        .size   = 0xFF, // Anyway
			        .raw    = page.get_raw(),
			        .policy = page.get_policy()};

			new ((void*) this) Tcb_t {};
			inactive.recycle();
		}

		MOS_INLINE inline void
		set_parent(TcbPtr_t parent) volatile
		{
			this->parent = parent;
		}

		MOS_INLINE inline TcbPtr_t
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
			return is_status(BLOCKED) && delay_ticks != 0;
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
			page.get_from_bottom(1) = xpsr_val;
		}

		MOS_INLINE inline void
		set_PC(const uint32_t pc_val) volatile
		{
			page.get_from_bottom(2) = pc_val;
		}

		MOS_INLINE inline void
		set_LR(const uint32_t lr_val) volatile
		{
			page.get_from_bottom(3) = lr_val;
		}

		MOS_INLINE inline void
		set_argv(const uint32_t argv_val) volatile
		{
			page.get_from_bottom(8) = argv_val;
		}

		MOS_INLINE inline void
		set_delay(const Tick_t ticks) volatile
		{
			delay_ticks = ticks;
		}

		MOS_INLINE inline uint32_t
		page_usage() volatile const
		{
			const uint32_t stk_top = (uint32_t) &page.get_from_bottom();
			const uint32_t atu     = (stk_top - (uint32_t) sp + sizeof(Tcb_t));
			return atu * 25 / page.get_size();
		}

		MOS_INLINE inline uint32_t
		stack_usage() volatile const
		{
			const uint32_t stk_top = (uint32_t) &page.get_from_bottom();
			const uint32_t atu     = (stk_top - (uint32_t) sp);
			return atu * 25 / (page.get_size() - sizeof(Tcb_t) / sizeof(void*));
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

		static inline TcbPtr_t
		build(Fn_t fn,
		      Argv_t argv = nullptr,
		      Prior_t pri = PRI_MIN,
		      Name_t name = "",
		      Page_t page = {0, nullptr, ERROR})
		{
			// Use inplace new
			return new (page.get_raw()) Tcb_t {
			        fn,
			        argv,
			        pri,
			        name,
			        page,
			};
		}
	};

	template <typename Fn, typename Ret = void>
	concept TcbListIterFn = Invocable<Fn, Ret, const Tcb_t&>;

	template <typename Fn>
	concept TcbCmpFn = Invocable<Fn, bool, Tcb_t*, Tcb_t*>;

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

		MOS_INLINE inline void
		insert(TcbPtr_t tcb, TcbPtr_t pos)
		{
			ListImpl_t::insert(tcb->node, (NodePtr_t) pos);
		}

		inline void
		insert_in_order(TcbPtr_t tcb, TcbCmpFn auto&& cmp)
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

		MOS_INLINE inline void
		remove(TcbPtr_t tcb)
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
		using Raw_t    = volatile TcbPtr_t[MAX_TASK_NUM];
		using Len_t    = volatile uint32_t;

		Raw_t raw = {nullptr};
		Len_t len = 0;

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
		    requires Invocable<decltype(fn), void, const TcbPtr_t&>
		{
			for (auto& pt: raw) {
				if (pt != nullptr) {
					fn(pt);
				}
			}
		}

		MOS_INLINE inline void
		iter_mut(auto&& fn) volatile
		    requires Invocable<decltype(fn), void, TcbPtr_t&>
		{
			for (auto& pt: raw) {
				if (pt != nullptr) {
					fn(pt);
				}
			}
		}

		MOS_INLINE inline TcbPtr_t
		iter_until(auto&& fn) volatile const
		    requires Invocable<decltype(fn), bool, TcbPtr_t&>
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