// MOS Kernel & Shell
#include "mos/kernel.hpp"
#include "mos/shell.hpp"

// User modules
#include "user/global.hpp"
#include "user/bsp.hpp"
#include "user/app.hpp"
#include "user/test.hpp"

int main(void)
{
	using namespace MOS;
	using UserGlobal::rx_buf;

	// Init hardware and clocks
	Bsp::config();

	// Create Calendar with RTC
	Task::create(App::Calendar, nullptr, 0, "Calendar");

	// Create Shell with rx_buf
	Task::create(Shell::launch, &rx_buf, 1, "Shell");

	// Create user tasks
	Task::create(App::Task0, nullptr, 2, "T0");
	// Task::create(App::GUI, nullptr, 3, "GUI");
	Task::create(App::LCD, nullptr, 3, "LCD");

	// Test examples
	// Task::create(Test::MutexTest, nullptr, 1, "T1");
	// Task::create(Test::MutexTest, nullptr, 2, "T2");
	// Task::create(Test::MutexTest, nullptr, 3, "T3");

	// Start scheduling, never return
	Scheduler::launch();

	while (true) {
		// Never run to here
	}
}