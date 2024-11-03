#ifndef _MOS_USER_APP_
#define _MOS_USER_APP_

// Import Kernel and Shell Module
#include "src/core/kernel.hpp"
#include "src/core/shell.hpp"

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
	using namespace Driver;

	using Color = Device::ST7735S_t::Color;

	namespace GUI
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

	void gui()
	{
		using namespace GUI;
		EXTERNAL_GFX_OP gfx_op {gfx_draw_pixel, nullptr};
		startHello3D(
		    nullptr, lcd.width, lcd.height,
		    1, &gfx_op
		);
	}

	void lcd_init(Device::ST7735S_t& lcd)
	{
		// A mutex wrapper of lcd&
		static Sync::Mutex_t lcd_mtx {lcd};

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
		Task::create(GIF, nullptr, pri, "gif");
		Task::create(Slogan, nullptr, pri, "slogan");
	}

	void time_init()
	{
		using HAL::STM32F4xx::RTC_t;

		auto print_rtc_info = [](auto argv) {
			IrqGuard_t guard;
			const auto date = RTC_t::get_date();
			const auto time = RTC_t::get_time();
			MOS_MSG(
			    "20%0.2d/%0.2d/%0.2d"
			    " %0.2d:%0.2d:%0.2d",
			    date.RTC_Year, date.RTC_Month, date.RTC_Date,
			    time.RTC_Hours, time.RTC_Minutes, time.RTC_Seconds
			);
		};

		Shell::add_usr_cmd({"time", print_rtc_info});
	}

	void led_init(Device::LED_t leds[])
	{
		static Sync::Barrier_t bar {2};

		static auto led1 = [](Device::LED_t leds[]) {
			bar.wait();
			for (auto _: Range(0, 20)) {
				leds[1].toggle();
				Task::delay(250_ms);
			}
			kprintf(
			    "%s exits...\n",
			    Task::current()->get_name()
			);
		};

		static auto led0 = [](Device::LED_t leds[]) {
			Task::create(
			    led1, leds,
			    Task::current()->get_pri(),
			    "led1"
			);
			bar.wait();
			while (true) {
				leds[0].toggle();
				Task::delay(500_ms);
			}
		};

		Task::create(
		    led0, leds,
		    Task::current()->get_pri(),
		    "led0"
		);
	}

	void wifi(DataType::SyncRxBuf_t<8>& buf)
	{
		auto runner = [](auto rx) {
			if (atoi(rx) % 10 == 0) {
				kprintf("[esp32] -> %s\n", rx);
				Global::sys_log_q.send(rx);
			}
		};

		while (true) {
			runner(buf.recv().as_str());
		}
	}

	void log_init(Sync::Mutex_t<FileSys::File_t>& sys_log)
	{
		using OpenMode = FileSys::File_t::OpenMode;

		static auto& log_mtx {sys_log}; // 系统日志文件

		static auto lgw_cmd = [](auto text) {
			auto mtx_grd = log_mtx.lock();
			auto& log    = mtx_grd.get();

			// 打开文件，如果文件不存在则创建它
			auto res = log.open("0:log.txt", OpenMode::Write);

			if (res == FR_OK) { // 将指定存储区内容写入到文件内
				auto [_, num] = log.write((void*) text, strlen(text));
				MOS_MSG("W(%d) <- \"%s\"", num, text);
			}
			else {
				MOS_MSG("File open failed!");
			}

			log.close(); // 关闭文件
		};

		static auto cat_cmd = [](auto name) {
			auto get_path = [](char* dest, const char* src) {
				while (*dest) { // 找到目标字符串的末尾
					dest++;
				}

				while (*src) { // 复制源字符串到目标字符串的末尾
					if ((*dest = *src) != '\0') {
						dest++;
						src++;
					}
					else {
						break;
					}
				}

				*dest = '\0'; // 确保目标字符串以'\0'结尾
			};

			auto mtx_grd = log_mtx.lock();
			auto& log    = mtx_grd.get();

			char dir[16] = "0:", r_buf[32] = "";
			get_path(dir, name);

			// 打开文件，如果文件不存在则创建它
			auto res = log.open(dir, OpenMode::Read);

			if (res == FR_OK) { // 将文件内容读取到缓冲区
				auto [_, num] = log.read((void*) r_buf, sizeof(r_buf));
				MOS_MSG("R(%d) -> \"%s\"", num, r_buf);
			}
			else {
				MOS_MSG("File open failed!");
			}

			log.close(); // 关闭文件
		};

		// log read cmd
		auto lgr_cmd = [](auto _) { cat_cmd("log.txt"); };
		Shell::add_usr_cmd({"cat", cat_cmd});
		Shell::add_usr_cmd({"lgr", lgr_cmd});
		Shell::add_usr_cmd({"lgw", lgw_cmd});

		static auto log = [] {
			while (true) {
				Global::sys_log_q.recv(1000_ms).ok_or(
					[](auto msg) { lgw_cmd(msg); },
					[] { /* oops */ }
				);
			}
		};

		Task::create(log, nullptr, Task::current()->get_pri(), "log");
	}
}

#endif