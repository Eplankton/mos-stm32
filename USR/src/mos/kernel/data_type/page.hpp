#ifndef _MOS_PAGE_
#define _MOS_PAGE_

#include "../utils.hpp"

namespace MOS::DataType
{
	struct Page_t
	{
		using Word_t = uint32_t;
		using Raw_t  = Word_t*;
		using Len_t  = const uint32_t;

		enum class Policy
		{
			ERROR = -1,
			STATIC,
			POOL,
			DYNAMIC,
		} policy = Policy::ERROR;

		Raw_t raw  = nullptr;
		Len_t size = 0;

		MOS_INLINE inline void
		recycle()
		{
			using enum Policy;
			if (policy == DYNAMIC) {
				delete[] raw;
			}
			raw = nullptr;
		}

		MOS_INLINE inline Len_t
		get_size() const volatile { return size; }

		MOS_INLINE inline Raw_t
		get_raw() const volatile { return raw; }

		MOS_INLINE inline Policy
		get_policy() const volatile { return policy; }

		MOS_INLINE inline bool
		is_policy(Policy policy)
		        const volatile { return get_policy() == policy; }

		MOS_INLINE inline Word_t&
		from_bottom(uint32_t offset = 0)
		        const volatile { return raw[size - offset]; }
	};
}

#endif