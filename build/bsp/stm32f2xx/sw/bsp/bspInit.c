/*=============================================================================================
*
* bspInit.c
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
#include <stdarg.h>

#include <common_define.h>
#include <project.h>

#include <bsp/bspError.h>
#include <bsp/bspConfig.h>
#include <bsp/gpio/gpio.h>
#include <bsp/spi/spi.h>
#include <SysLib/inc/stm32f2xx.h>
#include <SysLib/inc/core_cm3.h>
#include <SysLib/inc/misc.h>
#include <SysLib/inc/stm32f2xx_flash.h>
#include <bsp/setjmp/BspSetJmp.h>
#include <bsp/bspInit.h>
#include <bsp/clock/clock.h>
#include <bsp/led/Led.h>
#if defined (STM_WITH_EEPROM)
#include <bsp/eeprom/eeprom.h>
#endif

#include <bsp/gpio/gpio.h>


#define NVIC_CCR ((volatile unsigned long *)(0xE000ED14))

//Option Bytes 0x1FFFC000 - 0x1FFFC00F
typedef
#include <COMP_packBegin.h>
struct S_OPT_BYTES
{
#define OPTION_BYTES_ADDRESS (T_OPT_BYTES*) 0x1FFFC000

    //0x1FFFC000
    INT8U   i8uUserOptionBytes;	///< This byte is used to configure the following features:
                                ///< – Select the watchdog event: Hardware or software
                                ///< – Reset event when entering the Stop mode
                                ///< – Reset event when entering the Standby mode

    INT8U   i8uROP_OptionBytes;	///< 0xAA: Level 0, no protection
                                ///< 0xCC: Level 2, chip protection (debug and boot from RAM features disabled)
                                ///< Others: Level 1, read protection of memories (debug features limited)
    INT8U	i8uReserved1[6];
    //0x1FFFC008
    INT16U  i16uWOP_OptionBytes;///< nWRP :  Flash memory write protection option bytes
                                ///< Sectors 0 to 11 can be write protected.
                                ///< Bits 15:12 0xF: Not used
    INT8U	i8uReserved2[6];
}
#include <COMP_packEnd.h>
T_OPT_BYTES;


extern void bspSetExceptionPoint (BSP_TJumpBuf *ptJumpBuf_p);
extern void bspRegisterErrorHandler (void (*cbErrHandler_p)(INT32U i32uErrorCode_p, TBOOL bFatal_p,
  INT8U i8uParaCnt_p, va_list argptr_p));

extern TBOOL bInitErr_RamTest_g;       //!< indicates an initial RAM test error to the application
extern TBOOL bInitErr_CrcTest_g;       //!< indicates an initial CRC error over the firmware

//+=============================================================================================
//|    Function:	OptBytes_Init
//+---------------------------------------------------------------------------------------------
//!    Initialize Option Bytes.
//+---------------------------------------------------------------------------------------------
//|    Conditions:
//!    Thread-Safe: (NO)
//!    \pre		RESET
//!    \post	VBOR3
//+---------------------------------------------------------------------------------------------
//|    Annotations:
//+=============================================================================================
void OptBytes_Init( void )
{
    //T_KUNBUS_OPT_BYTES	tTmp_l;
    T_OPT_BYTES*	ptOrg_l = OPTION_BYTES_ADDRESS;
    volatile INT32U i32uTmp_l = 0;

    //check BOR_LEV
    if( (ptOrg_l->i8uUserOptionBytes & FLASH_OPTCR_BOR_LEV) != 0 ) //we want VBOR3
    { //Option Bytes have to be reset

        // Remove Option lock bit
        while(FLASH->OPTCR & FLASH_OPTCR_OPTLOCK)
        {
            FLASH->OPTKEYR = FLASH_OPT_KEY1;
            FLASH->OPTKEYR = FLASH_OPT_KEY2;
        }

        //wait until FLASH is not busy anymore
        while( FLASH->SR & FLASH_SR_BSY )
        {
            i32uTmp_l++;
        }

        FLASH->OPTCR &= ~FLASH_OPTCR_BOR_LEV;

        FLASH->OPTCR |= FLASH_OPTCR_OPTSTRT;

        //wait until FLASH is not busy anymore
        while( FLASH->SR & FLASH_SR_BSY )
        {
            i32uTmp_l++;
        }

    }
}

//*************************************************************************************************
//| Function: bspInit
//|
//! common function for all stm32 board functions initialization.
//!
//! The function contains the intialization of all "static" functionality for all boards with the
//! STM32 Microcontroller. Because most boards are very similar there is a benefit in holding the
//! complete initialization in a central place accross different products.
//! <p>
//!
//! The function can be customized via the include file "bspConfig.h".
//!
//!
//-------------------------------------------------------------------------------------------------
void bspInit (
    BSP_TJumpBuf *ptExceptionPoint_p,
    void (*cbErrHandler_p)(INT32U i32uErrorCode_p, TBOOL bFatal_p, INT8U i8uParaCnt_p, va_list argptr_p))
                            //!< [in] Pointer to application specific error handler
                            //! \return Error Code, defined in "bspError.h"

{
    INT32U i32uRv_l = BSPE_NO_ERROR;

#if defined(STM_WITH_EEPROM)
    INT32U i32uResetFlags_l = 0;
#endif

    bspSetExceptionPoint (ptExceptionPoint_p);
    bspRegisterErrorHandler (cbErrHandler_p);


    // Set STKALIGN in NVIC
    *NVIC_CCR = *NVIC_CCR | 0x200;

    // Configure number of bits for preemption priority and for subpriority
    NVIC_PriorityGroupConfig(STM_INTERRUPT_PRIORITYGROUP);

    //Initialize Option Bytes
    OptBytes_Init();

    //SystemClock Initialization
    i32uRv_l = Clk_Init();
    if (i32uRv_l != BSPE_NO_ERROR)
    {
        //try to config SYSTICK
        SysTick_Config (HSI_VALUE /8000);

    bspError (i32uRv_l, bTRUE, 0);
  }

#if defined	(STM_WITH_GPIO)
  gpio_JTAGcfg();
#endif

    //-------------------------------------------------------------------------------------------------
    // SYStick Initialization
    //-------------------------------------------------------------------------------------------------
    RCC_ClocksTypeDef RCC_Clocks;

    /* Configure Systick clock source as HCLK */
    SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);

    /* SystTick configuration: an interrupt every 1ms */
    BSP_RCC_GetClocksFreq(&RCC_Clocks);
    SysTick_Config(RCC_Clocks.HCLK_Frequency / 1000);

    //-------------------------------------------------------------------------------------------------
    // LED Initialization
    //-------------------------------------------------------------------------------------------------
