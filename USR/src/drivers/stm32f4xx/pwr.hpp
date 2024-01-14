#ifndef _DRIVER_PWR_
#define _DRIVER_PWR_

#include "stm32f4xx_pwr.h"

namespace HAL::STM32F4xx
{
	struct PWR_t : public PWR_TypeDef
	{
		// Type alias
		using Self_t  = PWR_t;
		using Raw_t   = PWR_TypeDef*;
		using State_t = FunctionalState;

		PWR_t()                  = delete;
		PWR_t(const Self_t& src) = delete;

		// Functions
		static inline constexpr PWR_t&
		convert(Raw_t raw) { return (Self_t&) (*raw); }

		static inline void
		backup_access_cmd(const State_t new_state)
		{
			PWR_BackupAccessCmd(new_state);
		}
	};
}

#endif