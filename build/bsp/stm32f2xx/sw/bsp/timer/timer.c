/*=============================================================================================
*
* timer.c
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
*/
#include <common_define.h>
#include <project.h>
#include <bsp\bspConfig.h>

#ifndef STM_WITH_COUNT_DOWN_TIMER
#error " timer cannot be used without STM_WITH_COUNT_DOWN_TIMER to be defined !"
#endif

#include <bsp\bspError.h>
#include <bsp\timer\timer.h>
#include <bsp\clock\clock.h>
#include <SysLib\inc\stm32f2xx_tim.h>
#include <SysLib\inc\stm32f2xx_rcc.h>
#include <SysLib\inc\misc.h>

//+=============================================================================================
//|        Konstanten / constants
//+=============================================================================================

#define TIM_MAX_TIMER        5

//+=============================================================================================
//|        Prototypen / prototypes
//+=============================================================================================

static  void (*cbTimerExpired_s[TIM_MAX_TIMER])(void);


//*************************************************************************************************
//| Function: TIM_initCountDownTimer
//|
//! brief.
//!
//! detailed.
//!
//!
//!
//! ingroup.
//-------------------------------------------------------------------------------------------------
void TIM_initCountDownTimer (
    INT8U i8uNr_p, 
    TIM_TCountDownTimerInit *ptConf_p)
    
