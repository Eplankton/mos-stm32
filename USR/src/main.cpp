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

	BSP::config(); // Init hardware and clocks

	// Create Calendar with RTC
	Task::create(App::time_init, nullptr, 1, "time/init");

	// Create Shell with sh_buf
	Task::create(Shell::launch, &stdio.buf, 1, "shell");

	// Create FatFs on SD card
	Task::create(FileSys::init, &fatfs, 2, "fs/init");

	// Create Log System
	Task::create(App::log_init, &sys_log, 2, "log/init");
	/* User Tasks */
	Task::create(App::led_init, &leds, 2, "led/init");
	// Task::create(App::gui, nullptr, 3, "gui", 256);
	Task::create(App::lcd_init, &lcd, 3, "lcd/init");
	Task::create(App::wifi, &esp32.buf, 3, "wifi");

	/* Test examples */
	// Test::MutexTest();
	Test::MsgQueueTest();

	// Start scheduling, never return
	Scheduler::launch();
}