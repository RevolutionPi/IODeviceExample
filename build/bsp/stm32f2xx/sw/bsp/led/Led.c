/*=============================================================================================
*
* Led.c
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
* LED handler.
* These functions enable or disable leds, let them flash, blink or flicker due to
* corresponding configuration.
*/
#include <common_define.h>
#include <project.h>

#ifndef STM_WITH_LED
#error STM_WITH_LED must be defined in project.h to use LED driver
#endif

#include <bsp/bspConfig.h>
#include <bsp/led/led.h>
#include <Syslib/inc/stm32f2xx.h>
#include <bsp/led/ledIntern.h>

#ifdef STM_LED_GPIO
#include <bsp/gpio/gpio.h>
#endif
INT16U    LED_i16uLedVariable_g;

#include <bsp/bspError.h>

// This variable is used to temporary prevent LED updates and is needed for two cases:
// a) To temporary use LED output pins for other purposes, e.g. as input values from rotary switches
// b) To make sure there are no LED changes during execution of LED_setLed(), which could
//    erroneously convert e.g. an intended red-green blinker to a yellow blinker.
TBOOL bReadSw_g = bFALSE;

#define HIGH 0
#define LOW  1

#ifndef LED_0_R_PIN
#define LED_0_R_PIN 0x0001
#endif
#ifndef LED_0_G_PIN
#define LED_0_G_PIN 0x0001
#endif
#ifndef LED_1_R_PIN
#define LED_1_R_PIN 0x0002
#endif
#ifndef LED_1_G_PIN
#define LED_1_G_PIN 0x0002
#endif
#ifndef LED_2_R_PIN
#define LED_2_R_PIN 0x0004
#endif
#ifndef LED_2_G_PIN
#define LED_2_G_PIN 0x0004
#endif
#ifndef LED_3_R_PIN
#define LED_3_R_PIN 0x0008
#endif
#ifndef LED_3_G_PIN
#define LED_3_G_PIN 0x0008
#endif
#ifndef LED_4_R_PIN
#define LED_4_R_PIN 0x0010
#endif
#ifndef LED_4_G_PIN
#define LED_4_G_PIN 0x0010
#endif
#ifndef LED_5_R_PIN
#define LED_5_R_PIN 0x0020
#endif
#ifndef LED_5_G_PIN
#define LED_5_G_PIN 0x0020
#endif
#ifndef LED_6_R_PIN
#define LED_6_R_PIN 0x0040
#endif
#ifndef LED_6_G_PIN
#define LED_6_G_PIN 0x0040
#endif
#ifndef LED_7_R_PIN
#define LED_7_R_PIN 0x0080
#endif
#ifndef LED_7_G_PIN
#define LED_7_G_PIN 0x0080
#endif
#ifndef LED_8_R_PIN
#define LED_8_R_PIN 0x0100
#endif
#ifndef LED_8_G_PIN
#define LED_8_G_PIN 0x0100
#endif
#ifndef LED_9_R_PIN
#define LED_9_R_PIN 0x0100
#endif
#ifndef LED_9_G_PIN
#define LED_9_G_PIN 0x0200
#endif
#ifndef LED_10_R_PIN
#define LED_10_R_PIN 0x0400
#endif
#ifndef LED_10_G_PIN
#define LED_10_G_PIN 0x0400
#endif
#ifndef LED_11_R_PIN
#define LED_11_R_PIN 0x0800
#endif
#ifndef LED_11_G_PIN
#define LED_11_G_PIN 0x0800
#endif
#ifndef LED_12_R_PIN
#define LED_12_R_PIN 0x1000
#endif
#ifndef LED_12_G_PIN
#define LED_12_G_PIN 0x1000
#endif
#ifndef LED_13_R_PIN
#define LED_13_R_PIN 0x2000
#endif
#ifndef LED_13_G_PIN
#define LED_13_G_PIN 0x2000
#endif
#ifndef LED_14_R_PIN
#define LED_14_R_PIN 0x4000
#endif
#ifndef LED_14_G_PIN
#define LED_14_G_PIN 0x4000
#endif
#ifndef LED_15_R_PIN
#define LED_15_R_PIN 0x8000
#endif
#ifndef LED_15_G_PIN
#define LED_15_G_PIN 0x8000
#endif


//*************************************************************************************************
//! array for the state management of the LEDs. each dual color LED occupies two consecutive entries
//! The lower one is for the red color, the upper one for the green color.
//! It is assumed that each color can be served independently by an own, dedicated pin from the GPIO
//! unit of the microprocessor