{
    INT32U i32uRv_l = BSPE_NO_ERROR;
    NVIC_InitTypeDef tNvic_l;
    INT32U i32uPSC; // Prescaler factor
    RCC_ClocksTypeDef RCC_Clocks;
    INT32U timerClockFrequency; // in Hz

    if (   (ptConf_p->i32uTimeBase < 0x00000001)   // ticklength_min=1us   (timeout_max=65ms)
        || (ptConf_p->i32uTimeBase > 0x00010000)   // ticklength_max=10ms  (timeout_max=650s)
       )
    {
        i32uRv_l = BSPE_TIMER_PRESCALE_RANGE;
        goto laError;   
    }    

    // Get clocks
    BSP_RCC_GetClocksFreq(&RCC_Clocks);
    // For STM32F207:
    //SYSCLK=HCLK=120MHz
    //PCLK1=30MHz
    //PCLK2=60MHz

    if (i8uNr_p==TIM_TIMER1)
    {
      timerClockFrequency=RCC_Clocks.PCLK2_Frequency; // 60MHz on STM32F207, 64MHz on STM32F107
      if ((RCC->CFGR & RCC_CFGR_PPRE2) != RCC_CFGR_PPRE2_DIV1)
      {
        // APBx timer clocks are muliplied by 2, if prescaler !=1 (see clock tree and register RCC->CFGR, bits 10-12,13-15)
        timerClockFrequency <<= 1;
      }
    }
    else
    {
      timerClockFrequency=RCC_Clocks.PCLK1_Frequency; // 30MHz on STM32F207, 64MHz on STM32F107
      if ((RCC->CFGR & RCC_CFGR_PPRE1) != RCC_CFGR_PPRE1_DIV1)
      {
        // APBx timer clocks are muliplied by 2, if prescaler !=1 (see clock tree and register RCC->CFGR, bits 10-12,13-15)
        timerClockFrequency <<= 1;
      }
    }

    // Prescaler factor (i.e. CK_PSC/CK_CNT = CK_PSC * ticklength)
    i32uPSC = ((timerClockFrequency) / 1000000) * ptConf_p->i32uTimeBase;

    // PSC Register is 16bit wide and will contain value i32uPSC-1
    if( i32uPSC < 1 || i32uPSC > 0x10000)
    {
        i32uRv_l = BSPE_TIMER_PRESCALE_RANGE;
        goto laError;   
    }

  #if (TIM_MAX_TIMER != 5)
    #error "Adjust TIM_MAX_TIMER"
  #endif    
    switch (i8uNr_p)
    {
        case TIM_TIMER1:     // use Timer TIM1
        {
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
            
            TIM1->CR1 = 0x001c;           // Clockdivison 0, Edge-aligned, Down Counter, Update Only Counter under/overflow, One Pulse Mode UpdateEnable, Counter Disabled
            TIM1->ARR = 0xffff;           // Autoload register = 0 blocks the counter -> therefore dummy value
            TIM1->PSC = i32uPSC - 1;      // Prescaler factor
            TIM1->EGR = TIM_EGR_UG;       // Generate Update Event 

            if( ptConf_p->cbTimerExpired )
            {
                // TIM1 Enable the global interrupt
                tNvic_l.NVIC_IRQChannel = TIM1_UP_TIM10_IRQn;
                tNvic_l.NVIC_IRQChannelPreemptionPriority = STM_INTERRUPT_PREEMPTIONPRIORITY_TIMER1;
                tNvic_l.NVIC_IRQChannelSubPriority = STM_INTERRUPT_SUBPRIORITY_TIMER1;
                tNvic_l.NVIC_IRQChannelCmd = ENABLE;
                NVIC_Init(&tNvic_l);

                TIM1->DIER |= TIM_IT_Update; // Enable Update Interrupt
            }
        }   break;

        case TIM_TIMER2:     // use Timer TIM2
        {
           RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

            TIM2->CR1 = 0x001c;           //Clockdivison 0, Edge-alligned, Down Counter, Update Only Counter under/overflow, One Pulse Mode UpdateEnable, Counter Disabled             
            TIM2->ARR = 0xffffffff;           // Autoload register = 0 blocks the counter -> therfore dummy value
            TIM2->PSC = i32uPSC - 1;      // Prescaler factor
            TIM2->EGR = TIM_EGR_UG;       // Generate Update Event
            
            if( ptConf_p->cbTimerExpired )
            {
                // TIM2 Enable the gloabal Interrupt
                tNvic_l.NVIC_IRQChannel = TIM2_IRQn;
                tNvic_l.NVIC_IRQChannelPreemptionPriority = STM_INTERRUPT_PREEMPTIONPRIORITY_TIMER2;
                tNvic_l.NVIC_IRQChannelSubPriority = STM_INTERRUPT_SUBPRIORITY_TIMER2;
                tNvic_l.NVIC_IRQChannelCmd = ENABLE;
                
                NVIC_Init(&tNvic_l);
                TIM2->DIER |= TIM_IT_Update; // Enable Update Interrupt
            }
            
        }   break;

        case TIM_TIMER3:     // use Timer TIM3
        {
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

            TIM3->CR1 = 0x001c;           //Clockdivison 0, Edge-aligned, Down Counter, Update Only Counter under/overflow, One Pulse Mode UpdateEnable, Counter Disabled             
            TIM3->ARR = 0xffff;           // Autoload register = 0 blocks the counter -> therfore dummy value
            TIM3->PSC = i32uPSC - 1;      // Prescaler factor
            TIM3->EGR = TIM_EGR_UG;       // Generate Update Event
            
            if( ptConf_p->cbTimerExpired )
            {
                // TIM3 Enable the global Interrupt
                tNvic_l.NVIC_IRQChannel = TIM3_IRQn;
                tNvic_l.NVIC_IRQChannelPreemptionPriority = STM_INTERRUPT_PREEMPTIONPRIORITY_TIMER3;
                tNvic_l.NVIC_IRQChannelSubPriority = STM_INTERRUPT_SUBPRIORITY_TIMER3;
                tNvic_l.NVIC_IRQChannelCmd = ENABLE;
                NVIC_Init(&tNvic_l);
                TIM3->DIER |= TIM_IT_Update; // Enable Update Interrupt
            }

            
        }   break;

        case TIM_TIMER4:     // use Timer TIM4
        {
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

            TIM4->CR1 = 0x001c;           //Clockdivison 0, Edge-alligned, Down Counter, Update Only Counter under/overflow, One Pulse Mode UpdateEnable, Counter Disabled             
            TIM4->ARR = 0xffff;           // Autoload register = 0 blocks the counter -> therfore dummy value
            TIM4->PSC = i32uPSC - 1;      // Prescaler factor
            TIM4->EGR = TIM_EGR_UG;       // Generate Update Event
            
            if( ptConf_p->cbTimerExpired )
            {
                // TIM4 Enable the global Interrupt
                tNvic_l.NVIC_IRQChannel = TIM4_IRQn;
                tNvic_l.NVIC_IRQChannelPreemptionPriority = STM_INTERRUPT_PREEMPTIONPRIORITY_TIMER4;
                tNvic_l.NVIC_IRQChannelSubPriority = STM_INTERRUPT_SUBPRIORITY_TIMER4;
                tNvic_l.NVIC_IRQChannelCmd = ENABLE;
                
                NVIC_Init(&tNvic_l);
                TIM4->DIER |= TIM_IT_Update; // Enable Update Interrupt
            }
            
        }   break;
        case TIM_TIMER5:     // use Timer TIM5
        {
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);

            TIM5->CR1 = 0x001c;           //Clockdivison 0, Edge-alligned, Down Counter, Update Only Counter under/overflow, One Pulse Mode UpdateEnable, Counter Disabled             
            TIM5->ARR = 0xffffffff;           // Autoload register = 0 blocks the counter -> therfore dummy value
            TIM5->PSC = i32uPSC - 1;      // Prescaler factor
            TIM5->EGR = TIM_EGR_UG;       // Generate Update Event
            
            if( ptConf_p->cbTimerExpired )
            {
                // TIM5 Enable the global Interrupt
                tNvic_l.NVIC_IRQChannel = TIM5_IRQn;
                tNvic_l.NVIC_IRQChannelPreemptionPriority = STM_INTERRUPT_PREEMPTIONPRIORITY_TIMER5;
                tNvic_l.NVIC_IRQChannelSubPriority = STM_INTERRUPT_SUBPRIORITY_TIMER5;
                tNvic_l.NVIC_IRQChannelCmd = ENABLE;
                
                NVIC_Init(&tNvic_l);
                TIM5->DIER |= TIM_IT_Update; // Enable Update Interrupt
            }
            
        }   break;
        default:
        {
            i32uRv_l = BSPE_TIMER_NUM_ERR;
        }    break;    
    }

    // Remember timeout notifier
    cbTimerExpired_s[i8uNr_p-1] = ptConf_p->cbTimerExpired;

    return;

