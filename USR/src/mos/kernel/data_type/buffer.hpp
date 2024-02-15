#ifndef _MOS_BUFFER_
#define _MOS_BUFFER_

#include "../utils.hpp"
#include "../sync.hpp"

namespace MOS::DataType
{
	template <typename T, size_t N>
	struct Buffer_t
	{
		using Raw_t = T[N];
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
		add(T data) volatile { raw[index++] = data; }

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

	template <size_t N>
	using RxBuf_t = Buffer_t<char, N>;

	template <size_t N>
	struct SyncRxBuf_t : public RxBuf_t<N>
	{
		MOS_INLINE inline void
		wait() { sema.down(); }

		MOS_INLINE inline void
		signal() { sema.up(); }

		MOS_INLINE inline void
		signal_from_isr() { sema.up_from_isr(); }

	private:
		Sync::Sema_t sema {0};
	};
}

#endif