LED_TLed LED_atLed_g[LED_MAX_DUAL_LED * 2] =
{

#if (LED_MAX_DUAL_LED >= 1)
    //---------- backside-leds ----------
    {    //[0] LED 0 red
        LED_ST_ALL_OFF,
            0x0000,
            0x0001,
            0x8000,
#ifdef STM_LED_GPIO
#if defined (LED_0_R_PORT) && (LED_0_R_ACTIVE == HIGH)
            LED_0_R_PORT,
            &LED_0_R_PORT->BSRRL,
            &LED_0_R_PORT->BSRRH,
#elif defined (LED_0_R_PORT) && (LED_0_R_ACTIVE == LOW)
            LED_0_R_PORT,
            &LED_0_R_PORT->BSRRH,
            &LED_0_R_PORT->BSRRL,
#else
            NULL,
            NULL,
            NULL,
#endif
#endif
            LED_0_R_PIN,
            1,
            1,
            bFALSE
    },
    {    //[1] LED 0 green
        LED_ST_ALL_OFF,
        0x0000,
        0x0001,
        0x8000,
#ifdef STM_LED_GPIO
#if defined (LED_0_G_PORT) && (LED_0_G_ACTIVE == HIGH)
        LED_0_G_PORT,
        &LED_0_G_PORT->BSRRL,
        &LED_0_G_PORT->BSRRH,
  #elif defined (LED_0_G_PORT) && (LED_0_G_ACTIVE == LOW)
        LED_0_G_PORT,
        &LED_0_G_PORT->BSRRH,
        &LED_0_G_PORT->BSRRL,
  #else
        NULL,
        NULL,
        NULL,
  #endif        
#endif
        LED_0_G_PIN,
        1,
        1,
        bFALSE
    },
#endif

#if (LED_MAX_DUAL_LED >= 2)
    //---------- upper-left-leds (1) ----------
    {   //[2] LED 1.1 red
        LED_ST_ALL_OFF,
        0x0000,
        0x0001,
        0x8000,
#ifdef STM_LED_GPIO
  #if defined (LED_1_R_PORT) && (LED_1_R_ACTIVE == HIGH)
        LED_1_R_PORT,
        &LED_1_R_PORT->BSRRL,
        &LED_1_R_PORT->BSRRH,
  #elif defined (LED_1_R_PORT) && (LED_1_R_ACTIVE == LOW)
        LED_1_R_PORT,
        &LED_1_R_PORT->BSRRH,
        &LED_1_R_PORT->BSRRL,
  #else
        NULL,
        NULL,
        NULL,
  #endif        
#endif
        LED_1_R_PIN,
        1,
        1,
        bFALSE
    },
    {   //[3] LED 2.1 green
        LED_ST_ALL_OFF,
        0x0000,
        0x0001,
        0x8000,
#ifdef STM_LED_GPIO
  #if defined (LED_1_G_PORT) && (LED_1_G_ACTIVE == HIGH)
        LED_1_G_PORT,
        &LED_1_G_PORT->BSRRL,
        &LED_1_G_PORT->BSRRH,
  #elif defined (LED_1_G_PORT) && (LED_1_G_ACTIVE == LOW)
        LED_1_G_PORT,
        &LED_1_G_PORT->BSRRH,
        &LED_1_G_PORT->BSRRL,
  #else
        NULL,
        NULL,
        NULL,
  #endif        
#endif
        LED_1_G_PIN,
        1,
        1,
        bFALSE
    },
#endif

#if (LED_MAX_DUAL_LED >= 3)
    //---------- upper-right-leds (2) ----------
    {   //[4] LED 1.2 red
        LED_ST_ALL_OFF,
        0x0000,
        0x0001,
        0x8000,
#ifdef STM_LED_GPIO
  #if defined (LED_2_R_PORT) && (LED_2_R_ACTIVE == HIGH)
        LED_2_R_PORT,
        &LED_2_R_PORT->BSRRL,
        &LED_2_R_PORT->BSRRH,
  #elif defined (LED_2_R_PORT) && (LED_2_R_ACTIVE == LOW)
        LED_2_R_PORT,
        &LED_2_R_PORT->BSRRH,
        &LED_2_R_PORT->BSRRL,
  #else
        NULL,
        NULL,
        NULL,
  #endif        
#endif
        LED_2_R_PIN,
        1,
        1,
        bFALSE
    },
    {   //[5] LED 2.2 green
        LED_ST_ALL_OFF,
        0x0000,
        0x0001,
        0x8000,
#ifdef STM_LED_GPIO
  #if defined (LED_2_G_PORT) && (LED_2_G_ACTIVE == HIGH)
        LED_2_G_PORT,
        &LED_2_G_PORT->BSRRL,
        &LED_2_G_PORT->BSRRH,
  #elif defined (LED_2_G_PORT) && (LED_2_G_ACTIVE == LOW)
        LED_2_G_PORT,
        &LED_2_G_PORT->BSRRH,
        &LED_2_G_PORT->BSRRL,
  #else
        NULL,
        NULL,
        NULL,
  #endif        
#endif
        LED_2_G_PIN,
        1,
        1,
        bFALSE
    },
#endif

#if (LED_MAX_DUAL_LED >= 4)
    //---------- lower-right-leds (3) ----------
    {   //[6] LED 1.3 red
        LED_ST_ALL_OFF,
        0x0000,
        0x0001,
        0x8000,
#ifdef STM_LED_GPIO
  #if defined (LED_3_R_PORT) && (LED_3_R_ACTIVE == HIGH)
        LED_3_R_PORT,
        &LED_3_R_PORT->BSRRL,
        &LED_3_R_PORT->BSRRH,
  #elif defined (LED_3_R_PORT) && (LED_3_R_ACTIVE == LOW)
        LED_3_R_PORT,
        &LED_3_R_PORT->BSRRH,
        &LED_3_R_PORT->BSRRL,
  #else
        NULL,
        NULL,
        NULL,
  #endif        
#endif
        LED_3_R_PIN,
        1,
        1,
        bFALSE
    },
    {   //[7] LED 2.3 green
        LED_ST_ALL_OFF,
        0x0000,
        0x0001,
        0x8000,
#ifdef STM_LED_GPIO
  #if defined (LED_3_G_PORT) && (LED_3_G_ACTIVE == HIGH)
        LED_3_G_PORT,
        &LED_3_G_PORT->BSRRL,
        &LED_3_G_PORT->BSRRH,
  #elif defined (LED_3_G_PORT) && (LED_3_G_ACTIVE == LOW)
        LED_3_G_PORT,
        &LED_3_G_PORT->BSRRH,
        &LED_3_G_PORT->BSRRL,
  #else
        NULL,
        NULL,
        NULL,
  #endif        
#endif
        LED_3_G_PIN,
        1,
        1,
        bFALSE
    },
#endif


#if (LED_MAX_DUAL_LED >= 5)
    //---------- lower-left-leds (4) ----------
    {   //[8] LED 1.4 red
        LED_ST_ALL_OFF,
        0x0000,
        0x0001,
        0x8000,
#ifdef STM_LED_GPIO
  #if defined (LED_4_R_PORT) && (LED_4_R_ACTIVE == HIGH)
        LED_4_R_PORT,
        &LED_4_R_PORT->BSRRL,
        &LED_4_R_PORT->BSRRH,
  #elif defined (LED_4_R_PORT) && (LED_4_R_ACTIVE == LOW)
        LED_4_R_PORT,
        &LED_4_R_PORT->BSRRH,
        &LED_4_R_PORT->BSRRL,
  #else
        NULL,
        NULL,
        NULL,
  #endif        
#endif
        LED_4_R_PIN,
        1,
        1,
        bFALSE
    },
    {   //[9] LED 2.4 green
        LED_ST_ALL_OFF,
        0x0000,
        0x0001,
        0x8000,
#ifdef STM_LED_GPIO
  #if defined (LED_4_G_PORT) && (LED_4_G_ACTIVE == HIGH)
        LED_4_G_PORT,
        &LED_4_G_PORT->BSRRL,
        &LED_4_G_PORT->BSRRH,
  #elif defined (LED_4_G_PORT) && (LED_4_G_ACTIVE == LOW)
        LED_4_G_PORT,
        &LED_4_G_PORT->BSRRH,
        &LED_4_G_PORT->BSRRL,
  #else
        NULL,
        NULL,
        NULL,
  #endif        
#endif
        LED_4_G_PIN,
        1,
        1,
        bFALSE
    },
#endif


#if (LED_MAX_DUAL_LED >= 6)
    {   //[10]
        LED_ST_ALL_OFF,
        0x0000,
        0x0001,
        0x8000,
#ifdef STM_LED_GPIO
  #if defined (LED_5_R_PORT) && (LED_5_R_ACTIVE == HIGH)
        LED_5_R_PORT,
        &LED_5_R_PORT->BSRRL,
        &LED_5_R_PORT->BSRRH,
  #elif defined (LED_5_R_PORT) && (LED_5_R_ACTIVE == LOW)
        LED_5_R_PORT,
        &LED_5_R_PORT->BSRRH,
        &LED_5_R_PORT->BSRRL,
  #else
        NULL,
        NULL,
        NULL,
  #endif        
#endif
        LED_5_R_PIN,
        1,
        1,
        bFALSE
    },
    {   //[11]
        LED_ST_ALL_OFF,
        0x0000,
        0x0001,
        0x8000,
#ifdef STM_LED_GPIO
  #if defined (LED_5_G_PORT) && (LED_5_G_ACTIVE == HIGH)
        LED_5_G_PORT,
        &LED_5_G_PORT->BSRRL,
        &LED_5_G_PORT->BSRRH,
  #elif defined (LED_5_G_PORT) && (LED_5_G_ACTIVE == LOW)
        LED_5_G_PORT,
        &LED_5_G_PORT->BSRRH,
        &LED_5_G_PORT->BSRRL,
  #else
        NULL,
        NULL,
        NULL,
  #endif        
#endif
        LED_5_G_PIN,
        1,
        1,
        bFALSE
    },
#endif

#if (LED_MAX_DUAL_LED >= 7)
    {   //[12] 
        LED_ST_ALL_OFF,
        0x0000,
        0x0001,
        0x8000,
#ifdef STM_LED_GPIO
  #if defined (LED_6_R_PORT) && (LED_6_R_ACTIVE == HIGH)
        LED_6_R_PORT,
        &LED_6_R_PORT->BSRRL,
        &LED_6_R_PORT->BSRRH,
  #elif defined (LED_6_R_PORT) && (LED_6_R_ACTIVE == LOW)
        LED_6_R_PORT,
        &LED_6_R_PORT->BSRRH,
        &LED_6_R_PORT->BSRRL,
  #else
        NULL,
        NULL,
        NULL,
  #endif        
#endif
        LED_6_R_PIN,
        1,
        1,
        bFALSE
    },
    {   //[13]
        LED_ST_ALL_OFF,
        0x0000,
        0x0001,
        0x8000,
#ifdef STM_LED_GPIO
  #if defined (LED_6_G_PORT) && (LED_6_G_ACTIVE == HIGH)
        LED_6_G_PORT,
        &LED_6_G_PORT->BSRRL,
        &LED_6_G_PORT->BSRRH,
  #elif defined (LED_6_G_PORT) && (LED_6_G_ACTIVE == LOW)
        LED_6_G_PORT,
        &LED_6_G_PORT->BSRRH,
        &LED_6_G_PORT->BSRRL,
  #else
        NULL,
        NULL,
        NULL,
  #endif        
#endif
        LED_6_G_PIN,
        1,
        1,
        bFALSE
    },
#endif

#if (LED_MAX_DUAL_LED >= 8)
    {   //[14]
        LED_ST_ALL_OFF,
        0x0000,
        0x0001,
        0x8000,
#ifdef STM_LED_GPIO
  #if defined (LED_7_R_PORT) && (LED_7_R_ACTIVE == HIGH)
        LED_7_R_PORT,
        &LED_7_R_PORT->BSRRL,
        &LED_7_R_PORT->BSRRH,
  #elif defined (LED_7_R_PORT) && (LED_7_R_ACTIVE == LOW)
        LED_7_R_PORT,
        &LED_7_R_PORT->BSRRH,
        &LED_7_R_PORT->BSRRL,
  #else
        NULL,
        NULL,
        NULL,
  #endif        
#endif
        LED_7_R_PIN,
        1,
        1,
        bFALSE
    },
    {   //[15]
        LED_ST_ALL_OFF,
        0x0000,
        0x0001,
        0x8000,
#ifdef STM_LED_GPIO
  #if defined (LED_7_G_PORT) && (LED_7_G_ACTIVE == HIGH)
        LED_7_G_PORT,
        &LED_7_G_PORT->BSRRL,
        &LED_7_G_PORT->BSRRH,
  #elif defined (LED_7_G_PORT) && (LED_7_G_ACTIVE == LOW)
        LED_7_G_PORT,
        &LED_7_G_PORT->BSRRH,
        &LED_7_G_PORT->BSRRL,
  #else
        NULL,
        NULL,
        NULL,
  #endif        
#endif
        LED_7_G_PIN,
        1,
        1,
        bFALSE
    },
#endif
#if (LED_MAX_DUAL_LED >= 9)
    {   //[16]
        LED_ST_ALL_OFF,
        0x0000,
        0x0001,
        0x8000,
#ifdef STM_LED_GPIO
  #if defined (LED_8_R_PORT) && (LED_8_R_ACTIVE == HIGH)
        LED_8_R_PORT,
        &LED_8_R_PORT->BSRRL,
        &LED_8_R_PORT->BSRRH,
  #elif defined (LED_8_R_PORT) && (LED_8_R_ACTIVE == LOW)
        LED_8_R_PORT,
        &LED_8_R_PORT->BSRRH,
        &LED_8_R_PORT->BSRRL,
  #else
        NULL,
        NULL,
        NULL,
  #endif        
#endif
        LED_8_R_PIN,
        1,
        1,
        bFALSE
    },
    {   //[17]
        LED_ST_ALL_OFF,
        0x0000,
        0x0001,
        0x8000,
#ifdef STM_LED_GPIO
  #if defined (LED_8_G_PORT) && (LED_8_G_ACTIVE == HIGH)
        LED_8_G_PORT,
        &LED_8_G_PORT->BSRRL,
        &LED_8_G_PORT->BSRRH,
  #elif defined (LED_8_G_PORT) && (LED_8_G_ACTIVE == LOW)
        LED_8_G_PORT,
        &LED_8_G_PORT->BSRRH,
        &LED_8_G_PORT->BSRRL,
  #else
        NULL,
        NULL,
        NULL,
  #endif        
#endif
        LED_8_G_PIN,
        1,
        1,
        bFALSE
    },
#endif
#if (LED_MAX_DUAL_LED >= 10)
    {   //[18]
        LED_ST_ALL_OFF,
        0x0000,
        0x0001,
        0x8000,
#ifdef STM_LED_GPIO
  #if defined (LED_9_R_PORT) && (LED_9_R_ACTIVE == HIGH)
        LED_9_R_PORT,
        &LED_9_R_PORT->BSRRL,
        &LED_9_R_PORT->BSRRH,
  #elif defined (LED_9_R_PORT) && (LED_9_R_ACTIVE == LOW)
        LED_9_R_PORT,
        &LED_9_R_PORT->BSRRH,
        &LED_9_R_PORT->BSRRL,
  #else
        NULL,
        NULL,
        NULL,
  #endif        
#endif
        LED_9_R_PIN,
        1,
        1,
        bFALSE
    },
    {   //[19]
        LED_ST_ALL_OFF,
        0x0000,
        0x0001,
        0x8000,
#ifdef STM_LED_GPIO
  #if defined (LED_9_G_PORT) && (LED_9_G_ACTIVE == HIGH)
        LED_9_G_PORT,
        &LED_9_G_PORT->BSRRL,
        &LED_9_G_PORT->BSRRH,
  #elif defined (LED_9_G_PORT) && (LED_9_G_ACTIVE == LOW)
        LED_9_G_PORT,
        &LED_9_G_PORT->BSRRH,
        &LED_9_G_PORT->BSRRL,
  #else
        NULL,
        NULL,
        NULL,
  #endif        
#endif
        LED_9_G_PIN,
        1,
        1,
        bFALSE
    },
#endif

#if (LED_MAX_DUAL_LED >= 11)
    {   //[20]
        LED_ST_ALL_OFF,
        0x0000,
        0x0001,
        0x8000,
#ifdef STM_LED_GPIO
  #if defined (LED_10_R_PORT) && (LED_10_R_ACTIVE == HIGH)
        LED_10_R_PORT,
        &LED_10_R_PORT->BSRRL,
        &LED_10_R_PORT->BSRRH,
  #elif defined (LED_10_R_PORT) && (LED_10_R_ACTIVE == LOW)
        LED_10_R_PORT,
        &LED_10_R_PORT->BSRRH,
        &LED_10_R_PORT->BSRRL,
  #else
        NULL,
        NULL,
        NULL,
  #endif        
#endif
        LED_10_R_PIN,
        1,
        1,
        bFALSE
    },
    {   //[21]
        LED_ST_ALL_OFF,
        0x0000,
        0x0001,
        0x8000,
#ifdef STM_LED_GPIO
  #if defined (LED_10_G_PORT) && (LED_10_G_ACTIVE == HIGH)
        LED_10_G_PORT,
        &LED_10_G_PORT->BSRRL,
        &LED_10_G_PORT->BSRRH,
  #elif defined (LED_10_G_PORT) && (LED_10_G_ACTIVE == LOW)
        LED_10_G_PORT,
        &LED_10_G_PORT->BSRRH,
        &LED_10_G_PORT->BSRRL,
  #else
        NULL,
        NULL,
        NULL,
  #endif        
#endif
        LED_10_G_PIN,
        1,
        1,
        bFALSE
    },
#endif

#if (LED_MAX_DUAL_LED >= 12)
    {   //[22]
        LED_ST_ALL_OFF,
        0x0000,
        0x0001,
        0x8000,
#ifdef STM_LED_GPIO
  #if defined (LED_11_R_PORT) && (LED_11_R_ACTIVE == HIGH)
        LED_11_R_PORT,
        &LED_11_R_PORT->BSRRL,
        &LED_11_R_PORT->BSRRH,
  #elif defined (LED_11_R_PORT) && (LED_11_R_ACTIVE == LOW)
        LED_11_R_PORT,
        &LED_11_R_PORT->BSRRH,
        &LED_11_R_PORT->BSRRL,
  #else
        NULL,
        NULL,
        NULL,
#endif
  #endif        
        LED_11_R_PIN,
        1,
        1,
        bFALSE
    },
    {   //[23]
        LED_ST_ALL_OFF,
        0x0000,
        0x0001,
        0x8000,
#ifdef STM_LED_GPIO
  #if defined (LED_11_G_PORT) && (LED_11_G_ACTIVE == HIGH)
        LED_11_G_PORT,
        &LED_11_G_PORT->BSRRL,
        &LED_11_G_PORT->BSRRH,
  #elif defined (LED_11_G_PORT) && (LED_11_G_ACTIVE == LOW)
        LED_11_G_PORT,
        &LED_11_G_PORT->BSRRH,
        &LED_11_G_PORT->BSRRL,
  #else
        NULL,
        NULL,
        NULL,
  #endif     
#endif
        LED_11_G_PIN,
        1,
        1,
        bFALSE
    },
#endif

#if (LED_MAX_DUAL_LED >= 13)
    {   //[24]
        LED_ST_ALL_OFF,
        0x0000,
        0x0001,
        0x8000,
#ifdef STM_LED_GPIO
#if defined (LED_12_R_PORT) && (LED_12_R_ACTIVE == HIGH)
        LED_12_R_PORT,
        &LED_12_R_PORT->BSRRL,
        &LED_12_R_PORT->BSRRH,
#elif defined (LED_12_R_PORT) && (LED_12_R_ACTIVE == LOW)
        LED_12_R_PORT,
        &LED_12_R_PORT->BSRRH,
        &LED_12_R_PORT->BSRRL,
#else
        NULL,
        NULL,
        NULL,
#endif
#endif        
        LED_12_R_PIN,
        1,
        1,
        bFALSE
    },
    {   //[25]
        LED_ST_ALL_OFF,
        0x0000,
        0x0001,
        0x8000,
#ifdef STM_LED_GPIO
#if defined (LED_12_G_PORT) && (LED_12_G_ACTIVE == HIGH)
        LED_12_G_PORT,
        &LED_12_G_PORT->BSRRL,
        &LED_12_G_PORT->BSRRH,
#elif defined (LED_12_G_PORT) && (LED_12_G_ACTIVE == LOW)
        LED_12_G_PORT,
        &LED_12_G_PORT->BSRRH,
        &LED_12_G_PORT->BSRRL,
#else
        NULL,
        NULL,
        NULL,
#endif     
#endif
        LED_12_G_PIN,
        1,
        1,
        bFALSE
    },
#endif

#if (LED_MAX_DUAL_LED >= 14)
    {   //[26]
        LED_ST_ALL_OFF,
        0x0000,
        0x0001,
        0x8000,
#ifdef STM_LED_GPIO
#if defined (LED_13_R_PORT) && (LED_13_R_ACTIVE == HIGH)
        LED_13_R_PORT,
        &LED_13_R_PORT->BSRRL,
        &LED_13_R_PORT->BSRRH,
#elif defined (LED_13_R_PORT) && (LED_13_R_ACTIVE == LOW)
        LED_13_R_PORT,
        &LED_13_R_PORT->BSRRH,
        &LED_13_R_PORT->BSRRL,
#else
        NULL,
        NULL,
        NULL,
#endif
#endif        
        LED_13_R_PIN,
        1,
        1,
        bFALSE
    },
    {   //[27]
        LED_ST_ALL_OFF,
        0x0000,
        0x0001,
        0x8000,
#ifdef STM_LED_GPIO
#if defined (LED_13_G_PORT) && (LED_13_G_ACTIVE == HIGH)
        LED_13_G_PORT,
        &LED_13_G_PORT->BSRRL,
        &LED_13_G_PORT->BSRRH,
#elif defined (LED_13_G_PORT) && (LED_13_G_ACTIVE == LOW)
        LED_13_G_PORT,
        &LED_13_G_PORT->BSRRH,
        &LED_13_G_PORT->BSRRL,
#else
        NULL,
        NULL,
        NULL,
#endif     
#endif
        LED_13_G_PIN,
        1,
        1,
        bFALSE
    },
#endif

#if (LED_MAX_DUAL_LED >= 15)
    {   //[28]
        LED_ST_ALL_OFF,
        0x0000,
        0x0001,
        0x8000,
#ifdef STM_LED_GPIO
#if defined (LED_14_R_PORT) && (LED_14_R_ACTIVE == HIGH)
        LED_14_R_PORT,
        &LED_14_R_PORT->BSRRL,
        &LED_14_R_PORT->BSRRH,
#elif defined (LED_14_R_PORT) && (LED_14_R_ACTIVE == LOW)
        LED_14_R_PORT,
        &LED_14_R_PORT->BSRRH,
        &LED_14_R_PORT->BSRRL,
#else
        NULL,
        NULL,
        NULL,
#endif
#endif        
        LED_14_R_PIN,
        1,
        1,
        bFALSE
    },
    {   //[29]
        LED_ST_ALL_OFF,
        0x0000,
        0x0001,
        0x8000,
#ifdef STM_LED_GPIO
#if defined (LED_14_G_PORT) && (LED_14_G_ACTIVE == HIGH)
        LED_14_G_PORT,
        &LED_14_G_PORT->BSRRL,
        &LED_14_G_PORT->BSRRH,
#elif defined (LED_14_G_PORT) && (LED_14_G_ACTIVE == LOW)
        LED_14_G_PORT,
        &LED_14_G_PORT->BSRRH,
        &LED_14_G_PORT->BSRRL,
#else
        NULL,
        NULL,
        NULL,
#endif     
#endif
        LED_14_G_PIN,
        1,
        1,
        bFALSE
    },
#endif

#if (LED_MAX_DUAL_LED >= 16)
    {   //[30]
        LED_ST_ALL_OFF,
        0x0000,
        0x0001,
        0x8000,
#ifdef STM_LED_GPIO
#if defined (LED_15_R_PORT) && (LED_15_R_ACTIVE == HIGH)
        LED_15_R_PORT,
        &LED_15_R_PORT->BSRRL,
        &LED_15_R_PORT->BSRRH,
#elif defined (LED_15_R_PORT) && (LED_15_R_ACTIVE == LOW)
        LED_15_R_PORT,
        &LED_15_R_PORT->BSRRH,
        &LED_15_R_PORT->BSRRL,
#else
        NULL,
        NULL,
        NULL,
#endif
#endif        
        LED_15_R_PIN,
        1,
        1,
        bFALSE
    },
    {   //[31]
        LED_ST_ALL_OFF,
        0x0000,
        0x0001,
        0x8000,
#ifdef STM_LED_GPIO
#if defined (LED_15_G_PORT) && (LED_15_G_ACTIVE == HIGH)
        LED_15_G_PORT,
        &LED_15_G_PORT->BSRRL,
        &LED_15_G_PORT->BSRRH,
#elif defined (LED_15_G_PORT) && (LED_15_G_ACTIVE == LOW)
        LED_15_G_PORT,
        &LED_15_G_PORT->BSRRH,
        &LED_15_G_PORT->BSRRL,
#else
        NULL,
        NULL,
        NULL,
#endif     
#endif
        LED_15_G_PIN,
        1,
        1,
        bFALSE
    },
#endif
};

