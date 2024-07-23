#ifndef _MOS_USER_APP_
#define _MOS_USER_APP_

// Import Kernel and Shell Module
#include "src/mos/kernel.hpp"
#include "src/mos/shell.hpp"

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
		// A mutex wrapper of lcd
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

		Shell::usr_cmds.add({"time", print_rtc_info});
	}

	// MOS_DEBUG_INFO bool f0 = 0, f1 = 0;
	Sync::Barrier_t bar {2};

	void led1(Device::LED_t leds[])
	{
		bar.wait();
		for (auto _: Range(0, 20)) {
			// f1 = !f1;
			leds[1].toggle();
			Task::delay(250_ms);
		}
		kprintf(
		    "%s exits...\n",
		    Task::current()->get_name()
		);
	}

	void led0(Device::LED_t leds[])
	{
		Task::create(
		    led1, leds,
		    Task::current()->get_pri(),
		    "led1"
		);
		bar.wait();
		while (true) {
			// f0 = !f0;
			leds[0].toggle();
			Task::delay(500_ms);
		}
	}

	void wifi(DataType::SyncRxBuf_t<8>& buf)
	{
		while (true) {
			buf.wait();
			auto rx = buf.as_str();
			if (atoi(rx) % 10 == 0) {
				kprintf("[esp32] -> %s\n", rx);
			}
			buf.clear();
		}
	}

	void log_init()
	{
		using FatFs  = FileSys::FatFs;
		using File_t = FatFs::File_t;

		static FIL file_raw; // 文件裸对象

		static auto lgw_cmd = [](auto text) {
			File_t file {file_raw}; // 文件对象
			auto res = file.open(   // 打开文件，如果文件不存在则创建它
			    "0:log.txt", File_t::OpenMode::Write
			);

			if (res == FR_OK) {
				auto [res, num] = file.write( // 将指定存储区内容写入到文件内
				    (void*) text, strlen(text)
				);
				MOS_MSG("Write(%d) <- \"%s\"", num, text);
			}
			else {
				MOS_MSG("File open failed!");
			}
		};

		static auto cat_cmd = [](auto name) {
			File_t file {file_raw}; // 文件对象
			auto append_str = [](char* dest, const char* src) {
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

			char path[16] = "0:", r_buf[32] = "";

			append_str(path, name);

			auto res = file.open( // 打开文件，如果文件不存在则创建它
			    path, File_t::OpenMode::Read
			);

			if (res == FR_OK) {
				auto [res, num] = file.read(
				    (void*) r_buf, sizeof(r_buf)
				);
				MOS_MSG("Read(%d) -> \"%s\"", num, r_buf);
			}
			else {
				MOS_MSG("File open failed!");
			}
		};

		// log read cmd
		static auto lgr_cmd = [](auto _) { cat_cmd("log.txt"); };

		Shell::usr_cmds.add({"cat", cat_cmd});
		Shell::usr_cmds.add({"lgr", lgr_cmd});
		Shell::usr_cmds.add({"lgw", lgw_cmd});
	}
}

#endif