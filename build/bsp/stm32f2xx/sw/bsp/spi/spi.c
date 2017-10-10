/*=============================================================================================
*
* spi.c
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

//+=============================================================================================
//|   Include-Dateien / include files
//+=============================================================================================

#include <common_define.h>
#include <project.h>
#include <bsp\bspError.h>
#include <bsp\bspConfig.h>


#include <bsp\gpio\gpio.h>
#include <bsp\spi\spi.h>
#include <bsp\timer\timer.h>
#include <bsp\clock\clock.h>



#include <SysLib\inc\stm32f2xx.h>
#include <SysLib\inc\stm32f2xx_rcc.h>
#include <SysLib\inc\stm32f2xx_spi.h>
#include <SysLib\inc\stm32f2xx_tim.h>
#include <SysLib\inc\misc.h>

#include <string.h>

//+=============================================================================================
//|     Globale Variablen / global variables
//+=============================================================================================


//+=============================================================================================
//|     Makros / macros
//+=============================================================================================
#if defined (STM_WITH_SPI1) || defined (STM_WITH_SPI2) || defined (STM_WITH_SPI3)
#error "Definition of STM_WITH_SPI1-3 is deprecated; please replace by STM_WITH_SPI without corresponding number !"
#endif

#if defined (STM_WITH_SPI)
#else
#error "SPI cannot be used without STM_WITH_SPI to be defined !"
#endif


#if defined (SPI1_IRQ_HANDLER) ||  defined (SPI2_IRQ_HANDLER) ||  defined (SPI3_IRQ_HANDLER)
#error "Definition of SPIX_IRQ_HANDLER is deprecated; please remove all references !"
#endif


#if defined (DELAY_80NS)
#else
#define DELAY_80NS
#endif

//+=============================================================================================
//|     Konstanten / constants
//+=============================================================================================

#define BSP_SPI_MAXPORT   3

#define SPI1_PORT     0
#define SPI2_PORT     1
#define SPI3_PORT     2

#define   DMA_LIFCR_ALL0  DMA_LIFCR_CTCIF0 | DMA_LIFCR_CHTIF0 | DMA_LIFCR_CTEIF0 | DMA_LIFCR_CDMEIF0 | DMA_LIFCR_CFEIF0
#define   DMA_LIFCR_ALL1  DMA_LIFCR_CTCIF1 | DMA_LIFCR_CHTIF1 | DMA_LIFCR_CTEIF1 | DMA_LIFCR_CDMEIF1 | DMA_LIFCR_CFEIF1
#define   DMA_LIFCR_ALL2  DMA_LIFCR_CTCIF2 | DMA_LIFCR_CHTIF2 | DMA_LIFCR_CTEIF2 | DMA_LIFCR_CDMEIF2 | DMA_LIFCR_CFEIF2
#define   DMA_LIFCR_ALL3  DMA_LIFCR_CTCIF3 | DMA_LIFCR_CHTIF3 | DMA_LIFCR_CTEIF3 | DMA_LIFCR_CDMEIF3 | DMA_LIFCR_CFEIF3

#define   DMA_HIFCR_ALL4  DMA_HIFCR_CTCIF4 | DMA_HIFCR_CHTIF4 | DMA_HIFCR_CTEIF4 | DMA_HIFCR_CDMEIF4 | DMA_HIFCR_CFEIF4
#define   DMA_HIFCR_ALL5  DMA_HIFCR_CTCIF5 | DMA_HIFCR_CHTIF5 | DMA_HIFCR_CTEIF5 | DMA_HIFCR_CDMEIF5 | DMA_HIFCR_CFEIF5
#define   DMA_HIFCR_ALL6  DMA_HIFCR_CTCIF6 | DMA_HIFCR_CHTIF6 | DMA_HIFCR_CTEIF6 | DMA_HIFCR_CDMEIF6 | DMA_HIFCR_CFEIF6
#define   DMA_HIFCR_ALL7  DMA_HIFCR_CTCIF7 | DMA_HIFCR_CHTIF7 | DMA_HIFCR_CTEIF7 | DMA_HIFCR_CDMEIF7 | DMA_HIFCR_CFEIF7


#if defined (SPI_DMA_MIN_LEN)
#else
#define SPI_DMA_MIN_LEN 30
#endif

//+=============================================================================================
//|     Prototypen / prototypes
//+=============================================================================================

//+=============================================================================================
//|     Globale Variablen / global variables
//+=============================================================================================

static void (*cbSpiIntRx_s[BSP_SPI_MAXPORT]) (void);
static void (*cbSpiIntTx_s[BSP_SPI_MAXPORT]) (void);
static void (*cbSpiIntErr_s[BSP_SPI_MAXPORT])(void);

// if non blocking mode is chosen the following flags can be used
// together with the DMA-ISR-registers TCIF flag to conroll any state machine of the calling routine
// and they are also used by the spi_transceive function to guarantee any pending DMA transfers have
// been finished before the next transfer is initiated
static TBOOL spiDMAisPending_s[BSP_SPI_MAXPORT]; 


//+=============================================================================================
//|   Function: spi_init
//+---------------------------------------------------------------------------------------------
//!   module initialisation function (detailed description)
//+---------------------------------------------------------------------------------------------
//|   Conditions:
//!   \pre  (pre-condition)
//!   \post (post-condition)
//+---------------------------------------------------------------------------------------------
//|   Annotations:
//+=============================================================================================
INT32U spi_init (
                 INT8U           i8uPort_p,    ///< [in] Number of SPI Controller
                 const HW_SPI_CONFIGURATION  *tHwConf_l    ///< [in] SPI Configuration structure
                 )   /// \return   success of Operation
{
    INT32U          tRv_l = SPI_RET_OK;

    NVIC_InitTypeDef        NVIC_InitStructure;
    GPIO_InitTypeDef		GPIO_InitStructure;
    SPI_InitTypeDef         spi_inits;
    RCC_ClocksTypeDef       clocks;
    INT32U                  i32uPrescaler_l, i32uBaudrate_l;
    TIM_TCountDownTimerInit tim4c;


#if (BSP_SPI_MAXPORT != 3)
#error "BSP_SPI_MAXPORT anpassen"
#endif    

    // check Remappings and correct usage of init in projects

    memset(&spi_inits, 0, sizeof(SPI_InitTypeDef));

    spi_inits.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    spi_inits.SPI_DataSize  = SPI_DataSize_8b;
    spi_inits.SPI_NSS   = SPI_NSS_Soft;

    if (tHwConf_l->mode == HW_SPI_MODE_MASTER)
    {
        spi_inits.SPI_Mode = SPI_Mode_Master;
    }

    if (tHwConf_l->polarity == HW_SPI_CLOCK_POL_HIGH)
    {
        spi_inits.SPI_CPOL = SPI_CPOL_High;
    }

    if (tHwConf_l->phase == HW_SPI_CLOCK_PHASE_TRAIL)
    {
        spi_inits.SPI_CPHA = SPI_CPHA_2Edge;
    }

    if (tHwConf_l->direction == HW_SPI_DATA_DIR_LSB)
    {
        spi_inits.SPI_FirstBit = SPI_FirstBit_LSB;
    }

    if (tHwConf_l->nss == HW_SPI_NSS_Hard)
    {
        spi_inits.SPI_NSS = SPI_NSS_Hard;
    }

    BSP_RCC_GetClocksFreq(&clocks);
    i32uPrescaler_l = 0;

    //-------------------- Baudrate calculation... --------------------
    switch(i8uPort_p)
    {
    case SPI1_PORT: //SPI1
        {   // PCLK2 max 60 MHz
            i32uBaudrate_l = clocks.PCLK2_Frequency / 2;
        } break;

    case SPI2_PORT: //SPI2
    case SPI3_PORT: //SPI3
    default:
        {   // PCLK1 max 30 MHz
            i32uBaudrate_l = clocks.PCLK1_Frequency / 2;
        } break;
    }

    while (i32uBaudrate_l > tHwConf_l->bitrate && i32uPrescaler_l < 7)
    {
        i32uBaudrate_l /= 2;
        i32uPrescaler_l++;
    }

    spi_inits.SPI_BaudRatePrescaler = i32uPrescaler_l << 3;

    switch(i8uPort_p)
    {
        //-------------------- Initialisation of SPI1 --------------------
    case SPI1_PORT: //SPI1
        {
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);

            // Enable SPI1 reset state
            RCC_APB2PeriphResetCmd(RCC_APB2Periph_SPI1, ENABLE);
            // Release SPI1 from reset state
            RCC_APB2PeriphResetCmd(RCC_APB2Periph_SPI1, DISABLE);

#if defined (STM_SPI1_REMAP)
            RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB, ENABLE);

            GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
            GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;   // the speed must be at least 25 MHz because SPI-Flash is running at 20 MHz
            GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
            GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;

            // SCK
            GPIO_PinAFConfig(GPIOB, GPIO_PinSource3, GPIO_AF_SPI1);
            GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
            GPIO_Init(GPIOB, &GPIO_InitStructure);

            // MOSI
            GPIO_PinAFConfig(GPIOB, GPIO_PinSource5, GPIO_AF_SPI1);
            GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
            GPIO_Init(GPIOB, &GPIO_InitStructure);

            // MISO
            GPIO_PinAFConfig(GPIOB, GPIO_PinSource4, GPIO_AF_SPI1);
            GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
            GPIO_Init(GPIOB, &GPIO_InitStructure);

            // Use slave select
            if(tHwConf_l->nss)
            {
                GPIO_PinAFConfig(GPIOA, GPIO_PinSource15, GPIO_AF_SPI1);
                GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
                GPIO_Init(GPIOA, &GPIO_InitStructure);

                GPIO_ResetBits(GPIOA, GPIO_InitStructure.GPIO_Pin);

                if(tHwConf_l->nss == HW_SPI_NSS_Hard)
                {
                    SPI1->CR1 &= ~SPI_CR1_SSM;
                    SPI1->CR1 &= ~SPI_CR1_SSI;
                }
            }
#elif defined (STM_SPI1_REMAP2)
            RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB, ENABLE);

            GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
            GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;
            GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
            GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;

            // SCK
            GPIO_PinAFConfig(GPIOA, GPIO_PinSource5, GPIO_AF_SPI1);
            GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;
            GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
            GPIO_Init(GPIOA, &GPIO_InitStructure);

            // MOSI
            GPIO_PinAFConfig(GPIOB, GPIO_PinSource5, GPIO_AF_SPI1);
            GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;
            GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
            GPIO_Init(GPIOB, &GPIO_InitStructure);

            // MISO
            GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_SPI1);
            GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;
            GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
            GPIO_Init(GPIOA, &GPIO_InitStructure);

            // Use slave select
            if(tHwConf_l->nss)
            {
                GPIO_PinAFConfig(GPIOA, GPIO_PinSource4, GPIO_AF_SPI1);
                GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;
                GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
                GPIO_Init(GPIOA, &GPIO_InitStructure);

                GPIO_ResetBits(GPIOA, GPIO_InitStructure.GPIO_Pin);

                if(tHwConf_l->nss == HW_SPI_NSS_Hard)
                {
                    SPI1->CR1 &= ~SPI_CR1_SSM;
                    SPI1->CR1 &= ~SPI_CR1_SSI;
                }
            }
#else
            RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

            GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
            GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
            GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
            GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;

            // SCK
            GPIO_PinAFConfig(GPIOA, GPIO_PinSource5, GPIO_AF_SPI1);
            GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
            GPIO_Init(GPIOA, &GPIO_InitStructure);

            // MOSI
            GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_SPI1);
            GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
            GPIO_Init(GPIOA, &GPIO_InitStructure);

            // MISO
            GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_SPI1);
            GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
            GPIO_Init(GPIOA, &GPIO_InitStructure);

            // Use slave select
            if(tHwConf_l->nss)
            {
                GPIO_PinAFConfig(GPIOA, GPIO_PinSource4, GPIO_AF_SPI1);
                GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
                GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
                GPIO_Init(GPIOA, &GPIO_InitStructure);

                GPIO_ResetBits(GPIOA, GPIO_InitStructure.GPIO_Pin);

                if(tHwConf_l->nss == HW_SPI_NSS_Hard)
                {
                    SPI1->CR1 &= ~SPI_CR1_SSM;
                    SPI1->CR1 &= ~SPI_CR1_SSI;
                }
            }
#endif

            SPI_Init(SPI1, &spi_inits);

            if( (tHwConf_l->receive_cb) || (tHwConf_l->transmit_cb) || (tHwConf_l->error_cb) )
            {
                NVIC_InitStructure.NVIC_IRQChannel = SPI1_IRQn;
                NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = STM_INTERRUPT_PREEMPTIONPRIORITY_SPI1;
                NVIC_InitStructure.NVIC_IRQChannelSubPriority = STM_INTERRUPT_SUBPRIORITY_SPI1;
                NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

                NVIC_Init(&NVIC_InitStructure);

                cbSpiIntRx_s [SPI1_PORT]  = tHwConf_l->receive_cb;
                cbSpiIntTx_s [SPI1_PORT]  = tHwConf_l->transmit_cb;
                cbSpiIntErr_s[SPI1_PORT]  = tHwConf_l->error_cb;

                if( tHwConf_l->receive_cb )
                {
                    BSP_SPI_ENABLE_RX_INT(SPI1);
                }
                if( tHwConf_l->transmit_cb )
                {
                    BSP_SPI_ENABLE_TX_INT(SPI1);
                }
                if( tHwConf_l->error_cb )
                {
                    BSP_SPI_ENABLE_ERR_INT(SPI1);
                }
            }

            // enable DMA controller
            RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);

            //Enable SPI1
            BSP_SPI_ENABLE(SPI1);

        } break;

        //-------------------- Initialisation of SPI2 --------------------
    case SPI2_PORT: //SPI2
        {
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);

            // Enable SPI2 reset state
            RCC_APB1PeriphResetCmd(RCC_APB1Periph_SPI2, ENABLE);
            // Release SPI2 from reset state
            RCC_APB1PeriphResetCmd(RCC_APB1Periph_SPI2, DISABLE);

#if defined (STM_SPI2_REMAP)
            RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOI, ENABLE);

            GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
            GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
            GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
            GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;

            // SCK
            GPIO_PinAFConfig(GPIOI, GPIO_PinSource1, GPIO_AF_SPI2);
            GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
            GPIO_Init(GPIOI, &GPIO_InitStructure);

            // MOSI
            GPIO_PinAFConfig(GPIOI, GPIO_PinSource3, GPIO_AF_SPI2);
            GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
            GPIO_Init(GPIOI, &GPIO_InitStructure);

            // MISO
            GPIO_PinAFConfig(GPIOI, GPIO_PinSource2, GPIO_AF_SPI2);
            GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
            GPIO_Init(GPIOI, &GPIO_InitStructure);

            // Use slave select
            if(tHwConf_l->nss)
            {
                GPIO_PinAFConfig(GPIOI, GPIO_PinSource0, GPIO_AF_SPI2);
                GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
                GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
                GPIO_Init(GPIOI, &GPIO_InitStructure);

                GPIO_ResetBits(GPIOI, GPIO_InitStructure.GPIO_Pin);

                if(tHwConf_l->nss == HW_SPI_NSS_Hard)
                {
                    SPI2->CR1 &= ~SPI_CR1_SSM;
                    SPI2->CR1 &= ~SPI_CR1_SSI;
                }
            }
#elif defined (STM_SPI2_REMAP2)
            RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

            GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
            GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
            GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
            GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;

            // SCK
            GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_SPI2);
            GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
            GPIO_Init(GPIOB, &GPIO_InitStructure);

            // MOSI
            GPIO_PinAFConfig(GPIOB, GPIO_PinSource15, GPIO_AF_SPI2);
            GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
            GPIO_Init(GPIOB, &GPIO_InitStructure);

            // MISO
            GPIO_PinAFConfig(GPIOB, GPIO_PinSource14, GPIO_AF_SPI2);
            GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
            GPIO_Init(GPIOB, &GPIO_InitStructure);

            // Use slave select
            if(tHwConf_l->nss)
            {
                GPIO_PinAFConfig(GPIOB, GPIO_PinSource12, GPIO_AF_SPI2);
                GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
                GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
                GPIO_Init(GPIOB, &GPIO_InitStructure);

                GPIO_ResetBits(GPIOB, GPIO_InitStructure.GPIO_Pin);

                if(tHwConf_l->nss == HW_SPI_NSS_Hard)
                {
                    SPI2->CR1 &= ~SPI_CR1_SSM;
                    SPI2->CR1 &= ~SPI_CR1_SSI;
                }
            }
#elif defined (STM_SPI2_REMAP3)
            RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
            GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
            GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
            GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;

            // SCK
            GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_SPI2);
            GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
            GPIO_Init(GPIOB, &GPIO_InitStructure);

            // MOSI
            GPIO_PinAFConfig(GPIOC, GPIO_PinSource3, GPIO_AF_SPI2);
            GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
            GPIO_Init(GPIOC, &GPIO_InitStructure);

            // MISO
            GPIO_PinAFConfig(GPIOC, GPIO_PinSource2, GPIO_AF_SPI2);
            GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
            GPIO_Init(GPIOC, &GPIO_InitStructure);

            // Use slave select
            if (tHwConf_l->nss)
            {
                GPIO_PinAFConfig(GPIOB, GPIO_PinSource12, GPIO_AF_SPI2);
                GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
                GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
                GPIO_Init(GPIOB, &GPIO_InitStructure);

                GPIO_ResetBits(GPIOB, GPIO_InitStructure.GPIO_Pin);

                if (tHwConf_l->nss == HW_SPI_NSS_Hard)
                {
                    SPI2->CR1 &= ~SPI_CR1_SSM;
                    SPI2->CR1 &= ~SPI_CR1_SSI;
                }
            }
#else
            RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

            GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
            GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
            GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
            GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;

            // SCK
            GPIO_PinAFConfig(GPIOB, GPIO_PinSource13, GPIO_AF_SPI2);
            GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
            GPIO_Init(GPIOB, &GPIO_InitStructure);

            // MOSI
            GPIO_PinAFConfig(GPIOB, GPIO_PinSource15, GPIO_AF_SPI2);
            GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
            GPIO_Init(GPIOB, &GPIO_InitStructure);

            // MISO
            GPIO_PinAFConfig(GPIOB, GPIO_PinSource14, GPIO_AF_SPI2);
            GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
            GPIO_Init(GPIOB, &GPIO_InitStructure);

            // Use slave select
            if(tHwConf_l->nss)
            {
                GPIO_PinAFConfig(GPIOB, GPIO_PinSource12, GPIO_AF_SPI2);
                GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
                GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
                GPIO_Init(GPIOB, &GPIO_InitStructure);

                GPIO_ResetBits(GPIOB, GPIO_InitStructure.GPIO_Pin);

                if(tHwConf_l->nss == HW_SPI_NSS_Hard)
                {
                    SPI2->CR1 &= ~SPI_CR1_SSM;
                    SPI2->CR1 &= ~SPI_CR1_SSI;
                }
            }
#endif

            SPI_Init(SPI2, &spi_inits);

            if( (tHwConf_l->receive_cb) || (tHwConf_l->transmit_cb) || (tHwConf_l->error_cb) )
            {
                NVIC_InitStructure.NVIC_IRQChannel = SPI2_IRQn;
                NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = STM_INTERRUPT_PREEMPTIONPRIORITY_SPI2;
                NVIC_InitStructure.NVIC_IRQChannelSubPriority = STM_INTERRUPT_SUBPRIORITY_SPI2;
                NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

                NVIC_Init(&NVIC_InitStructure);

                cbSpiIntRx_s [SPI2_PORT]  = tHwConf_l->receive_cb;
                cbSpiIntTx_s [SPI2_PORT]  = tHwConf_l->transmit_cb;
                cbSpiIntErr_s[SPI2_PORT]  = tHwConf_l->error_cb;

                if( tHwConf_l->receive_cb )
                {
                    BSP_SPI_ENABLE_RX_INT(SPI2);
                }
                if( tHwConf_l->transmit_cb )
                {
                    BSP_SPI_ENABLE_TX_INT(SPI2);
                }
                if( tHwConf_l->error_cb )
                {
                    BSP_SPI_ENABLE_ERR_INT(SPI2);
                }
            }

            // enable DMA controller
            RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);

            //Enable SPI1
            BSP_SPI_ENABLE(SPI2);

        } break;

        //-------------------- Initialisation of SPI3 --------------------
    case SPI3_PORT:
        {
            //Provide peripheral clock
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI3, ENABLE);

            // Enable SPI3 reset state
            RCC_APB1PeriphResetCmd(RCC_APB1Periph_SPI3, ENABLE);
            // Release SPI3 from reset state
            RCC_APB1PeriphResetCmd(RCC_APB1Periph_SPI3, DISABLE);

#if defined (STM_SPI3_REMAP)
            RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOC, ENABLE);

            GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
            GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
            GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
            GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;

            // SCK
            GPIO_PinAFConfig(GPIOC, GPIO_PinSource10, GPIO_AF_SPI3);
            GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
            GPIO_Init(GPIOC, &GPIO_InitStructure);

            // MOSI
            GPIO_PinAFConfig(GPIOC, GPIO_PinSource12, GPIO_AF_SPI3);
            GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
            GPIO_Init(GPIOC, &GPIO_InitStructure);

            // MISO
            GPIO_PinAFConfig(GPIOC, GPIO_PinSource11, GPIO_AF_SPI3);
            GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
            GPIO_Init(GPIOC, &GPIO_InitStructure);

            // Use slave select
            if(tHwConf_l->nss)
            {
                GPIO_PinAFConfig(GPIOA, GPIO_PinSource15, GPIO_AF_SPI3);
                GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
                GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
                GPIO_Init(GPIOA, &GPIO_InitStructure);

                GPIO_ResetBits(GPIOA, GPIO_InitStructure.GPIO_Pin);

                if(tHwConf_l->nss == HW_SPI_NSS_Hard)
                {
                    SPI3->CR1 &= ~SPI_CR1_SSM;
                    SPI3->CR1 &= ~SPI_CR1_SSI;
                }
            }
#else
            RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB, ENABLE);

            GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
            GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
            GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
            GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;

            // SCK
            GPIO_PinAFConfig(GPIOB, GPIO_PinSource3, GPIO_AF_SPI3);
            GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
            GPIO_Init(GPIOB, &GPIO_InitStructure);

            // MOSI
            GPIO_PinAFConfig(GPIOB, GPIO_PinSource5, GPIO_AF_SPI3);
            GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
            GPIO_Init(GPIOB, &GPIO_InitStructure);

            // MISO
            GPIO_PinAFConfig(GPIOB, GPIO_PinSource4, GPIO_AF_SPI3);
            GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
            GPIO_Init(GPIOB, &GPIO_InitStructure);

            // Use slave select
            if(tHwConf_l->nss)
            {
                GPIO_PinAFConfig(GPIOA, GPIO_PinSource15, GPIO_AF_SPI3);
                GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
                GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
                GPIO_Init(GPIOA, &GPIO_InitStructure);

                GPIO_ResetBits(GPIOA, GPIO_InitStructure.GPIO_Pin);

                if(tHwConf_l->nss == HW_SPI_NSS_Hard)
                {
                    SPI3->CR1 &= ~SPI_CR1_SSM;
                    SPI3->CR1 &= ~SPI_CR1_SSI;
                }
            }
#endif

            SPI_Init(SPI3, &spi_inits);

            if( (tHwConf_l->receive_cb) || (tHwConf_l->transmit_cb) || (tHwConf_l->error_cb) )
            {
                NVIC_InitStructure.NVIC_IRQChannel = SPI3_IRQn;
                NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = STM_INTERRUPT_PREEMPTIONPRIORITY_SPI3;
                NVIC_InitStructure.NVIC_IRQChannelSubPriority = STM_INTERRUPT_SUBPRIORITY_SPI3;
                NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

                NVIC_Init(&NVIC_InitStructure);

                cbSpiIntRx_s [SPI3_PORT]  = tHwConf_l->receive_cb;
                cbSpiIntTx_s [SPI3_PORT]  = tHwConf_l->transmit_cb;
                cbSpiIntErr_s[SPI3_PORT]  = tHwConf_l->error_cb;

                if( tHwConf_l->receive_cb )
                {
                    BSP_SPI_ENABLE_RX_INT(SPI3);
                }
                if( tHwConf_l->transmit_cb )
                {
                    BSP_SPI_ENABLE_TX_INT(SPI3);
                }
                if( tHwConf_l->error_cb )
                {
                    BSP_SPI_ENABLE_ERR_INT(SPI3);
                }
            }

            // enable DMA controller
            RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);

            //Enable SPI3
            BSP_SPI_ENABLE(SPI3);

        } break;

    default:
        {
            bspError( BSPE_SPI_PRT_RANGE, bTRUE, 1, i8uPort_p );
        } break;
    }


    spiDMAisPending_s[i8uPort_p]=bFALSE; // reset flag for pending DMA
    
    //-------------------- use Timer TIM4 --------------------
    //UNDONE move to PNOZcom, etc.
    tim4c.i32uTimeBase = 1;
    tim4c.cbTimerExpired = NULL;
    TIM_initCountDownTimer(TIM_TIMER4, &tim4c);


    return (tRv_l);

}



//+=============================================================================================
//|   Function: reset_spi
//+---------------------------------------------------------------------------------------------
//!   Reset Spi.
//!     (detailed description)
//+---------------------------------------------------------------------------------------------
//|   Conditions:
//!   \pre  (pre-condition)
//!   \post (post-condition)
//+---------------------------------------------------------------------------------------------
//|   Annotations:
//+=============================================================================================
void reset_spi( INT8U spi )
{
    volatile INT32U   cr1;
    volatile INT32U   cr2;

    switch(spi)
    {
    case SPI1_PORT:
        {
            NVIC_DisableIRQ(SPI1_IRQn);

            TIM_CountDownReTrigger(TIM_TIMER4,15);
            while( bFALSE == TIM_TimerExpired(TIM_TIMER4));      // Reset all time until PNOZ doesn't transmit any more

            SPI1->CR1 &= ~SPI_CR1_SPE;
            cr1 = SPI1->DR;
            cr1 = SPI1->DR;
            cr1 = SPI1->CR1;
            cr2 = SPI1->CR2;

            (RCC->APB2RSTR) |= (RCC_APB2Periph_SPI1);
            (RCC->APB2RSTR) &= (~RCC_APB2Periph_SPI1);

            SPI1->CR1 = cr1;
            SPI1->CR2 = cr2;
            SPI1->DR = 0x80;  //PNOZ_SPI_COMM_FREE
            cr1 = SPI1->DR;
            SPI1->CR1 |= SPI_CR1_SPE;

            NVIC_EnableIRQ(SPI1_IRQn);

        } break;

    case SPI2_PORT:
        {
            NVIC_DisableIRQ(SPI2_IRQn);

            TIM_CountDownReTrigger(TIM_TIMER4,15);
            while( bFALSE == TIM_TimerExpired(TIM_TIMER4));      // Reset all time until PNOZ doesn't transmit any more

            SPI2->CR1 &= ~SPI_CR1_SPE;
            cr1 = SPI2->DR;
            cr1 = SPI2->DR;
            cr1 = SPI2->CR1;
            cr2 = SPI2->CR2;

            (RCC->APB1RSTR) |= (RCC_APB1Periph_SPI2);
            (RCC->APB1RSTR) &= (~RCC_APB1Periph_SPI2);

            SPI2->CR1 = cr1;
            SPI2->CR2 = cr2;
            SPI2->DR = 0x80;  //PNOZ_SPI_COMM_FREE
            cr1 = SPI2->DR;
            SPI2->CR1 |= SPI_CR1_SPE;

            NVIC_EnableIRQ(SPI2_IRQn);

        } break;

    case SPI3_PORT:
        {
            NVIC_DisableIRQ(SPI3_IRQn);

            TIM_CountDownReTrigger(TIM_TIMER4,15);
            while( bFALSE == TIM_TimerExpired(TIM_TIMER4));      // Reset all time until PNOZ doesn't transmit any more

            SPI3->CR1 &= ~SPI_CR1_SPE;
            cr1 = SPI3->DR;
            cr1 = SPI3->DR;
            cr1 = SPI3->CR1;
            cr2 = SPI3->CR2;

            (RCC->APB1RSTR) |= (RCC_APB1Periph_SPI3);
            (RCC->APB1RSTR) &= (~RCC_APB1Periph_SPI3);

            SPI3->CR1 = cr1;
            SPI3->CR2 = cr2;
            SPI3->DR = 0x80;  //PNOZ_SPI_COMM_FREE
            cr1 = SPI3->DR;
            SPI3->CR1 |= SPI_CR1_SPE;

            NVIC_EnableIRQ(SPI3_IRQn);

        } break;
    default:
        {
            bspError(BSPE_SPI_INV_PRT, bTRUE, 0);
        } break;
    }
}

TBOOL   spi_transceive_done ( 
        INT8U i8uPort_p      ///< [in] SPI Port [0..2] used for transmission
        )
{
    DMA_TypeDef *dma = 0;
    DMA_Stream_TypeDef *dmaRx = 0;
    DMA_Stream_TypeDef *dmaTx = 0;
    INT32U    dmaLFinished, dmaHFinished;

    //select SPI Port to use...
    switch(i8uPort_p)
    {
        //use SPI1
    case SPI1_PORT:
        {
            dmaRx  = DMA2_Stream2;
            dmaTx  = DMA2_Stream3;
            dmaLFinished = DMA_LISR_TCIF2 | DMA_LISR_TCIF3;
            dmaHFinished = 0;
        } break;

        //use SPI2
    case SPI2_PORT:
        {
            dmaRx  = DMA1_Stream3;
            dmaTx  = DMA1_Stream4;
            dmaLFinished = DMA_LISR_TCIF3;
            dmaHFinished = DMA_HISR_TCIF4;
        } break;

        //use SPI3
    case SPI3_PORT:
        {
            dmaRx  = DMA1_Stream0;
            dmaTx  = DMA1_Stream7;
            dmaLFinished = DMA_LISR_TCIF0; 
            dmaHFinished = DMA_HISR_TCIF7;
        } break;

        //error; SPI not avail
    default:
        {
            return bTRUE;
        }
    }

    if ( spiDMAisPending_s[i8uPort_p]
        && 
        (  (dmaRx->CR & DMA_SxCR_EN)
        || (dmaTx->CR & DMA_SxCR_EN)
        )
        && (  (dma->LISR & dmaLFinished) != dmaLFinished
        || (dma->HISR & dmaHFinished) != dmaHFinished
        )
        )
    {
        return bFALSE;
    }

    return bTRUE;
}

//+=============================================================================================
//|   Function: spi_transceive
//+---------------------------------------------------------------------------------------------
//!   read and/or write data from/to spi port
//!   if more than 3 byte must be transferred a DMA will be used
//!   has not been tested for SPI1 and 2 ! (MD 13.5.11)
//+---------------------------------------------------------------------------------------------
//|   Conditions:
//!   \pre  Initialisation of desired port must be finished
//!   \post (post-condition)
//+---------------------------------------------------------------------------------------------
//|   Annotations: Buffers have to remain valid until transmission is finished !
//+=============================================================================================
INT32U spi_transceive (
                       INT8U   i8uPort_p,  ///< [in] SPI Port [0..2] used for transmission
                       INT8U       *tx_p,    ///< [in] bytes to transmit
                       INT8U       *rx_p,    ///< [out]  bytes to receive
                       INT32U      len_p,    ///< [in] length to transmit/receive
                       TBOOL   bBlock_p  ///< [in] wait for transmission end (blocking)
                       )           /// \return success of operation
{

    INT32U    i32uRetVal_l = BSPE_NO_ERROR;
    SPI_TypeDef *ptSPI_l;
    DMA_TypeDef *dma = 0;
    DMA_Stream_TypeDef *dmaRx = 0;
    DMA_Stream_TypeDef *dmaTx = 0;
    INT8U   tmp;
    INT8U   chanRx = 0, chanTx = 0;
    INT32U    dmaLFinished = 0, dmaHFinished = 0, dmaLISRReset = 0, dmaHISRReset = 0;

    if (len_p == 0)
    {
        return BSPE_NO_ERROR;
    }

    //select SPI Port to use...
    switch(i8uPort_p)
    {
        //use SPI1
    case SPI1_PORT:
        {
            ptSPI_l = SPI1;
        } break;

        //use SPI2
    case SPI2_PORT:
        {
            ptSPI_l = SPI2;
        } break;

        //use SPI3
    case SPI3_PORT:
        {
            ptSPI_l = SPI3; 
        } break;

        //error; SPI not avail
    default:
        {
            return BSPE_SPI_PORT_ERR;
        }
    }

    /* the following code has been changed 12.9.2014 by VdH to result in a more universal applicability of this driver 
    *  from now on the driver may be called with mixed length of data resulting in mixed mode transceive oprations (DMA/non DMA)
    *  and it will always work coorect if blocking and non-blocking operations are used consecutively */

    //if (dma && bBlock_p == bFALSE)
    //{
    //    if (  (  (dmaRx->CR & DMA_SxCR_EN)
    //        || (dmaTx->CR & DMA_SxCR_EN)
    //        )
    //        && (  (dma->LISR & dmaLFinished) != dmaLFinished
    //        || (dma->HISR & dmaHFinished) != dmaHFinished
    //        )
    //        )
    //    {
    //        /* transfer is not complete -> abort*/
    //        return BSPE_SPI_BUS_JOB_PENDING;
    //    }

    //    dmaRx->CR &= ~DMA_SxCR_EN;
    //    dmaTx->CR &= ~DMA_SxCR_EN;
    //    ptSPI_l->CR2 &= ~(SPI_I2S_DMAReq_Tx | SPI_I2S_DMAReq_Rx);

    //    if (dmaLISRReset) dma->LIFCR |= dmaLISRReset;
    //    if (dmaHISRReset) dma->HIFCR |= dmaHISRReset;
    //}

     if ( ( (len_p > SPI_DMA_MIN_LEN) || spiDMAisPending_s[i8uPort_p])  
        && (   (tx_p == (INT8U *)0)|| (tx_p >= (INT8U *)0x20000000) ) 
        && (   (rx_p == (INT8U *)0)|| (rx_p >= (INT8U *)0x20000000) )
        )
    {
        //select SPI Port to use...
        switch(i8uPort_p)
        {
            //use SPI1
        case SPI1_PORT:
            {
                dma    = DMA2;        // DMA controller 1
                dmaRx  = DMA2_Stream2;
                chanRx = 3;
                dmaTx  = DMA2_Stream3;
                chanTx = 3;
                dmaLFinished = DMA_LISR_TCIF2 | DMA_LISR_TCIF3;
                dmaLISRReset = DMA_LIFCR_ALL2 | DMA_LIFCR_ALL3;
            } break;

            //use SPI2
        case SPI2_PORT:
            {
                dma    = DMA1;        // DMA controller 1
                dmaRx  = DMA1_Stream3;
                dmaTx  = DMA1_Stream4;
                dmaLFinished = DMA_LISR_TCIF3;
                dmaHFinished = DMA_HISR_TCIF4;
                dmaLISRReset = DMA_LIFCR_ALL3;
                dmaHISRReset = DMA_HIFCR_ALL4;
            } break;

            //use SPI3
        case SPI3_PORT:
            {
                dma    = DMA1;        // DMA controller 1
                dmaRx  = DMA1_Stream0;
                dmaTx  = DMA1_Stream7;
                dmaLFinished = DMA_LISR_TCIF0; 
                dmaHFinished = DMA_HISR_TCIF7;
                dmaLISRReset = DMA_LIFCR_ALL0;
                dmaHISRReset = DMA_HIFCR_ALL7;
            } break;

            //error; SPI not avail
        default:
            {
                return BSPE_SPI_PORT_ERR;
            }
        }
    }

    // wait for DMA to be completed if it is pending. Then reset DMA in order to make it available again
    // if this blocking character of the driver is not adequate you must simply call spi_transceive_done()
    // prior to call this trnasceive function in orde to check for any pending DMA
    // This check and reset of flags must not be dependent on current SPI transceive data length but only on 
    // any prior DMA activity indicated by spiDMAisPending_s[] flag!

    if (spiDMAisPending_s[i8uPort_p])
    {
        while ( (  (dmaRx->CR & DMA_SxCR_EN)
                    || (dmaTx->CR & DMA_SxCR_EN)
                )
                && 
                (  (dma->LISR & dmaLFinished) != dmaLFinished
                    || (dma->HISR & dmaHFinished) != dmaHFinished
                )
            );

            dmaRx->CR &= ~DMA_SxCR_EN;
            dmaTx->CR &= ~DMA_SxCR_EN;
            ptSPI_l->CR2 &= ~(SPI_I2S_DMAReq_Tx | SPI_I2S_DMAReq_Rx);

            if (dmaLISRReset) dma->LIFCR |= dmaLISRReset;
            if (dmaHISRReset) dma->HIFCR |= dmaHISRReset; 
            spiDMAisPending_s[i8uPort_p]=bFALSE; // reset flag for pending DMA   
    }
    
    //empty SPI Data register
    tmp = ptSPI_l->DR;

    if (dma)  // use DMA
    {
        dmaRx->CR &= ~DMA_SxCR_EN;
        dmaTx->CR &= ~DMA_SxCR_EN;

        if (dmaLISRReset) dma->LIFCR |= dmaLISRReset;
        if (dmaHISRReset) dma->HIFCR |= dmaHISRReset;
        dmaRx->PAR = (uint32_t)&ptSPI_l->DR;
        dmaRx->NDTR = len_p;
        dmaTx->PAR = (uint32_t)&ptSPI_l->DR;
        dmaTx->NDTR = len_p;
        if (rx_p)
        {
            dmaRx->M0AR = (uint32_t)rx_p;
            dmaRx->CR = (chanRx << 25)
                | DMA_SxCR_PL_1
                | DMA_SxCR_MINC;
        }
        else
        {
            dmaRx->M0AR = (uint32_t)&tmp;
            dmaRx->CR = (chanRx << 25)
                | DMA_SxCR_PL_1;
        }

        if (tx_p)
        {
            dmaTx->M0AR = (uint32_t)tx_p;
            dmaTx->CR = (chanTx << 25)
                | DMA_SxCR_PL_0
                | DMA_SxCR_DIR_0
                | DMA_SxCR_MINC;
        }
        else
        {
            dmaTx->M0AR = (uint32_t)&tmp;
            dmaTx->CR = (chanTx << 25)
                | DMA_SxCR_PL_0
                | DMA_SxCR_DIR_0;
            tmp = 0;
        }

        /* Enable SPI_MASTER DMA Tx/Rx request */
        ptSPI_l->CR2 |= SPI_I2S_DMAReq_Tx | SPI_I2S_DMAReq_Rx;

        dmaRx->CR |= DMA_SxCR_EN;
        dmaTx->CR |= DMA_SxCR_EN;
        // set flag to DMA is pending => if non blocking mode is chosen the flag can be used
        // together with the DMA-ISR-registers TCIF flag to conroll any state machine of the calling routine
        spiDMAisPending_s[i8uPort_p]=bTRUE;

        if(bBlock_p)
        {
            while ( (dma->LISR & dmaLFinished) != dmaLFinished ||
                (dma->HISR & dmaHFinished) != dmaHFinished  )
            {
                /* wait for transfer complete */  
            }

            dmaRx->CR &= ~DMA_SxCR_EN;
            dmaTx->CR &= ~DMA_SxCR_EN;
            ptSPI_l->CR2 &= ~(SPI_I2S_DMAReq_Tx | SPI_I2S_DMAReq_Rx);

            if (dmaLISRReset) dma->LIFCR |= dmaLISRReset;
            if (dmaHISRReset) dma->HIFCR |= dmaHISRReset;
            spiDMAisPending_s[i8uPort_p]=bFALSE; // reset flag for pending DMA
        }
    }
    else
    {
        while( len_p-- > 0 )
        {
            while ((ptSPI_l->SR & SPI_SR_TXE) == 0);
            if (tx_p)
            {
                // read data from given pointer
                tmp = *tx_p++;
            }
            ptSPI_l->DR = tmp;

            while ((ptSPI_l->SR & SPI_SR_RXNE) == 0);
            tmp = ptSPI_l->DR;
            if (rx_p)
            { 
                // store received data to given pointer
                *rx_p++ = tmp;
            }
        }
    }
    return  i32uRetVal_l;
}