//-------------------------------------------------------------------------------------------------
laError:

    bspError (i32uRv_l, bTRUE, 0);
    return;
}

    
//*************************************************************************************************
//| Function: TIM_CountDownReTrigger
//|
//! brief.
//!
//! detailed.
//!
//!
//!
//! ingroup.
//-------------------------------------------------------------------------------------------------
void TIM_CountDownReTrigger (
    INT8U i8uNr_p,                      //!< Logical Timer Number
    INT32U i32uTime_p)                  //!< Time to count down in configured timebase units
    
{
    switch (i8uNr_p)
    {
        case TIM_TIMER1:     // use Timer TIM1
        {
            TIM1->CNT = (INT16U)i32uTime_p;
            TIM1->CR1 |= TIM_CR1_CEN;          // Counter Enable
        } break;

        case TIM_TIMER2:     // use Timer TIM2
        {
            TIM2->CNT = i32uTime_p;
            TIM2->CR1 |= TIM_CR1_CEN;          // Counter Enable
        } break;

        case TIM_TIMER3:     // use Timer TIM3
        {
            TIM3->CNT = i32uTime_p;
            TIM3->CR1 |= TIM_CR1_CEN;          // Counter Enable
        } break;
        
        case TIM_TIMER4:     // use Timer TIM4
        {
            TIM4->CNT = i32uTime_p;
            TIM4->CR1 |= TIM_CR1_CEN;          // Counter Enable
        } break;
        case TIM_TIMER5:     // use Timer TIM5
        {
            TIM5->CNT = i32uTime_p;
            TIM5->CR1 |= TIM_CR1_CEN;          // Counter Enable
        } break;
        default:
            break;
    }    
}

//*************************************************************************************************
//| Function: TIM_StartTimer
//|
//! brief.
//!
//! detailed.
//!
//!
//!
//! ingroup.
//-------------------------------------------------------------------------------------------------
void TIM_StartTimer (
    INT8U i8uNr_p)                      //!< Logical Timer Number

{
    switch (i8uNr_p)
    {
        case TIM_TIMER1:     // use Timer TIM3
        {
            TIM1->CR1 |= TIM_CR1_CEN;          // Counter enable
        } break;

        case TIM_TIMER2:     // use Timer TIM3
        {
            TIM2->CR1 |= TIM_CR1_CEN ;          // Counter enable
        } break;
        
        case TIM_TIMER3:     // use Timer TIM3
        {
            TIM3->CR1 |= TIM_CR1_CEN;          // Counter enable
        } break;

        case TIM_TIMER4:     // use Timer TIM3
        {
            TIM4->CR1 |= TIM_CR1_CEN ;          // Counter enable
        } break;
        case TIM_TIMER5:     // use Timer TIM5
        {
            TIM5->CR1 |= TIM_CR1_CEN ;          // Counter enable
        } break;
        default:
        {
        }    break;
    }    
}


//*************************************************************************************************
//| Function: TIM_StopTimer
//|
//! brief.
//!
//! detailed.
//!
//!
//!
//! ingroup.
//-------------------------------------------------------------------------------------------------
void TIM_StopTimer (
    INT8U i8uNr_p)                      //!< Logical Timer Number

{
    switch (i8uNr_p)
    {
        case TIM_TIMER1:     // use Timer TIM3
        {
            TIM1->CR1 &= ~TIM_CR1_CEN;          // Counter disable
        } break;

        case TIM_TIMER2:     // use Timer TIM3
        {
            TIM2->CR1 &= ~TIM_CR1_CEN ;          // Counter disable
        } break;
        
        case TIM_TIMER3:     // use Timer TIM3
        {
            TIM3->CR1 &= ~TIM_CR1_CEN;          // Counter disable
        } break;

        case TIM_TIMER4:     // use Timer TIM3
        {
            TIM4->CR1 &= ~TIM_CR1_CEN ;          // Counter disable
        } break;
        case TIM_TIMER5:     // use Timer TIM5
        {
            TIM5->CR1 &= ~TIM_CR1_CEN ;          // Counter disable
        } break;
        default:
        {
        }    break;
    }    
}


