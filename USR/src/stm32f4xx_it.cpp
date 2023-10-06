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

__attribute__((naked, used)) void ContextSwitch(void)
{
	// 步骤1 - 保存当前任务的上下文
	// 处理器已经将 xPSR, PC, LR, R12, R3, R2, R1 和 R0 压入处理器堆栈。
	// 需要压入剩下的寄存器 {R4-R11} 以保存当前任务的上下文。
	// 禁用中断
	MOS_DISABLE_IRQ();

	// 压入寄存器R4到R7
	asm("PUSH    {R4-R7}");

	// 压入寄存器R8-R11
	asm("MOV     R4, R8");
	asm("MOV     R5, R9");
	asm("MOV     R6, R10");
	asm("MOV     R7, R11");
	asm("PUSH    {R4-R7}");

	// R0 = &curTCB
	asm("LDR     R0, =curTCB");

	// R1 = *R0 = curTCB
	asm("LDR     R1, [R0]");

	// R4 = SP
	asm("MOV     R4, SP");

	// 将堆栈指针的值（复制到R4）存储到 curTCB.sp
	asm("STR     R4, [R1,#8]");// TCB.sp偏移量 = 8

	// 保存旧任务上下文的流程到此结束

	// 步骤2：更新 curTCB，从其堆栈加载新任务上下文到 CPU 寄存器
	asm("PUSH    {R0,LR}");// 保存上下文 R0，LR 的值
	asm("BL      nextTCB");// 自定义调度函数，将下一个 TCB* 返回到R0
	asm("MOV     R1, R0"); // R1 = R0
	asm("POP     {R0,LR}");// 恢复 R0，LR 的值

	// curTCB = curTCB.next
	asm("STR     R1, [R0]");

	// 将下一个任务的堆栈指针的内容加载到 curTCB，相当于将 curTCB 指向新任务的 TCB
	// 使用 R4 将新任务的 sp 加载到 SP
	asm("LDR     R4, [R1,#8]");// TCB.sp offset = 8
	asm("MOV     SP, R4");

	// 弹出寄存器 R8-R11
	asm("POP     {R4-R7}");
	asm("MOV     R8, R4");
	asm("MOV     R9, R5");
	asm("MOV     R10, R6");
	asm("MOV     R11, R7");

	// 弹出寄存器 R4-R7
	asm("POP     {R4-R7}");

	// 使能中断
	MOS_ENABLE_IRQ();

	// 从中断返回
	asm("BX      LR");
}

__attribute__((naked, used)) void SysTick_Handler(void)
{
	asm("B     ContextSwitch");
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
