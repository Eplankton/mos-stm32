#ifndef _MOS_DRIVER_KEY_
#define _MOS_DRIVER_KEY_

#include "gpio.hpp"
#include "delay.hpp"

namespace MOS::Driver
{
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
		    : port(_port), pin(_pin) { init(); }

		KEY_t(GPIO_t::Raw_t _port, Pin_t _pin)
		    : KEY_t(GPIO_t::convert(_port), _pin) {}

		// Init the key
		inline void init()
		{
			port.as_input(pin);
		}

		// Buttom pressed
		inline constexpr bool
		is_set() const
		{
			return port.read_input_bit(pin) == Bit_SET;
		}

		// Buttom not pressed
		inline constexpr bool
		is_reset() const { return !is_set(); }

		// Buttom clicked
		inline constexpr bool
		click() const
		{
			// Action detected
			if (is_set()) {
				// Wait until loose
				delay(100);
				while (is_set())
					;
				return true;
			}
			else
				return false;
		}
	};
}

#endif
