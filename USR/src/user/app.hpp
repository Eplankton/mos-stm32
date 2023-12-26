#ifndef _MOS_USER_APP_
#define _MOS_USER_APP_

#include "src/mos/kernel/sync.hpp"
#include "src/user/global.hpp"
#include "src/mos/kernel/task.hpp"
#include "src/user/gui/GuiLite.h"
#include "src/user/img/cat_gif.h"

namespace MOS::App
{
	namespace GuiPort
	{
		using Color = enum Driver::ST7735S::Color;

		extern "C" void gui_delay_ms(uint32_t ms) { Task::delay(ms); }
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

	static Sync::MutexImpl_t lcd_mutex {1};

	void LCD(void* argv)
	{
		using namespace Driver;
		using enum ST7735S::Color;
		using UserGlobal::lcd;

		auto Slogan = [](void* argv) {
			while (true) {
				lcd_mutex.lock();
				lcd.show_string(0, 130, "Hello, World!", GREEN);
				lcd_mutex.unlock();
			}
		};

		auto GIF = [](void* argv) {
			while (true) {
				for (auto frame: cat_gif) {
					lcd_mutex.lock();
					lcd.draw_img(0, 0, 128, 128, frame);
					lcd_mutex.unlock();
				}
			}
		};

		Task::create(Slogan, nullptr, 1, "Slogan");
		Task::create(GIF, nullptr, 1, "GIF");
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