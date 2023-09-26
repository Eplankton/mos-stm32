#ifndef _NUTS_PIPE_
#define _NUTS_PIPE_

#include <etl/circular_buffer.h>
#include "type.hpp"

namespace nuts
{
	class PipeBase
	{
	protected:
		u32 m_len = 0, m_head = 0, m_tail = 0;

		inline void* front(void* src, const u32 size) volatile
		{
			return (void*) ((u32) (src) + m_head * size);
		}

		inline void* back(void* src, const u32 size) volatile
		{
			return (void*) ((u32) (src) + (m_len - 1) * size);
		}

		inline void
		push(void* dest, const void* src,
		     const u32 size, const u32 N) volatile
		{
			if (!full()) {
				memcpy(dest, src, size);
				m_tail = (m_tail + 1) % N;
				m_len++;
			}
		}

		inline void pop(u32 N) volatile
		{
			if (!empty()) {
				m_head = (m_head + 1) % N;
				m_len--;
			}
		}

	public:
		inline u32 size() const volatile { return m_len; }
		inline bool full() const volatile { return m_len != 0 && m_head == m_tail; }
		inline bool empty() const volatile { return m_len == 0; }
		inline void clear() volatile { m_head = m_tail = m_len = 0; }
	};

	template <typename T, u32 N, typename Base = PipeBase>
	class Pipe : public Base
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

		Pipe()  = default;
		~Pipe() = default;

		static inline constexpr u32 capacity() { return N; }

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

	template <typename T, u32 N>
	class Pipe<T, N, etl::circular_buffer<T, N>>
	    : public etl::circular_buffer<T, N>
	{
		// ETL Version
		// public:
		// 	inline void iter(auto&& fn)
		// 	{
		// 		for (auto& x: *this) {
		// 			fn(x);
		// 		}
		// 	}

		// 	inline void iter(auto&& fn) const
		// 	{
		// 		for (const auto& x: *this) {
		// 			fn(x);
		// 		}
		// 	}
	};
}

#endif