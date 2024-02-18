#ifndef _MOS_BIT_MAP_
#define _MOS_BIT_MAP_

#include "../utils.hpp"

namespace MOS::DataType
{
	template <size_t N>
	struct BitMap_t
	{
		using Raw_t = volatile uint32_t[(N + 31) / 32];

		Raw_t data = {0};

		MOS_INLINE
		inline BitMap_t() = default;

		MOS_INLINE inline void
		set(uint32_t pos) volatile
		{
			const uint32_t index = pos / 32;
			const uint32_t bit   = 31 - pos % 32;
			data[index] |= (1 << bit);
		}

		MOS_INLINE inline void
		reset(uint32_t pos) volatile
		{
			const uint32_t index = pos / 32;
			const uint32_t bit   = 31 - pos % 32;
			data[index] &= ~(1 << bit);
		}

		MOS_INLINE inline bool
		test(uint32_t pos) const volatile
		{
			const uint32_t index = pos / 32;
			const uint32_t bit   = 31 - pos % 32;
			return (data[index] & (1 << bit)) != 0;
		}

		inline int32_t
		first_zero() const volatile
		{
			constexpr auto end = (N + 31) / 32;
			for (size_t i = 0; i < end; ++i) {
				// Check if all bits are 1
				if (data[i] != ~0U) {
					// first zero from left to right
					const uint32_t bit = __builtin_clz(~data[i]);
					return i * 32 + bit;
				}
			}
			return -1; // All bits are 1
		}
	};
}

#endif