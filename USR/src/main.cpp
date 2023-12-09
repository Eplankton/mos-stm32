// Kernel & Shell
#include "mos/kernel.hpp"
#include "mos/shell.hpp"

// User space modules
#include "user/global.hpp"
#include "user/bsp.hpp"
#include "user/app.hpp"

int main(void)
{
	using namespace MOS;

	// Init resource
	Bsp::config();

	// Create shell as monitor
	Task::create(Shell::launch, nullptr, 1, "Shell");

	// Create user tasks
	Task::create(App::Task0, nullptr, 1, "T0");
	Task::create(App::GUI, nullptr, 1, "GUI");

	// Task::create(App::MutexTest, nullptr, 1, "T1");
	// Task::create(App::MutexTest, nullptr, 2, "T2");
	// Task::create(App::MutexTest, nullptr, 3, "T3");

	// Start scheduling, never return
	Scheduler::launch();

	while (true) {
		// Never comes here
	}
}