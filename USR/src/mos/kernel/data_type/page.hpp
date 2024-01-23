#ifndef _MOS_PAGE_
#define _MOS_PAGE_

#include "../utils.hpp"

namespace MOS::DataType
{
	struct Page_t
	{
		using Raw_t = uint32_t*;
		using Len_t = uint32_t;

		Len_t size = 0;
		Raw_t raw  = nullptr;

		enum class Policy
		{
			POOL,
			DYNAMIC,
			STATIC,
			ERROR,
		} policy = Policy::ERROR;

		MOS_INLINE inline void
		recycle()
		{
			using enum Policy;
			if (policy == DYNAMIC) {
				delete[] raw;
			}
		}

		MOS_INLINE inline Len_t
		get_size() const volatile { return size; }

		MOS_INLINE inline Raw_t
		get_raw() const volatile { return raw; }

		MOS_INLINE inline Policy
		get_policy() const volatile { return policy; }

		MOS_INLINE inline auto&
		get_from_bottom(uint32_t n = 0)
		        const volatile { return raw[size - n]; }
	};
}

#endif