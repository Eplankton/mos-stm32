#ifndef _MOS_QUEUE_
#define _MOS_QUEUE_

#include "../utils.hpp"

namespace MOS::DataType
{
	class QueueImpl_t
	{
		using Len_t = volatile uint32_t;

	protected:
		Len_t m_len  = 0,
		      m_head = 0,
		      m_tail = 0;

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
		MOS_INLINE inline uint32_t
		size() const volatile { return m_len; }

		MOS_INLINE inline bool
		full() const volatile { return m_len != 0 && m_head == m_tail; }

		MOS_INLINE inline bool
		empty() const volatile { return m_len == 0; }

		MOS_INLINE inline void
		clear() volatile { m_head = m_tail = m_len = 0; }
	};

	template <typename T, uint32_t N, typename Base = QueueImpl_t>
	class Queue_t : public Base
	{
	protected:
		using Base::front;
		using Base::back;
		using Base::push;
		using Base::pop;

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

		static inline constexpr uint32_t
		capacity() { return N; }

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
			push(
			    (void*) (m_data + this->m_tail),
			    (void*) &val,
			    sizeof(T),
			    N
			);
		}

		inline value_type
		serve() volatile
		{
			auto tmp = front();
			pop();
			return tmp;
		}

		inline void
		iter(auto&& fn) const volatile
		{
			if (empty()) return;
			else {
				auto i = this->m_head;
				do {
					fn(m_data[i]);
					i = (i + 1) % N;
				} while (i != this->m_tail);
			}
		}

		inline void
		iter_mut(auto&& fn) volatile
		{
			if (empty()) return;
			else {
				auto i = this->m_head;
				do {
					fn(m_data[i]);
					i = (i + 1) % N;
				} while (i != this->m_tail);
			}
		}
	};
}

#endif