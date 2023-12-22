#ifndef _MOS_USER_APP_
#define _MOS_USER_APP_

#include "src/user/global.hpp"
#include "src/mos/kernel/task.hpp"
#include "src/user/gui/GuiLite.h"
#include "src/user/img/cat_gif.h"

namespace MOS::App
{
	namespace GuiPort
	{
		using Color = enum Driver::ST7735S::Color;

		extern "C" void gui_delay_ms(uint32_t ms) { Util::delay(ms); }
		extern "C" void gfx_draw_pixel(int x, int y, unsigned int rgb)
		{
			UserGlobal::lcd.draw_point(x, y, (Color) GL_RGB_32_to_16(rgb));
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
		using enum ST7735S::Color;
		using UserGlobal::lcd;

		Terminal terminal {lcd};

		constexpr auto logo = " A_A       _\n"
		                      "o'' )_____//\n"
		                      " `_/  MOS  )\n"
		                      " (_(_/--(_/ \n";

		while (true) {
			// terminal.println("Hello, World!", GREEN);
			// Task::delay(250);

			// terminal.print(logo, YELLOW);
			// Task::delay(250);

			for (auto frame: cat_gif_frames) {
				lcd.draw_img(0, 0, 128, 128, frame);
			}
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
}

#endif