//+=============================================================================================
//|   Function: spi_slave_transceive_init
//+---------------------------------------------------------------------------------------------
//!   initialize reading and/or writing data from/to spi port
//+---------------------------------------------------------------------------------------------
//|   Conditions:
//!   \pre  Initialisation of desired port must be finished
//!   \post (post-condition)
//+---------------------------------------------------------------------------------------------
//|   Annotations: Buffers have to remain valid until transmission is finished !
//+=============================================================================================
void spi_slave_transceive_init (
                     INT8U   i8uPort_p,  ///< [in] SPI Port [0..2] used for transmission
                     INT8U       *tx_p,    ///< [in] bytes to transmit
                     INT8U       *rx_p,    ///< [out]  bytes to receive
                     INT32U      len_p ///< [in] wait for transmission end (blocking)
                     )
{
    SPI_TypeDef *ptSPI_l = 0;
    DMA_Stream_TypeDef *dmaRx = 0;
    DMA_Stream_TypeDef *dmaTx = 0;
    static INT8U   dmaRxM0AR[BSP_SPI_MAXPORT];
    static INT8U   dmaTxM0AR[BSP_SPI_MAXPORT];
    INT8U   chanRx = 0, chanTx = 0;

    //select SPI Port to use...
    switch(i8uPort_p)
    {
        //use SPI1
        case SPI1_PORT:
        {
            ptSPI_l = SPI1;
            dmaRx  = DMA2_Stream2;
            chanRx = 3;
            dmaTx  = DMA2_Stream3;
            chanTx = 3;
        } break;

        //use SPI2
        case SPI2_PORT:
        {
            ptSPI_l = SPI2;
            dmaRx  = DMA1_Stream3;
            dmaTx  = DMA1_Stream4;
        } break;

        //use SPI3
        case SPI3_PORT:
        {
            ptSPI_l = SPI3; 
            dmaRx  = DMA1_Stream0;
            dmaTx  = DMA1_Stream7;
        } break;

        //error; SPI not avail
        default:
        {
            bspError(BSPE_SPI_INV_PRT, bTRUE, 0);
        } break;
    }

    if (   (dmaRx->CR & DMA_SxCR_EN)
        || (dmaTx->CR & DMA_SxCR_EN)
       )
    {
        /* previous transfer is not complete -> abort*/
        bspError(BSPE_SPI_BUS_JOB_PENDING, bTRUE, 0);
    }

    dmaRx->PAR = (uint32_t)&ptSPI_l->DR;
    dmaRx->NDTR = len_p;
    dmaTx->PAR = (uint32_t)&ptSPI_l->DR;
    dmaTx->NDTR = len_p;
    if (rx_p)
    {
        dmaRx->M0AR = (uint32_t)rx_p;
        dmaRx->CR = (chanRx << 25)
            | DMA_SxCR_PL_1
            | DMA_SxCR_MINC;
    }
    else
    {
        dmaRx->M0AR = (uint32_t)&dmaRxM0AR[i8uPort_p];
        dmaRx->CR = (chanRx << 25)
            | DMA_SxCR_PL_1;
    }

    if (tx_p)
    {
        dmaTx->M0AR = (uint32_t)tx_p;
        dmaTx->CR = (chanTx << 25)
            | DMA_SxCR_PL_0
            | DMA_SxCR_DIR_0
            | DMA_SxCR_MINC;
    }
    else
    {
        dmaTx->M0AR = (uint32_t)&dmaTxM0AR[i8uPort_p];
        dmaTx->CR = (chanTx << 25)
            | DMA_SxCR_PL_0
            | DMA_SxCR_DIR_0;
        dmaTxM0AR[i8uPort_p] = 0;
    }

    dmaRx->CR |= DMA_SxCR_EN;
    dmaTx->CR |= DMA_SxCR_EN;

    /* Enable SPI DMA Tx/Rx request */
    ptSPI_l->CR2 |= SPI_I2S_DMAReq_Tx | SPI_I2S_DMAReq_Rx;
}

