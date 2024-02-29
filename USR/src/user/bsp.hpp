#ifndef _MOS_USER_BSP_
#define _MOS_USER_BSP_

// STM32F4xx HAL
#include "src/drivers/stm32f4xx/hal.hpp"
#include "src/mos/kernel/task.hpp"
#include "src/user/global.hpp"

namespace MOS::User::BSP
{
	using namespace HAL::STM32F4xx;

	// For printf_
	extern "C" void
	MOS_PUTCHAR(char ch)
	{
		using Global::stdio;
		stdio.send_data(ch);
		stdio.wait_flag(USART_FLAG_TXE);
	}

	static inline void
	LED_Config()
	{
		using Global::leds;
		RCC_t::AHB1::enable(RCC_AHB1Periph_GPIOB);
		for (auto& led: leds) {
			led.init();
		}
	}

	static inline void
	NVIC_GroupConfig()
	{
		NVIC_t::group_config(NVIC_PriorityGroup_2);
	}

	static inline void
	SysTick_Config()
	{
		SysTick_t::config(Macro::SYSTICK);
	}

	static inline void
	K1_IRQ_Config()
	{
		RCC_t::AHB1::enable(RCC_AHB1Periph_GPIOC);
		RCC_t::APB2::enable(RCC_APB2Periph_SYSCFG);

		SYSCFG_t::exti_line_config( // K1 -> PC13
		    EXTI_PortSourceGPIOC,
		    EXTI_PinSource13
		);

		EXTI_t::init( // K1 Intr Config
		    EXTI_Line13,
		    EXTI_Mode_Interrupt,
		    EXTI_Trigger_Rising, ENABLE
		);

		NVIC_t::init(EXTI15_10_IRQn, 1, 1, ENABLE);
	}

	static inline void
	USART_Config()
	{
		RCC_t::AHB1::enable(RCC_AHB1Periph_GPIOD);
		RCC_t::APB1::enable(
		    RCC_APB1Periph_USART2 |
		    RCC_APB1Periph_USART3
		);

		NVIC_t::init(USART2_IRQn, 1, 1, ENABLE);
		NVIC_t::init(USART3_IRQn, 1, 1, ENABLE);

		// stdio uart config
		Global::stdio
		    .init( // 57600-8-1-N
		        57600,
		        USART_WordLength_8b,
		        USART_StopBits_1,
		        USART_Parity_No
		    )
		    .rx_config( // RX -> PD9
		        GPIOD,
		        GPIO_t::get_pin_src(9),
		        GPIO_AF_USART3
		    )
		    .tx_config( // TX -> PD8
		        GPIOD,
		        GPIO_t::get_pin_src(8),
		        GPIO_AF_USART3
		    )
		    .it_enable(USART_IT_RXNE)
		    .enable();

		// esp32-wifi uart config
		Global::esp32
		    .init( // 115200-8-1-N
		        115200,
		        USART_WordLength_8b,
		        USART_StopBits_1,
		        USART_Parity_No
		    )
		    .rx_config( // RX -> PD6
		        GPIOD,
		        GPIO_t::get_pin_src(6),
		        GPIO_AF_USART2
		    )
		    .tx_config( // TX -> PD5
		        GPIOD,
		        GPIO_t::get_pin_src(5),
		        GPIO_AF_USART2
		    )
		    .it_enable(USART_IT_RXNE)
		    .enable();
	}

	static inline void
	LCD_Config()
	{
		using Global::lcd;

		RCC_t::APB2::enable(RCC_APB2Periph_SPI1);
		RCC_t::AHB1::enable(
		    RCC_AHB1Periph_GPIOA |
		    RCC_AHB1Periph_GPIOB |
		    RCC_AHB1Periph_GPIOD
		);

		lcd.spi
		    .sclk_config( // SCLK -> PA5
		        GPIOA,
		        GPIO_t::get_pin_src(5),
		        GPIO_AF_SPI1
		    )
		    .mosi_config( // MOSI -> PA7
		        GPIOA,
		        GPIO_t::get_pin_src(7),
		        GPIO_AF_SPI1
		    );

		lcd.init();
	}

