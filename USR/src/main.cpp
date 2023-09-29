#include "main.h"
#include "mos.hpp"

static void NVIC_Config()
{
	NVIC_t::group_config(NVIC_PriorityGroup_2);
}

static void LED_Config()
{
	using MOS::GlobalRes::leds;

	RCC_t::AHB1::clock_cmd(RCC_AHB1Periph_GPIOB, ENABLE);
	for (auto& led: leds) {
		led.init();
	}
}

static void USART_Config()
{
	using MOS::GlobalRes::uart;

	RCC_t::AHB1::clock_cmd(RCC_AHB1Periph_GPIOD, ENABLE);
	RCC_t::APB1::clock_cmd(RCC_APB1Periph_USART3, ENABLE);

	uart.init(9600, USART_WordLength_8b, USART_StopBits_1, USART_Parity_No)
	        .rx_config(GPIOD, GPIO_t::get_pin_src(9), GPIO_AF_USART3)
	        .tx_config(GPIOD, GPIO_t::get_pin_src(8), GPIO_AF_USART3)
	        .enable();
}

static void Welcome()
{
	using MOS::GlobalRes::uart;
	uart.send_string("[MOS]: Hello :)\n");
}

static void Resource_Config()
{
	NVIC_Config();
	USART_Config();
	LED_Config();
	Welcome();
}

void Task0(void* argv)
{
	using namespace MOS::GlobalRes;
	while (true) {
		delay(500);
		leds[0].toggle();
		uart.send_string("[MOS]: Task0\n");
	}
}

void Task1(void* argv)
{
	using namespace MOS::GlobalRes;
	while (true) {
		delay(750);
		leds[1].toggle();
		uart.send_string("[MOS]: Task1\n");
	}
}

void Task2(void* argv)
{
	using namespace MOS::GlobalRes;
	while (true) {
		delay(1000);
		leds[2].toggle();
		uart.send_string("[MOS]: Task2\n");
	}
}

void Task3(void* argv)
{
	using namespace MOS::GlobalRes;
	for (uint8_t i = 0; i < 5; i++) {
		delay(2000);
		uart.send_string("[MOS]: Task3\n");
	}

	MOS::Task::terminate();
}

int main(void)
{
	using namespace MOS;

	Resource_Config();

	Task::create(Task0, nullptr, 0);
	Task::create(Task1, nullptr, 1);
	Task::create(Task2, nullptr, 2);
	// Task::create(Task3, nullptr, 3);

	Scheduler::launch();

	while (true) {
		// loop!()
	}
}