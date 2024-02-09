#ifndef _MOS_USER_TEST_
#define _MOS_USER_TEST_

#include "src/mos/kernel/task.hpp"
#include "src/mos/kernel/sync.hpp"
#include "src/mos/kernel/ipc.hpp"
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
			for (uint8_t i = 0; i < 20; i++) {
				leds[1].toggle(); // green
				Task::delay(250);
			}
			Task::delay(1000);
			kprintf("Async J1 exits...\n");
		};

		static auto J0 = [](void* argv) {
			auto future = Task::async(J1, nullptr, "J1");

			for (uint8_t i = 0; i < 10; i++) {
				leds[2].toggle(); // blue
				Task::delay(500);
			}

			future.await();

			while (true) {
				leds[0].toggle(); // red
				Task::delay(500);
			}
		};

		Task::create(J0, nullptr, 2, "J0");
	}

	void MsgQueueTest()
	{
		using Utils::DisIntrGuard_t;
		using MailBox_t = IPC::MsgQueue_t<int, 3>;

		static MailBox_t mailbox;

		static auto send = [](void* argv) {
			while (true) {
				auto& val = *(int*) argv;
				mailbox.send(val, 0);
				Task::delay(5);
			}
		};

		static auto recv = [](void* argv) {
			while (true) {
				int val  = -1;
				auto res = mailbox.recv(val, 100);

				DisIntrGuard_t guard;
				// kprintf(res ? "%d\n" : "Timeout!\n", val);
				kprintf(res ? "" : "Timeout!\n");
			}
		};

		static auto impl = [](void* argv) {
			Task::create(recv, nullptr, 5, "recv");
			static int s[] = {0, 1, 2, 3, 4};
			for (auto& i: s) {
				Task::create(send, &i, 6, "send");
			}
		};

		Task::create(impl, nullptr, 0, "MsgQueTst");
	}
}

#endif