//+=============================================================================================
//|        Function:    LED_initLed
//+---------------------------------------------------------------------------------------------
//!        Init led GPIOs.
//+---------------------------------------------------------------------------------------------
//|        Conditions:
//!        \pre    (pre-condition)
//!        \post    (post-condition)
//+---------------------------------------------------------------------------------------------
//|        Annotations:
//+=============================================================================================
INT32U LED_initLed ( void )    //! \return  success of operation
{
    INT16U i16uI_l;
#ifdef STM_LED_GPIO
    KB_GPIO_InitTypeDef GPIO_InitStructure;

    for (i16uI_l = 0; i16uI_l < LED_MAX_DUAL_LED * 2; i16uI_l++)
    {
      if (LED_atLed_g[i16uI_l].ptGPIO)
      {
        GPIO_InitStructure.GPIO_Pin = LED_atLed_g[i16uI_l].i16uLedBit; 
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        kbGPIO_InitCLK(LED_atLed_g[i16uI_l].ptGPIO, &GPIO_InitStructure);
        LED_atLed_g[i16uI_l].enLedState = LED_ST_RED_ON; // make sure LEDs get switched off below
      }
    }
#endif
    
    // switch off all LEDs
    for (i16uI_l = 0; i16uI_l < LED_MAX_DUAL_LED; i16uI_l++)
    {
        LED_setLed (i16uI_l, LED_ST_ALL_OFF);
    }


    return 0;
}

