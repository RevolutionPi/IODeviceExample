/*=============================================================================================
*
* led.h
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
* There are 3 ways to use the new LED driver:
* 
* with GPIO
*     - the LEDs are connected to GPIOs directly
*     - the defines STM_WITH_LED and STM_LED_GPIO must be defined in project.h
*     - the define LED_MAX_DUAL_LED must be set to number of LEDs
*     - defines LED_XX_R_PIN, LED_XX_R_PORT and LED_XX_R_ACTIVE must be set in project.h for each LED
*
* with SSC
*     - the LEDs are connected to SSC output
*     - the defines STM_WITH_LED and STM_LED_SSC  must be defined in project.h
*     - the define LED_MAX_DUAL_LED must be set to number of LEDs
*     - by default the output pins correlate to the LED number directly. e.g. LED_setLed(2, LED_ST_RED_ON) 
*       sets bit 2 of the output byte to 1. Red and green are not distinguished, LED_setLed(2, LED_ST_RED_ON) 
*       and LED_setLed(2, LED_ST_GREEN_ON) set the same bit.
*     - it is also possible to use 2 output bits for one LED. Then the defines LED_XX_R_PIN and LED_XX_G_PIN
*       must be defined in project.h. If one of these defines is set in project.h it will be necessary to set
*       all defines mostly.
*     example:
*       #define LED_0_R_PIN     0x0002
*       #define LED_0_G_PIN     0x0001
*       #define LED_1_R_PIN     0x0008
*       #define LED_1_G_PIN     0x0004
*       #define LED_2_R_PIN     0x0010
*       #define LED_2_G_PIN     0x0010
*       - LED 0 and 1 are dual color LEDs, 2 has only one color
*       - pin 0 is connected to the green part of LED 0, pin 1 is the red part of LED 0
*       - pin 2 is connected to the green part of LED 1, pin 3 is the red part of LED 1
*       - pin 5 is connected to LED 2, not caring about the color
* 
* something else
*     - the LEDs are in another way to the processor
*     - the define STM_WITH_LED must be defined in project.h
*     - the define LED_MAX_DUAL_LED must be set to number of LEDs
*     - a function void LED_output(INT16U LED_i16uLedVariable_g); must be written to set the LEDs
*     - by default only one color LEDs are supported with the fixed correlation of LED n to Bit n
*       If another mapping has to be used, the defines LED_XX_R_PIN and LED_XX_G_PIN
*       must be defined in project.h as described above.
*/

#ifndef LED_H_INC
#define LED_H_INC

#include <project.h>

//+=============================================================================================
//|		Konstanten / constants
//+=============================================================================================

#define LED_ST_RED_MASK    0xff00
#define LED_ST_GREEN_MASK  0x00ff

//+=============================================================================================
//|     Typen / types
//+=============================================================================================