//+=============================================================================================
//|   Function: spi_slave_transceive_done
//+---------------------------------------------------------------------------------------------
//!   check whether DMA transmission is completed
//+---------------------------------------------------------------------------------------------
//|   Conditions:
//!   \pre  Initialisation of desired port must be finished
//!   \post (post-condition)
//+---------------------------------------------------------------------------------------------
//|   Annotations: Buffers have to remain valid until transmission is finished !
//+=============================================================================================
TBOOL   spi_slave_transceive_done ( 
                                   INT8U i8uPort_p,      ///< [in] SPI Port [0..2] used for transmission
                                   TBOOL bStopWaiting_p  ///< [in] stop waiting for transmission end
                                   )
{
    TBOOL bResult = bTRUE;
    SPI_TypeDef *ptSPI_l = 0;
    DMA_TypeDef *dma = 0;
    DMA_Stream_TypeDef *dmaRx = 0;
    DMA_Stream_TypeDef *dmaTx = 0;
    INT32U    dmaLFinished = 0, dmaHFinished = 0, dmaLISRReset = 0, dmaHISRReset = 0;

    //select SPI Port to use...
    switch(i8uPort_p)
    {
        //use SPI1
    case SPI1_PORT:
        {
            ptSPI_l = SPI1;
            dma    = DMA2;        // DMA controller 2
            dmaRx  = DMA2_Stream2;
            dmaTx  = DMA2_Stream3;
            dmaLFinished = DMA_LISR_TCIF2 | DMA_LISR_TCIF3;
            dmaLISRReset = DMA_LIFCR_ALL2 | DMA_LIFCR_ALL3;
        } break;

        //use SPI2
    case SPI2_PORT:
        {
            ptSPI_l = SPI2;
            dma    = DMA1;        // DMA controller 1
            dmaRx  = DMA1_Stream3;
            dmaTx  = DMA1_Stream4;
            dmaLFinished = DMA_LISR_TCIF3;
            dmaHFinished = DMA_HISR_TCIF4;
            dmaLISRReset = DMA_LIFCR_ALL3;
            dmaHISRReset = DMA_HIFCR_ALL4;
        } break;

        //use SPI3
    case SPI3_PORT:
        {
            ptSPI_l = SPI3;
            dma    = DMA1;        // DMA controller 1
            dmaRx  = DMA1_Stream0;
            dmaTx  = DMA1_Stream7;
            dmaLFinished = DMA_LISR_TCIF0; 
            dmaHFinished = DMA_HISR_TCIF7;
            dmaLISRReset = DMA_LIFCR_ALL0;
            dmaHISRReset = DMA_HIFCR_ALL7;
        } break;

        //error; SPI not avail
    default:
        {
            bspError(BSPE_SPI_INV_PRT, bTRUE, 0);
        } break;
    }

    if (   ((dma->LISR & dmaLFinished) != dmaLFinished)
        || ((dma->HISR & dmaHFinished) != dmaHFinished)
       )
    {
        if (bStopWaiting_p == bTRUE)
        {
            bResult = bFALSE;

            dmaRx->CR &= ~DMA_SxCR_EN;
            dmaTx->CR &= ~DMA_SxCR_EN;

            while((dmaRx->CR & DMA_SxCR_EN) || (dmaTx->CR & DMA_SxCR_EN));
        }
        else
        {
            return bFALSE;
        }
    }

    if (dmaLISRReset) dma->LIFCR |= dmaLISRReset;
    if (dmaHISRReset) dma->HIFCR |= dmaHISRReset;

    ptSPI_l->CR2 &= ~(SPI_I2S_DMAReq_Tx | SPI_I2S_DMAReq_Rx);

    return bResult;
}

