#ifndef _MOS_ALLOC_
#define _MOS_ALLOC_

// #include <stdlib.h>
#include "global.hpp"

namespace MOS::Alloc
{
	using PagePtr_t = DataType::Page_t::PagePtr_t;

	// Page allocator with fixed size, could be replaced by a dynamic allocator
	inline PagePtr_t palloc()
	{
		using KernelGlobal::page_pool;

		for (auto& page: page_pool) {
			if (!page.is_used()) {
				return &page;
			}
		}

		MOS_ASSERT(false, "Page Alloc Error");
		return nullptr;
	}
}

#endif