/// possible states of a LED
typedef enum _LED_EState
{
    //for all leds
    LED_ST_ALL_OFF            = 0x0000,

    // red led's states
    LED_ST_RED_OFF            = 0x0000,
    LED_ST_RED_ON             = 0x0100,  
    LED_ST_RED_BLINK_50       = 0x0200,  // 10Hz
    LED_ST_RED_BLINK_50_R     = 0x0300,  // 10Hz
    LED_ST_RED_BLINK_150      = 0x0400,  // 3.3 Hz
    LED_ST_RED_BLINK_150_R    = 0x0500,  // 3.3 Hz
    LED_ST_RED_BLINK_200      = 0x0600,  // 2.5 Hz
    LED_ST_RED_BLINK_200_R    = 0x0700,  // 2.5 Hz, Reverse
    LED_ST_RED_BLINK_250      = 0x0800,  // 2 Hz
    LED_ST_RED_BLINK_250_R    = 0x0900,  // 2 Hz
    LED_ST_RED_BLINK_500      = 0x0A00,  // 1 Hz
    LED_ST_RED_BLINK_500_R    = 0x0B00,  // 1 Hz
    LED_ST_RED_1_FLASH        = 0x0C00,  // 200ms on, 1000ms off
    LED_ST_RED_1_FLASH_R      = 0x0D00,  // 200ms Off, 200ms On, 800ms Off
    LED_ST_RED_2_FLASH        = 0x0E00,  // 200ms on, 200ms off, 200ms on, 1000ms off
    LED_ST_RED_2_FLASH_R      = 0x0F00,  // 200ms off, 200ms on, 200ms off, 200ms on, 800ms off
    LED_ST_RED_3_FLASH        = 0x1000,
    LED_ST_RED_3_FLASH_R      = 0x1100,
    LED_ST_RED_4_FLASH        = 0x1200,
    LED_ST_RED_4_FLASH_R      = 0x1300,
    LED_ST_RED_S3_CP1         = 0x1400,
    LED_ST_RED_S3_CP2         = 0x1500,
    LED_ST_RED_S3_CP3         = 0x1600,
    LED_ST_RED_S3_HP1         = 0x1700,
    LED_ST_RED_S3_HP2         = 0x1800,
#if defined REGTEST_PRODUCT && REGTEST_PRODUCT == KUNBUS_FW_DESCR_TYP_MG_SERCOS3
    LED_ST_RED_S3_HP0         = 0x1900,
#endif
    
    // green led's states
    LED_ST_GREEN_OFF          = 0x0000,
    LED_ST_GREEN_ON           = 0x0001,
    LED_ST_GREEN_BLINK_50     = 0x0002,
    LED_ST_GREEN_BLINK_50_R   = 0x0003,
    LED_ST_GREEN_BLINK_150    = 0x0004,
    LED_ST_GREEN_BLINK_150_R  = 0x0005,
    LED_ST_GREEN_BLINK_200    = 0x0006,
    LED_ST_GREEN_BLINK_200_R  = 0x0007, // 2.5 Hz, Reverse
    LED_ST_GREEN_BLINK_250    = 0x0008, // 2 Hz
    LED_ST_GREEN_BLINK_250_R  = 0x0009, // 2 Hz
    LED_ST_GREEN_BLINK_500    = 0x000A,
    LED_ST_GREEN_BLINK_500_R  = 0x000B,
    LED_ST_GREEN_1_FLASH      = 0x000C,
    LED_ST_GREEN_1_FLASH_R    = 0x000D, // 200ms Off, 200ms On, 800ms Off
    LED_ST_GREEN_2_FLASH      = 0x000E,
    LED_ST_GREEN_2_FLASH_R    = 0x000F, // 200ms off, 200ms on, 200ms off, 200ms on, 800ms off
    LED_ST_GREEN_3_FLASH      = 0x0010,
    LED_ST_GREEN_3_FLASH_R    = 0x0011,
    LED_ST_GREEN_4_FLASH      = 0x0012,
    LED_ST_GREEN_4_FLASH_R    = 0x0013,
#if defined REGTEST_PRODUCT && REGTEST_PRODUCT == KUNBUS_FW_DESCR_TYP_MG_SERCOS3
    // maybe only needed for compatibility ("green" is always on in these states)
    LED_ST_GREEN_S3_CP1       = 0x0014,
    LED_ST_GREEN_S3_CP2       = 0x0015,
    LED_ST_GREEN_S3_CP3       = 0x0016,
    LED_ST_GREEN_S3_HP1       = 0x0017,
    LED_ST_GREEN_S3_HP2       = 0x0018,
    LED_ST_GREEN_S3_HP0       = 0x0019,
#endif
                                   
}  LED_EState;                     
                                   
//+=============================================================================================
//|     Globale Variablen / global v18ariables
//+=============================================================================================
extern INT16U		LED_i16uLedVariable_g;
extern TBOOL bReadSw_g;

//+=============================================================================================
//|		Prototypen / prototypes
//+=============================================================================================

#ifdef __cplusplus
extern "C" { 
#endif 

INT32U      LED_initLed		( void );
void        LED_setLed		( INT8U i8uLedNr_p, LED_EState enState_p );
LED_EState  getLedState		( INT8U i8uLedNr_p );
void        LED_modifyLed	( INT8U i8uLedNr_p, LED_EState i8uLedMask_p, LED_EState enState_p );
void        LED_systick     ( void );

// if STM_LED_OUTPUT_FUNC is defined, this callback function must be implemented in the project to set the LEDs
void        LED_output(INT16U i16uLedBits);

#ifdef __cplusplus
}    
#endif 

#endif  // LED_H_INC
