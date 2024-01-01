#ifndef _DEVICE_ST7735S_
#define _DEVICE_ST7735S_

#include "../stm32f4xx/spi.hpp"

namespace Driver::Device
{
	using HAL::STM32F4xx::SPI_t;
	using HAL::STM32F4xx::GPIO_t;

	static constexpr uint8_t asc2_1608[1520] = {
	        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x00, 0x00, 0x18, 0x18, 0x00, 0x00,
	        0x00, 0x48, 0x6C, 0x24, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0x24, 0x24, 0x24, 0x7F, 0x12, 0x12, 0x12, 0x7F, 0x12, 0x12, 0x12, 0x00, 0x00,
	        0x00, 0x00, 0x08, 0x1C, 0x2A, 0x2A, 0x0A, 0x0C, 0x18, 0x28, 0x28, 0x2A, 0x2A, 0x1C, 0x08, 0x08,
	        0x00, 0x00, 0x00, 0x22, 0x25, 0x15, 0x15, 0x15, 0x2A, 0x58, 0x54, 0x54, 0x54, 0x22, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0x0C, 0x12, 0x12, 0x12, 0x0A, 0x76, 0x25, 0x29, 0x11, 0x91, 0x6E, 0x00, 0x00,
	        0x00, 0x06, 0x06, 0x04, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	        0x00, 0x40, 0x20, 0x10, 0x10, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x10, 0x10, 0x20, 0x40, 0x00,
	        0x00, 0x02, 0x04, 0x08, 0x08, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x08, 0x08, 0x04, 0x02, 0x00,
	        0x00, 0x00, 0x00, 0x00, 0x08, 0x08, 0x6B, 0x1C, 0x1C, 0x6B, 0x08, 0x08, 0x00, 0x00, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0x00, 0x08, 0x08, 0x08, 0x08, 0x7F, 0x08, 0x08, 0x08, 0x08, 0x00, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x06, 0x04, 0x03,
	        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x06, 0x00, 0x00,
	        0x00, 0x00, 0x80, 0x40, 0x40, 0x20, 0x20, 0x10, 0x10, 0x08, 0x08, 0x04, 0x04, 0x02, 0x02, 0x00,
	        0x00, 0x00, 0x00, 0x18, 0x24, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x24, 0x18, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0x08, 0x0E, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x3E, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0x3C, 0x42, 0x42, 0x42, 0x20, 0x20, 0x10, 0x08, 0x04, 0x42, 0x7E, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0x3C, 0x42, 0x42, 0x20, 0x18, 0x20, 0x40, 0x40, 0x42, 0x22, 0x1C, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0x20, 0x30, 0x28, 0x24, 0x24, 0x22, 0x22, 0x7E, 0x20, 0x20, 0x78, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0x7E, 0x02, 0x02, 0x02, 0x1A, 0x26, 0x40, 0x40, 0x42, 0x22, 0x1C, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0x38, 0x24, 0x02, 0x02, 0x1A, 0x26, 0x42, 0x42, 0x42, 0x24, 0x18, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0x7E, 0x22, 0x22, 0x10, 0x10, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0x3C, 0x42, 0x42, 0x42, 0x24, 0x18, 0x24, 0x42, 0x42, 0x42, 0x3C, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0x18, 0x24, 0x42, 0x42, 0x42, 0x64, 0x58, 0x40, 0x40, 0x24, 0x1C, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x08, 0x04,
	        0x00, 0x00, 0x00, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7F, 0x00, 0x00, 0x00, 0x7F, 0x00, 0x00, 0x00, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0x3C, 0x42, 0x42, 0x46, 0x40, 0x20, 0x10, 0x10, 0x00, 0x18, 0x18, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0x1C, 0x22, 0x5A, 0x55, 0x55, 0x55, 0x55, 0x2D, 0x42, 0x22, 0x1C, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0x08, 0x08, 0x18, 0x14, 0x14, 0x24, 0x3C, 0x22, 0x42, 0x42, 0xE7, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0x1F, 0x22, 0x22, 0x22, 0x1E, 0x22, 0x42, 0x42, 0x42, 0x22, 0x1F, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0x7C, 0x42, 0x42, 0x01, 0x01, 0x01, 0x01, 0x01, 0x42, 0x22, 0x1C, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0x1F, 0x22, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x22, 0x1F, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0x3F, 0x42, 0x12, 0x12, 0x1E, 0x12, 0x12, 0x02, 0x42, 0x42, 0x3F, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0x3F, 0x42, 0x12, 0x12, 0x1E, 0x12, 0x12, 0x02, 0x02, 0x02, 0x07, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0x3C, 0x22, 0x22, 0x01, 0x01, 0x01, 0x71, 0x21, 0x22, 0x22, 0x1C, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0xE7, 0x42, 0x42, 0x42, 0x42, 0x7E, 0x42, 0x42, 0x42, 0x42, 0xE7, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0x3E, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x3E, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0x7C, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x11, 0x0F,
	        0x00, 0x00, 0x00, 0x77, 0x22, 0x12, 0x0A, 0x0E, 0x0A, 0x12, 0x12, 0x22, 0x22, 0x77, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0x07, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x42, 0x7F, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0x77, 0x36, 0x36, 0x36, 0x36, 0x2A, 0x2A, 0x2A, 0x2A, 0x2A, 0x6B, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0xE3, 0x46, 0x46, 0x4A, 0x4A, 0x52, 0x52, 0x52, 0x62, 0x62, 0x47, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0x1C, 0x22, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x22, 0x1C, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0x3F, 0x42, 0x42, 0x42, 0x42, 0x3E, 0x02, 0x02, 0x02, 0x02, 0x07, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0x1C, 0x22, 0x41, 0x41, 0x41, 0x41, 0x41, 0x4D, 0x53, 0x32, 0x1C, 0x60, 0x00,
	        0x00, 0x00, 0x00, 0x3F, 0x42, 0x42, 0x42, 0x3E, 0x12, 0x12, 0x22, 0x22, 0x42, 0xC7, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0x7C, 0x42, 0x42, 0x02, 0x04, 0x18, 0x20, 0x40, 0x42, 0x42, 0x3E, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0x7F, 0x49, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x1C, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0xE7, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x3C, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0xE7, 0x42, 0x42, 0x22, 0x24, 0x24, 0x14, 0x14, 0x18, 0x08, 0x08, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0x6B, 0x49, 0x49, 0x49, 0x49, 0x55, 0x55, 0x36, 0x22, 0x22, 0x22, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0xE7, 0x42, 0x24, 0x24, 0x18, 0x18, 0x18, 0x24, 0x24, 0x42, 0xE7, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0x77, 0x22, 0x22, 0x14, 0x14, 0x08, 0x08, 0x08, 0x08, 0x08, 0x1C, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0x7E, 0x21, 0x20, 0x10, 0x10, 0x08, 0x04, 0x04, 0x42, 0x42, 0x3F, 0x00, 0x00,
	        0x00, 0x78, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x78, 0x00,
	        0x00, 0x00, 0x02, 0x02, 0x04, 0x04, 0x08, 0x08, 0x08, 0x10, 0x10, 0x20, 0x20, 0x20, 0x40, 0x40,
	        0x00, 0x1E, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1E, 0x00,
	        0x00, 0x38, 0x44, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF,
	        0x00, 0x06, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x42, 0x78, 0x44, 0x42, 0x42, 0xFC, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0x03, 0x02, 0x02, 0x02, 0x1A, 0x26, 0x42, 0x42, 0x42, 0x26, 0x1A, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x38, 0x44, 0x02, 0x02, 0x02, 0x44, 0x38, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0x60, 0x40, 0x40, 0x40, 0x78, 0x44, 0x42, 0x42, 0x42, 0x64, 0xD8, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x42, 0x7E, 0x02, 0x02, 0x42, 0x3C, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0xF0, 0x88, 0x08, 0x08, 0x7E, 0x08, 0x08, 0x08, 0x08, 0x08, 0x3E, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7C, 0x22, 0x22, 0x1C, 0x02, 0x3C, 0x42, 0x42, 0x3C,
	        0x00, 0x00, 0x00, 0x03, 0x02, 0x02, 0x02, 0x3A, 0x46, 0x42, 0x42, 0x42, 0x42, 0xE7, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0x0C, 0x0C, 0x00, 0x00, 0x0E, 0x08, 0x08, 0x08, 0x08, 0x08, 0x3E, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0x30, 0x30, 0x00, 0x00, 0x38, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x22, 0x1E,
	        0x00, 0x00, 0x00, 0x03, 0x02, 0x02, 0x02, 0x72, 0x12, 0x0A, 0x16, 0x12, 0x22, 0x77, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0x0E, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x3E, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7F, 0x92, 0x92, 0x92, 0x92, 0x92, 0xB7, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3B, 0x46, 0x42, 0x42, 0x42, 0x42, 0xE7, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x42, 0x42, 0x42, 0x42, 0x42, 0x3C, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1B, 0x26, 0x42, 0x42, 0x42, 0x22, 0x1E, 0x02, 0x07,
	        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 0x44, 0x42, 0x42, 0x42, 0x44, 0x78, 0x40, 0xE0,
	        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x77, 0x4C, 0x04, 0x04, 0x04, 0x04, 0x1F, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7C, 0x42, 0x02, 0x3C, 0x40, 0x42, 0x3E, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x08, 0x3E, 0x08, 0x08, 0x08, 0x08, 0x08, 0x30, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x63, 0x42, 0x42, 0x42, 0x42, 0x62, 0xDC, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE7, 0x42, 0x24, 0x24, 0x14, 0x08, 0x08, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xEB, 0x49, 0x49, 0x55, 0x55, 0x22, 0x22, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x76, 0x24, 0x18, 0x18, 0x18, 0x24, 0x6E, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE7, 0x42, 0x24, 0x24, 0x14, 0x18, 0x08, 0x08, 0x07,
	        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7E, 0x22, 0x10, 0x08, 0x08, 0x44, 0x7E, 0x00, 0x00,
	        0x00, 0xC0, 0x20, 0x20, 0x20, 0x20, 0x20, 0x10, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0xC0, 0x00,
	        0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
	        0x00, 0x06, 0x08, 0x08, 0x08, 0x08, 0x08, 0x10, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x06, 0x00,
	        0x0C, 0x32, 0xC2, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

	struct ST7735S
	{
		using Port_t    = GPIO_t&;
		using PortRaw_t = GPIO_t::Raw_t;
		using Pin_t     = GPIO_t::Pin_t;
		using Pixel_t   = uint16_t;

		enum Color
		{
			WHITE      = 0xFFFF,
			BLACK      = 0x0000,
			BLUE       = 0x001F,
			BRED       = 0XF81F,
			GRED       = 0XFFE0,
			GBLUE      = 0X07FF,
			RED        = 0xF800,
			MAGENTA    = 0xF81F,
			GREEN      = 0x07E0,
			CYAN       = 0x7FFF,
			YELLOW     = 0xFFE0,
			BROWN      = 0XBC40, //棕色
			BRRED      = 0XFC07, //棕红色
			GRAY       = 0X8430, //灰色
			DARKBLUE   = 0X01CF, //深蓝色
			LIGHTBLUE  = 0X7D7C, //浅蓝色
			GRAYBLUE   = 0X5458, //灰蓝色
			LIGHTGREEN = 0X841F, //浅绿色
			LGRAY      = 0XC618, //浅灰色(PANNEL),窗体背景色
			LGRAYBLUE  = 0XA651, //浅灰蓝色(中间层颜色)
			LBBLUE     = 0X2B12, //浅棕蓝色(选择条目的反色)
		};

		struct PortPin_t
		{
			Port_t port;
			Pin_t pin;

			PortPin_t(PortRaw_t port, Pin_t pin)
			    : port(GPIO_t::convert(port)), pin(pin) {}

			void as_output() { port.as_output(pin); }
			void clr() { port.reset_bits(pin); }
			void set() { port.set_bits(pin); }
		};

		SPI_t& spi;
		PortPin_t sclk; // SCLK(SCL)
		PortPin_t mosi; // MOSI(SDA)
		PortPin_t cs;   // CS(SS)
		PortPin_t rst;  // RESET(RES)
		PortPin_t dc;   // DC(RS)

		const Color bkgd = Color::BLACK;

		const Pixel_t width = 128, height = 160;
		const uint8_t direction = 1;

		ST7735S(SPI_t::Raw_t spi, PortPin_t sclk, PortPin_t mosi,
		        PortPin_t cs, PortPin_t rst, PortPin_t dc)
		    : spi(SPI_t::convert(spi)),
		      sclk(sclk),
		      mosi(mosi),
		      cs(cs),
		      rst(rst),
		      dc(dc) {}

		inline void sclk_clear() { sclk.clr(); }
		inline void sclk_set() { sclk.set(); }
		inline void mosi_clear() { mosi.clr(); }
		inline void mosi_set() { mosi.set(); }
		inline void rst_clear() { rst.clr(); }
		inline void rst_set() { rst.set(); }
		inline void dc_clear() { dc.clr(); }
		inline void dc_set() { dc.set(); }
		inline void cs_clear() { cs.clr(); }
		inline void cs_set() { cs.set(); }

		static inline void
		spi_delay(volatile uint32_t n)
		{
			n *= 1000;
			while (n--) {
				asm volatile("");
			}
		}

		inline void write_8bit_data(uint8_t data)
		{
			cs.clr();
			dc.set(); //写数据
			spi.write_bus(data);
			cs.set();
		}

		inline void write_16bit_data(uint16_t data)
		{
			cs.clr();
			dc.set(); //写数据
			spi.write_bus(data >> 8);
			spi.write_bus(data);
			cs.set();
		}

		inline void write_cmd(uint8_t cmd)
		{
			cs.clr();
			dc.clr(); //写命令
			spi.write_bus(cmd);
			cs.set();
		}

		inline void address_set(Pixel_t x1, Pixel_t y1, Pixel_t x2, Pixel_t y2)
		{
			write_cmd(0x2a); //列地址设置
			write_16bit_data(x1);
			write_16bit_data(x2);
			write_cmd(0x2b); //行地址设置
			write_16bit_data(y1);
			write_16bit_data(y2);
			write_cmd(0x2c); //储存器写
		}

		inline void clear(Color color)
		{
			address_set(0, 0, width - 1, height - 1);
			for (uint16_t i = 0; i < width; i++) {
				for (uint16_t j = 0; j < height; j++) {
					write_16bit_data(bkgd);
				}
			}
		}

		inline void draw_point(Pixel_t x, Pixel_t y, Color color = WHITE)
		{
			address_set(x, y, x, y); //设置光标位置
			write_16bit_data(color);
		}

		inline void draw_img(Pixel_t x, Pixel_t y, Pixel_t w, Pixel_t h, const uint16_t* img)
		{
			// 设置绘图区域
			address_set(x, y, x + w - 1, y + h - 1);

			// 遍历图片数据并绘制
			for (uint16_t i = 0; i < h; i++) {
				for (uint16_t j = 0; j < w; j++) {
					// 从图片数据中获取颜色并绘制
					uint16_t color = img[i * w + j];
					write_16bit_data(color);
				}
			}
		}

		void show_char(Pixel_t x, Pixel_t y, uint8_t num, uint8_t mode, Color color = WHITE)
		{
			uint8_t temp;
			uint16_t x0 = x;
			if (x > width - 16 || y > height - 16) return; //设置窗口
			num = num - ' ';                               //得到偏移后的值
			address_set(x, y, x + 8 - 1, y + 16 - 1);      //设置光标位置

			for (uint8_t pos = 0; pos < 16; pos++) {
				uint8_t ch = asc2_1608[(uint16_t) num * 16 + pos]; //调用1608字体
				for (uint8_t t = 0; t < 8; t++) {
					if (ch & 0x01) {
						if (!mode) { //非叠加方式
							write_16bit_data(color);
						}
						else {                                 //叠加方式
							draw_point(x + t, y + pos, color); //画一个点
						}
					}
					else if (!mode) {
						write_16bit_data(bkgd);
					}
					ch >>= 1;
					x++;
				}
				x = x0;
				y++;
			}
		}

		void show_string(Pixel_t x, Pixel_t y, const char* str, Color color = WHITE)
		{
			while (*str != '\0') {
				if (*str == '\n') {
					x = 0;
					y += 16;
				}
				else {
					if (x > width - 16) {
						x = 0;
						y += 16;
					}
					if (y > height - 16) {
						y = x = 0;
						clear(bkgd);
					}
					show_char(x, y, *str, 0, color);
					x += 8;
				}
				str++;
			}
		}

		void test_gray16(void)
		{
			address_set(0, 0, width - 1, height - 1);

			for (uint16_t i = 0; i < height; i++) {
				for (uint16_t j = 0; j < width % 8; j++) {
					write_16bit_data(0);
				}

				for (uint16_t j = 0; j < 16; j++) {
					for (uint16_t k = 0; k < width / 16; k++) {
						write_8bit_data(((j * 2) << 3) | ((j * 4) >> 3));
						write_8bit_data(((j * 4) << 5) | (j * 2));
					}
				}
			}
		}

		void init(void)
		{
			spi.init(SPI_t::Init_t {
			        .SPI_Direction         = SPI_Direction_1Line_Tx,
			        .SPI_Mode              = SPI_Mode_Master,
			        .SPI_DataSize          = SPI_DataSize_8b,
			        .SPI_CPOL              = SPI_CPOL_Low,
			        .SPI_CPHA              = SPI_CPHA_1Edge,
			        .SPI_NSS               = SPI_NSS_Soft,
			        .SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2,
			        .SPI_FirstBit          = SPI_FirstBit_MSB,
			        .SPI_CRCPolynomial     = 7,
			});

			spi.enable();
			cs.as_output();
			dc.as_output();
			rst.as_output();

			cs_set();
			rst_clear();
			dc_set();

			rst_clear();
			spi_delay(20);
			rst_set();
			spi_delay(100);

			//************* Start Initial Sequence **********//
			write_cmd(0x11); //Sleep out
			spi_delay(120);  //Delay 120ms

			//------------------------------------ST7735S Frame Rate-----------------------------------------//
			write_cmd(0xB1);
			write_8bit_data(0x05);
			write_8bit_data(0x3C);
			write_8bit_data(0x3C);
			write_cmd(0xB2);
			write_8bit_data(0x05);
			write_8bit_data(0x3C);
			write_8bit_data(0x3C);
			write_cmd(0xB3);
			write_8bit_data(0x05);
			write_8bit_data(0x3C);
			write_8bit_data(0x3C);
			write_8bit_data(0x05);
			write_8bit_data(0x3C);
			write_8bit_data(0x3C);
			//------------------------------------End ST7735S Frame Rate---------------------------------//

			write_cmd(0xB4); //Dot inversion
			write_8bit_data(0x03);

			//------------------------------------ST7735S Power Sequence---------------------------------//
			write_cmd(0xC0);
			write_8bit_data(0x28);
			write_8bit_data(0x08);
			write_8bit_data(0x04);
			write_cmd(0xC1);
			write_8bit_data(0XC0);
			write_cmd(0xC2);
			write_8bit_data(0x0D);
			write_8bit_data(0x00);
			write_cmd(0xC3);
			write_8bit_data(0x8D);
			write_8bit_data(0x2A);
			write_cmd(0xC4);
			write_8bit_data(0x8D);
			write_8bit_data(0xEE);
			//---------------------------------End ST7735S Power Sequence-------------------------------------//

			write_cmd(0xC5); //VCOM
			write_8bit_data(0x1A);
			write_cmd(0x36); //MX, MY, RGB mode
			switch (direction) {
				case 0:
					write_8bit_data(0x00);
					break;
				case 1:
					write_8bit_data(0xC0);
					break;
				case 2:
					write_8bit_data(0x70);
					break;
				case 3:
					write_8bit_data(0xA0);
					break;
			}

			//------------------------------------ST7735S Gamma Sequence---------------------------------//
			constexpr uint8_t gamma_sequence1[] =
			        {0x04, 0x22, 0x07, 0x0A, 0x2E, 0x30, 0x25, 0x2A,
			         0x28, 0x26, 0x2E, 0x3A, 0x00, 0x01, 0x03, 0x13};
			constexpr uint8_t gamma_sequence2[] =
			        {0x04, 0x16, 0x06, 0x0D, 0x2D, 0x26, 0x23, 0x27,
			         0x27, 0x25, 0x2D, 0x3B, 0x00, 0x01, 0x04, 0x13};

			write_cmd(0xE0);
			for (uint8_t i = 0; i < sizeof(gamma_sequence1); i++) {
				write_8bit_data(gamma_sequence1[i]);
			}

			write_cmd(0xE1);
			for (uint8_t i = 0; i < sizeof(gamma_sequence2); i++) {
				write_8bit_data(gamma_sequence2[i]);
			}
			//------------------------------------End ST7735S Gamma Sequence-----------------------------//

			write_cmd(0x3A); //65k mode
			write_8bit_data(0x05);
			write_cmd(0x29); //Display on

			// Clear out the screen
			clear(bkgd);
		}
	};

	struct Terminal
	{
		using Pixel_t = ST7735S::Pixel_t;
		using Color   = ST7735S::Color;

		ST7735S& lcd;
		Pixel_t c_x, c_y;

		Terminal(ST7735S& lcd): lcd(lcd), c_x(0), c_y(0) {}

		inline void print(const char* str, Color color = Color::WHITE)
		{
			while (*str != '\0') {
				if (*str == '\n' || c_x > lcd.width - 16) {
					c_x = 0;
					c_y += 16;
				}
				if (c_y > lcd.height - 16) {
					lcd.clear(lcd.bkgd);
					c_x = 0;
					c_y = 0;
				}
				if (*str != '\n') {
					lcd.show_char(c_x, c_y, *str, 0, color);
					c_x += 8;
				}
				str++;
			}
		}

		inline void println(const char* str, Color color = Color::WHITE)
		{
			print(str, color);
			print("\n");
		}
	};
}

#endif