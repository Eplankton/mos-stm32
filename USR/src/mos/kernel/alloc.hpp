#ifndef _MOS_ALLOC_
#define _MOS_ALLOC_

// #include <stdlib.h>
#include "global.hpp"

namespace MOS::Kernel::Alloc
{
	using Utils::IntrGuard_t;
	using Page_t    = DataType::Page_t;
	using PageRaw_t = Page_t::Raw_t;
	using PgSz_t    = Page_t::Size_t;

	using enum Page_t::Policy;

	// Page Allocator
	inline PageRaw_t // -1(0xFFFFFFFF) as invalid
	palloc(Page_t::Policy policy, PgSz_t pg_sz = -1)
	{
		IntrGuard_t guard;
		switch (policy) {
			case POOL: {
				using Global::page_pool;

				// Whether a page is unused
				auto unused = [](PageRaw_t raw) {
					auto ptr = (void*) raw[0]; // ptr = tcb.link.prev
					return ptr == nullptr ||   // Uninit -> first alloc
					       ptr == raw;         // Deinit -> tcb is self-linked
				};

				for (const auto raw: page_pool) {
					if (unused(raw)) {
						return raw;
					}
				}

				return nullptr;
			}

			case DYNAMIC: {
				MOS_ASSERT(pg_sz != -1, "Page Size Error");
				return new uint32_t[pg_sz];
			}

			default:
				return nullptr;
		}
	}
}

#endif