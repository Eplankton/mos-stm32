#ifndef _MOS_USER_TEST_
#define _MOS_USER_TEST_

#include "src/mos/kernel/task.hpp"
#include "src/mos/kernel/sync.hpp"
#include "src/mos/kernel/ipc.hpp"
#include "global.hpp"

namespace MOS::User::Test
{
	using namespace Kernel;
	using namespace Utils;

	void MutexTest()
	{
		static Sync::Mutex_t mutex;

		static auto mtx_test = [](uint32_t ticks) {
			auto name = Task::current()->get_name();
			while (true) {
				mutex.exec([&] {
					for (auto _: Range(0, 5)) {
						kprintf("%s is working...\n", name);
						Task::delay(100_ms);
					}
				});
				Task::delay(ticks);
			}
		};

		static auto launch = [] {
			Task::create(mtx_test, 10_ms, 3, "Mtx3");
			Task::delay(5_ms);
			Task::create(mtx_test, 20_ms, 2, "Mtx2");
			Task::delay(5_ms);
			Task::create(mtx_test, 30_ms, 1, "Mtx1");
		};

		Task::create(launch, nullptr, Macro::PRI_MAX, "MutexTest");
	}

	void AsyncTest()
	{
		using Global::leds;

		static auto T1 = [] {
			for (auto _: Range(0, 20)) {
				leds[1].toggle(); // green
				Task::delay(250_ms);
			}
			Task::delay(1000_ms);
			kprintf("Async T1 exits...\n");
		};

		static auto T0 = [] {
			auto future = Task::async(T1, nullptr, "T1");

			for (auto _: Range(0, 10)) {
				leds[2].toggle(); // blue
				Task::delay(500_ms);
			}

			future.await();

			while (true) {
				leds[0].toggle(); // red
				Task::delay(500_ms);
			}
		};

		Task::create(T0, nullptr, 2, "T0");
	}

	void MsgQueueTest()
	{
		static IPC::MsgQueue_t<int, 3> msg_q;

		static auto producer = [](int& msg) {
			while (true) {
				msg_q.send(msg++);
				Task::delay(50_ms);
			}
		};

		static auto consumer = [] {
			while (true) {
				int msg  = -1;
				auto res = msg_q.recv(msg, 100_ms);

				IntrGuard_t guard;
				kprintf(res ? "" : "Timeout!\n");
				// kprintf(res ? "%d\n" : "Timeout!\n", msg);
			}
		};

		static auto launch = [] {
			// Create a Consumer
			Task::create(consumer, nullptr, 4, "recv");

			// Mutable Data Sequences
			static int data[] = {5, 6, 7, 8, 9};

			// Create some Producers
			for (auto& i: data) {
				Task::create(
				    producer, &i, (Task::Prior_t) i, "send"
				);
			}
		};

		Task::create(
		    launch, nullptr, Macro::PRI_MAX, "MsgQueueTest"
		);
	}
}

#endif