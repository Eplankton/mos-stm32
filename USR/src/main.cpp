// MOS Kernel & Shell
#include "mos/kernel.hpp"
#include "mos/shell.hpp"

// User Application
#include "user/global.hpp"
#include "user/bsp.hpp"
#include "user/app.hpp"
#include "user/test.hpp"

int main()
{
	using namespace MOS;
	using namespace Kernel;
	using namespace User;

	// Init hardware and clocks
	BSP::config();

	// Create Calendar with RTC
	Task::create(App::Calendar, nullptr, 0, "Calendar");

	// Create Shell with io_buf
	Task::create(
	    Shell::launch,
	    &User::Global::io_buf,
	    1, "Shell"
	);

	/* User Tasks */
	Task::create(App::Task0, nullptr, 2, "T0");
	// Task::create(App::GUI, nullptr, 3, "GUI", 256);
	Task::create(App::LCD, nullptr, 3, "LCD");
	Task::create(App::Wifi, nullptr, 3, "wifi");

	/* Test examples */
	// Test::MutexTest();
	// Test::AsyncTest();
	Test::MsgQueueTest();

	// Start scheduling, never return
	Scheduler::launch();

	while (true) {
		// Never run to here
	}
}