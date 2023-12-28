#ifndef _MOS_USER_APP_
#define _MOS_USER_APP_

#include "src/mos/kernel/task.hpp"
#include "src/mos/kernel/sync.hpp"
#include "src/user/global.hpp"
#include "src/user/gui/GuiLite.h"
#include "src/user/img/cat_gif.h"

namespace MOS::App
{
	namespace Gui
	{
		using Color = enum Driver::ST7735S::Color;
		using UserGlobal::lcd;

		extern "C" void gui_delay_ms(uint32_t ms) { Task::delay(ms); }
		extern "C" void gfx_draw_pixel(int x, int y, unsigned int rgb)
		{
			lcd.draw_point(x, y, (Color) GL_RGB_32_to_16(rgb));
		}

		struct EXTERNAL_GFX_OP
		{
			void (*draw_pixel)(int x, int y, unsigned int rgb);
			void (*fill_rect)(int x0, int y0, int x1, int y1, unsigned int rgb);
		};

		// GUI entry point
		extern "C" void startHello3D(
		        void* phy_fb,
		        int width,
		        int height,
		        int color_bytes,
		        EXTERNAL_GFX_OP* gfx_op);
	}

	void GUI(void* argv)
	{
		using namespace Gui;
		EXTERNAL_GFX_OP gfx_op {gfx_draw_pixel, nullptr};
		startHello3D(NULL, lcd.width, lcd.height, 1, &gfx_op);
	}

	void LCD(void* argv)
	{
		using enum Driver::ST7735S::Color;
		using UserGlobal::lcd;

		static Sync::Mutex_t<void> lcd_mutex {1};

		auto GIF = [](void* argv) {
			while (true) {
				for (auto frame: cat_gif) {
					auto guard = lcd_mutex.lock();
					lcd.draw_img(0, 0, 128, 128, frame);
				}
			}
		};

		auto Slogan = [](void* argv) {
			while (true) {
				auto guard = lcd_mutex.lock();
				lcd.show_string(0, 130, "Hello, World!", GREEN);
			}
		};

		Task::create(GIF, nullptr, 1, "GIF");
		Task::create(Slogan, nullptr, 1, "Slogan");

		while (true) {
			asm volatile("");
		}
	}

	void Task1(void* argv)
	{
		using UserGlobal::leds;

		for (uint32_t i = 0; i < 20; i++) {
			leds[1].toggle();
			Task::delay(100);
		}

		kprintf("T1 exits...\n");
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