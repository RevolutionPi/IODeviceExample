/*=============================================================================================
*
* timer.h
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
#ifndef TIMER_H_INC
#define TIMER_H_INC

// Map SW to HW timers
#define TIM_TIMER0  0
#define TIM_TIMER1  1
#define TIM_TIMER2  2
#define TIM_TIMER3  3
#define TIM_TIMER4  4
#define TIM_TIMER5  5
#define TIM_TIMER6  6
#define TIM_TIMER7  7

//+=============================================================================================
//|   Typen / types
//+=============================================================================================
typedef struct  _TIM_TCountDownTimerInit
{
    void  (*cbTimerExpired)(void);
    INT32U  i32uTimeBase;                //!< Tick length of the timer in us
} TIM_TCountDownTimerInit;


//+=============================================================================================
//|   Prototypen / prototypes
//+=============================================================================================
#ifdef __cplusplus
extern "C" { 
#endif 

void   TIM_initCountDownTimer  ( INT8U i8uNr_p, TIM_TCountDownTimerInit *ptConf_p );
void   TIM_CountDownReTrigger  ( INT8U i8uNr_p, INT32U i32uTime_p );
void   TIM_StartTimer          ( INT8U i8uNr_p );
void   TIM_StopTimer           ( INT8U i8uNr_p );
TBOOL  TIM_TimerExpired        ( INT8U i8uNr_p );
void   BSP_Wait                (INT32U i32uTicks_p);


//+=============================================================================================
/*  for STM32F2XX: i32uUnits_p is the number us to increase the counter by 1
    for STM32F1XX: number of clock cycles to increase the counter by 1 */
void     BSP_count_up_timer_init     ( INT32U i32uUnits_p );
void     BSP_count_up_timer_start    ( void );
void     BSP_count_up_timer_stop     ( void );
INT32U   BSP_count_up_timer_get      ( void );

//+=============================================================================================

void TIM1_intHandler (void);
void TIM2_intHandler (void);
void TIM3_intHandler (void);
void TIM4_intHandler (void);
void TIM5_intHandler (void);
void TIM7_intHandler (void);

#ifdef __cplusplus
}    
#endif 

#endif //TIMER_H_INC