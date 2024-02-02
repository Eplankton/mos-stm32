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
	template <PagePolicy policy>
	inline PageRaw_t
	palloc(PageLen_t pg_sz = 0xFF) // pg_sz == 0xFF as invalid
	{
		DisIntrGuard_t guard;

		if constexpr (policy == PagePolicy::POOL) {
			using KernelGlobal::page_pool;

			// Whether the page is used
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

		if constexpr (policy == PagePolicy::DYNAMIC) {
			return new uint32_t[pg_sz];
		}
	}
}

#endif