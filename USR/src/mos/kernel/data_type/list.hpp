#ifndef _MOS_LIST_
#define _MOS_LIST_

#include "../concepts.hpp"
#include "../utils.hpp"

namespace MOS::DataType
{
	struct ListNode_t
	{
		using Self_t    = ListNode_t;
		using SelfPtr_t = Self_t*;

		// Self-linked as default
		SelfPtr_t prev = this, next = this;
	};

	using Concepts::Invocable;

	template <typename Fn, typename Ret = void>
	concept ListIterFn = Invocable<Fn, Ret, const ListNode_t&>;

	template <typename Fn, typename Ret = void>
	concept ListIterMutFn = Invocable<Fn, Ret, ListNode_t&>;

	template <typename Fn>
	concept NodeCmpFn = Invocable<Fn, bool, const ListNode_t&, const ListNode_t&>;

	using List_t = struct ListImpl_t
	{
		using Self_t         = ListImpl_t;
		using Node_t         = ListNode_t;
		using NodePtr_t      = Node_t*;
		using ConstNodePtr_t = const Node_t*;

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
		iter_mut(ListIterMutFn auto&& fn)
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

		MOS_INLINE void
		insert(Node_t& node, NodePtr_t pos)
		{
			MOS_DSB(); // Avoid reordering
			MOS_ISB();
			if (pos == nullptr)
				return;
			node.next       = pos;
			node.prev       = pos->prev;
			pos->prev->next = &node;
			pos->prev       = &node;
			len += 1;
		}

		MOS_INLINE void
		remove(Node_t& node)
		{
			MOS_DSB(); // Avoid reordering
			MOS_ISB();
			auto prev  = node.prev,
			     next  = node.next;
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
		send_to(Node_t& node, Self_t& dest)
		{
			remove(node);
			dest.add(node);
		}

		MOS_INLINE inline void
		send_to_in_order(
		    Node_t& node,
		    Self_t& dest,
		    NodeCmpFn auto&& cmp
		)
		{
			remove(node);
			dest.insert_in_order(node, cmp);
		}

		MOS_INLINE inline void
		re_insert(Node_t& node, NodeCmpFn auto&& cmp)
		{
			send_to_in_order(node, *this, cmp);
		}
	};
}

#endif