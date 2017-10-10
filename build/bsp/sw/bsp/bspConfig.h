/*=============================================================================================
*
* bspConfig.h
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
#ifndef BSPCONFIG_H_INC
#define BSPCONFIG_H_INC

#include <project.h>

//+=============================================================================================
//|        determine clock speed
//+=============================================================================================
#if defined (STM_CLOCK_8_64)
    #define STM_SYSCLOCK 64000000
    #define DELAY_80NS    asm volatile ("nop\n nop\n nop\n nop\n nop\n nop");
#elif defined (STM_CLOCK_8_72)
    #define STM_SYSCLOCK 72000000
    #define DELAY_80NS    asm volatile ("nop\n nop\n nop\n nop\n nop\n nop");
#elif defined (STM_CLOCK_16_64)
    #define STM_SYSCLOCK 64000000
    #define DELAY_80NS    asm volatile ("nop\n nop\n nop\n nop\n nop");
#elif defined (STM_CLOCK_12_72)
    #define STM_SYSCLOCK 72000000
    #define DELAY_80NS    asm volatile ("nop\n nop\n nop\n nop\n nop\n nop");
#elif defined (STM_CLOCK_8_120) || defined (STM_CLOCK_12_120) || defined (STM_CLOCK_16_120) || defined (STM_CLOCK_25_120)
    #define STM_SYSCLOCK 120000000
    #define DELAY_80NS    asm volatile ("nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop");
#elif defined (STM_CLOCK_8_128)     // for CAN Controllers (multiple of 32 MHz)
    #define STM_SYSCLOCK 128000000
    #define DELAY_80NS    asm volatile ("nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop");
#elif defined (STM_CLOCK_10_70)
    #define STM_SYSCLOCK 70000000
#elif defined (STM_CLOCK_25_75)
    #define STM_SYSCLOCK 75000000
    #define DELAY_80NS    asm volatile ("nop\n nop\n nop\n nop\n nop\n nop");
#elif defined (STM_CLOCK_25_96)
    #define STM_SYSCLOCK 96000000
    #define DELAY_80NS    asm volatile ("nop\n nop\n nop\n nop\n nop\n nop\n nop");
#elif defined (STM_CLOCK_32_64)
    #define STM_SYSCLOCK 64000000
#elif defined (STM_CLOCK_48_72)
    #define STM_SYSCLOCK 72000000
#elif defined (STM_CLOCK_5_70)
    #define STM_SYSCLOCK 70000000
#elif defined (STM_CLOCK_INT8_36)
    #define STM_SYSCLOCK 36000000
#elif defined (STM_CLOCK_8_96)   // for CAN Controllers (multiple of 32 MHz)
    #define STM_SYSCLOCK 96000000
    #define DELAY_80NS    asm volatile ("nop\n nop\n nop\n nop\n nop\n nop\n nop");
#elif defined (STM_CLOCK_16_96)   // for CAN Controllers (multiple of 32 MHz)
    #define STM_SYSCLOCK 96000000
    #define DELAY_80NS    asm volatile ("nop\n nop\n nop\n nop\n nop\n nop\n nop");
#elif defined (STM_CLOCK_16_72)   // for STM323F30x
    #define STM_SYSCLOCK 72000000
    #define DELAY_80NS    asm volatile ("nop\n nop\n nop\n nop\n nop\n nop");
#elif defined (STM_CLOCK_16_168)   // for STM323F40x
    #define STM_SYSCLOCK 168000000
    #define DELAY_80NS    asm volatile ("nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop");
#elif defined (STM_CLOCK_16_180) || defined (STM_CLOCK_25_180)  // for STM323F42x
    #define STM_SYSCLOCK 180000000
    #define DELAY_80NS    asm volatile ("nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop");
#elif defined (_MSC_VER)
    // dummy for windows
#elif defined (__NIOS_GENERIC__)
    // dummy for windows
#elif defined (__NIOS_GENERIC__)
    // dummy for windows
#elif defined (__KUNBUSPI__)
    // dummy for raspberry
#elif defined (__KUNBUSPI_KERNEL_)
    // dummy for raspberry
#elif defined (__TIAM335XGENERIC__)
    // dummy for Ti Sitara
#elif defined (__SF2_GENERIC__)
    #define STM_SYSCLOCK 100000000
    #define DELAY_80NS    asm volatile ("nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop");
#else
    #error "No Clock Speed defined"            
#endif

//+=============================================================================================
//|        determine processor architecture
//+=============================================================================================
// Needed for determining Pinout and for determing Flash Lay Out
// #define STM32F10X_LD        STM32 Low density devices
// #define STM32F10X_MD        STM32 Medium density devices
// #define STM32F10X_HD        STM32 High density devices
// #define STM32F10X_CL        STM32 connectivity line devices
//
//- Low density devices are STM32F101xx, STM32F102xx and STM32F103xx microcontrollers
//  where the Flash memory density ranges between 16 and 32 Kbytes.
//- Medium density devices are STM32F101xx, STM32F102xx and STM32F103xx microcontrollers
//  where the Flash memory density ranges between 64 and 128 Kbytes.
//- High density devices are STM32F101xx and STM32F103xx microcontrollers where
//  the Flash memory density ranges between 256 and 512 Kbytes.
//- Connectivity line devices are STM32F105xx and STM32F107xx microcontrollers.

#if defined (STM32F101)
    #define STM32F10X_LD
#elif defined (STM32F103R8) || defined (STM32F103RB) || defined (STM32F103CB) || defined (STM32F100RB)
    #define STM32F10X_MD 1
#elif defined (STM32F103ZE) || defined (STM32F103VE) || defined (STM32F103ZD) || defined (STM32F103VC)
    #define STM32F10X_HD 1
#elif defined (STM32F107VC) || defined (STM32F107RC)
    #define STM32F10X_CL 1
#elif defined (STM32F207VE) || defined (STM32F205RE)
    #define STM32F2XX 1
#elif defined (STM32F2XX) || defined (STM32F30X) || defined (STM32F40_41xxx) || defined (STM32F427_437xx) || defined (STM32F429_439xx) || defined (STM32F401xx) 
    // nur das ist erlaubt hier
#elif defined (_MSC_VER)
    // nur das ist erlaubt hier
#elif defined (__NIOS_GENERIC__)
    // nur das ist erlaubt hier
#elif defined (__SF2_GENERIC__)
    // nur das ist erlaubt hier
#elif defined (__KUNBUSPI__)
    // dummy for raspberry
#elif defined (__KUNBUSPI_KERNEL_)
    // dummy for raspberry
#elif defined (__TIAM335XGENERIC__)
    // nur das ist erlaubt hier
#else
    #error "could not determine Processor architecture and corresponding Flash layout"
#endif

#ifdef __cplusplus
extern "C" { 
#endif 


#ifdef  __cplusplus 
} 
#endif 

//-------------------- reinclude-protection -------------------- 
#else
//    #pragma message "The header bspConfig.h is included twice or more !"
#endif //BSPCONFIG_H_INC


