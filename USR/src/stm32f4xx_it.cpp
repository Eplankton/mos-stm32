/**
  ******************************************************************************
  * @file    Project/STM32F4xx_StdPeriph_Templates/stm32f4xx_it.c 
  * @author  MCD Application Team
  * @version V1.8.0
  * @date    04-November-2016
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
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

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_it.h"
#include "main.h"

/** @addtogroup Template_Project
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M4 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
	/* Go to infinite loop when Hard Fault exception occurs */
	while (1)
	{
	}
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
	/* Go to infinite loop when Memory Manage exception occurs */
	while (1)
	{
	}
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
	/* Go to infinite loop when Bus Fault exception occurs */
	while (1)
	{
	}
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
	/* Go to infinite loop when Usage Fault exception occurs */
	while (1)
	{
	}
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}

#ifdef __cplusplus
extern "C" {
#endif

__attribute__((naked)) void SysTick_Handler(void)
{
	// STEP 1 - SAVE THE CURRENT TASK CONTEXT
	// At this point the processor has already pushed PSR, PC, LR, R12, R3, R2, R1 and R0
	// onto the stack. We need to push the rest(i.e R4, R5, R6, R7, R8, R9, R10, R11) to save the context of the current task.
	// Disable interrupts
	asm("CPSID   I");
	// Push registers R4 to R7
	asm("PUSH    {R4-R7}");
	// Push registers R8-R11
	asm("MOV     R4, R8");
	asm("MOV     R5, R9");
	asm("MOV     R6, R10");
	asm("MOV     R7, R11");
	asm("PUSH    {R4-R7}");
	// Load R0 with the address of pCurntTcb
	asm("LDR     R0, =curTCB");
	// Load R1 with the content of pCurntTcb(i.e post this, R1 will contain the address of current TCB).
	asm("LDR     R1, [R0]");
	// Move the SP value to R4
	asm("MOV     R4, SP");
	// Store the value of the stack pointer(copied in R4) to the current tasks "stackPt" element in its TCB.
	// This marks an end to saving the context of the current task.
	asm("STR     R4, [R1, #4]");// sp offest

	// STEP 2: LOAD THE NEW TASK CONTEXT FROM ITS STACK TO THE CPU REGISTERS, UPDATE pCurntTcb.
	// Load the address of the next task TCB onto the R1.
	asm("LDR     R1, [R1, #8]");// next offest
	// Load the contents of the next tasks stack pointer to pCurntTcb, equivalent to pointing pCurntTcb to
	// the newer tasks TCB. Remember R1 contains the address of pCurntTcb.
	asm("STR     R1, [R0]");
	// Load the newer tasks TCB to the SP using R4.
	asm("LDR     R4, [R1, #4]");// sp offest
	asm("MOV     SP, R4");
	// Pop registers R8-R11
	asm("POP     {R4-R7}");
	asm("MOV     R8, R4");
	asm("MOV     R9, R5");
	asm("MOV     R10, R6");
	asm("MOV     R11, R7");
	// Pop registers R4-R7
	asm("POP     {R4-R7}");

	asm("CPSIE   I ");
	asm("BX      LR");
}

#ifdef __cplusplus
}
#endif

/******************************************************************************/
/*                 STM32F4xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f4xx.s).                                               */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