//+=============================================================================================
//|   Function: reset_spi_slave
//+---------------------------------------------------------------------------------------------
//!   Reset Spi slave.
//!     (detailed description)
//+---------------------------------------------------------------------------------------------
//|   Conditions:
//!   \pre  (pre-condition)
//!   \post (post-condition)
//+---------------------------------------------------------------------------------------------
//|   Annotations:
//+=============================================================================================
void reset_spi_slave( INT8U spi )
{
    volatile INT32U   cr1;
    volatile INT32U   cr2;

    switch(spi)
    {
    case SPI1_PORT:
        {
            SPI1->CR1 &= ~SPI_CR1_SPE;
            cr1 = SPI1->CR1;
            cr2 = SPI1->CR2;

            (RCC->APB2RSTR) |= (RCC_APB2Periph_SPI1);
            (RCC->APB2RSTR) &= (~RCC_APB2Periph_SPI1);

            SPI1->CR1 = cr1;
            SPI1->CR2 = cr2;
            SPI1->CR1 |= SPI_CR1_SPE;
        } break;

    case SPI2_PORT:
        {
            SPI2->CR1 &= ~SPI_CR1_SPE;
            cr1 = SPI2->CR1;
            cr2 = SPI2->CR2;

            (RCC->APB1RSTR) |= (RCC_APB1Periph_SPI2);
            (RCC->APB1RSTR) &= (~RCC_APB1Periph_SPI2);

            SPI2->CR1 = cr1;
            SPI2->CR2 = cr2;
            SPI2->CR1 |= SPI_CR1_SPE;
        } break;

    case SPI3_PORT:
        {
            SPI3->CR1 &= ~SPI_CR1_SPE;
            cr1 = SPI3->CR1;
            cr2 = SPI3->CR2;

            (RCC->APB1RSTR) |= (RCC_APB1Periph_SPI3);
            (RCC->APB1RSTR) &= (~RCC_APB1Periph_SPI3);

            SPI3->CR1 = cr1;
            SPI3->CR2 = cr2;
            SPI3->CR1 |= SPI_CR1_SPE;
        } break;
    default:
        {
            bspError(BSPE_SPI_INV_PRT, bTRUE, 0);
        } break;
    }
}