//*************************************************************************************************
//| Function: LED_setLed
//|
//! sets the state of a dual color LED
//!
//! The state is combined for both colors in one argument
//!
//! Attention: do not use within ISR !
//! ingroup. bspApi
//-------------------------------------------------------------------------------------------------
void LED_setLed (
        INT8U       i8uLedNr_p, //!< [in] index of the dual color LED
        LED_EState  enState_p   //!< [in] combined state of the dual color LED
    )        //! \return  success of operation
{
    INT8U   i8uIndex_l = i8uLedNr_p * 2;
#ifdef STM_LED_OUTPUT_FUNC
    INT16U  i16uOldLed_l = LED_i16uLedVariable_g;
#endif


    if ( i8uLedNr_p < LED_MAX_DUAL_LED )
    {
        if	(
                ( LED_atLed_g[i8uIndex_l].enLedState != (enState_p & LED_ST_RED_MASK) ) ||
                ( LED_atLed_g[i8uIndex_l + 1].enLedState != (enState_p & LED_ST_GREEN_MASK) )
            )
        {

            bReadSw_g = bTRUE; // Prevent systick interrupts from modifying the i16uMask while we are in this function.
        
            // 1. Handle red LED
            LED_atLed_g[i8uIndex_l].i16uMask = 1;      // Reset shifter
            LED_atLed_g[i8uIndex_l].i8uPreScale = 1;   // Reset downcounter
            LED_atLed_g[i8uIndex_l].enLedState = enState_p & LED_ST_RED_MASK; // remember new state

#ifdef STM_LED_GPIO
            if (LED_atLed_g[i8uIndex_l].pi16uResetReg)
            {
            }
            else
#endif
            {
                if (LED_atLed_g[i8uIndex_l].i16uLedBit == LED_atLed_g[i8uIndex_l+1].i16uLedBit)
                {
                    // if the red and green LED use the same bit, reset the bit now
                    LED_i16uLedVariable_g &= ~(LED_atLed_g[i8uIndex_l].i16uLedBit); // reset bit
                }
            }

            switch (enState_p & LED_ST_RED_MASK)
            {
                case LED_ST_RED_OFF:
                {
                    LED_atLed_g[i8uIndex_l].bBlink = bFALSE;  // first set to false to disable interrupt handling for this LED
#ifdef STM_LED_GPIO
                    if (LED_atLed_g[i8uIndex_l].pi16uResetReg)
                    {
                        // Reset LED to OFF
                        *LED_atLed_g[i8uIndex_l].pi16uResetReg = LED_atLed_g[i8uIndex_l].i16uLedBit;
                    }
                    else
#endif
                    {
                        if (LED_atLed_g[i8uIndex_l].i16uLedBit != LED_atLed_g[i8uIndex_l+1].i16uLedBit)
                        {
                            LED_i16uLedVariable_g &= ~(LED_atLed_g[i8uIndex_l].i16uLedBit); // reset bit
                        }
                    }
                }   break;
                case LED_ST_RED_ON:
                {
                    LED_atLed_g[i8uIndex_l].bBlink = bFALSE;
#ifdef STM_LED_GPIO
                    if (LED_atLed_g[i8uIndex_l].pi16uSetReg)
                    {
                        // Set LED
                        *LED_atLed_g[i8uIndex_l].pi16uSetReg = LED_atLed_g[i8uIndex_l].i16uLedBit;
                    }
                    else
#endif
                    {
                        LED_i16uLedVariable_g |= (LED_atLed_g[i8uIndex_l].i16uLedBit); // set bit
                    }
                }   break;
                case LED_ST_RED_BLINK_50:
                {
                    // 10Hz Blinker: On(50ms), Off(50ms)
                    LED_atLed_g[i8uIndex_l].i16uCode = 0x5555;     // Toggle On/Off in each step
                    LED_atLed_g[i8uIndex_l].i16uMaxMask = 0xc000;
                    LED_atLed_g[i8uIndex_l].i8uMaxPreScale = 1;    // Stepsize: 50ms (period=100ms)
                    LED_atLed_g[i8uIndex_l].bBlink = bTRUE;
                }   break;
                case LED_ST_RED_BLINK_150:
                {
                    // 3.3Hz Blinker: On(150ms), Off(150ms)
                    LED_atLed_g[i8uIndex_l].i16uCode = 0x5555;     // Alternate On/Off in each step
                    LED_atLed_g[i8uIndex_l].i16uMaxMask = 0xc000;
                    LED_atLed_g[i8uIndex_l].i8uMaxPreScale = 3;    // Stepsize: 3*50ms=150ms (period=300ms)
                    LED_atLed_g[i8uIndex_l].bBlink = bTRUE;
                }   break;
                case LED_ST_RED_BLINK_150_R:
                {
                    // 3.3Hz Blinker: Off(150ms), On(150ms)
                    // On/Off sequence is reversed compared to LED_ST_RED_BLINK_150
                    LED_atLed_g[i8uIndex_l].i16uCode = 0xaaaa;     // Alternate Off/On in each step
                    LED_atLed_g[i8uIndex_l].i16uMaxMask = 0xc000;
                    LED_atLed_g[i8uIndex_l].i8uMaxPreScale = 3;    // Stepsize: 3*50ms=150ms (period=300ms)
                    LED_atLed_g[i8uIndex_l].bBlink = bTRUE;
                }   break;
                case LED_ST_RED_BLINK_200:
                {
                    // 2.5Hz Blinker: On(200ms), Off(200ms)
                    LED_atLed_g[i8uIndex_l].i16uCode = 0x5555;     // Alternate On/Off in each step
                    LED_atLed_g[i8uIndex_l].i16uMaxMask = 0xc000;
                    LED_atLed_g[i8uIndex_l].i8uMaxPreScale = 4;    // Stepsize: 4*50ms=200ms (period=400ms)
                    LED_atLed_g[i8uIndex_l].bBlink = bTRUE;
                }   break;
                case LED_ST_RED_BLINK_200_R:
                {
                    // 2.5Hz Blinker: Off(200ms), On(200ms)
                    LED_atLed_g[i8uIndex_l].i16uCode = 0xAAAA;     // Alternate On/Off in each step
                    LED_atLed_g[i8uIndex_l].i16uMaxMask = 0xc000;
                    LED_atLed_g[i8uIndex_l].i8uMaxPreScale = 4;    // Stepsize: 4*50ms=200ms (period=400ms)
                    LED_atLed_g[i8uIndex_l].bBlink = bTRUE;
                }   break;
                case LED_ST_RED_BLINK_250:
                {
                    // 2Hz Blinker: On(250ms), Off(250ms)
                    LED_atLed_g[i8uIndex_l].i16uCode = 0x5555;
                    LED_atLed_g[i8uIndex_l].i16uMaxMask = 0xc000;
                    LED_atLed_g[i8uIndex_l].i8uMaxPreScale = 5;
                    LED_atLed_g[i8uIndex_l].bBlink = bTRUE;
                }   break;
                case LED_ST_RED_BLINK_250_R:
                {
                    // 2Hz Blinker: Off(250ms), On(250ms)
                    LED_atLed_g[i8uIndex_l].i16uCode = 0xaaaa;
                    LED_atLed_g[i8uIndex_l].i16uMaxMask = 0xc000;
                    LED_atLed_g[i8uIndex_l].i8uMaxPreScale = 5;
                    LED_atLed_g[i8uIndex_l].bBlink = bTRUE;
                }   break;
                case LED_ST_RED_BLINK_500:
                {
                    // 1Hz Blinker: On(500ms), Off(500ms)
                    LED_atLed_g[i8uIndex_l].i16uCode = 0x5555;      // Alternate On/Off in each step
                    LED_atLed_g[i8uIndex_l].i16uMaxMask = 0xc000;
                    LED_atLed_g[i8uIndex_l].i8uMaxPreScale = 10;    // Stepsize: 10*50ms=500ms (period=1s)
                    LED_atLed_g[i8uIndex_l].bBlink = bTRUE;
                }   break;
                case LED_ST_RED_BLINK_500_R:
                {
                    // 1Hz Blinker: Off(500ms), On(500ms)
                    LED_atLed_g[i8uIndex_l].i16uCode = 0xaaaa;      // Alternate On/Off in each step
                    LED_atLed_g[i8uIndex_l].i16uMaxMask = 0xc000;
                    LED_atLed_g[i8uIndex_l].i8uMaxPreScale = 10;    // Stepsize: 10*50ms=500ms (period=1s)
                    LED_atLed_g[i8uIndex_l].bBlink = bTRUE;
                }   break;
                case LED_ST_RED_1_FLASH:
                {
                    // On: 200ms Off: 1s
                    LED_atLed_g[i8uIndex_l].i16uCode = 0x0001;
                    LED_atLed_g[i8uIndex_l].i16uMaxMask = 0xffc0;
                    LED_atLed_g[i8uIndex_l].i8uMaxPreScale = 4;     // Stepsize: 4*50ms=200ms
                    LED_atLed_g[i8uIndex_l].bBlink = bTRUE;
                }   break;
                case LED_ST_RED_1_FLASH_R:
                {
                    // Off (200mS), On (200ms), Off (800ms)
                    LED_atLed_g[i8uIndex_l].i16uCode = 0x0002;
                    LED_atLed_g[i8uIndex_l].i16uMaxMask = 0xffc0;
                    LED_atLed_g[i8uIndex_l].i8uMaxPreScale = 4;     // Stepsize: 4*50ms=200ms
                    LED_atLed_g[i8uIndex_l].bBlink = bTRUE;
                }   break;
                case LED_ST_RED_2_FLASH:
                {
                    // On(200ms),Off(200ms),On(200ms),Off(1s)
                    LED_atLed_g[i8uIndex_l].i16uCode = 0x0005;
                    LED_atLed_g[i8uIndex_l].i16uMaxMask = 0xff00;
                    LED_atLed_g[i8uIndex_l].i8uMaxPreScale = 4;     // Stepsize: 4*50ms=200ms
                    LED_atLed_g[i8uIndex_l].bBlink = bTRUE;
                }   break;
                case LED_ST_RED_2_FLASH_R:
                {
                    // Off (200ms), On(200ms), Off(200ms), On(200ms), Off(800ms)
                    LED_atLed_g[i8uIndex_l].i16uCode = 0x000a;
                    LED_atLed_g[i8uIndex_l].i16uMaxMask = 0xff00;
                    LED_atLed_g[i8uIndex_l].i8uMaxPreScale = 4;     // Stepsize: 4*50ms=200ms
                    LED_atLed_g[i8uIndex_l].bBlink = bTRUE;
                }   break;
                case LED_ST_RED_3_FLASH:
                {
                    // On(200ms),Off(200ms),On(200ms),Off(200ms),On(200ms),Off(1s)
                    LED_atLed_g[i8uIndex_l].i16uCode = 0x0015;
                    LED_atLed_g[i8uIndex_l].i16uMaxMask = 0xfc00;
                    LED_atLed_g[i8uIndex_l].i8uMaxPreScale = 4;      // Stepsize: 4*50ms=200ms
                    LED_atLed_g[i8uIndex_l].bBlink = bTRUE;
                }   break;
                case LED_ST_RED_3_FLASH_R:
                {
                    // Off(200ms),On(200ms),Off(200ms),On(200ms),Off(200ms),On(200ms),Off(800ms)
                    LED_atLed_g[i8uIndex_l].i16uCode = 0x002a;
                    LED_atLed_g[i8uIndex_l].i16uMaxMask = 0xfc00;
                    LED_atLed_g[i8uIndex_l].i8uMaxPreScale = 4;      // Stepsize: 4*50ms=200ms
                    LED_atLed_g[i8uIndex_l].bBlink = bTRUE;
                }   break;
                case LED_ST_RED_4_FLASH:
                {
                    // On(200ms),Off(200ms),On(200ms),Off(200ms),On(200ms),Off(200ms),On(200ms),Off(1s)
                    LED_atLed_g[i8uIndex_l].i16uCode = 0x0055;
                    LED_atLed_g[i8uIndex_l].i16uMaxMask = 0xf000;
                    LED_atLed_g[i8uIndex_l].i8uMaxPreScale = 4;
                    LED_atLed_g[i8uIndex_l].bBlink = bTRUE;
                }   break;
                case LED_ST_RED_4_FLASH_R:
                {
                    // Off(200ms),On(200ms),Off(200ms),On(200ms),Off(200ms),On(200ms),Off(200ms),On(200ms),Off(800ms)
                    LED_atLed_g[i8uIndex_l].i16uCode = 0x00aa;
                    LED_atLed_g[i8uIndex_l].i16uMaxMask = 0xf000;
                    LED_atLed_g[i8uIndex_l].i8uMaxPreScale = 4;
                    LED_atLed_g[i8uIndex_l].bBlink = bTRUE;
                }   break;
                case LED_ST_RED_S3_CP1:
                {
                    // Off: 350ms On: 2800ms
                    LED_atLed_g[i8uIndex_l].i16uCode    = 0xFFFE;
                    LED_atLed_g[i8uIndex_l].i16uMaxMask = 0xFE00;
                    LED_atLed_g[i8uIndex_l].i8uMaxPreScale = 7;     // Stepsize: 7*50ms=350ms
                    LED_atLed_g[i8uIndex_l].bBlink = bTRUE;
                }   break;
                case LED_ST_RED_S3_HP1:
                {
                    // On: 250ms Off: 1750ms
                    LED_atLed_g[i8uIndex_l].i16uCode    = 0x0001;
                    LED_atLed_g[i8uIndex_l].i16uMaxMask = 0xff00;
                    LED_atLed_g[i8uIndex_l].i8uMaxPreScale = 5;     // Stepsize: 5*50ms=250ms
                    LED_atLed_g[i8uIndex_l].bBlink = bTRUE;
                }   break;
                case LED_ST_RED_S3_CP2:
                {
                    // Off: 350ms On: 350ms Off: 350ms On: 2100ms
                    LED_atLed_g[i8uIndex_l].i16uCode    = 0xFFFA;
                    LED_atLed_g[i8uIndex_l].i16uMaxMask = 0xFE00;
                    LED_atLed_g[i8uIndex_l].i8uMaxPreScale = 7;     // Stepsize: 7*50ms=350ms
                    LED_atLed_g[i8uIndex_l].bBlink = bTRUE;
                }   break;
                case LED_ST_RED_S3_HP2:
                {
                    // On: 250ms Off: 250ms On: 250ms Off: 1250ms
                    LED_atLed_g[i8uIndex_l].i16uCode    = 0x0005;
                    LED_atLed_g[i8uIndex_l].i16uMaxMask = 0xff00;
                    LED_atLed_g[i8uIndex_l].i8uMaxPreScale = 5;     // Stepsize: 5*50ms=250ms
                    LED_atLed_g[i8uIndex_l].bBlink = bTRUE;
                }   break;
                case LED_ST_RED_S3_CP3:
                {
                    // Off: 350ms On: 350ms Off: 350ms On: 350ms Off: 350ms On: 1400ms
                    LED_atLed_g[i8uIndex_l].i16uCode    = 0xFFEA;
                    LED_atLed_g[i8uIndex_l].i16uMaxMask = 0xFE00;
                    LED_atLed_g[i8uIndex_l].i8uMaxPreScale = 7;     // Stepsize: 7*50ms=350ms
                    LED_atLed_g[i8uIndex_l].bBlink = bTRUE;
                }   break;
                default:
                {
                    LED_atLed_g[i8uIndex_l].bBlink = bFALSE;
                    bspError( BSPE_LED_INVALID_STATE, bTRUE, 2, i8uLedNr_p, (enState_p & LED_ST_RED_MASK) >> 8);
                }   break;
            }

            // 2. Handle green LED
            i8uIndex_l++;

            LED_atLed_g[i8uIndex_l].i16uMask = 1;
            LED_atLed_g[i8uIndex_l].i8uPreScale = 1;
            LED_atLed_g[i8uIndex_l].enLedState = enState_p & LED_ST_GREEN_MASK;

            switch (enState_p & LED_ST_GREEN_MASK)
            {
                case LED_ST_GREEN_OFF:
                {
                    LED_atLed_g[i8uIndex_l].bBlink = bFALSE;
#ifdef STM_LED_GPIO
                    if (LED_atLed_g[i8uIndex_l].pi16uResetReg)
                    {
                        *LED_atLed_g[i8uIndex_l].pi16uResetReg = LED_atLed_g[i8uIndex_l].i16uLedBit;
                    }
                    else
#endif
                    {
                        if (LED_atLed_g[i8uIndex_l].i16uLedBit != LED_atLed_g[i8uIndex_l-1].i16uLedBit)
                        {
                            LED_i16uLedVariable_g &= ~(LED_atLed_g[i8uIndex_l].i16uLedBit); // reset bit
                        }
                    }
                }   break;
                
                case LED_ST_GREEN_ON:
                {
                    LED_atLed_g[i8uIndex_l].bBlink = bFALSE;
#ifdef STM_LED_GPIO
                    if (LED_atLed_g[i8uIndex_l].pi16uSetReg)
                    {
                        *LED_atLed_g[i8uIndex_l].pi16uSetReg = LED_atLed_g[i8uIndex_l].i16uLedBit;
                    }
                    else
#endif
                    {
                        LED_i16uLedVariable_g |= (LED_atLed_g[i8uIndex_l].i16uLedBit); // set bit
                    }
                }   break;
                
                case LED_ST_GREEN_BLINK_50:
                {
                    // 10Hz Blinker: On(50ms), Off(50ms)
                    LED_atLed_g[i8uIndex_l].i16uCode = 0x5555;
                    LED_atLed_g[i8uIndex_l].i16uMaxMask = 0xc000;
                    LED_atLed_g[i8uIndex_l].i8uMaxPreScale = 1;
                    LED_atLed_g[i8uIndex_l].bBlink = bTRUE;
                }   break;
                
                case LED_ST_GREEN_BLINK_150:
                {
                    // 3.3Hz Blinker: On(150ms), Off(150ms)
                    LED_atLed_g[i8uIndex_l].i16uCode = 0x5555;     // Alternate On/Off in each step
                    LED_atLed_g[i8uIndex_l].i16uMaxMask = 0xc000;
                    LED_atLed_g[i8uIndex_l].i8uMaxPreScale = 3;    // Stepsize: 3*50ms=150ms (period=300ms)
                    LED_atLed_g[i8uIndex_l].bBlink = bTRUE;
                }   break;
                
                case LED_ST_GREEN_BLINK_150_R:
                {
                    // 3.3Hz Blinker: Off(150ms), On(150ms)
                    // On/Off sequence is reversed compared to LED_ST_GREEN_BLINK_150
                    // to yield same OnOff sequence as in LED_ST_RED_BLINK_150 (0x5555)
                    // LED_ST_RED_BLINK_150 | LED_ST_GREEN_BLINK_150_R => LED_ST_YELLOW_BLINK_150
                    LED_atLed_g[i8uIndex_l].i16uCode = 0xaaaa;     // Alternate Off/On in each step
                    LED_atLed_g[i8uIndex_l].i16uMaxMask = 0xc000;
                    LED_atLed_g[i8uIndex_l].i8uMaxPreScale = 3;    // Stepsize: 3*50ms=150ms (period=300ms)
                    LED_atLed_g[i8uIndex_l].bBlink = bTRUE;
                }   break;

                case LED_ST_GREEN_BLINK_200:
                {
                    // 2.5Hz Blinker: On(200ms), Off(200ms)
                    LED_atLed_g[i8uIndex_l].i16uCode = 0x5555;
                    LED_atLed_g[i8uIndex_l].i16uMaxMask = 0xc000;
                    LED_atLed_g[i8uIndex_l].i8uMaxPreScale = 4;
                    LED_atLed_g[i8uIndex_l].bBlink = bTRUE;
                }   break;
                
                case LED_ST_GREEN_BLINK_200_R:
                {
                    // 2.5Hz Blinker: On(200ms), Off(200ms)
                    LED_atLed_g[i8uIndex_l].i16uCode = 0xAAAA;
                    LED_atLed_g[i8uIndex_l].i16uMaxMask = 0xc000;
                    LED_atLed_g[i8uIndex_l].i8uMaxPreScale = 4;
                    LED_atLed_g[i8uIndex_l].bBlink = bTRUE;
                }   break;
                
                case LED_ST_GREEN_BLINK_250:
                {
                    // 2Hz Blinker: On(250ms), Off(250ms)
                    LED_atLed_g[i8uIndex_l].i16uCode = 0x5555;
                    LED_atLed_g[i8uIndex_l].i16uMaxMask = 0xc000;
                    LED_atLed_g[i8uIndex_l].i8uMaxPreScale = 5;
                    LED_atLed_g[i8uIndex_l].bBlink = bTRUE;
                }   break;
                
                case LED_ST_GREEN_BLINK_250_R:
                {
                    // 2Hz Blinker: Off(250ms), On(250ms)
                    LED_atLed_g[i8uIndex_l].i16uCode = 0xaaaa;
                    LED_atLed_g[i8uIndex_l].i16uMaxMask = 0xc000;
                    LED_atLed_g[i8uIndex_l].i8uMaxPreScale = 5;
                    LED_atLed_g[i8uIndex_l].bBlink = bTRUE;
                }   break;
                
                case LED_ST_GREEN_BLINK_500:
                {
                    // 1Hz Blinker: On(500ms), Off(500ms)
                    LED_atLed_g[i8uIndex_l].i16uCode = 0x5555;
                    LED_atLed_g[i8uIndex_l].i16uMaxMask = 0xc000;
                    LED_atLed_g[i8uIndex_l].i8uMaxPreScale = 10;
                    LED_atLed_g[i8uIndex_l].bBlink = bTRUE;
                }   break;
                
                case LED_ST_GREEN_BLINK_500_R:
                {
                    // 1Hz Blinker: Off(500ms), On(500ms)
                    LED_atLed_g[i8uIndex_l].i16uCode = 0xaaaa;
                    LED_atLed_g[i8uIndex_l].i16uMaxMask = 0xc000;
                    LED_atLed_g[i8uIndex_l].i8uMaxPreScale = 10;
                    LED_atLed_g[i8uIndex_l].bBlink = bTRUE;
                }   break;
                case LED_ST_GREEN_1_FLASH:
                {
                    // On: 200ms Off: 1s
                    LED_atLed_g[i8uIndex_l].i16uCode = 0x0001;
                    LED_atLed_g[i8uIndex_l].i16uMaxMask = 0xffc0;
                    LED_atLed_g[i8uIndex_l].i8uMaxPreScale = 4;     // Stepsize: 4*50ms=200ms
                    LED_atLed_g[i8uIndex_l].bBlink = bTRUE;
                }   break;
                case LED_ST_GREEN_1_FLASH_R:
                {
                    // Off (200mS), On (200ms), Off (800ms)
                    LED_atLed_g[i8uIndex_l].i16uCode = 0x0002;
                    LED_atLed_g[i8uIndex_l].i16uMaxMask = 0xffc0;
                    LED_atLed_g[i8uIndex_l].i8uMaxPreScale = 4;     // Stepsize: 4*50ms=200ms
                    LED_atLed_g[i8uIndex_l].bBlink = bTRUE;
                }   break;
                case LED_ST_GREEN_2_FLASH:
                {
                    // On(200ms),Off(200ms),On(200ms),Off(1s)
                    LED_atLed_g[i8uIndex_l].i16uCode = 0x0005;
                    LED_atLed_g[i8uIndex_l].i16uMaxMask = 0xff00;
                    LED_atLed_g[i8uIndex_l].i8uMaxPreScale = 4;     // Stepsize: 4*50ms=200ms
                    LED_atLed_g[i8uIndex_l].bBlink = bTRUE;
                }   break;
                case LED_ST_GREEN_2_FLASH_R:
                {
                    // Off (200ms), On(200ms), Off(200ms), On(200ms), Off(800ms)
                    LED_atLed_g[i8uIndex_l].i16uCode = 0x000a;
                    LED_atLed_g[i8uIndex_l].i16uMaxMask = 0xff00;
                    LED_atLed_g[i8uIndex_l].i8uMaxPreScale = 4;     // Stepsize: 4*50ms=200ms
                    LED_atLed_g[i8uIndex_l].bBlink = bTRUE;
                }   break;
                case LED_ST_GREEN_3_FLASH:
                {
                    // On(200ms),Off(200ms),On(200ms),Off(200ms),On(200ms),Off(1s)
                    LED_atLed_g[i8uIndex_l].i16uCode = 0x0015;
                    LED_atLed_g[i8uIndex_l].i16uMaxMask = 0xfc00;
                    LED_atLed_g[i8uIndex_l].i8uMaxPreScale = 4;      // Stepsize: 4*50ms=200ms
                    LED_atLed_g[i8uIndex_l].bBlink = bTRUE;
                }   break;
                case LED_ST_GREEN_3_FLASH_R:
                {
                    // Off(200ms),On(200ms),Off(200ms),On(200ms),Off(200ms),On(200ms),Off(800ms)
                    LED_atLed_g[i8uIndex_l].i16uCode = 0x002a;
                    LED_atLed_g[i8uIndex_l].i16uMaxMask = 0xfc00;
                    LED_atLed_g[i8uIndex_l].i8uMaxPreScale = 4;      // Stepsize: 4*50ms=200ms
                    LED_atLed_g[i8uIndex_l].bBlink = bTRUE;
                }   break;
                case LED_ST_GREEN_4_FLASH:
                {
                    // On(200ms),Off(200ms),On(200ms),Off(200ms),On(200ms),Off(200ms),On(200ms),Off(1s)
                    LED_atLed_g[i8uIndex_l].i16uCode = 0x0055;
                    LED_atLed_g[i8uIndex_l].i16uMaxMask = 0xf000;
                    LED_atLed_g[i8uIndex_l].i8uMaxPreScale = 4;
                    LED_atLed_g[i8uIndex_l].bBlink = bTRUE;
                }   break;
                case LED_ST_GREEN_4_FLASH_R:
                {
                    // Off(200ms),On(200ms),Off(200ms),On(200ms),Off(200ms),On(200ms),Off(200ms),On(200ms),Off(800ms)
                    LED_atLed_g[i8uIndex_l].i16uCode = 0x00aa;
                    LED_atLed_g[i8uIndex_l].i16uMaxMask = 0xf000;
                    LED_atLed_g[i8uIndex_l].i8uMaxPreScale = 4;
                    LED_atLed_g[i8uIndex_l].bBlink = bTRUE;
                }   break;
                default:
                {
                    LED_atLed_g[i8uIndex_l].bBlink = bFALSE;
                    bspError( BSPE_LED_INVALID_STATE, bTRUE, 2, i8uLedNr_p, enState_p & LED_ST_GREEN_MASK);
                }   break;
            }

            // Allow systick interrupt to handle LED states again.
            bReadSw_g = bFALSE;
        }
        else
        {
            //Led State already set ...
        }
    }
    else
    {
        bspError( BSPE_LED_SET_OUT_OF_RANGE, bTRUE, 1, i8uLedNr_p );
    }

#ifdef STM_LED_OUTPUT_FUNC
    if (i16uOldLed_l != LED_i16uLedVariable_g)
    {
        LED_output(LED_i16uLedVariable_g);
    }
#endif

}


