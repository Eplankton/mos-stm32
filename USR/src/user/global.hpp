#ifndef _MOS_USER_GLOBAL_
#define _MOS_USER_GLOBAL_

// STM32F4xx HAL
#include "src/drivers/stm32f4xx/hal.hpp"

// Devices
#include "src/drivers/device/st7735s.hpp"
#include "src/drivers/device/led.hpp"
#include "src/drivers/device/sd.hpp"

// Buffer
#include "src/mos/kernel/data_type/buffer.hpp"

// MsgQueue Channel
#include "src/mos/kernel/ipc.hpp"

// FatFs File System
#include "src/user/fatfs.hpp"

namespace MOS::User::Global
{
	using namespace HAL::STM32F4xx;
	using namespace Driver::Device;
	using namespace Kernel::Sync;
	using namespace Kernel::IPC;
	using namespace DataType;
	using namespace FileSys;

	// File System Components
	FatFs fatfs;
	RawFile_t raw_sys_log;
	Mutex_t sys_log {File_t {raw_sys_log}};
	MsgQueue_t<const char*, 2> sys_log_q;

	template <size_t N>
	struct SyncUartDev_t
	{
		using Port_t = USART_t;
		using Buf_t  = SyncRxBuf_t<N>;

		Port_t& port;
		Buf_t buf;

		void read_line(auto&& oops)
		{
			port.handle_it(USART_IT_RXNE, [&] {
				char8_t data = port.recv_data();
				if (!buf.full()) {
					if (data == '\n') // read a line
						buf.signal_from_isr();
					else
						buf.add(data);
				}
				else {
					buf.clear();
					oops();
				}
			});
		}
	};

	// Serial Input/Output UART
	auto stdio = SyncUartDev_t<SHELL_BUF_SIZE> {convert(USART3)};

	// ESP32C3 WiFi Module UART
	auto esp32 = SyncUartDev_t<8> {convert(USART2)};

	// RGB LEDs
	LED_t leds[] = {
	    {GPIOB, GPIO_Pin_14}, // red -> PB14
	    {GPIOB,  GPIO_Pin_0}, // green -> PB0
	    {GPIOB,  GPIO_Pin_7}, // blue -> PB7
	};

	// LCD with ST7735S SPI Driver
	ST7735S_t lcd {
	    SPI1,
	    {GPIOA,  GPIO_Pin_5}, // SCLK(SCL)  -> PA5
	    {GPIOA,  GPIO_Pin_7}, // MOSI(SDA)  -> PA7
	    {GPIOD, GPIO_Pin_14}, // CS(SS)     -> PD14
	    {GPIOB,  GPIO_Pin_8}, // RESET(RES) -> PB8
	    {GPIOB,  GPIO_Pin_9}, // DC(RS)     -> PB9
	};

	// SD Card with SPI Driver
	SD_t sd {
	    SPI5,
	    {GPIOF, GPIO_Pin_7}, // PF7 -> SCLK
	    {GPIOF, GPIO_Pin_8}, // PF8 -> MISO
	    {GPIOF, GPIO_Pin_9}, // PF9 -> MOSI
	    {GPIOE, GPIO_Pin_3}, // PE3 -> CS
	};
}

#endif