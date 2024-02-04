#ifndef _MOS_ALLOC_
#define _MOS_ALLOC_

// #include <stdlib.h>
#include "global.hpp"

namespace MOS::Alloc
{
	using Utils::DisIntrGuard_t;
	using Page_t     = DataType::Page_t;
	using PagePolicy = Page_t::Policy;
	using PageRaw_t  = Page_t::Raw_t;
	using PageLen_t  = Page_t::Len_t;

	// Page Allocator
	inline PageRaw_t // -1(0xFFFFFFFF) as invalid
	palloc_raw(PagePolicy policy, PageLen_t pg_sz = -1)
	{
		DisIntrGuard_t guard;
		switch (policy) {
			case PagePolicy::POOL: {
				using KernelGlobal::page_pool;

				// Whether a page has been used
				static auto is_used = [](PageRaw_t raw) {
					auto tst = (void*) raw[0];
					return tst != nullptr && tst != raw;
				};

				for (auto raw: page_pool) {
					if (!is_used(raw)) {
						return raw;
					}
				}

				return nullptr;
			}

			case PagePolicy::DYNAMIC: {
				MOS_ASSERT(pg_sz != -1, "Page Size Error");
				return new uint32_t[pg_sz];
			}

			default:
				return nullptr;
		}
	}
}

#endif