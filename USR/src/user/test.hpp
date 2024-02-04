#ifndef _MOS_USER_TEST_
#define _MOS_USER_TEST_

#include "src/mos/kernel/sync.hpp"
#include "src/mos/kernel/async.hpp"
#include "global.hpp"

namespace MOS::Test
{
	void MutexTest()
	{
		static Sync::Mutex_t mutex; // Take by 1-2-3... order
		static auto mtx_test = [](void* argv) {
			auto name = Task::current()->get_name();
			while (true) {
				mutex.exec([&] {
					for (uint8_t i = 0; i < 5; i++) {
						kprintf("%s is working...\n", name);
						Task::delay(100);
					}
				});
				Task::delay(5);
			}
		};

		Task::create(mtx_test, nullptr, 1, "T1");
		Task::create(mtx_test, nullptr, 2, "T2");
		Task::create(mtx_test, nullptr, 3, "T3");
	}

	void AsyncTest()
	{
		using UserGlobal::leds;

		static auto J1 = [](void* argv) {
			for (uint8_t i = 0; i < 10; i++) {
				leds[1].toggle();
				Task::delay(250);
			}
			kprintf("async exits...\n");
		};

		static auto J0 = [](void* argv) {
			auto future = Async::create(J1, nullptr);

			for (uint8_t i = 0; i < 10; i++) {
				leds[2].toggle();
				Task::delay(500);
			}

			future.await(); // Lazy

			while (true) {
				leds[0].toggle();
				Task::delay(500);
			}
		};

		Task::create(J0, nullptr, 1, "J0");
	}
}

#endif