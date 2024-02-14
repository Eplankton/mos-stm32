// MOS Shell
#include "mos/shell.hpp"

// User Application
#include "user/global.hpp"
#include "user/bsp.hpp"
#include "user/app.hpp"
#include "user/test.hpp"

uint32_t tst_pkb[256];

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

	DataType::Page_t page {
	        .policy = DataType::Page_t::Policy::STATIC,
	        .raw    = tst_pkb,
	        .size   = sizeof(tst_pkb) / sizeof(size_t),
	};

	// Create user tasks
	Task::create(App::Task0, nullptr, 2, "T0", page);
	// Task::create(App::GUI, nullptr, 3, "GUI");
	Task::create(App::LCD, nullptr, 3, "LCD");

	// Test examples
	// Test::MutexTest();
	// Test::AsyncTest();
	// Test::MsgQueueTest();

	// Start scheduling, never return
	Scheduler::launch();

	while (true) {
		// Never run to here
	}
}