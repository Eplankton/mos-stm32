#ifndef _MOS_USER_BSP_
#define _MOS_USER_BSP_

// STM32F4xx HAL
#include "src/drivers/stm32f4xx/hal.hpp"
#include "src/mos/kernel/task.hpp"
#include "src/user/global.hpp"

namespace MOS::BSP
{
	using namespace HAL::STM32F4xx;

	// For printf_
	extern "C" void _putchar(char ch)
	{
		using UserGlobal::uart;
		uart.send_data(ch);
		uart.wait_flag(USART_FLAG_TXE);
	}

	static inline void LED_Config()
	{
		using UserGlobal::leds;
		RCC_t::AHB1::enable(RCC_AHB1Periph_GPIOB);
		for (auto& led: leds) {
			led.init();
		}
	}

	static inline void NVIC_GroupConfig()
	{
		NVIC_t::group_config(NVIC_PriorityGroup_2);
	}

	static inline void SysTick_Config()
	{
		SysTick_t::config(Macro::SYSTICK);
	}

	static inline void K1_IRQ_Config()
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

	static inline void USART_Config()
	{
		using UserGlobal::uart;

		RCC_t::AHB1::enable(RCC_AHB1Periph_GPIOD);
		RCC_t::APB1::enable(RCC_APB1Periph_USART3);
		NVIC_t::init(USART3_IRQn, 1, 1, ENABLE);

		uart.init( // 38400-8-1-N
		        38400,
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
	}

	static inline void LCD_Config()
	{
		using UserGlobal::lcd;

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

	static inline void RTC_Config()
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

	static inline void config()
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
	extern "C" {
		void EXTI15_10_IRQHandler() // K1 IRQ Handler
		{
			using HAL::STM32F4xx::EXTI_t;
			using UserGlobal::leds;
			using Utils::Range;

			// To simulate a burst task
			static auto K1_IRQ = [] {
				for (auto _: Range(0, 10)) {
					leds[2].toggle();
					Task::print_name();
					Task::delay(250);
				}
			};

			EXTI_t::handle_line(EXTI_Line13, [] {
				static uint32_t k1_cnt = 0;
				MOS_MSG("K1 Cnt = %d", ++k1_cnt);
				Task::create_from_isr(
				    K1_IRQ,
				    nullptr,
				    Task::current()->get_pri(),
				    "K1"
				);
			});
		}

		void USART3_IRQHandler() // UART3 IRQ Handler
		{
			using UserGlobal::uart;
			using UserGlobal::rx_buf;

			if (uart.get_it_status(USART_IT_RXNE) != RESET) {
				if (!rx_buf.full()) {
					char8_t data = uart.recv_data();
					rx_buf.add(data);
					if (data == '\n') { // Cmd received
						rx_buf.signal_from_isr();
					}
				}
				else {
					rx_buf.clear();
					MOS_MSG("Oops! Command too long!");
				}
			}
		}
	}
}

#endif