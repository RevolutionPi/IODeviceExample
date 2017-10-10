/*=============================================================================================
*
* clock.h
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
* Initialization and configuration functions for STM32 system clocks.
*
*/
#ifndef _CLOCK_H_
#define	_CLOCK_H_

#ifdef __cplusplus
extern "C" {
#endif

//+=============================================================================================
//|		Include-Dateien / include files
//+=============================================================================================
#if defined (STM32F30X)
  #include <SysLib/inc/stm32f30x_rcc.h>
#elif defined (STM32F2XX)
  #include <SysLib/inc/stm32f2xx_rcc.h>
#elif defined (STM32F40_41xxx) || defined (STM32F427_437xx) || defined (STM32F429_439xx) || defined (STM32F401xx)
  #include <SysLib/inc/stm32f4xx_rcc.h>
#elif defined (STM32F10X_CL) || defined (STM32F10X_HD) || defined (STM32F10X_MD) || defined (STM32F10X_LD)
  #include <SysLib/inc/stm32f10x_rcc.h>
#endif

//+=============================================================================================
//|		Konstanten / constants
//+=============================================================================================

//#define

//+=============================================================================================
//|		Typen / types
//+=============================================================================================

//+=============================================================================================
//|		Makros / macros
//+=============================================================================================

//#define

//+=============================================================================================
//|		Globale Variablen / global variables
//+=============================================================================================

// INT8U ...

//+=============================================================================================
//|		Prototypen / prototypes
//+=============================================================================================

///	\defgroup	kb_clock_handlers   kb_clock_handlers
/// @{
extern INT32U   Clk_Init (void);	///< initialize clock (brief description)


#if defined (STM32F30X) || defined (STM32F2XX) || defined (STM32F40_41xxx) \
 || defined (STM32F427_437xx) || defined (STM32F429_439xx) || defined (STM32F401xx) \
 || defined (STM32F10X_CL) || defined (STM32F10X_HD) || defined (STM32F10X_MD) || defined (STM32F10X_LD)
    extern void BSP_RCC_GetClocksFreq(RCC_ClocksTypeDef* RCC_Clocks);
#endif


/// @}


#ifdef __cplusplus
}
#endif

//+=============================================================================================
//|		Aenderungsjournal
//+=============================================================================================
#ifdef __DOCGEN
/*!
@page revisions Revisions

*/
#endif


#endif//_CLOCK_H_
