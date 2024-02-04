#ifndef _MOS_ASYNC_
#define _MOS_ASYNC_

#include "task.hpp"

namespace MOS::Async
{
	using namespace Macro;
	using namespace Utils;
	using namespace Alloc;

	using DataType::Tcb_t;
	using DataType::Page_t;

	using TcbPtr_t   = Tcb_t::TcbPtr_t;
	using Fn_t       = Tcb_t::Fn_t;
	using Argv_t     = Tcb_t::Argv_t;
	using Status     = Tcb_t::Status;
	using PageLen_t  = Page_t::Len_t;
	using PagePolicy = Page_t::Policy;

	static inline void async_exit()
	{
		// Assert if irq disabled
		MOS_ASSERT(test_irq(), "Disabled Interrupt");
		DisIntrGuard_t guard;

		auto cur    = Task::current(),
		     parent = cur->get_parent();

		Task::terminate_raw(cur);
		Task::resume_raw(parent);
		return Task::yield();
	}

	struct Future_t
	{
		TcbPtr_t tcb;

		MOS_INLINE ~Future_t() { await(); }
		MOS_INLINE inline void
		await()
		{
			Task::resume(tcb);
			Task::block();
			tcb = nullptr;
		}
	};

	MOS_INLINE inline Future_t
	create_raw(Fn_t fn, Argv_t argv, Page_t page)
	{
		DisIntrGuard_t guard;
		auto tcb = Task::create_raw(
		        fn,
		        argv,
		        Task::current()->get_pri(),
		        "async",
		        page);

		// Set LR to a special exit routine
		tcb->set_LR((uint32_t) async_exit);
		Task::block_to_raw(tcb);
		return Future_t {tcb};
	}

	MOS_INLINE inline Future_t
	create(Fn_t fn, Argv_t argv, Page_t page)
	{
		return create_raw(fn, argv, page);
	}

	MOS_INLINE inline Future_t
	create(Fn_t fn, Argv_t argv)
	{
		auto page = Task::page_alloc(PagePolicy::POOL, PAGE_SIZE);
		return create_raw(fn, argv, page);
	}

	MOS_INLINE inline Future_t
	create(Fn_t fn, Argv_t argv, PageLen_t pg_sz)
	{
		auto page = Task::page_alloc(PagePolicy::DYNAMIC, pg_sz);
		return create_raw(fn, argv, page);
	}
}

#endif