//+=============================================================================================
//|   Function: SPI_Spi1IntHandler
//+---------------------------------------------------------------------------------------------
//!   Coretex specific generic SPI interrupt handler
//+---------------------------------------------------------------------------------------------
//|   Conditions:
//!   \pre  SPI interrupt occured
//!   \post (post-condition)
//+---------------------------------------------------------------------------------------------
//|   Annotations:
//+=============================================================================================
void SPI_Spi1IntHandler (void)
{
    //---------- SPI ERR Interrupt ----------
    if( (SPI1->CR2 & SPI_CR2_ERRIE ) && ((SPI1->SR & SPI_SR_MODF) || (SPI1->SR & SPI_SR_OVR)) )
    {
        if (cbSpiIntErr_s[0])
        {
            cbSpiIntErr_s[0] ();
        }
        else
        {
            BSP_SPI_DISABLE_ERR_INT(SPI1);
        }
    }
    else
    {
        //---------- SPI TX Interrupt ----------
        if ( (SPI1->CR2 & SPI_CR2_TXEIE) && (SPI1->SR & SPI_SR_TXE))
        {
            if (cbSpiIntTx_s[0])
            {
                cbSpiIntTx_s[0] ();
            }
            else
            {
                BSP_SPI_DISABLE_TX_INT(SPI1);
            }
        }
        //---------- SPI RX Interrupt ----------
        if( (SPI1->CR2 & SPI_CR2_RXNEIE) && (SPI1->SR & SPI_SR_RXNE) )
        {
            if (cbSpiIntRx_s[0])
            {
                cbSpiIntRx_s[0] ();    
            }
            else
            {
                BSP_SPI_DISABLE_RX_INT(SPI1);
            }
        }
    }
}


