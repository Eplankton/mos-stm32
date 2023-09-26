#ifndef _NUTS_SYSCFG_
#define _NUTS_SYSCFG_

#include "stm32f4xx.h"

namespace nuts
{
	struct SYSCFG_t : public SYSCFG_TypeDef
	{
		// Type alias
		using Self_t    = SYSCFG_t;
		using Raw_t     = SYSCFG_TypeDef*;
		using PortSrc_t = const uint8_t;
		using PinSrc_t  = const uint8_t;

		SYSCFG_t()                  = delete;
		SYSCFG_t(const Self_t& src) = delete;

		// Functions
		static constexpr inline SYSCFG_t&
		convert(Raw_t raw) { return (Self_t&) (*raw); }

		static inline void
		exti_line_config(PortSrc_t port_src, PinSrc_t pin_src)
		{
			SYSCFG_EXTILineConfig(port_src, pin_src);
		}
	};
}

#endif