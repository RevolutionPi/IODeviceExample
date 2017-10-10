/*=============================================================================================
*
* clock.c
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

//+=============================================================================================
//|		Include-Dateien / include files
//+=============================================================================================

#include <string.h>
#include <common_define.h>
#include <project.h>

#include <bsp/bspConfig.h>
#include <bsp/bspError.h>
#include <bsp/gpio/gpio.h>
#include <bsp/clock/clock.h>

#include <SysLib/inc/stm32f2xx.h>
#include <SysLib/inc/stm32f2xx_rcc.h>
#include <SysLib/inc/core_cm3.h>


// HSE_VALUE must be redefined if the external clock does not have 25MHz
#undef HSE_VALUE
#if defined (STM_CLOCK_8_120) || defined (STM_CLOCK_8_128) || defined (STM_CLOCK_8_64) || defined (STM_CLOCK_8_96) || defined (STM_CLOCK_8_72)
    #define HSE_VALUE            ((uint32_t) 8000000) /*!< Value of the External oscillator in Hz */
#elif defined (STM_CLOCK_10_70)
    #define HSE_VALUE            ((uint32_t)10000000) /*!< Value of the External oscillator in Hz */
#elif defined (STM_CLOCK_12_120) || defined (STM_CLOCK_12_72)		
    #define HSE_VALUE            ((uint32_t)12000000) /*!< Value of the External oscillator in Hz */
#elif defined (STM_CLOCK_16_120) || defined (STM_CLOCK_16_64) || defined (STM_CLOCK_16_96) || defined (STM_CLOCK_16_72) || defined (STM_CLOCK_16_168)
    #define HSE_VALUE            ((uint32_t)16000000) /*!< Value of the External oscillator in Hz */
#elif defined (STM_CLOCK_25_120) || defined (STM_CLOCK_25_75) || defined (STM_CLOCK_25_96)
    #define HSE_VALUE            ((uint32_t)25000000) /*!< Value of the External oscillator in Hz */
#else
    #error "NO VALID CLOCK SPEED SELECTED !"
#endif



//+=============================================================================================
//|		Globale Variablen / global variables
//+=============================================================================================

