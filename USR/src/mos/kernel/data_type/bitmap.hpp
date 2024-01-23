#ifndef _MOS_BIT_MAP_
#define _MOS_BIT_MAP_

#include "../utils.hpp"

namespace MOS::DataType
{
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
}

#endif