//+=============================================================================================
//|   Function: SPI_Spi2IntHandler
//+---------------------------------------------------------------------------------------------
//!   Coretex specific generic SPI interrupt handler
//+---------------------------------------------------------------------------------------------
//|   Conditions:
//!   \pre  SPI interrupt occured
//!   \post (post-condition)
//+---------------------------------------------------------------------------------------------
//|   Annotations:
//+=============================================================================================
void SPI_Spi2IntHandler (void)
{
    //---------- SPI ERR Interrupt ----------
    if( (SPI2->CR2 & SPI_CR2_ERRIE ) && ((SPI2->SR & SPI_SR_MODF) || (SPI2->SR & SPI_SR_OVR)) )
    {
        if (cbSpiIntErr_s[1])
        {
            cbSpiIntErr_s[1] ();    
        }
        else
        {
            BSP_SPI_DISABLE_ERR_INT(SPI2);
        }
    }
    else
    {
        //---------- SPI TX Interrupt ----------
        if ( (SPI2->CR2 & SPI_CR2_TXEIE) && (SPI2->SR & SPI_SR_TXE))
        {
            if (cbSpiIntTx_s[1])
            {
                cbSpiIntTx_s[1] ();
            }
            else
            {
                BSP_SPI_DISABLE_TX_INT(SPI2);
            }
        }
        //---------- SPI RX Interrupt ----------
        if( (SPI2->CR2 & SPI_CR2_RXNEIE) && (SPI2->SR & SPI_SR_RXNE) )
        {
            if (cbSpiIntRx_s[1])
            {
                cbSpiIntRx_s[1] ();    
            }
            else
            {
                BSP_SPI_DISABLE_RX_INT(SPI2);
            }
        }
    }
}