//+============================================================================================
//|		Function:	clock_init
//+---------------------------------------------------------------------------------------------
//!		module initialisation function.
//!
//!     Clock and PLL Initialisation
//!
//!		\code
//!		-
//!		\endcode
//+---------------------------------------------------------------------------------------------
//|		Conditions:
//!		\pre	(pre-condition)
//!		\post	(post-condition)
//+---------------------------------------------------------------------------------------------
//|		Annotations:
//+=============================================================================================
INT32U Clk_Init(void)
{
    volatile INT32U i32uHSIwatchDog = STM_SYSCLOCK/1000;
    KB_GPIO_InitTypeDef GPIO_InitStructure;

    memset(&GPIO_InitStructure, 0, sizeof(KB_GPIO_InitTypeDef));

    // 1. Clocking the controller from internal HSI RC (8 resp 16MHz)
    RCC_HSICmd (ENABLE);
    // wait until the HSI is ready
    while (RCC_GetFlagStatus (RCC_FLAG_HSIRDY) == RESET)
    {
        i32uHSIwatchDog--;
        if( i32uHSIwatchDog < 1)
        {
            return BSPE_CC_INIT_HSI_ERR;
        }
    }

    RCC_SYSCLKConfig (RCC_SYSCLKSource_HSI);


    // 2. Enable ext. high frequency OSC
    RCC_HSEConfig (RCC_HSE_ON);
    // wait until the HSE is ready
    while (RCC_GetFlagStatus (RCC_FLAG_HSERDY) == RESET)
    {
        i32uHSIwatchDog--;
        if( i32uHSIwatchDog < 1)
        {
            return BSPE_CC_INIT_HSE_ERR;
        }
    }

    // 3. Init PLL
    RCC_PLLCmd (DISABLE);

#if defined (STM_CLOCK_25_120)
    RCC_PLLConfig(RCC_PLLSource_HSE, 25, 240, 2, 5);    //SysClk=120MHz
#elif defined (STM_CLOCK_25_96)
    RCC_PLLConfig(RCC_PLLSource_HSE, 25, 192, 2, 4);    //SysClk=96 MHz
#elif defined (STM_CLOCK_8_120) 
    RCC_PLLConfig(RCC_PLLSource_HSE,  8, 240, 2, 5);    //SysClk=120MHz
#elif defined (STM_CLOCK_12_120)
    RCC_PLLConfig(RCC_PLLSource_HSE, 12, 240, 2, 5);    //SysClk=120MHz
#elif defined (STM_CLOCK_16_120)
    RCC_PLLConfig(RCC_PLLSource_HSE, 16, 240, 2, 5);    //SysClk=120MHz
#elif defined (STM_CLOCK_16_168)
    RCC_PLLConfig(RCC_PLLSource_HSE, 16, 336, 2, 7);    //SysClk=168MHz
#elif defined (STM_CLOCK_8_64)
    RCC_PLLConfig(RCC_PLLSource_HSE,  8, 128, 2, 6);    //SysClk=64 MHz
#elif defined (STM_CLOCK_8_128)
    RCC_PLLConfig(RCC_PLLSource_HSE,  8, 256, 2, 6);    //SysClk=128 MHz
#elif defined (STM_CLOCK_8_96)
    RCC_PLLConfig(RCC_PLLSource_HSE,  8, 192, 2, 4);    //SysClk=96 MHz
#elif defined (STM_CLOCK_16_96)
    RCC_PLLConfig(RCC_PLLSource_HSE, 16, 192, 2, 4);    //SysClk=96 MHz
#elif defined (STM_CLOCK_12_72)
    #error this call is not correct
    RCC_PLLConfig(RCC_PLLSource_HSE, 12, 240, 2, 5);    //SysClk=120MHz
#else
    #error "no valid pll/clock settings !"
#endif

    RCC_PLLCmd (ENABLE);
    // wait until the PLL is ready
    while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
    {
        i32uHSIwatchDog--;
        if( i32uHSIwatchDog < 1)
        {
            return BSPE_CC_INIT_PLL_ERR;
        }
    }


    // 4. Set system clock dividers
    RCC_HCLKConfig(RCC_SYSCLK_Div1);	// 120 MHz
    RCC_PCLK1Config(RCC_HCLK_Div4);		// 30MHz => timers 2-7 run with 60MHz
    RCC_PCLK2Config(RCC_HCLK_Div2);		// 60MHz => timers 1,8 run with 120MHz

    // Flash 3 wait state, Prefetchbufferenable
    FLASH->ACR  = FLASH_ACR_LATENCY_3WS 
        | FLASH_ACR_PRFTEN
        | FLASH_ACR_DCEN
        | FLASH_ACR_ICEN;
    // 5. Clock system from PLL
    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK); // SYSCLK=PLLCLK

#if defined (STM_MCO1)
    /* PHY.CLK(PA08, out) */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    kbGPIO_InitCLK(GPIOA, &GPIO_InitStructure);

    RCC_MCO1Config(RCC_MCO1Source_HSE, RCC_MCO1Div_1 );
#endif

    return BSPE_NO_ERROR;
}