	static inline void
	RTC_Config()
	{
		constexpr auto ASYHCHPREDIV = 0x7F;        // 异步分频因子
		constexpr auto SYHCHPREDIV  = 0xFF;        // 同步分频因子
		constexpr auto RTC_BKP_DR   = RTC_BKP_DR0; // 备份寄存器
		constexpr auto RTC_BKP_DATA = 0x1234;      // 备份标记

		/*=====================同步/异步预分频器的值======================*/
		/* 驱动日历的时钟 ck_spare = LSE/[(255+1)*(127+1)] = 1HZ */
		constexpr RTC_t::Init_t init_cfg {
		    .RTC_HourFormat   = RTC_HourFormat_24,
		    .RTC_AsynchPrediv = ASYHCHPREDIV,
		    .RTC_SynchPrediv  = SYHCHPREDIV,
		};

		constexpr RTC_t::Time_t boot_time {
		    .RTC_Hours   = 2,
		    .RTC_Minutes = 15,
		    .RTC_Seconds = 0,
		    .RTC_H12     = RTC_H12_AM,
		};

		constexpr RTC_t::Date_t boot_date {
		    .RTC_WeekDay = 7,
		    .RTC_Month   = 1,
		    .RTC_Date    = 14,
		    .RTC_Year    = 24,
		};

		static auto clock_config = [&] {
			/*使能 PWR 时钟*/
			RCC_t::APB1::enable(RCC_APB1Periph_PWR);

			/* PWR_CR:DBF置1，使能RTC、RTC备份寄存器和备份SRAM的访问 */
			PWR_t::backup_access_cmd(ENABLE);

			/* 使能LSE */
			RCC_t::lse_config(RCC_LSE_ON);

			/* 等待LSE稳定 */
			while (RCC_t::get_flag_status(RCC_FLAG_LSERDY) == RESET) {
				MOS_NOP();
			}

			/* 选择LSE做为RTC的时钟源 */
			RCC_t::rtc_clk_config(RCC_RTCCLKSource_LSE);

			/* 使能RTC时钟 */
			RCC_t::rtc_clk_cmd(ENABLE);

			/* 等待 RTC APB 寄存器同步 */
			RTC_t::wait_for_sync();

			/* 用RTC_InitStructure的内容初始化RTC寄存器 */
			MOS_ASSERT(RTC_t::init(init_cfg) != ERROR, "RTC Init Failed!\n");
		};

		static auto set_time_and_date = [&] {
			RTC_t::set_date(boot_date); // 初始化日期
			RTC_t::set_time(boot_time); // 初始化时间
			RTC_t::write_backup_reg(RTC_BKP_DR, RTC_BKP_DATA);
		};

		// 当配置过RTC时间之后就往备份寄存器写入一个数据做标记，每次程序重新运行的时候就通过
		// 检测备份寄存器的值来判断RTC是否已经配置过，如果配置过那就继续运行，如果没有配置过就初始化RTC。

		if (RTC_t::read_backup_reg(RTC_BKP_DR) != RTC_BKP_DATA) {
			clock_config();      // RTC配置：选择时钟源，设置RTC_CLK的分频系数
			set_time_and_date(); // 设置时间和日期
		}
		else {
			/* 使能 PWR 时钟 */
			RCC_t::APB1::enable(RCC_APB1Periph_PWR);

			/* PWR_CR:DBF置1，使能RTC、RTC备份寄存器和备份SRAM的访问 */
			PWR_t::backup_access_cmd(ENABLE);

			/* 等待 RTC APB 寄存器同步 */
			RTC_t::wait_for_sync();
		}
	}

	static inline void
	config()
	{
		NVIC_GroupConfig();
		USART_Config();
		LED_Config();
		K1_IRQ_Config();
		LCD_Config();
		RTC_Config();
		SysTick_Config();
	}
}

namespace MOS::ISR
{
	using namespace Kernel;

	extern "C" {
		void EXTI15_10_IRQHandler() // K1 IRQ Handler
		{
			using HAL::STM32F4xx::EXTI_t;
			using User::Global::leds;
			using Utils::Range;

			// To simulate a burst task
			static auto k1_irq = [] {
				for (auto _: Range(0, 10)) {
					leds[2].toggle(); // blue
					Task::print_name();
					Task::delay(250);
				}
			};

			EXTI_t::handle_line(EXTI_Line13, [] {
				static uint32_t k1_cnt = 0;
				MOS_MSG("K1 Cnt = %d", ++k1_cnt);
				Task::create_from_isr(
				    k1_irq,
				    nullptr,
				    Task::current()->get_pri(),
				    "K1"
				);
			});
		}

		void USART2_IRQHandler() // ESP32C3 WiFi I/O
		{
			using User::Global::esp32;
			using User::Global::wifi_buf;

			esp32.handle_it(USART_IT_RXNE, [] {
				char8_t data = esp32.recv_data();
				if (!wifi_buf.full()) {
					if (data == '\n') // data received
						wifi_buf.signal_from_isr();
					else
						wifi_buf.add(data);
				}
				else {
					wifi_buf.clear();
				}
			});
		}

		void USART3_IRQHandler() // Shell I/O
		{
			using User::Global::stdio;
			using User::Global::io_buf;

			stdio.handle_it(USART_IT_RXNE, [] {
				char8_t data = stdio.recv_data();
				if (!io_buf.full()) {
					if (data == '\n') // cmd received
						io_buf.signal_from_isr();
					else
						io_buf.add(data);
				}
				else {
					io_buf.clear();
					MOS_MSG("Oops! Command too long!");
				}
			});
		}
	}
}

#endif