/*=============================================================================================
*
* bspGlobalIntCtrl.h
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
* functions for disable and enable of global interrupts
*
*/
#ifndef BSPGLOBALINTCTRL_H_INC
#define BSPGLOBALINTCTRL_H_INC

#if defined (STM32F3XX)
  #include "stm32f30x.h"

  #define	GLOBAL_INTERRUPT_ENABLE()		__enable_irq()
  #define	GLOBAL_INTERRUPT_DISABLE()		__disable_irq()

#elif defined (STM32F40_41xxx) || defined (STM32F427_437xx) || defined (STM32F429_439xx) || defined (STM32F401xx)
  #include <syslib/inc/stm32f4xx.h>

  #define	GLOBAL_INTERRUPT_ENABLE()		__enable_irq()
  #define	GLOBAL_INTERRUPT_DISABLE()		__disable_irq()

#elif defined (_MSC_VER)
  #include <bsp/globalIntCtrl/bspGlobalIntCtrl_win32.h>

  #define	GLOBAL_INTERRUPT_ENABLE()		bspGlobalIntEnable()
  #define	GLOBAL_INTERRUPT_DISABLE()		bspGlobalIntDisable()

#elif defined (__NIOS_GENERIC__)

  #include <sys/alt_irq.h>

  #define	GLOBAL_INTERRUPT_ENABLE()	{\
                                            alt_irq_context context;\
                                            NIOS2_READ_STATUS (context);\
                                            NIOS2_WRITE_STATUS (context | NIOS2_STATUS_PIE_MSK);\
                                        }
  #define	GLOBAL_INTERRUPT_DISABLE()	{\
                                            alt_irq_context context;\
                                            NIOS2_READ_STATUS (context);\
                                            NIOS2_WRITE_STATUS (context & ~NIOS2_STATUS_PIE_MSK);\
                                        }

#elif defined (__KUNBUSPI__)
  #include <bsp/globalIntCtrl/bspGlobalIntCtrl_linuxRT.h>  

  #define	GLOBAL_INTERRUPT_ENABLE()		bspGlobalIntEnable()
  #define	GLOBAL_INTERRUPT_DISABLE()		bspGlobalIntDisable()

#elif defined (__KUNBUSPI_KERNEL__)
  #define	GLOBAL_INTERRUPT_ENABLE()
  #define	GLOBAL_INTERRUPT_DISABLE()
#elif defined (__SF2_GENERIC__)
  #include <m2sxxx.h>

  #define	GLOBAL_INTERRUPT_ENABLE()		__enable_irq()
  #define	GLOBAL_INTERRUPT_DISABLE()		__disable_irq()
#else
  #define	GLOBAL_INTERRUPT_ENABLE()		__enable_irq()
  #define	GLOBAL_INTERRUPT_DISABLE()		__disable_irq()
#endif



#ifdef __cplusplus
extern "C" {
#endif


#ifdef __cplusplus
}
#endif

#endif  // BSPGLOBALINTCTRL_H_INC


