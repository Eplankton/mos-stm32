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
		using Color = Driver::Device::ST7735S_t::Color;
		using UserGlobal::lcd;

		extern "C" void gui_delay_ms(uint16_t ms) { Task::delay(ms); }

		extern "C" void gfx_draw_pixel(
		        int32_t x, int32_t y,
		        uint32_t rgb)
		{
			lcd.draw_point(x, y, (Color) GL_RGB_32_to_16(rgb));
		}

		struct EXTERNAL_GFX_OP
		{
			using DrawPixelFn_t = void (*)(
			        int32_t x, int32_t y,
			        uint32_t rgb);

			using FillRectFn_t = void (*)(
			        int32_t x0, int32_t y0,
			        int32_t x1, int32_t y1,
			        uint32_t rgb);

			DrawPixelFn_t draw_pixel;
			FillRectFn_t fill_rect;
		};

		// GUI entry point
		extern "C" void startHello3D(
		        void* phy_fb,
		        int32_t width, int32_t height,
		        int32_t color_bytes,
		        EXTERNAL_GFX_OP* gfx_op);
	}

	void GUI(void* argv)
	{
		using namespace Gui;
		EXTERNAL_GFX_OP gfx_op {gfx_draw_pixel, nullptr};
		startHello3D(nullptr, lcd.width, lcd.height, 1, &gfx_op);
	}

	void LCD(void* argv)
	{
		using Color = Driver::Device::ST7735S_t::Color;
		using Sync::Mutex_t;
		using UserGlobal::lcd;

		const auto sub_ceiling = Task::current()->get_pri();
		static auto lcd_mtx    = Mutex_t {lcd, sub_ceiling};

		auto GIF = [](void* argv) {
			while (true) {
				for (auto frame: cat_gif) {
					lcd_mtx.lock().get().draw_img(
					        0, 0,
					        128, 128,
					        frame);
				}
				Task::delay(1);
			}
		};

		auto Slogan = [](void* argv) {
			constexpr Color rgb[] = {
			        Color::RED,
			        Color::GREEN,
			        Color::GRAYBLUE,
			};

			while (true) {
				for (const auto color: rgb) {
					lcd_mtx.lock().get().show_string(
					        0, 130,
					        "Hello, World!",
					        color);
					Task::delay(100);
				}
			}
		};

		Task::create(GIF, nullptr, sub_ceiling, "GIF");
		Task::create(Slogan, nullptr, sub_ceiling, "Slogan");

		while (true) {
			Task::block();
		}
	}

	void Calendar(void* argv)
	{
		using HAL::STM32F4xx::RTC_t;
		using Utils::DisIntrGuard_t;

		static auto print_date_and_time = [] {
			DisIntrGuard_t guard;
			const auto date = RTC_t::get_date();
			const auto time = RTC_t::get_time();
			MOS_MSG("20%0.2d/%0.2d/%0.2d "
			        "%0.2d:%0.2d:%0.2d\n",
			        date.RTC_Year, date.RTC_Month, date.RTC_Date,
			        time.RTC_Hours, time.RTC_Minutes, time.RTC_Seconds);
		};

		while (true) {
			Task::block();
			print_date_and_time();
		}
	}

	void Task1(void* argv)
	{
		using UserGlobal::leds;
		for (uint8_t i = 0; i < 20; i++) {
			leds[1].toggle();
			Task::delay(125);
		}
		kprintf("T1 exits...\n");
	}

	void Task0(void* argv)
	{
		using UserGlobal::leds;
		Task::create(Task1, nullptr, 1, "T1");
		while (true) {
			leds[0].toggle();
			Task::delay(250);
		}
	}
}

#endif