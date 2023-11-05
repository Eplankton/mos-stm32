#ifndef _MOS_DRIVER_LED_
#define _MOS_DRIVER_LED_

#include "gpio.hpp"

namespace MOS::Driver
{
	struct LED_t
	{
		// Type alias
		using Port_t = GPIO_t&;
		using Pin_t  = GPIO_t::Pin_t;
		using Time_t = const uint32_t;

		// Members
		Port_t port;
		Pin_t pin;

		// Functions
		LED_t() = delete;

		LED_t(Port_t _port, Pin_t _pin)
		    : port(_port), pin(_pin) {}

		LED_t(GPIO_t::Raw_t _port, Pin_t _pin)
		    : LED_t(GPIO_t::convert(_port), _pin) {}

		// Init the led
		inline void init()
		{
			port.as_output(pin, GPIO_High_Speed, GPIO_OType_PP, GPIO_PuPd_UP);
			off();
		}

		// Turn the led on
		inline void on()
		{
			port.set_bits(pin);
		}

		// Turn the led off
		inline void off()
		{
			port.reset_bits(pin);
		}

		// Toggle the led
		inline void toggle()
		{
			port.toggle_bits(pin);
		}
	};
}

#endif