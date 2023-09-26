#ifndef _NUTS_IO_
#define _NUTS_IO_

#include <stdio.h>
#include "stm32f4xx.h"
#include "usart.hpp"

using nuts::USART_t;

#define IN_PORT  USART3
#define OUT_PORT USART3

#define _NUTS_DEBUG_INFO_
#ifdef _NUTS_DEBUG_INFO_
#define debug_info(format, ...) printf(format, ##__VA_ARGS__)
#else
#define debug_info(format, ...) ((void) 0)
#endif

#endif
