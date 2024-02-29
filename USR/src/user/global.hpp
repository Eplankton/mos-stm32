#ifndef _MOS_USER_GLOBAL_
#define _MOS_USER_GLOBAL_

// STM32F4xx HAL
#include "src/drivers/stm32f4xx/hal.hpp"

// Devices
#include "src/drivers/device/st7735s.hpp"
#include "src/drivers/device/led.hpp"

// Buffer
#include "src/mos/kernel/data_type/buffer.hpp"

namespace MOS::User::Global
{
	using namespace HAL::STM32F4xx;
	using namespace Driver::Device;
	using namespace DataType;

	// Serial Input/Output UART
	auto& stdio = convert(USART3);
	SyncRxBuf_t<16> io_buf;

	// ESP32C3 WiFi Module UART
	auto& esp32 = convert(USART2);
	SyncRxBuf_t<8> wifi_buf;

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
}

#endif