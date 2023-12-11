#ifndef _MOS_USER_TEST_
#define _MOS_USER_TEST_

#include "src/user/global.hpp"

namespace MOS::Test
{
	void MutexTest(void* argv)
	{
		using UserGlobal::mutex;

		auto cur = Task::current_task();
		while (true) {
			mutex.lock();
			for (uint8_t i = 0; i < 5; i++) {
				MOS_MSG("%s is working...\n", cur->get_name());
				Task::delay(100);
			}
			mutex.unlock();
			Task::delay(100);
		}
	}
}

#endif