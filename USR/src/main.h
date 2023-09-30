/**
  ******************************************************************************
  * @file    Project/STM32F4xx_StdPeriph_Templates/main.h 
  * @author  MCD Application Team
  * @version V1.8.0
  * @date    04-November-2016
  * @brief   Header for main.c module
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2016 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "etl_profile.h"
#include "../nuts/mod.hpp"

#define DISABLE_IRQ() asm("CPSID I")
#define ENABLE_IRQ()  asm("CPSIE I")

// Modules begin

#define MODULE(MOD) using nuts::MOD;
MODULE(convert)
MODULE(delay)
MODULE(GPIO_t)
MODULE(RCC_t)
MODULE(NVIC_t)
MODULE(EXTI_t)
MODULE(SYSCFG_t)
MODULE(SysTick_t)
MODULE(USART_t)
MODULE(LED_t)
MODULE(KEY_t)
MODULE(DMA_Stream_t)
MODULE(I2C_t)
MODULE(TIM_t)

// Modules end

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
