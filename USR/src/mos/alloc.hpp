#ifndef _MOS_ALLOC_
#define _MOS_ALLOC_

#include "global.hpp"

namespace MOS::Alloc
{
	using PagePtr_t = DataType::Page_t::PagePtr_t;

	inline PagePtr_t
	page_alloc()
	{
		for (auto& page: KernelGlobal::page_pool) {
			if (!page.is_used()) {
				return &page;
			}
		}
		MOS_ASSERT(false, "Page Alloc Error");
		return nullptr;
	}
}

#endif