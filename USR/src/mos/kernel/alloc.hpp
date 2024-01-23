#ifndef _MOS_ALLOC_
#define _MOS_ALLOC_

// #include <stdlib.h>
#include "global.hpp"

namespace MOS::Alloc
{
	using Page_t     = DataType::Page_t;
	using PagePolicy = Page_t::Policy;
	using PageRaw_t  = Page_t::Raw_t;
	using PageLen_t  = Page_t::Len_t;

	// Page Allocator
	template <PagePolicy policy>
	inline PageRaw_t palloc(PageLen_t pg_sz = 0xFF)
	{
		if constexpr (policy == PagePolicy::POOL) {
			using KernelGlobal::page_pool;

			static auto is_used = [](PageRaw_t raw) {
				const auto tst = (uint32_t*) raw[0];
				return tst != nullptr && tst != raw;
			};

			for (auto raw: page_pool) {
				if (!is_used(raw)) {
					return raw;
				}
			}

			return nullptr;
		}

		if constexpr (policy == PagePolicy::DYNAMIC) {
			return new uint32_t[pg_sz];
		}
	}
}

#endif