/*=============================================================================================
*
* timer_count_up.c
*
* ---------------------------------------------------------------------------------------------
*
* MIT License
*
* Copyright(C) 2017 : KUNBUS GmbH, Heerweg 15C, 73370 Denkendorf, Germany
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files(the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions :
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
*=============================================================================================
*
*Implementation of count up timer for communication timeout and testing measurements.
*
* changed type of BSP_count_up_timer_get() because on STM32F2XX TIM5 has a 32-bit counter
*/
#include <project.h>
#include <common_define.h>

#include <bsp/bspConfig.h>
#include <bsp/timer/timer.h>
#include <bsp/clock/clock.h>


#include <SysLib/inc/stm32f2xx_tim.h>
#include <SysLib/inc/stm32f2xx_rcc.h>
#include <SysLib/inc/misc.h>

//+=============================================================================================
//|		Function:	BSP_count_up_timer_init
//+---------------------------------------------------------------------------------------------
//!		Initialization of count up timer 
//+---------------------------------------------------------------------------------------------
//!     Parameters:
//!     
//+---------------------------------------------------------------------------------------------
//|		Conditions:
//!		\pre	
//!		\post	
//+---------------------------------------------------------------------------------------------
//|		Annotations:
//+=============================================================================================
void BSP_count_up_timer_init ( INT32U i32uUnits_p )
{
    INT32U timerClockFrequency = 0xffffffff; // in Hz

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);

    TIM5->CR1 = 0x0000;		//Clockdivison 0, Edge-alligned, Down Counter, Update Only Counter under/overflow, One Pulse Mode UpdateEnable, Counter Disabled             
    TIM5->ARR = timerClockFrequency;	// Autoload register = 0 blocks the counter -> therfore dummy value

    RCC_ClocksTypeDef RCC_Clocks;

    BSP_RCC_GetClocksFreq(&RCC_Clocks);
    timerClockFrequency=RCC_Clocks.PCLK1_Frequency; // 30MHz on STM32F207, 64MHz on STM32F107

    if ((RCC->CFGR & RCC_CFGR_PPRE1) != RCC_CFGR_PPRE1_DIV1)
    {
        // APBx timer clocks are muliplied by 2, if prescaler !=1 (see clock tree and register RCC->CFGR, bits 10-12,13-15)
        timerClockFrequency <<= 1;
    }
    //TIM5->PSC = ((timerClockFrequency * i32uUnits_p) / 1000000) - 1;  // Prescaler in us; TIM5 is attached to slow internal bus.
    TIM5->PSC = ((timerClockFrequency / 1000000) * i32uUnits_p) - 1;
    TIM5->EGR = TIM_EGR_UG;       // Generate Update Event 


    TIM5->CNT = (INT16U)0x0000;
}

//+=============================================================================================
//|		Function:	BSP_count_up_timer_start
//+---------------------------------------------------------------------------------------------
//!		Start of count up timer 
//+---------------------------------------------------------------------------------------------
//!     Parameters:
//!     
//+---------------------------------------------------------------------------------------------
//|		Conditions:
//!		\pre	
//!		\post	
//+---------------------------------------------------------------------------------------------
//|		Annotations:
//+=============================================================================================
void BSP_count_up_timer_start ( void )
{
    // Disable counter if it's already running
    if ( ( TIM5->CR1 & TIM_CR1_CEN ) != 0 )
    {
        TIM5->CR1 &= (~TIM_CR1_CEN);      // Counter Disable
        TIM5->CNT = (INT16U)0x0000;
    }

    TIM5->CR1 |= TIM_CR1_CEN;   // Counter Enable
}

//+=============================================================================================
//|		Function:	BSP_count_up_timer_stop
//+---------------------------------------------------------------------------------------------
//!		Stop of count up timer 
//+---------------------------------------------------------------------------------------------
//!     Parameters:
//!     
//+---------------------------------------------------------------------------------------------
//|		Conditions:
//!		\pre	
//!		\post	
//+---------------------------------------------------------------------------------------------
//|		Annotations:
//+=============================================================================================
void BSP_count_up_timer_stop ( void )
{
    TIM5->CR1 &= (~TIM_CR1_CEN);      // Counter Disable
    TIM5->CNT = (INT16U)0x0000;
}

//+=============================================================================================
//|		Function:	BSP_count_up_timer_get
//+---------------------------------------------------------------------------------------------
//!		Gets actual state of count up timer 
//+---------------------------------------------------------------------------------------------
//!     Parameters:
//!     
//+---------------------------------------------------------------------------------------------
//|		Conditions:
//!		\pre	
//!		\post	
//+---------------------------------------------------------------------------------------------
//|		Annotations:
//+=============================================================================================
INT32U BSP_count_up_timer_get ( void )
{
    INT32U i32uCounter_l = 0x0;

    TIM5->CR1 &= (~TIM_CR1_CEN); // Counter Disable
    i32uCounter_l = TIM5->CNT;
    TIM5->CR1 |= TIM_CR1_CEN;    // Counter Enable

    return i32uCounter_l;
}

//*************************************************************************************************

    