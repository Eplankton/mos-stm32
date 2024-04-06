// MOS Kernel & Shell
#include "mos/kernel.hpp"
#include "mos/shell.hpp"

// User Application
#include "user/global.hpp"
#include "user/bsp.hpp"
#include "user/app.hpp"
#include "user/fatfs.hpp"
#include "user/test.hpp"

int main()
{
	using namespace MOS;
	using namespace Kernel;
	using namespace User;
	using namespace User::Global;

	// Init hardware and clocks
	BSP::config();

	// Create Calendar with RTC
	Task::create(App::Calendar, nullptr, 0, "Calendar");

	// Create Shell with io_buf
	Task::create(Shell::launch, &io_buf, 1, "Shell");

	// Create FatFs on SD card
	Task::create(FatFs::test, nullptr, 2, "FatFs");

	/* User Tasks */
	Task::create(App::LED_0, &leds, 2, "L0");
	// Task::create(App::GUI, nullptr, 3, "gui", 256);
	Task::create(App::LCD, &lcd, 3, "LCD");
	Task::create(App::WiFi, &wifi_buf, 3, "WiFi");

	/* Test examples */
	// Test::MutexTest();
	// Test::MsgQueueTest();

	// Start scheduling, never return
	Scheduler::launch();
}