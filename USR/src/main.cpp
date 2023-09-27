#include "main.h"

namespace MOS
{
	// Task Control Block => TCB
	struct __attribute__((packed)) TCB
	{
		using Self_t     = TCB;
		using StackPtr_t = uint32_t*;// 32-bit address
		using Ret_t      = int8_t;
		using Argv_t     = void*;
		using Fn_t       = Ret_t (*)(Argv_t);

		struct list_node
		{
			TCB* prev = nullptr;
			TCB* next = nullptr;
		} node;

		StackPtr_t sp = nullptr;

		// Other elements
		Fn_t func   = nullptr;
		Argv_t argv = nullptr;

		TCB() = default;
		TCB(Fn_t&& f): func(f) {}
	};

	// Global Resources
	namespace GlobalRes
	{
		auto& uart = convert(USART3);

		LED_t leds[] = {
		        {GPIOB, GPIO_Pin_14},
		        {GPIOB,  GPIO_Pin_0},
		        {GPIOB,  GPIO_Pin_7},
		};

		extern "C" {
		// Put in extern "C" because the name is referred in asm("") for name demangle
		__attribute__((used)) volatile TCB* curTCB;
		}
	}

	// All tasks
	namespace Tasks
	{
		using MOS::GlobalRes::uart;
		using MOS::GlobalRes::leds;

		int8_t Task0(void* argv)
		{
			while (true) {
				delay(500);
				leds[0].toggle();
				uart.send_string("[MOS]: Task0\n");
			}
		}

		int8_t Task1(void* argv)
		{
			while (true) {
				delay(750);
				leds[1].toggle();
				uart.send_string("[MOS]: Task1\n");
			}
		}

		int8_t Task2(void* argv)
		{
			while (true) {
				delay(1000);
				leds[2].toggle();
				uart.send_string("[MOS]: Task2\n");
			}
		}

		// Add more tasks here...

		int8_t Task3(void* argv)
		{
			while (true) {
				delay(1500);
				uart.send_string("[MOS]: Task3\n");
			}
		}
	}

	namespace GloBalRes
	{
		using namespace MOS::Tasks;

		// Add more tasks here...
		TCB TCBs[] = {Task0, Task1, Task2, Task3};
	}

	namespace Macro
	{
		using GloBalRes::TCBs;

		constexpr uint32_t THREAD_NUM_MAX = sizeof(TCBs) / sizeof(TCB);
		constexpr uint32_t STACK_SIZE     = 64U;
	}

	namespace GloBalRes
	{
		using namespace MOS::Macro;
		uint32_t Page[THREAD_NUM_MAX][STACK_SIZE];// 32-bits registers
	}

	struct Scheduler
	{
		static void thread_init()
		{
			using GlobalRes::curTCB;
			using GloBalRes::TCBs;
			using GloBalRes::Page;
			using Macro::THREAD_NUM_MAX;
			using Macro::STACK_SIZE;

			asm("CPSID   I");// Disable interrupt to enter critical section

			for (uint32_t i = 0; i < THREAD_NUM_MAX; ++i) {
				// Make the TCB linked list circular
				TCBs[i].node.next = &TCBs[(i + 1) % THREAD_NUM_MAX];

				// Setup the stack such that it is holding one task context.
				// Remember it is a descending stack and a context consists of 16 registers.
				TCBs[i].sp = &Page[i][STACK_SIZE - 16];

				// Set the 'T' bit in stacked xPSR to '1' to notify processor on exception return about the thumb state.
				// V6-m and V7-m cores can only support thumb state hence this should be always set to '1'.
				Page[i][STACK_SIZE - 1] = 0x01000000;

				// Set the stacked PC to point to the task
				Page[i][STACK_SIZE - 2] = (uint32_t) TCBs[i].func;
			}

			curTCB = &TCBs[0];// Make current tcb pointer point to task0

			asm("CPSIE   I ");// Enable interrupt, leave critical section
		}

		static void launch() __attribute__((naked))
		{
			// R0 contains the address of currentPt
			asm("LDR     R0, =curTCB");

			// R2 contains the address in currentPt(value of currentPt)
			asm("LDR     R2, [R0]");

			// Load the SP reg with the stacked SP value
			asm("LDR     R4, [R2,#8]");// sp offest = 8
			asm("MOV     SP, R4");

			// Pop registers R8-R11(user saved context)
			asm("POP     {R4-R7}");
			asm("MOV     R8, R4");
			asm("MOV     R9, R5");
			asm("MOV     R10, R6");
			asm("MOV     R11, R7");

			// Pop registers R4-R7(user saved context)
			asm("POP     {R4-R7}");

			// Start poping the stacked exception frame.
			asm("POP     {R0-R3}");
			asm("POP     {R4}");
			asm("MOV     R12, R4");

			// Skip the saved LR
			asm("ADD     SP,SP,#4");

			// POP the saved PC into LR via R4, We do this to jump into the first task when we execute the branch instruction to exit this routine.
			asm("POP     {R4}");
			asm("MOV     LR, R4");
			asm("ADD     SP,SP,#4");

			// Enable interrupts
			asm("CPSIE   I ");
			asm("BX      LR");
		}
	};
}

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

static void welcome()
{
	using MOS::GlobalRes::uart;
	uart.send_string("[MOS]: Hello :)\n");
}

static void Resource_Init()
{
	using MOS::Scheduler;

	NVIC_Config();
	USART_Config();
	LED_Config();
	Scheduler::thread_init();
	welcome();
}

int main(void)
{
	using MOS::Scheduler;
	Resource_Init();

	SysTick_t::config(100000);
	Scheduler::launch();

	while (true) {
		// loop!()
	}
}