#if defined (STM_WITH_LED)
    i32uRv_l = LED_initLed ();
    if (i32uRv_l != BSPE_NO_ERROR)
    {
        bspError (i32uRv_l, bTRUE, 0);
    }
#endif

    //-------------------------------------------------------------------------------------------------
    // EEPROM Initialization
    //-------------------------------------------------------------------------------------------------
#if defined (STM_WITH_EEPROM)
    BSP_EEPROM_init ();

    if (ctKunbusApplDescription_g.i32uBootFlags & KUNBUS_APPL_DESCR_BOOT_FACTORY_RESET)
    {
        BSP_EEPROM_factoryReset();
        i32uResetFlags_l |= KUNBUS_APPL_DESCR_BOOT_FACTORY_RESET;   // set the flag to 0 later
    }

    // set flags to 0 in flash
    if (i32uResetFlags_l)
    {
        int i;
        INT8U i8uVal;
        INT32U i32uVal;
        i32uVal = ctKunbusApplDescription_g.i32uBootFlags;

        // use Syslib-functions directly, otherwise it would be necessary to add the flash driver
        /* Unlock the Flash Program Erase controller */
        FLASH_Unlock();

        /* Clear All pending flags */
        FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);

        for (i = 0; i < sizeof(i32uResetFlags_l); i++)
        {
            i8uVal = (INT8U)i32uResetFlags_l;
            if (i8uVal)
            {
                i8uVal &= ~(INT8U)i32uVal;

                FLASH_ProgramByte(((INT32U)&ctKunbusApplDescription_g.i32uBootFlags) + i, i8uVal);


            }
            i32uVal >>= 8;
            i32uResetFlags_l >>= 8;
        }
        /* restore flash lock state */
        FLASH_Lock();
    }
#endif

    if (bInitErr_RamTest_g == bTRUE)
    {
        bspError (BSPE_STARTUP_RAM, bTRUE, 0);
    }

    if (bInitErr_CrcTest_g == bTRUE)
    {
        bspError (BSPE_STARTUP_CRC, bTRUE, 0);
    }
}

//*************************************************************************************************
//| Function: BSP_systemReset
//|
//! \brief
//!
//! \detailed
//!
//!
//!
//! \ingroup
//-------------------------------------------------------------------------------------------------
void BSP_systemReset (
    void)

{
    NVIC_SystemReset ();
}

//*************************************************************************************************
