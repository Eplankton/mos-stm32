#ifndef _MOS_ALLOC_
#define _MOS_ALLOC_

// #include <stdlib.h>
#include "global.hpp"

namespace MOS::Alloc
{
	using Utils::DisIntrGuard_t;
	using Page_t    = DataType::Page_t;
	using PageRaw_t = Page_t::Raw_t;
	using PgSz_t    = Page_t::Size_t;

	using enum Page_t::Policy;

	// Page Allocator
	inline PageRaw_t // -1(0xFFFFFFFF) as invalid
	palloc(Page_t::Policy policy, PgSz_t pg_sz = -1)
	{
		DisIntrGuard_t guard;
		switch (policy) {
			case POOL: {
				using KernelGlobal::page_pool;

				// Whether a page is unused
				auto is_unused = [](PageRaw_t raw) {
					auto ptr = (void*) raw[0]; // ptr = tcb.node.prev
					return ptr == nullptr ||   // Uninit-> first alloc
					       ptr == raw;         // Deinit-> tcb.node is self-linked
				};

				for (auto raw: page_pool) {
					if (is_unused(raw)) {
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