//*************************************************************************************************
//| Function: getLedState
//|
//! reads out the set value of the LED
//!
//! gives back the set value as a LED_EState value. Red and green LED state are combined in one
//! value.
//!
//!
//! ingroup. bspApi
//-------------------------------------------------------------------------------------------------
LED_EState getLedState (
    INT8U i8uLedNr_p        //!< [in] index of LED to ask for
    )    //! \return actual LED state

{
    LED_EState enRv_l = LED_ST_ALL_OFF;

    INT8U i8uIndex_l = i8uLedNr_p * 2;

    if (i8uLedNr_p <= LED_MAX_DUAL_LED - 1)
    {   // combine state of red and green LED
        // (there can only be one !)
        enRv_l = LED_atLed_g[i8uIndex_l].enLedState | LED_atLed_g[i8uIndex_l + 1].enLedState;
    }
    else
    {
        bspError( BSPE_LED_GET_OUT_OF_RANGE, bTRUE, 2, LED_MAX_DUAL_LED, i8uLedNr_p );
    }

    return (enRv_l);

}

//+=============================================================================================
//|		Function:	LED_modifyLed
//+---------------------------------------------------------------------------------------------
//!		Modifies led behaviour to provided value.
//!     Does not overwrite unspecified 2nd led, if no new value is provided ....
//+---------------------------------------------------------------------------------------------
//|		Conditions:
//!		\pre	(pre-condition)
//!		\post	(post-condition)
//+---------------------------------------------------------------------------------------------
//|		Annotations:
//+=============================================================================================
void LED_modifyLed(
        INT8U       i8uLedNr_p,		//!< [in] index of the dual color LED
        LED_EState	enLedMask_p,	//!< [in] type of the dual color LED
        LED_EState  enState_p		//!< [in] combined state of the dual color LED 
    )
{
    LED_EState  enState_l;
    
    if(LED_ST_RED_MASK & enLedMask_p)
    {
        //get current LED states ...
        enState_l = getLedState( i8uLedNr_p );
        
        //Keep green LED settings ...
        enState_l &= LED_ST_GREEN_MASK;
        
        //Overwrite red LED settings ...
        enState_l |= enState_p;
        
        //apply settings ...
        LED_setLed( i8uLedNr_p, enState_l);

    }
    else if(LED_ST_GREEN_MASK & enLedMask_p)
    {
        //get current LED states ...
        enState_l = getLedState( i8uLedNr_p );
        
        //Keep red LED settings ...
        enState_l &= LED_ST_RED_MASK;
        
        //Overwrite green LED settings ...
        enState_l |= enState_p;
        
        //apply settings ...
        LED_setLed( i8uLedNr_p, enState_l);
    }
    else //RED+GREEN
    {
        //overwrite both of them ...
        LED_setLed(i8uLedNr_p, enState_p);	
    }
}