//+=============================================================================================
//|   Function: SPI_Spi3IntHandler
//+---------------------------------------------------------------------------------------------
//!   Coretex specific generic SPI interrupt handler
//+---------------------------------------------------------------------------------------------
//|   Conditions:
//!   \pre  SPI interrupt occured
//!   \post (post-condition)
//+---------------------------------------------------------------------------------------------
//|   Annotations:
//+=============================================================================================
void SPI_Spi3IntHandler (void)
{
    //---------- SPI ERR Interrupt ----------
    if( (SPI3->CR2 & SPI_CR2_ERRIE ) && ((SPI3->SR & SPI_SR_MODF) || (SPI3->SR & SPI_SR_OVR)) )
    {
        if (cbSpiIntErr_s[2])
        {
            cbSpiIntErr_s[2] ();    
        }
        else
        {
            BSP_SPI_DISABLE_ERR_INT(SPI3);
        }
    }
    else
    {
        //---------- SPI TX Interrupt ----------
        if ( (SPI3->CR2 & SPI_CR2_TXEIE) && (SPI3->SR & SPI_SR_TXE))
        {
            if (cbSpiIntTx_s[2])
            {
                cbSpiIntTx_s[2] ();
            }
            else
            {
                BSP_SPI_DISABLE_TX_INT(SPI3);
            }
        }
        //---------- SPI RX Interrupt ----------
        if( (SPI3->CR2 & SPI_CR2_RXNEIE) && (SPI3->SR & SPI_SR_RXNE) )
        {
            if (cbSpiIntRx_s[2])
            {
                cbSpiIntRx_s[2] ();    
            }
            else
            {
                BSP_SPI_DISABLE_RX_INT(SPI3);
            }
        }
    }
}

//*************************************************************************************************
//| Function: BSP_SPI_RWPERI_init
//|
//! \brief
//!
//! 
//!
//!
//!
//! \ingroup
//-------------------------------------------------------------------------------------------------
void BSP_SPI_RWPERI_init (
    INT8U i8uPort_p, 
    const HW_SPI_CONFIGURATION *ptHwConf_p, 
    BSP_SPI_TRwPeriData *ptRwPeriData_p)
    
{
    INT32U i32uRv_l;
    KB_GPIO_InitTypeDef tGpioInit_l;
    
    i32uRv_l = spi_init (i8uPort_p, ptHwConf_p);
    if (i32uRv_l != BSPE_NO_ERROR)
    {
        bspError (i32uRv_l, bTRUE, 0);
    }

#if (BSP_SPI_MAXPORT != 3)
    #error "BSP_SPI_MAXPORT anpassen"
#endif    
    
    switch (i8uPort_p)
    {
        case SPI1_PORT: //SPI1
        {
            ptRwPeriData_p->vpSpi = SPI1;
            ptRwPeriData_p->i16uCR1 = SPI1->CR1;
            ptRwPeriData_p->i16uCR2 = SPI1->CR2;
            
#if defined (STM_SPI1_REMAP)

            ptRwPeriData_p->vpSckPort = GPIOB;          // SCK
            ptRwPeriData_p->i16uSckPin = GPIO_Pin_3;

            ptRwPeriData_p->vpMosiPort = GPIOB;          // MOSI
            ptRwPeriData_p->i16uMosiPin = GPIO_Pin_5;

            ptRwPeriData_p->vpMisoPort = GPIOB;          // MISO
            ptRwPeriData_p->i16uMisoPin = GPIO_Pin_4;

#elif defined (STM_SPI1_REMAP2)

            ptRwPeriData_p->vpSckPort = GPIOA;          // SCK
            ptRwPeriData_p->i16uSckPin = GPIO_Pin_5;

            ptRwPeriData_p->vpMosiPort = GPIOB;          // MOSI
            ptRwPeriData_p->i16uMosiPin = GPIO_Pin_5;

            ptRwPeriData_p->vpMisoPort = GPIOA;          // MISO
            ptRwPeriData_p->i16uMisoPin = GPIO_Pin_6;

#else

            ptRwPeriData_p->vpSckPort = GPIOA;          // SCK
            ptRwPeriData_p->i16uSckPin = GPIO_Pin_5;

            ptRwPeriData_p->vpMosiPort = GPIOA;          // MOSI
            ptRwPeriData_p->i16uMosiPin = GPIO_Pin_7;

            ptRwPeriData_p->vpMisoPort = GPIOA;          // MISO
            ptRwPeriData_p->i16uMisoPin = GPIO_Pin_6;

#endif
        }   break;
        case SPI2_PORT: //SPI2
        {
            ptRwPeriData_p->vpSpi = SPI2;
            ptRwPeriData_p->i16uCR1 = SPI2->CR1;
            ptRwPeriData_p->i16uCR2 = SPI2->CR2;
            
#if defined (STM_SPI2_REMAP)

            ptRwPeriData_p->vpSckPort = GPIOI;          // SCK
            ptRwPeriData_p->i16uSckPin = GPIO_Pin_1;

            ptRwPeriData_p->vpMosiPort = GPIOI;          // MOSI
            ptRwPeriData_p->i16uMosiPin = GPIO_Pin_3;

            ptRwPeriData_p->vpMisoPort = GPIOI;          // MISO
            ptRwPeriData_p->i16uMisoPin = GPIO_Pin_2;

#elif defined (STM_SPI2_REMAP2)

            ptRwPeriData_p->vpSckPort = GPIOB;          // SCK
            ptRwPeriData_p->i16uSckPin = GPIO_Pin_10;

            ptRwPeriData_p->vpMosiPort = GPIOB;          // MOSI
            ptRwPeriData_p->i16uMosiPin = GPIO_Pin_15;

            ptRwPeriData_p->vpMisoPort = GPIOB;          // MISO
            ptRwPeriData_p->i16uMisoPin = GPIO_Pin_14;

#elif defined (STM_SPI2_REMAP3)

            ptRwPeriData_p->vpSckPort = GPIOB;          // SCK
            ptRwPeriData_p->i16uSckPin = GPIO_Pin_10;

            ptRwPeriData_p->vpMosiPort = GPIOC;          // MOSI
            ptRwPeriData_p->i16uMosiPin = GPIO_Pin_3;

            ptRwPeriData_p->vpMisoPort = GPIOC;          // MISO
            ptRwPeriData_p->i16uMisoPin = GPIO_Pin_2;

#else

            ptRwPeriData_p->vpSckPort = GPIOB;          // SCK
            ptRwPeriData_p->i16uSckPin = GPIO_Pin_13;

            ptRwPeriData_p->vpMosiPort = GPIOB;          // MOSI
            ptRwPeriData_p->i16uMosiPin = GPIO_Pin_15;

            ptRwPeriData_p->vpMisoPort = GPIOB;          // MISO
            ptRwPeriData_p->i16uMisoPin = GPIO_Pin_14;

#endif
        }   break;
        case SPI3_PORT: //SPI3
        {
            ptRwPeriData_p->vpSpi = SPI3;
            ptRwPeriData_p->i16uCR1 = SPI3->CR1;
            ptRwPeriData_p->i16uCR2 = SPI3->CR2;
            
#if defined (STM_SPI3_REMAP)

            ptRwPeriData_p->vpSckPort = GPIOC;          // SCK
            ptRwPeriData_p->i16uSckPin = GPIO_Pin_10;

            ptRwPeriData_p->vpMosiPort = GPIOC;          // MOSI
            ptRwPeriData_p->i16uMosiPin = GPIO_Pin_12;

            ptRwPeriData_p->vpMisoPort = GPIOC;          // MISO
            ptRwPeriData_p->i16uMisoPin = GPIO_Pin_11;

#else

            ptRwPeriData_p->vpSckPort = GPIOB;          // SCK
            ptRwPeriData_p->i16uSckPin = GPIO_Pin_3;

            ptRwPeriData_p->vpMosiPort = GPIOB;          // MOSI
            ptRwPeriData_p->i16uMosiPin = GPIO_Pin_5;

            ptRwPeriData_p->vpMisoPort = GPIOB;          // MISO
            ptRwPeriData_p->i16uMisoPin = GPIO_Pin_4;

#endif
        }   break;
        default:
        {
        }   break;
    }
 
    if (ptRwPeriData_p->vpCsPort != NULL)
    {
        tGpioInit_l.GPIO_Pin = ptRwPeriData_p->i16uCsPin;
        tGpioInit_l.GPIO_Mode = GPIO_Mode_Out_PP;
        tGpioInit_l.GPIO_Speed = GPIO_Speed_50MHz;
        kbGPIO_InitCLK ((GPIO_TypeDef *)ptRwPeriData_p->vpCsPort, &tGpioInit_l);
    }
}

