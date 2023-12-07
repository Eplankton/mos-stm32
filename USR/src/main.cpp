// Kernel & Shell
#include "mos/kernel/kernel.hpp"
#include "mos/shell.hpp"

// User space
#include "user/global.hpp"
#include "user/bsp.hpp"
#include "user/app.hpp"

namespace MOS::ISR
{
	extern "C" void PendSV_Handler()
	{
		asm volatile("B     ContextSwitch");
	}

	extern "C" void SysTick_Handler()
	{
		using KernelGlobal::os_ticks;

		// Trigger PendSV
		Util::DisIntrGuard guard;
		os_ticks += 1;
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
	extern "C" void USART3_IRQHandler()
	{
		using enum DataType::TCB_t::Status_t;
		using KernelGlobal::rx_buf;
		using UserGlobal::uart;
		using UserGlobal::shell_handler;

		if (uart.get_it_status(USART_IT_RXNE) != RESET) {
			char data = uart.receive_data();
			if (!rx_buf.full()) {
				rx_buf.add(data);
				if (data == '\n' && shell_handler != nullptr) {
					Task::resume(shell_handler);
				}
			}
			else {
				rx_buf.clear();
				MOS_MSG("[MOS]: Command too long\n\n");
			}
		}
	}
}

int main(void)
{
	using namespace MOS;
	using UserGlobal::shell_handler;

	// Init resource
	Bsp::config();

	// Create shell as monitor
	shell_handler = Task::create(Shell::launch,
	                             nullptr,
	                             Macro::PRI_MAX,
	                             "Shell");

	// Create user tasks
	Task::create(App::Task0, nullptr, 1, "T0");
	Task::create(App::GUI, nullptr, 1, "GUI");

	// Task::create(App::MutexTest, nullptr, 1, "T1");
	// Task::create(App::MutexTest, nullptr, 2, "T2");
	// Task::create(App::MutexTest, nullptr, 3, "T3");

	// Start scheduling, never return
	Scheduler::launch();

	while (true) {
		// Never comes here
	}
}