//*************************************************************************************************
//| Function: TIM_TimerExpired
//|
//! brief.
//!
//! detailed.
//!
//!
//!
//! ingroup.
//-------------------------------------------------------------------------------------------------
TBOOL TIM_TimerExpired (
    INT8U i8uNr_p)                      //!< Logical Timer Number

{
    TBOOL retval = bFALSE;
    
    switch (i8uNr_p)
    {
        case TIM_TIMER1:     // use Timer TIM1
        {
            if(TIM1->CNT == 0xFFFF) // always 16bit wide
            {
                retval = bTRUE;
            }
        } break;
        
        case TIM_TIMER2:     // use Timer TIM2
        {
            if( TIM2->CNT == 0xffffffff)
            {
                retval = bTRUE;
            }
        }    break;

        case TIM_TIMER3:     // use Timer TIM3
        {
            if( TIM3->CNT == 0xffff )
            {
                retval = bTRUE;
            }
            
        } break;

        case TIM_TIMER4:     // use Timer TIM4
        {
            if( TIM4->CNT == 0xffff )
            {
                retval = bTRUE;
            }
        }    break;
        case TIM_TIMER5:     // use Timer TIM5
        {
            if( TIM5->CNT == 0xffffffff)
            {
                retval = bTRUE;
            }
        }    break;
        default:
        {
        }    break;
    }
    return retval;
}

//*************************************************************************************************
//| Function: TIM1_intHandler
//|
//! brief.
//! user configurable count down timer interrupt
//! detailed.
//!    kbPnoz_int_spi_frame_tmr
//!
//!
//! ingroup.
//-------------------------------------------------------------------------------------------------
void TIM1_intHandler (
    void)
    
{
    // Note that this line has been changed in location and contents for STM32F207XX
    TIM1->SR &= ~TIM_FLAG_Update;  // clear update flag in Status register

    if (cbTimerExpired_s[TIM_TIMER1-1])
    {
        cbTimerExpired_s[TIM_TIMER1-1] ();
    }    
}

//*************************************************************************************************
//| Function: TIM2_intHandler
//|
//! brief.
//!
//! detailed.
//!
//!
//!
//! ingroup.
//-------------------------------------------------------------------------------------------------
void TIM2_intHandler (
    void)
{
    // Note that this line has been changed in location and contents for STM32F207XX
    TIM2->SR &= ~TIM_FLAG_Update;  // clear update flag in Status register

    if (cbTimerExpired_s[TIM_TIMER2-1])
    {
        cbTimerExpired_s[TIM_TIMER2-1] ();
    }    
}


//*************************************************************************************************
//| Function: TIM3_intHandler
//|
//! brief.
//!
//! detailed.
//!
//!
//!
//! ingroup.
//-------------------------------------------------------------------------------------------------
void TIM3_intHandler (
    void)
{
    // Note that this line has been changed in location and contents for STM32F207XX
    TIM3->SR &= ~TIM_FLAG_Update;  // clear update flag in Status register

    if (cbTimerExpired_s[TIM_TIMER3-1])
    {
        cbTimerExpired_s[TIM_TIMER3-1] ();
    }
}


//*************************************************************************************************
//| Function: TIM4_intHandler
//|
//! brief.
//!
//! detailed.
//!
//!
//!
//! ingroup.
//-------------------------------------------------------------------------------------------------
void TIM4_intHandler (
    void)
    
{
    // Note that this line has been changed in location and contents for STM32F207XX
    TIM4->SR &= ~TIM_FLAG_Update;  // clear update flag in Status register

    if (cbTimerExpired_s[TIM_TIMER4-1])
    {
        cbTimerExpired_s[TIM_TIMER4-1] ();
    }    
}

//*************************************************************************************************
//| Function: TIM5_intHandler
//|
//! brief.
//!
//! detailed.
//!
//!
//!
//! ingroup.
//-------------------------------------------------------------------------------------------------
void TIM5_intHandler (
    void)
{
    // Note that this line has been changed in location and contents for STM32F207XX
    TIM5->SR &= ~TIM_FLAG_Update;  // clear update flag in Status register

    if (cbTimerExpired_s[TIM_TIMER5-1])
    {
        cbTimerExpired_s[TIM_TIMER5-1] ();
    }    
}

//*************************************************************************************************
