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
	using PgSz_t     = Page_t::Size_t;

	// Page Allocator
	inline PageRaw_t // -1(0xFFFFFFFF) as invalid
	palloc(PagePolicy policy, PgSz_t pg_sz = -1)
	{
		DisIntrGuard_t guard;
		switch (policy) {
			case PagePolicy::POOL: {
				using KernelGlobal::page_pool;

				// Whether a page is unused
				static auto unused = [](PageRaw_t raw) {
					auto ptr = (void*) raw[0]; // ptr = tcb.node.prev
					return ptr == nullptr      // Uninitialized -> first alloc
					       || ptr == raw;      // Deinitialized -> tcb.node is self-linked
				};

				for (auto raw: page_pool) {
					if (unused(raw)) {
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