//*************************************************************************************************
//| Function: BSP_SPI_RWPERI_chipSelectEnable
//|
//! \brief
//!
//! 
//!
//!
//!
//! \ingroup
//-------------------------------------------------------------------------------------------------
void BSP_SPI_RWPERI_chipSelectEnable (
    BSP_SPI_TRwPeriData *ptRwPeriData_p)
    
{
    if (ptRwPeriData_p->vpCsPort == NULL)
        return;

    GPIO_TypeDef *ptGpio_l = (GPIO_TypeDef *)ptRwPeriData_p->vpCsPort;
    INT16U i16uPin_l = ptRwPeriData_p->i16uCsPin;

    if (ptRwPeriData_p->bActiveHigh == bTRUE)
    {
        GPIO_SetBits(ptGpio_l, i16uPin_l);
        while (GPIO_ReadInputDataBit (ptGpio_l, i16uPin_l) == Bit_RESET)
        {
        
        }
    }
    else
    {
        GPIO_ResetBits (ptGpio_l, i16uPin_l);
        while (GPIO_ReadInputDataBit (ptGpio_l, i16uPin_l) == Bit_SET)
        {
        
        }
    }
}    

//*************************************************************************************************
//| Function: BSP_SPI_RWPERI_chipSelectDisable
//|
//! \brief
//!
//! 
//!
//!
//!
//! \ingroup
//-------------------------------------------------------------------------------------------------
void BSP_SPI_RWPERI_chipSelectDisable (
    BSP_SPI_TRwPeriData *ptRwPeriData_p)
    
{
    if (ptRwPeriData_p->vpCsPort == NULL)
        return;

    GPIO_TypeDef *ptGpio_l = (GPIO_TypeDef *)ptRwPeriData_p->vpCsPort;
    INT16U i16uPin_l = ptRwPeriData_p->i16uCsPin;

    if (ptRwPeriData_p->bActiveHigh == bTRUE)
    {
        GPIO_ResetBits (ptGpio_l, i16uPin_l);
        while (GPIO_ReadInputDataBit (ptGpio_l, i16uPin_l) == Bit_SET)
        {
        
        }
    }
    else
    {
        GPIO_SetBits(ptGpio_l, i16uPin_l);
        while (GPIO_ReadInputDataBit (ptGpio_l, i16uPin_l) == Bit_RESET)
        {
        
        }
    }
    DELAY_80NS;
}    

//*************************************************************************************************
//| Function: BSP_SPI_RWPERI_prepareSpi
//|
//! \brief
//!
//! 
//!
//!
//!
//! \ingroup
//-------------------------------------------------------------------------------------------------
void BSP_SPI_RWPERI_prepareSpi (
    BSP_SPI_TRwPeriData *ptRwPeriData_p)
    
{
    
    SPI_TypeDef *ptSpi_l = (SPI_TypeDef *)ptRwPeriData_p->vpSpi;
    
    ptSpi_l->CR1 = ptRwPeriData_p->i16uCR1;
    ptSpi_l->CR2 = ptRwPeriData_p->i16uCR2;
}    

//*************************************************************************************************
//| Function: BSP_SPI_RWPERI_spiDisable
//|
//! disables the SPI and switches the corresponding PINs to GPIOs
//!
//!
//!
//!
//! \ingroup
//-------------------------------------------------------------------------------------------------
void BSP_SPI_RWPERI_spiDisable (
    BSP_SPI_TRwPeriData *ptRwPeriData_p)
    
{

    GPIO_InitTypeDef tGpioInit_l;


    tGpioInit_l.GPIO_Mode  = GPIO_Mode_OUT;
    tGpioInit_l.GPIO_Speed = GPIO_Speed_50MHz;
    tGpioInit_l.GPIO_OType = GPIO_OType_PP;
    tGpioInit_l.GPIO_PuPd  = GPIO_PuPd_DOWN;

    // SCK
    GPIO_ResetBits ((GPIO_TypeDef *)ptRwPeriData_p->vpSckPort, ptRwPeriData_p->i16uSckPin);
    tGpioInit_l.GPIO_Pin = ptRwPeriData_p->i16uSckPin;
    GPIO_Init ((GPIO_TypeDef *)ptRwPeriData_p->vpSckPort, &tGpioInit_l);

    // MOSI
    GPIO_ResetBits ((GPIO_TypeDef *)ptRwPeriData_p->vpMosiPort, ptRwPeriData_p->i16uMosiPin);
    tGpioInit_l.GPIO_Pin = ptRwPeriData_p->i16uMosiPin;
    GPIO_Init ((GPIO_TypeDef *)ptRwPeriData_p->vpMosiPort, &tGpioInit_l);

    tGpioInit_l.GPIO_Mode  = GPIO_Mode_IN;
    tGpioInit_l.GPIO_Speed = GPIO_Speed_50MHz;

    // MISO
    tGpioInit_l.GPIO_Pin = ptRwPeriData_p->i16uMisoPin;
    GPIO_Init ((GPIO_TypeDef *)ptRwPeriData_p->vpMisoPort, &tGpioInit_l);


}    

//*************************************************************************************************
//| Function: BSP_SPI_RWPERI_spiEnable
//|
//! \brief
//!
//! 
//!
//!
//!
//! \ingroup
//-------------------------------------------------------------------------------------------------
void BSP_SPI_RWPERI_spiEnable (
    BSP_SPI_TRwPeriData *ptRwPeriData_p)
    
{
    GPIO_InitTypeDef tGpioInit_l;

    tGpioInit_l.GPIO_Mode  = GPIO_Mode_AF;
    tGpioInit_l.GPIO_Speed = GPIO_Speed_50MHz;
    tGpioInit_l.GPIO_OType = GPIO_OType_PP;
    tGpioInit_l.GPIO_PuPd  = GPIO_PuPd_NOPULL;

    // SCK
    tGpioInit_l.GPIO_Pin = ptRwPeriData_p->i16uSckPin;
    GPIO_Init ((GPIO_TypeDef *)ptRwPeriData_p->vpSckPort, &tGpioInit_l);

    // MOSI
    tGpioInit_l.GPIO_Pin = ptRwPeriData_p->i16uMosiPin;
    GPIO_Init ((GPIO_TypeDef *)ptRwPeriData_p->vpMosiPort, &tGpioInit_l);

    // MISO
    tGpioInit_l.GPIO_Pin = ptRwPeriData_p->i16uMisoPin;
    GPIO_Init ((GPIO_TypeDef *)ptRwPeriData_p->vpMisoPort, &tGpioInit_l);
}    


//+=============================================================================================
//|     Aenderungsjournal
//+=============================================================================================
#ifdef __DOCGEN
/*!
@page revisions Revisions

*/
#endif
