#ifndef _MOS_USER_TEST_
#define _MOS_USER_TEST_

#include "src/mos/kernel/task.hpp"
#include "src/mos/kernel/sync.hpp"
#include "src/mos/kernel/ipc.hpp"
#include "global.hpp"

namespace MOS::Test
{
	using namespace Utils;

	void MutexTest()
	{
		static Sync::Mutex_t mutex; // Take by 1-2-3... order

		static auto mtx_test = [](void* argv) {
			auto name = Task::current()->get_name();
			while (true) {
				mutex.exec([&] {
					for (auto _: Range(0, 5)) {
						kprintf("%s is working...\n", name);
						Task::delay(100);
					}
				});
				Task::delay(5);
			}
		};

		Task::create(mtx_test, nullptr, 1, "Mtx1");
		Task::create(mtx_test, nullptr, 2, "Mtx2");
		Task::create(mtx_test, nullptr, 3, "Mtx3");
	}

	void AsyncTest()
	{
		using UserGlobal::leds;

		static auto T1 = [](void* argv) {
			for (auto _: Range(0, 20)) {
				leds[1].toggle(); // green
				Task::delay(250);
			}
			Task::delay(1000);
			kprintf("Async T1 exits...\n");
		};

		static auto T0 = [](void* argv) {
			auto future = Task::async(T1, nullptr, "T1");

			for (auto _: Range(0, 10)) {
				leds[2].toggle(); // blue
				Task::delay(500);
			}

			future.await();

			while (true) {
				leds[0].toggle(); // red
				Task::delay(500);
			}
		};

		Task::create(T0, nullptr, 2, "T0");
	}

	void MsgQueueTest()
	{
		using MsgQueue_t = IPC::MsgQueue_t<int, 3>;

		static MsgQueue_t msg_q;

		static auto send = [](int msg) {
			while (true) {
				msg_q.send(msg);
				Task::delay(50);
			}
		};

		static auto recv = [](void* argv) {
			while (true) {
				int msg  = -1;
				auto res = msg_q.recv(msg, 100);

				DisIntrGuard_t guard;
				kprintf(res ? "" : "Timeout!\n");
				// kprintf(res ? "%d\n" : "Timeout!\n", msg);
			}
		};

		static auto launch = [](void* argv) {
			const int data[] = {5, 6, 7, 8, 9};
			Task::create(recv, nullptr, 4, "recv");
			for (auto msg: data) {
				Task::create(send, msg, (Task::Prior_t) msg, "send");
			}
		};

		Task::create(launch, nullptr, 0, "msgq");
	}
}

#endif