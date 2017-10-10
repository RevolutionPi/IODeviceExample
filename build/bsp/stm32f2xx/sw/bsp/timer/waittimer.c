//+=============================================================================================
//|
//!		\file STM_WAITTIMER.c
//!
//!		Implementation of blocking timer (wait for time event).
//|
//+---------------------------------------------------------------------------------------------
//|
//|		File-ID:		$Id: waittimer.c 7663 2014-12-03 18:52:40Z fbrandauer $
//|		Location:		$URL: http://srv-kunbus03.de.pilz.local/feldbus/software/trunk/platform/bsp/stm32f2xx/sw/bsp/timer/waittimer.c $
//|		Company:		$Cpn: KUNBUS GmbH $
//|
//+---------------------------------------------------------------------------------------------
//|
//|		Files required:	(none)
//|
//+=============================================================================================
#include <common_define.h>
#include <project.h>
#include <bsp\bspConfig.h>

#include <bsp/timer/timer.h>
#include <SysLib/inc/stm32f2xx_tim.h>
#include <SysLib/inc/stm32f2xx_rcc.h>
#include <SysLib/inc/misc.h>

#include <bsp/bspError.h>

#if !defined (STM_WAITTIMER)
    #error "STM_WAITTIMER not defined"
#endif


//+=============================================================================================
//|		Function:	BSP_Wait
//+---------------------------------------------------------------------------------------------
//!		Implementation of blocking wait timer
//+---------------------------------------------------------------------------------------------
//!     Parameters:
//!     i32uMicroSeconds_p number of microseconds to wait
//+---------------------------------------------------------------------------------------------
//|		Conditions:
//!		\pre	u-controller reset
//!		\post	pnoz-interface ready for communication
//+---------------------------------------------------------------------------------------------
//|		Annotations:
//+=============================================================================================
void BSP_Wait (INT32U i32uMicroSeconds_p)
{

    if (STM_WAITTIMER == TIM2)
    {
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
        STM_WAITTIMER->PSC = ((STM_SYSCLOCK) / 1000000) - 1;  
    }
    else if (STM_WAITTIMER == TIM3)
    {
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
        STM_WAITTIMER->PSC = ((STM_SYSCLOCK) / 1000000) - 1;  
    }
    else if (STM_WAITTIMER == TIM4)
    {
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
        STM_WAITTIMER->PSC = ((STM_SYSCLOCK) / 1000000) - 1;  
    }
    else if (STM_WAITTIMER == TIM5)
    {
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);
        STM_WAITTIMER->PSC = ((STM_SYSCLOCK) / 1000000) - 1;  
    }
    else if (STM_WAITTIMER == TIM6)
    {
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);
        STM_WAITTIMER->PSC = ((STM_SYSCLOCK) / 1000000) - 1;  
    }
    else if (STM_WAITTIMER == TIM7)
    {
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);
        STM_WAITTIMER->PSC = ((STM_SYSCLOCK) / 1000000) - 1;  
    }
    else if (STM_WAITTIMER == TIM12)
    {
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM12, ENABLE);
        STM_WAITTIMER->PSC = ((STM_SYSCLOCK) / 1000000) - 1;  
    }
    else if (STM_WAITTIMER == TIM13)
    {
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM13, ENABLE);
        STM_WAITTIMER->PSC = ((STM_SYSCLOCK) / 1000000) - 1;  
    }
    else if (STM_WAITTIMER == TIM14)
    {
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM14, ENABLE);
        STM_WAITTIMER->PSC = ((STM_SYSCLOCK) / 1000000) - 1;  
    }
    else
    {
        bspError (BSPE_WAITTIMER_INV, bTRUE, 0);
    }  

    STM_WAITTIMER->DIER = 0x0000;          // Disable all DMA and Interrupt request
    STM_WAITTIMER->CR1 = 0x000c;           //Clockdivison 0, Edge-alligned, Up Counter, Update Only Counter under/overflow, One Pulse Mode UpdateEnable, Counter Disabled             
    STM_WAITTIMER->ARR = 0xffff;           // Autoload register = 0 blocks the timer, therefore upper end


    STM_WAITTIMER->EGR = TIM_EGR_UG;  // Generate Update Event 

    STM_WAITTIMER->CNT = (INT16U)(0x10000 - i32uMicroSeconds_p);

    STM_WAITTIMER->SR = 0;
    STM_WAITTIMER->CR1 |= TIM_CR1_CEN;          // Counter Enable

    while (!(STM_WAITTIMER->SR & TIM_SR_UIF))
    {

    }
}