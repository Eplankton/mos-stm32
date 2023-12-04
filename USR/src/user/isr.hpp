#ifndef _MOS_USER_ISR_
#define _MOS_USER_ISR_

#include "src/user/global.hpp"
#include "src/mos/kernel/global.hpp"

namespace MOS::ISR
{
	extern "C" void PendSV_Handler(void)
	{
		asm("B     ContextSwitch");
	}

	extern "C" void SysTick_Handler(void)
	{
		using KernelGlobal::os_ticks;

		// Trigger PendSV
		Util::DisIntrGuard guard;
		os_ticks++;
		MOS_TRIGGER_PENDSV_INTR();
	}

	// K1 IRQ Handler
	extern "C" void EXTI15_10_IRQHandler()
	{
		using UserGlobal::leds;
		using HAL::STM32F4xx::EXTI_t;

		static auto K1_IRQ = [](void* argv) {
			for (uint32_t i = 0; i < 10; i++) {
				leds[2].toggle();
				Task::print_name();
				Task::delay(100);
			}
		};

		EXTI_t::handle_line(EXTI_Line13, [] {
			MOS_MSG("[MOS]: K1 IRQ!\n");
			Task::create(K1_IRQ, nullptr, 1, "K1");
			// Task::print_all_tasks();
		});
	}

	// UART3 IRQ Handler
	extern "C" void USART3_IRQHandler(void)
	{
		using KernelGlobal::rx_buf;
		using UserGlobal::uart;

		if (uart.get_it_status(USART_IT_RXNE) != RESET) {
			char data = uart.receive_data();
			if (!rx_buf.full()) {
				rx_buf.add(data);
			}
			else {
				rx_buf.clear();
				MOS_MSG("[MOS]: Command too long\n\n");
			}
		}
	}
}

#endif