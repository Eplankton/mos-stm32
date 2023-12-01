#ifndef _DEVICE_KEY_
#define _DEVICE_KEY_

#include "../stm32f4xx/gpio.hpp"

namespace Driver
{
	using HAL::STM32F4xx::GPIO_t;

	struct KEY_t
	{
		// Type alias
		using Port_t = GPIO_t&;
		using Pin_t  = GPIO_t::Pin_t;

		// Members
		Port_t port;
		Pin_t pin;

		// Functions
		KEY_t() = delete;

		KEY_t(Port_t _port, Pin_t _pin)
		    : port(_port), pin(_pin) {}

		KEY_t(GPIO_t::Raw_t _port, Pin_t _pin)
		    : KEY_t(GPIO_t::convert(_port), _pin) {}

		// Init the key
		inline void init()
		{
			port.as_input(pin);
		}

		// Buttom pressed
		inline bool
		is_set() const
		{
			return port.read_input_bit(pin) == Bit_SET;
		}

		// Buttom not pressed
		inline bool
		is_reset() const { return !is_set(); }
	};
}

#endif
