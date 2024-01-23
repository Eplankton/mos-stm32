#ifndef _MOS_RX_BUFFER_
#define _MOS_RX_BUFFER_

#include "../utils.hpp"

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
}

#endif