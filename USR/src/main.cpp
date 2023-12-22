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

	// Init hardware and clocks
	Bsp::config();

	// Create shell as monitor
	Task::create(Shell::launch, nullptr, 1, "Shell");

	// Create user tasks
	Task::create(App::Task0, nullptr, 1, "T0");
	Task::create(App::GUI, nullptr, 1, "GUI");
	// Task::create(App::LCD, nullptr, 1, "LCD");

	// Test example
	// Task::create(Test::MutexTest, nullptr, 1, "T1");
	// Task::create(Test::MutexTest, nullptr, 2, "T2");
	// Task::create(Test::MutexTest, nullptr, 3, "T3");

	// Start scheduling, never return
	Scheduler::launch();

	while (true) {
		// Never comes here
	}
}