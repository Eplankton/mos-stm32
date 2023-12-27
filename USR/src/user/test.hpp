#ifndef _MOS_USER_TEST_
#define _MOS_USER_TEST_

#include "src/mos/kernel/sync.hpp"

namespace MOS::Test
{
	static Sync::MutexImpl_t mutex;

	void MutexTest(void* argv)
	{
		auto cur = Task::current_task();
		while (true) {
			mutex.lock();
			for (uint8_t i = 0; i < 5; i++) {
				kprintf("%s is working...\n", cur->get_name());
				Task::delay(100);
			}
			mutex.unlock();
			Task::delay(100);
		}
	}
}

#endif