#ifndef _DRIVER_USART_
#define _DRIVER_USART_

#include "gpio.hpp"
#include "stm32f4xx_usart.h"

namespace HAL::STM32F4xx
{
	struct USART_t : public USART_TypeDef
	{
		// Type alias
		using Self_t     = USART_t;
		using Raw_t      = USART_TypeDef*;
		using Init_t     = USART_InitTypeDef;
		using Port_t     = GPIO_t&;
		using PinSrc_t   = GPIO_t::PinSrc_t;
		using AF_t       = GPIO_t::AF_t;
		using Speed_t    = GPIO_t::Speed_t;
		using State_t    = FunctionalState;
		using BaudRate_t = const uint32_t;
		using WordLen_t  = const uint16_t;
		using StopBits_t = const uint16_t;
		using Parity_t   = const uint16_t;
		using Mode_t     = const uint16_t;
		using HFC_t      = const uint16_t;
		using IT_t       = const uint16_t;
		using Flag_t     = const uint16_t;
		using Data_t     = const uint16_t;

		// Functions
		static inline constexpr USART_t&
		convert(Raw_t USARTx) { return (Self_t&) (*USARTx); }

		USART_t()                  = delete;
		USART_t(const Self_t& src) = delete;

		inline Raw_t get_raw() { return this; }

		inline auto&
		init(Init_t&& cfg)
		{
			USART_Init(this, &cfg);
			return *this;
		}

		inline auto&
		init(BaudRate_t baud_rate  = 115200,
		     WordLen_t word_length = USART_WordLength_8b,
		     StopBits_t stop_bits  = USART_StopBits_1,
		     Parity_t parity       = USART_Parity_No,
		     Mode_t mode           = USART_Mode_Rx | USART_Mode_Tx,
		     HFC_t hfc             = USART_HardwareFlowControl_None)
		{
			return init(Init_t {
			        .USART_BaudRate            = baud_rate,
			        .USART_WordLength          = word_length,
			        .USART_StopBits            = stop_bits,
			        .USART_Parity              = parity,
			        .USART_Mode                = mode,
			        .USART_HardwareFlowControl = hfc,
			});
		}

		inline auto&
		attach(GPIO_t::Raw_t port, PinSrc_t src, AF_t af,
		       Speed_t speed = GPIO_High_Speed)
		{
			GPIO_t::convert(port)
			        .init(GPIO_t::get_pin(src),
			              GPIO_Mode_AF,
			              speed,
			              GPIO_OType_PP,
			              GPIO_PuPd_UP)
			        .pin_af_config(src, af);
			return *this;
		}

		inline auto&
		rx_config(GPIO_t::Raw_t rx_port,
		          PinSrc_t rx_src, AF_t rx_af,
		          Speed_t rx_speed = GPIO_High_Speed)
		{
			return attach(rx_port, rx_src, rx_af, rx_speed);
		}

		inline auto&
		tx_config(GPIO_t::Raw_t tx_port,
		          PinSrc_t tx_src, AF_t tx_af,
		          Speed_t tx_speed = GPIO_High_Speed)
		{
			return attach(tx_port, tx_src, tx_af, tx_speed);
		}

		inline auto&
		it_config(IT_t usart_it, State_t new_state)
		{
			USART_ITConfig(this, usart_it, new_state);
			return *this;
		}

		inline auto&
		it_enable(IT_t usart_it)
		{
			return it_config(usart_it, ENABLE);
		}

		inline auto&
		it_disable(IT_t usart_it)
		{
			return it_config(usart_it, DISABLE);
		}

		inline auto&
		cmd(State_t new_state)
		{
			USART_Cmd(this, new_state);
			return *this;
		}

		inline auto&
		enable()
		{
			return cmd(ENABLE);
		}

		inline auto&
		disable()
		{
			return cmd(DISABLE);
		}

		inline auto
		get_it_status(IT_t usart_it) const
		{
			return USART_GetITStatus((Raw_t) this, usart_it);
		}

		inline auto
		get_flag_status(Flag_t flag) const
		{
			return USART_GetFlagStatus((Raw_t) this, flag);
		}

		inline void
		send_data(Data_t data)
		{
			USART_SendData(this, data);
		}

		inline void
		send_byte(uint8_t ch)
		{
			send_data(ch);
			while (get_flag_status(USART_FLAG_TXE) == RESET)
				;
		}

		inline void
		send(const void* buf, const uint32_t len)
		{
			uint32_t k = 0;
			do {
				send_byte(*((uint8_t*) buf + k++));
			} while (k != len);

			while (get_flag_status(USART_FLAG_TC) == RESET)
				;
		}

		inline void send_string(const char* str)
		{
			uint32_t k = 0;
			while (*(str + k) != '\0') {
				send_byte(*(str + k++));
			}

			while (get_flag_status(USART_FLAG_TC) == RESET)
				;
		}

		inline void println(const char* str)
		{
			send_string(str);
			send_byte('\n');
		}

		inline Data_t
		receive_data() const
		{
			return USART_ReceiveData((Raw_t) this);
		}

		inline auto&
		dma_tx_cmd(State_t new_state)
		{
			USART_DMACmd(this, USART_DMAReq_Tx, new_state);
			return *this;
		}

		inline auto&
		dma_tx_enable()
		{
			return dma_tx_cmd(ENABLE);
		}

		inline auto&
		dma_tx_disable()
		{
			return dma_tx_cmd(DISABLE);
		}

		inline auto&
		dma_rx_cmd(State_t new_state)
		{
			USART_DMACmd(this, USART_DMAReq_Rx, new_state);
			return *this;
		}

		inline auto&
		dma_rx_enable()
		{
			return dma_rx_cmd(ENABLE);
		}

		inline auto&
		dma_rx_disable()
		{
			return dma_rx_cmd(DISABLE);
		}
	};
}

#endif
