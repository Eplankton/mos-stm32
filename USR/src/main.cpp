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
	Task::create(App::time_init, nullptr, 1, "time/init");

	// Create Shell with io_buf
	Task::create(Shell::launch, &sh_buf, 1, "shell");

	// Create FatFs on SD card
	Task::create(FileSys::rw_test, &fatfs, 2, "fs/rw_test");

	// Create Log System
	Task::create(App::log_init, nullptr, 2, "log/init");

	/* User Tasks */
	Task::create(App::led0, &leds, 2, "led0");
	// Task::create(App::gui, nullptr, 3, "gui", 256);
	Task::create(App::lcd_init, &lcd, 3, "lcd/init");
	Task::create(App::wifi, &wifi_buf, 3, "wifi");

	/* Test examples */
	// Test::MutexTest();
	Test::MsgQueueTest();

	// Start scheduling, never return
	Scheduler::launch();
}