//*************************************************************************************************

//+=============================================================================================
//|		Function:	LED_systick
//+---------------------------------------------------------------------------------------------
//!		
//!     
//+---------------------------------------------------------------------------------------------
//|		Conditions:
//!		\pre	(pre-condition)
//!		\post	(post-condition)
//+---------------------------------------------------------------------------------------------
//|		Annotations:
//+=============================================================================================
static INT32U i32uLedClk_s = 0;


void LED_systick()
{
    INT8U i8uInd_l;
    LED_TLed *ptLed_l;

    i32uLedClk_s++;

    if (i32uLedClk_s >= 50)
    {   // Do only every 50ms something
#ifdef STM_LED_OUTPUT_FUNC
        INT16U i16uOldLed_l = LED_i16uLedVariable_g;
#endif
        ptLed_l = &LED_atLed_g[0];

        if (bReadSw_g == bFALSE)
        {
            for (i8uInd_l = 0; i8uInd_l < LED_MAX_DUAL_LED * 2; i8uInd_l++, ptLed_l++)
            {
                if (ptLed_l->bBlink == bTRUE)
                {   // only do something, if LED should alternate its state
                    ptLed_l->i8uPreScale--;
                    if (!ptLed_l->i8uPreScale)
                    {
                        ptLed_l->i8uPreScale = ptLed_l->i8uMaxPreScale;
                        if (ptLed_l->i16uCode & ptLed_l->i16uMask)
                        {
#ifdef STM_LED_GPIO
                            if (ptLed_l->pi16uSetReg)
                            {
                                *ptLed_l->pi16uSetReg = ptLed_l->i16uLedBit;
                            }
                            else
#endif
                            {
                                LED_i16uLedVariable_g |= ptLed_l->i16uLedBit;
                            }    
                        }
                        else
                        {
#ifdef STM_LED_GPIO
                            if (ptLed_l->pi16uResetReg)
                            {
                                *ptLed_l->pi16uResetReg = ptLed_l->i16uLedBit;
                            }
                            else
#endif
                            {
                                LED_i16uLedVariable_g &= ~(ptLed_l->i16uLedBit);
                            }
                        }

                        ptLed_l->i16uMask <<= 1;

                        if (ptLed_l->i16uMask & ptLed_l->i16uMaxMask)
                        {
                            ptLed_l->i16uMask = 1;
                        }
                    }    
                }
            }
            
            // reset counter for 50ms
            i32uLedClk_s -= 50;
        }

#ifdef STM_LED_OUTPUT_FUNC
        if (i16uOldLed_l != LED_i16uLedVariable_g)
        {
            LED_output(LED_i16uLedVariable_g);
        }
#endif
    }  // if 50ms
}




