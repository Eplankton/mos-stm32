#ifndef _MOS_USER_APP_
#define _MOS_USER_APP_

#include "src/user/global.hpp"
#include "src/user/gui/GuiLite.h"

namespace MOS::App
{
	namespace GuiPort
	{
		extern "C" void gui_delay_ms(uint32_t ms) { Task::delay(ms); }
		extern "C" void gfx_draw_pixel(int x, int y, unsigned int rgb)
		{
			UserGlobal::lcd.draw_point(x, y, (Driver::ST7735S::Color) GL_RGB_32_to_16(rgb));
		}

		struct EXTERNAL_GFX_OP
		{
			void (*draw_pixel)(int x, int y, unsigned int rgb);
			void (*fill_rect)(int x0, int y0, int x1, int y1, unsigned int rgb);
		};

		// GUI entry point
		extern "C" void startHello3D(void* phy_fb, int width, int height,
		                             int color_bytes, EXTERNAL_GFX_OP* gfx_op);
	}

	void GUI(void* argv)
	{
		using namespace GuiPort;
		using UserGlobal::lcd;

		EXTERNAL_GFX_OP gfx_op {gfx_draw_pixel, nullptr};
		startHello3D(NULL, lcd.width, lcd.height, 1, &gfx_op);
	}

	void LCD(void* argv)
	{
		using namespace Driver;
		using UserGlobal::lcd;
		using enum ST7735S::Color;

		Terminal terminal {lcd};

		constexpr auto logo = " A_A       _\n"
		                      "o'' )_____//\n"
		                      " `_/  MOS  )\n"
		                      " (_(_/--(_/ \n";

		while (true) {
			terminal.println("Hello, World!", GREEN);
			Task::delay(250);

			terminal.print(logo, YELLOW);
			Task::delay(250);
		}
	}

	void Task1(void* argv)
	{
		using UserGlobal::leds;

		for (uint32_t i = 0; i < 20; i++) {
			leds[1].toggle();
			Task::delay(100);
		}
	}

	void Task0(void* argv)
	{
		using UserGlobal::leds;

		Task::create(App::Task1, nullptr, 1, "T1");
		while (true) {
			leds[0].toggle();
			Task::delay(200);
		}
	}

	void MutexTest(void* argv)
	{
		using UserGlobal::mutex;

		auto cur = Task::current_task();
		while (true) {
			mutex.lock();
			for (uint8_t i = 0; i < 5; i++) {
				MOS_MSG("%s is working...\n", cur->get_name());
				Task::delay(100);
			}
			mutex.unlock();
			Task::delay(100);
		}
	}
}

#endif