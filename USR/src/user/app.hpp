#ifndef _MOS_USER_APP_
#define _MOS_USER_APP_

// Import Kernel Module
#include "src/mos/kernel.hpp"

#include "src/user/global.hpp"
#include "src/user/gui/GuiLite.h"

// GIFs
#include "src/user/img/face_gif/frames.h"
#include "src/user/img/mac_gif/frames.h"
#include "src/user/img/cat_gif/frames.h"

namespace MOS::User::App
{
	using namespace Utils;
	using namespace Kernel;

	using Color = Driver::Device::ST7735S_t::Color;

	namespace Gui
	{
		using Global::lcd;

		extern "C" void
		gui_delay_ms(uint32_t ms) { Task::delay(ms); }

		extern "C" void
		gfx_draw_pixel(int32_t x, int32_t y, uint32_t rgb)
		{
			lcd.draw_point(x, y, (Color) GL_RGB_32_to_16(rgb));
		}

		struct EXTERNAL_GFX_OP
		{
			using DrawPixelFn_t = void (*)(
			    int32_t x, int32_t y,
			    uint32_t rgb
			);

			using FillRectFn_t = void (*)(
			    int32_t x0, int32_t y0,
			    int32_t x1, int32_t y1,
			    uint32_t rgb
			);

			DrawPixelFn_t draw_pixel;
			FillRectFn_t fill_rect;
		};

		// GUI entry point
		extern "C" void startHello3D(
		    void* phy_fb,
		    int32_t width, int32_t height,
		    int32_t color_bytes,
		    EXTERNAL_GFX_OP* gfx_op
		);
	}

	void GUI()
	{
		using namespace Gui;
		EXTERNAL_GFX_OP gfx_op {gfx_draw_pixel, nullptr};
		startHello3D(
		    nullptr, lcd.width, lcd.height,
		    1, &gfx_op
		);
	}

	void LCD()
	{
		using Sync::Mutex_t;
		using Global::lcd;

		// A mutex wrapper of lcd
		static Mutex_t lcd_mtx {lcd};

		auto GIF = [] {
			while (true) {
				for (auto frame: cat_gif) {
					lcd_mtx.lock().get().draw_img(
					    0, 0, 128, 128, frame
					);
					Task::delay(25_ms);
				}
			}
		};

		auto Slogan = [] {
			constexpr Color rgb[] = {
			    Color::RED,
			    Color::GREEN,
			    Color::GRAYBLUE,
			};

			while (true) {
				for (auto color: rgb) {
					lcd_mtx.lock().get().show_str(
					    5, 132, "hello, world!", color
					);
					Task::delay(250_ms);
				}
			}
		};

		auto pri = Task::current()->get_pri();
		Task::create(GIF, nullptr, pri, "GIF");
		Task::create(Slogan, nullptr, pri, "Slogan");
	}

	void Calendar()
	{
		using HAL::STM32F4xx::RTC_t;

		auto print_rtc_info = [] {
			IntrGuard_t guard;
			const auto date = RTC_t::get_date();
			const auto time = RTC_t::get_time();
			MOS_MSG(
			    "20%0.2d/%0.2d/%0.2d"
			    " %0.2d:%0.2d:%0.2d",
			    date.RTC_Year, date.RTC_Month, date.RTC_Date,
			    time.RTC_Hours, time.RTC_Minutes, time.RTC_Seconds
			);
		};

		while (true) {
			Task::block();
			print_rtc_info();
		}
	}

	// MOS_DEBUG_INFO bool f0 = 0, f1 = 0;
	Sync::Barrier_t bar {2};

	void Task1()
	{
		using Global::leds;
		bar.wait();
		for (auto _: Range(0, 20)) {
			// f1 = !f1;
			leds[1].toggle();
			Task::delay(250_ms);
		}
		kprintf("T1 exits...\n");
	}

	void Task0()
	{
		using Global::leds;
		Task::create(Task1, nullptr, 1, "T1");
		bar.wait();
		while (true) {
			// f0 = !f0;
			leds[0].toggle();
			Task::delay(500_ms);
		}
	}

	void Wifi()
	{
		using Global::wifi_buf;
		while (true) {
			wifi_buf.wait();
			auto rx = wifi_buf.as_str();
			if (atoi(rx) % 10 == 0) {
				kprintf("[esp32] -> %s\n", rx);
			}
			wifi_buf.clear();
		}
	}
}

#endif