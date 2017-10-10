/*=============================================================================================
*
* common_include.h
*
* Common include for all u-Controllers.
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
#ifndef _COMMON_INCLUDE_H_
#define	_COMMON_INCLUDE_H_

#ifdef __cplusplus
extern "C" {
#endif

//+=============================================================================================
//|		u-Controller Include-Dateien / u-controller include files
//+=============================================================================================

//----- PIC32 MX 360 F 512 L -----
#if defined(__32MX360F512L__)
#define _PIC32_U_CONTROLLER_
#include <plib.h>
#include <bsp/kbPic32mxmap.h>

//----- PIC32 MX GENERIC -----
#elif defined(__32MXGENERIC__)
#define _PIC32_U_CONTROLLER_
#include <plib.h>

#elif defined(__STM32GENERIC__)
#ifdef STM32F2XX
#include "SysLib/inc/stm32f2xx.h"
#elif defined (STM32F30X)
#include "stm32f30x.h"
#elif defined (STM32F40_41xxx) || defined (STM32F427_437xx) || defined (STM32F429_439xx) || defined (STM32F401xx)
#include "syslib/inc/stm32f4xx.h"
#else
#define _STM32_U_CONTROLLER_
#include <SysLib/CM3/stm32f10x.h>
#endif
#elif defined(_MSC_VER)
#include <bsp/windows_map.h>
#else
# error "Unknown processor !"
#endif


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

#else //_COMMON_INCLUDE_H_
	#pragma message "_COMMON_INCLUDE_H_ already defined !"
#endif//_COMMON_INCLUDE_H_
