#ifndef _MOS_USER_GLOBAL_
#define _MOS_USER_GLOBAL_

// STM32F4xx HAL
#include "src/drivers/stm32f4xx/hal.hpp"

// Devices
#include "src/drivers/device/st7735s.hpp"
#include "src/drivers/device/led.hpp"
#include "src/mos/kernel/sync.hpp"

// Put user global data here
namespace MOS::UserGlobal
{
	using namespace HAL::STM32F4xx;
	using namespace Driver;

	// Serial input and output
	auto& uart = convert(USART3);

	// Shell handler
	Task::TcbPtr_t shell_handler = nullptr;

	// LEDs
	LED_t leds[] = {
	        {GPIOB, GPIO_Pin_14}, // red
	        {GPIOB,  GPIO_Pin_0}, // green
	        {GPIOB,  GPIO_Pin_7}, // blue
	};

	ST7735S lcd {
	        SPI1,
	        {GPIOA,  GPIO_Pin_5}, // SCLK(SCL)  -> PA5
	        {GPIOA,  GPIO_Pin_7}, // MOSI(SDA)  -> PA7
	        {GPIOD, GPIO_Pin_14}, // CS(SS)     -> PD14
	        {GPIOB,  GPIO_Pin_8}, // RESET(RES) -> PB8
	        {GPIOB,  GPIO_Pin_9}, // DC(RS)     -> PB9
	};

	Sync::MutexImpl_t mutex;
}

#endif