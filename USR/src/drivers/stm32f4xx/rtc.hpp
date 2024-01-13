#ifndef _DRIVER_RTC_
#define _DRIVER_RTC_

#include "stm32f4xx_rtc.h"

namespace HAL::STM32F4xx
{
	struct RTC_t : public RTC_TypeDef
	{
		// Type alias
		using Self_t = RTC_t;
		using Raw_t  = RTC_TypeDef*;
		using Init_t = RTC_InitTypeDef;
		using Time_t = RTC_TimeTypeDef;
		using Date_t = RTC_DateTypeDef;

		// Functions
		RTC_t()                  = delete;
		RTC_t(const Self_t& src) = delete;

		static inline constexpr RTC_t&
		convert(Raw_t raw) { return (Self_t&) (*raw); }

		static inline auto
		init(Init_t&& cfg)
		{
			return RTC_Init(&cfg);
		}

		static inline auto
		init(const Init_t& cfg)
		{
			return init((Init_t&&) cfg);
		}

		static inline auto
		wait_for_sync()
		{
			return RTC_WaitForSynchro();
		}

		static inline auto
		set_time(Time_t&& time, const uint32_t format = RTC_Format_BIN)
		{
			return RTC_SetTime(format, &time);
		}

		static inline auto
		set_time(const Time_t& time, const uint32_t format = RTC_Format_BIN)
		{
			return set_time((Time_t&&) time, format);
		}

		static inline auto
		set_date(Date_t&& date, const uint32_t format = RTC_Format_BIN)
		{
			return RTC_SetDate(format, &date);
		}

		static inline auto
		set_date(const Date_t& date, const uint32_t format = RTC_Format_BIN)
		{
			return set_date((Date_t&&) date, format);
		}

		static inline void
		write_backup_reg(const uint32_t bkp_dr, const uint32_t data)
		{
			RTC_WriteBackupRegister(bkp_dr, data);
		}

		static inline auto
		read_backup_reg(const uint32_t bkp_dr)
		{
			return RTC_ReadBackupRegister(bkp_dr);
		}

		static inline auto
		get_time(const uint32_t format = RTC_Format_BIN)
		{
			Time_t time;
			RTC_GetTime(format, &time);
			return time;
		}

		static inline auto
		get_date(const uint32_t format = RTC_Format_BIN)
		{
			Date_t date;
			RTC_GetDate(format, &date);
			return date;
		}
	};
}

#endif
