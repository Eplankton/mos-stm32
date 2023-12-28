#ifndef _MOS_USER_TEST_
#define _MOS_USER_TEST_

#include "src/mos/kernel/sync.hpp"

namespace MOS::Test
{
	static Sync::Mutex_t mutex;

	void MutexTest(void* argv)
	{
		auto name = Task::current_task()->get_name();
		while (true) {
			{// MutexGuard Scope
				auto guard = mutex.lock();
				for (uint8_t i = 0; i < 5; i++) {
					kprintf("%s is working...\n", name);
					Task::delay(100);
				}
			}
			Task::delay(10);
		}
	}
}

#endif