/**
* @brief  Returns the frequencies of different on chip clocks; SYSCLK, HCLK, 
*         PCLK1 and PCLK2.       
* 
* @note   The system frequency computed by this function is not the real 
*         frequency in the chip. It is calculated based on the predefined 
*         constant and the selected clock source:
* @note     If SYSCLK source is HSI, function returns values based on HSI_VALUE(*)
* @note     If SYSCLK source is HSE, function returns values based on HSE_VALUE(**)
* @note     If SYSCLK source is PLL, function returns values based on HSE_VALUE(**) 
*           or HSI_VALUE(*) multiplied/divided by the PLL factors.         
* @note     (*) HSI_VALUE is a constant defined in stm32f2xx.h file (default value
*               16 MHz) but the real value may vary depending on the variations
*               in voltage and temperature.
* @note     (**) HSE_VALUE is a constant defined in stm32f2xx.h file (default value
*                25 MHz), user has to ensure that HSE_VALUE is same as the real
*                frequency of the crystal used. Otherwise, this function may
*                have wrong result.
*                
* @note   The result of this function could be not correct when using fractional
*         value for HSE crystal.
*   
* @param  RCC_Clocks: pointer to a RCC_ClocksTypeDef structure which will hold
*          the clocks frequencies.
*     
* @note   This function can be used by the user application to compute the 
*         baudrate for the communication peripherals or configure other parameters.
* @note   Each time SYSCLK, HCLK, PCLK1 and/or PCLK2 clock changes, this function
*         must be called to update the structure's field. Otherwise, any
*         configuration based on this function will be incorrect.
*    
* @retval None
*/
void BSP_RCC_GetClocksFreq (RCC_ClocksTypeDef* RCC_Clocks)
{
    uint32_t i32uTmpReg_l = 0, i32uPresc_l = 0, pllvco = 0, pllp = 2, i32uPllsource_l = 0, pllm = 2;
    static uint8_t APBAHBPrescTable[16] = {0, 0, 0, 0, 1, 2, 3, 4, 1, 2, 3, 4, 6, 7, 8, 9};

    /* Get SYSCLK source -------------------------------------------------------*/
    i32uTmpReg_l = RCC->CFGR & RCC_CFGR_SWS;

    switch (i32uTmpReg_l)
    {
    case 0x00:  /* HSI used as system clock source */
        RCC_Clocks->SYSCLK_Frequency = HSI_VALUE;
        break;
    case 0x04:  /* HSE used as system clock  source */
        RCC_Clocks->SYSCLK_Frequency = HSE_VALUE;
        break;
    case 0x08:  /* PLL used as system clock  source */

        /* PLL_VCO = (HSE_VALUE or HSI_VALUE / PLLM) * PLLN
        SYSCLK = PLL_VCO / PLLP
        */    
        i32uPllsource_l = (RCC->PLLCFGR & RCC_PLLCFGR_PLLSRC) >> 22;
        pllm = RCC->PLLCFGR & RCC_PLLCFGR_PLLM;

        if (i32uPllsource_l != 0)
        {
            /* HSE used as PLL clock source */
            pllvco = (HSE_VALUE / pllm) * ((RCC->PLLCFGR & RCC_PLLCFGR_PLLN) >> 6);
        }
        else
        {
            /* HSI used as PLL clock source */
            pllvco = (HSI_VALUE / pllm) * ((RCC->PLLCFGR & RCC_PLLCFGR_PLLN) >> 6);      
        }

        pllp = (((RCC->PLLCFGR & RCC_PLLCFGR_PLLP) >>16) + 1 ) *2;
        RCC_Clocks->SYSCLK_Frequency = pllvco/pllp;
        break;
    default:
        RCC_Clocks->SYSCLK_Frequency = HSI_VALUE;
        break;
    }
    /* Compute HCLK, PCLK1 and PCLK2 clocks frequencies ------------------------*/

    /* Get HCLK prescaler */
    i32uTmpReg_l = RCC->CFGR & RCC_CFGR_HPRE;
    i32uTmpReg_l = i32uTmpReg_l >> 4;
    i32uPresc_l = APBAHBPrescTable[i32uTmpReg_l];
    /* HCLK clock frequency */
    RCC_Clocks->HCLK_Frequency = RCC_Clocks->SYSCLK_Frequency >> i32uPresc_l;

    /* Get PCLK1 prescaler */
    i32uTmpReg_l = RCC->CFGR & RCC_CFGR_PPRE1;
    i32uTmpReg_l = i32uTmpReg_l >> 10;
    i32uPresc_l = APBAHBPrescTable[i32uTmpReg_l];
    /* PCLK1 clock frequency */
    RCC_Clocks->PCLK1_Frequency = RCC_Clocks->HCLK_Frequency >> i32uPresc_l;

    /* Get PCLK2 prescaler */
    i32uTmpReg_l = RCC->CFGR & RCC_CFGR_PPRE2;
    i32uTmpReg_l = i32uTmpReg_l >> 13;
    i32uPresc_l = APBAHBPrescTable[i32uTmpReg_l];
    /* PCLK2 clock frequency */
    RCC_Clocks->PCLK2_Frequency = RCC_Clocks->HCLK_Frequency >> i32uPresc_l;

}

//+=============================================================================================
//|     Aenderungsjournal
//+=============================================================================================
#ifdef __DOCGEN
/*!
@page revisions Revisions

*/
#endif
