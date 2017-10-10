/*=============================================================================================
*
* uart.c
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
//|		Include-Dateien / include files
//+=============================================================================================
#include <common_define.h>
#include <project.h>
#include <bsp\bspError.h>
#include <bsp\bspConfig.h>
#include <bsp\clock\clock.h>

#include <bsp\uart\uart.h>
#include <bsp\uart\uart_intern.h>

#include <SysLib\inc\stm32f2xx_usart.h>
#include <SysLib\inc\misc.h>
#include <SysLib\inc\stm32f2xx.h>
#include <SysLib\inc\stm32f2xx_rcc.h>
#include <bsp\gpio\gpio.h>

/*!< USART CR1 register clear Mask ((~(uint16_t)0xE9F3)) */
#define CR1_CLEAR_MASK            ((uint16_t)(USART_CR1_M  | USART_CR1_PCE | \
    USART_CR1_PS | USART_CR1_TE | \
    USART_CR1_RE | USART_CR1_OVER8))
// with OVER8==0 lower bitrates are possible
#define CR1_OVER8		0		// set OVER8 to 0


//+=============================================================================================
//|     Makros / macros
//+=============================================================================================
#ifndef STM_WITH_UART
#error "Uart cannot be used without STM_WITH_UART to be defined !"
#endif


//+=============================================================================================
//|     Konstanten / constants
//+=============================================================================================
#define BSP_UART_MAXPORT	6


//+=============================================================================================
//|     Prototypen / prototypes
//+=============================================================================================
extern void _fault_handler (void);


static void (*cbReceive_s[BSP_UART_MAXPORT])(INT8U i8uChar_p);             ///< Receive callback
static TBOOL (*cbTransmit_s[BSP_UART_MAXPORT])(void);                       ///< Transmit callback
static void (*cbError_s[BSP_UART_MAXPORT])(UART_ERecError enRecError_p);   ///< Error callback
static void UART_genericIntHandler (INT8U i8uPort,
                             USART_TypeDef *pUART,
                             GPIO_TypeDef *pRS485Port,
                             INT16U i16uR485Pin);

//+=============================================================================================
//|		Globale Variablen / global variables
//+=============================================================================================

static const INT8U		*api8uData_s	[BSP_UART_MAXPORT];
static INT16U		        ai16uDataLen_s	[BSP_UART_MAXPORT];
static UART_EBufferSendState	*apenState_s	[BSP_UART_MAXPORT];
static INT16U			ai16uActIndex_s	[BSP_UART_MAXPORT];

#ifdef DMX_UART_PORT

#define DMX_DATA_BAUDRATE 250000	// baudrate during transmission of data frames

static INT16U i16u_DMX_BRR_DATA_g;  // UART BRR register value for DMX data
static INT16U i16u_DMX_BRR_BREAK_g; // UART BRR register value for DMX break

#endif  // DMX_UART_PORT


//*************************************************************************************************
//| Function: UART_init
//|
//! brief.
//!
//! detailed.
//!
//!
//!
//! ingroup.
//-------------------------------------------------------------------------------------------------
INT32U UART_init (
                  INT8U i8uPort_p, 
                  const UART_TConfig *tConf_p)

{
    INT32U 				i32uRv_l = BSPE_NO_ERROR;
    INT16U 				i16uTmpReg_l;

    USART_TypeDef 		*ptUart_l;
    NVIC_InitTypeDef	tNvic_l;
    GPIO_InitTypeDef	tGpio_l;

#if (BSP_UART_MAXPORT != 6)
#error "BSP_UART_MAXPORT anpassen"
#endif    

    INT32U apbclock = 0x00;
    INT32U integerdivider = 0x00;
    INT32U fractionaldivider = 0x00;
    INT32U baudrate = 0;
    RCC_ClocksTypeDef RCC_ClocksStatus;

    // RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    tGpio_l.GPIO_Speed = GPIO_Speed_50MHz;
    tGpio_l.GPIO_Mode = GPIO_Mode_AF;
    tGpio_l.GPIO_OType = GPIO_OType_PP;
    tGpio_l.GPIO_PuPd = GPIO_PuPd_NOPULL;

    switch (i8uPort_p)
    {
    case UART_PORT_1:
        {	//-------------------- USART1 --------------------

            //Provide USART1 peripheral clock
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

#ifndef STM_UART_DISABLE_PERIPH_RESET
            // Enable USART1 reset state
            RCC_APB2PeriphResetCmd(RCC_APB2Periph_USART1, ENABLE);
#endif
            // Release USART1 from reset state
            RCC_APB2PeriphResetCmd(RCC_APB2Periph_USART1, DISABLE);

#if defined (STM_UART1_REMAP)
            RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);	

            tGpio_l.GPIO_Pin = GPIO_Pin_6;				// Tx Pin is on Port B, bit 6
            GPIO_Init(GPIOB, &tGpio_l);
            GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_USART1);	

            tGpio_l.GPIO_Pin = GPIO_Pin_7;				// Rx Pin is on Port B, bit 7
            GPIO_Init(GPIOB, &tGpio_l);
            GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_USART1);	
#else
            RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);	

            tGpio_l.GPIO_Pin = GPIO_Pin_9;				// Tx Pin is on Port A, bit 9
            GPIO_Init(GPIOA, &tGpio_l);
            GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);	

            tGpio_l.GPIO_Pin = GPIO_Pin_10;				// Rx Pin is on Port A, bit 10
            GPIO_Init(GPIOA, &tGpio_l);
            GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);	
#endif

#if defined (STM_UART1_RS485_PIN) && defined (STM_UART1_RS485_PORT)
            if(tConf_p->enRS485 == UART_enRS485_ENABLE)
            {
                KB_GPIO_InitTypeDef	tGpio_l;
                kbGPIO_ResetBitsCLK(STM_UART1_RS485_PORT, STM_UART1_RS485_PIN);		// Disable Rs485 Transmitter

                tGpio_l.GPIO_Pin = STM_UART1_RS485_PIN;
                tGpio_l.GPIO_Mode = GPIO_Mode_Out_PP;
                tGpio_l.GPIO_Speed = GPIO_Speed_50MHz;
                kbGPIO_InitCLK(STM_UART1_RS485_PORT, &tGpio_l);
            }
#endif

            ptUart_l = USART1;

            if( (tConf_p->cbError) || (tConf_p->cbReceive) || (tConf_p->cbTransmit) )
            {
                tNvic_l.NVIC_IRQChannel = USART1_IRQn;
                tNvic_l.NVIC_IRQChannelPreemptionPriority = 0;
                tNvic_l.NVIC_IRQChannelSubPriority = 1;
                tNvic_l.NVIC_IRQChannelCmd = ENABLE;

                NVIC_Init(&tNvic_l);
            }

        }   break;

    case UART_PORT_2:
        {	//-------------------- USART2  --------------------

            //Provide USART2 peripheral clock
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

#ifndef STM_UART_DISABLE_PERIPH_RESET
            // Enable USART2 reset state
            RCC_APB1PeriphResetCmd(RCC_APB1Periph_USART2, ENABLE);
#endif
            // Release USART2 from reset state
            RCC_APB1PeriphResetCmd(RCC_APB1Periph_USART2, DISABLE);

#if defined (STM_UART2_REMAP)
            RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);	

            tGpio_l.GPIO_Pin = GPIO_Pin_5;				// Tx Pin is on Port D, bit 5
            GPIO_Init(GPIOD, &tGpio_l);
            GPIO_PinAFConfig(GPIOD, GPIO_PinSource5, GPIO_AF_USART2);	

            tGpio_l.GPIO_Pin = GPIO_Pin_6;				// Rx Pin is on Port D, bit 6
            GPIO_Init(GPIOD, &tGpio_l);
            GPIO_PinAFConfig(GPIOD, GPIO_PinSource6, GPIO_AF_USART2);	
#else
            RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);	

            tGpio_l.GPIO_Pin = GPIO_Pin_2;				// Tx Pin is on Port A, bit 2
            GPIO_Init(GPIOA, &tGpio_l);
            GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2);	

            tGpio_l.GPIO_Pin = GPIO_Pin_3;				// Rx Pin is on Port A, bit 3
            GPIO_Init(GPIOA, &tGpio_l);
            GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART2);	
#endif

#if defined (STM_UART2_RS485_PIN) && defined (STM_UART2_RS485_PORT)
            if(tConf_p->enRS485 == UART_enRS485_ENABLE)
            {
                KB_GPIO_InitTypeDef	tGpio_l;

                kbGPIO_ResetBitsCLK(STM_UART2_RS485_PORT, STM_UART2_RS485_PIN);		// Disable Rs485 Transmitter

                tGpio_l.GPIO_Pin = STM_UART2_RS485_PIN;
                tGpio_l.GPIO_Mode = GPIO_Mode_Out_PP;
                tGpio_l.GPIO_Speed = GPIO_Speed_50MHz;
                kbGPIO_InitCLK(STM_UART2_RS485_PORT, &tGpio_l);
            }
#endif

            ptUart_l = USART2;

            if( (tConf_p->cbError) || (tConf_p->cbReceive) || (tConf_p->cbTransmit) )
            {
                tNvic_l.NVIC_IRQChannel = USART2_IRQn;
                tNvic_l.NVIC_IRQChannelPreemptionPriority = 0;
                tNvic_l.NVIC_IRQChannelSubPriority = 1;
                tNvic_l.NVIC_IRQChannelCmd = ENABLE;

                NVIC_Init(&tNvic_l);
            }

        }   break;

    case UART_PORT_3:
        {	//-------------------- USART3 --------------------

            //Provide USART2 peripheral clock
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

#ifndef STM_UART_DISABLE_PERIPH_RESET
            // Enable USART2 reset state
            RCC_APB1PeriphResetCmd(RCC_APB1Periph_USART3, ENABLE);
#endif
            // Release USART2 from reset state
            RCC_APB1PeriphResetCmd(RCC_APB1Periph_USART3, DISABLE);

#if defined (STM_UART3_REMAP_FULL) || defined (STM_UART3_REMAP)
            RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);	

            tGpio_l.GPIO_Pin = GPIO_Pin_8;				// Tx Pin is on Port D, bit 8
            GPIO_Init(GPIOD, &tGpio_l);
            GPIO_PinAFConfig(GPIOD, GPIO_PinSource8, GPIO_AF_USART3);	

            tGpio_l.GPIO_Pin = GPIO_Pin_9;				// Rx Pin is on Port D, bit 9
            GPIO_Init(GPIOD, &tGpio_l);
            GPIO_PinAFConfig(GPIOD, GPIO_PinSource9, GPIO_AF_USART3);	
#elif defined (STM_UART3_REMAP_PARTIAL)
            RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);	

            tGpio_l.GPIO_Pin = GPIO_Pin_10;				// Tx Pin is on Port C, bit 10
            GPIO_Init(GPIOC, &tGpio_l);
            GPIO_PinAFConfig(GPIOC, GPIO_PinSource10, GPIO_AF_USART3);	

            tGpio_l.GPIO_Pin = GPIO_Pin_11;				// Rx Pin is on Port C, bit 11
            GPIO_Init(GPIOC, &tGpio_l);
            GPIO_PinAFConfig(GPIOC, GPIO_PinSource11, GPIO_AF_USART3);	
#else
            RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);	

            tGpio_l.GPIO_Pin = GPIO_Pin_10;				// Tx Pin is on Port B, bit 10
            GPIO_Init(GPIOB, &tGpio_l);
            GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_USART3);	

            tGpio_l.GPIO_Pin = GPIO_Pin_11;				// Rx Pin is on Port B, bit 11
            GPIO_Init(GPIOB, &tGpio_l);
            GPIO_PinAFConfig(GPIOB, GPIO_PinSource11, GPIO_AF_USART3);	
#endif

#if defined (STM_UART3_RS485_PIN) && defined (STM_UART3_RS485_PORT)
            if(tConf_p->enRS485 == UART_enRS485_ENABLE)
            {
                KB_GPIO_InitTypeDef	tGpio_l;
                kbGPIO_ResetBitsCLK(STM_UART3_RS485_PORT, STM_UART3_RS485_PIN);		// Disable Rs485 Transmitter

                tGpio_l.GPIO_Pin = STM_UART3_RS485_PIN;
                tGpio_l.GPIO_Mode = GPIO_Mode_Out_PP;
                tGpio_l.GPIO_Speed = GPIO_Speed_50MHz;
                kbGPIO_InitCLK(STM_UART3_RS485_PORT, &tGpio_l);
            }
#endif

            ptUart_l = USART3;

            if( (tConf_p->cbError) || (tConf_p->cbReceive) || (tConf_p->cbTransmit) )
            {
                tNvic_l.NVIC_IRQChannel = USART3_IRQn;
                tNvic_l.NVIC_IRQChannelPreemptionPriority = 0;
                tNvic_l.NVIC_IRQChannelSubPriority = 1;
                tNvic_l.NVIC_IRQChannelCmd = ENABLE;

                NVIC_Init(&tNvic_l);
            }

        } break;

    case UART_PORT_4:
        {	//-------------------- UART4 --------------------

            //Provide UART4 peripheral clock
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);

#ifndef STM_UART_DISABLE_PERIPH_RESET
            // Enable UART4 reset state
            RCC_APB1PeriphResetCmd(RCC_APB1Periph_UART4, ENABLE);
#endif
            // Release UART4 from reset state
            RCC_APB1PeriphResetCmd(RCC_APB1Periph_UART4, DISABLE);

#if defined (STM_UART4_REMAP)
            RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);	

            tGpio_l.GPIO_Pin = GPIO_Pin_10;				// Tx Pin is on Port C, bit 10
            GPIO_Init(GPIOC, &tGpio_l);
            GPIO_PinAFConfig(GPIOC, GPIO_PinSource10, GPIO_AF_UART4);	

            tGpio_l.GPIO_Pin = GPIO_Pin_11;				// Rx Pin is on Port C, bit 11
            GPIO_Init(GPIOC, &tGpio_l);
            GPIO_PinAFConfig(GPIOC, GPIO_PinSource11, GPIO_AF_UART4);	
#else
            RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);	

            tGpio_l.GPIO_Pin = GPIO_Pin_0;				// Tx Pin is on Port A, bit 0
            GPIO_Init(GPIOA, &tGpio_l);
            GPIO_PinAFConfig(GPIOA, GPIO_PinSource0, GPIO_AF_UART4);	

            tGpio_l.GPIO_Pin = GPIO_Pin_1;				// Rx Pin is on Port A, bit 1
            GPIO_Init(GPIOA, &tGpio_l);
            GPIO_PinAFConfig(GPIOA, GPIO_PinSource1, GPIO_AF_UART4);	
#endif

            ptUart_l = UART4;

            if( (tConf_p->cbError) || (tConf_p->cbReceive) || (tConf_p->cbTransmit) )
            {
                tNvic_l.NVIC_IRQChannel = UART4_IRQn;
                tNvic_l.NVIC_IRQChannelPreemptionPriority = 0;
                tNvic_l.NVIC_IRQChannelSubPriority = 1;
                tNvic_l.NVIC_IRQChannelCmd = ENABLE;

                NVIC_Init(&tNvic_l);
            }

        } break;

    case UART_PORT_5:
        {	//-------------------- UART5 --------------------

            //Provide UART5 peripheral clock
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5, ENABLE);

#ifndef STM_UART_DISABLE_PERIPH_RESET
            // Enable UART5 reset state
            RCC_APB1PeriphResetCmd(RCC_APB1Periph_UART5, ENABLE);
#endif
            // Release UART5 from reset state
            RCC_APB1PeriphResetCmd(RCC_APB1Periph_UART5, DISABLE);

            RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);	
            RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);	

            tGpio_l.GPIO_Pin = GPIO_Pin_12;				// Tx Pin is on Port C, bit 12
            GPIO_Init(GPIOC, &tGpio_l);
            GPIO_PinAFConfig(GPIOC, GPIO_PinSource12, GPIO_AF_UART5);	

            tGpio_l.GPIO_Pin = GPIO_Pin_2;				// Rx Pin is on Port D, bit 2
            GPIO_Init(GPIOD, &tGpio_l);
            GPIO_PinAFConfig(GPIOD, GPIO_PinSource2, GPIO_AF_UART5);	

            ptUart_l = UART5;

            if( (tConf_p->cbError) || (tConf_p->cbReceive) || (tConf_p->cbTransmit) )
            {
                tNvic_l.NVIC_IRQChannel = UART5_IRQn;
                tNvic_l.NVIC_IRQChannelPreemptionPriority = 0;
                tNvic_l.NVIC_IRQChannelSubPriority = 1;
                tNvic_l.NVIC_IRQChannelCmd = ENABLE;

                NVIC_Init(&tNvic_l);
            }

        } break;

    case UART_PORT_6:
        {	//-------------------- USART6 --------------------

            //Provide USART6 peripheral clock
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART6, ENABLE);

#ifndef STM_UART_DISABLE_PERIPH_RESET
            // Enable USART6 reset state
            RCC_APB2PeriphResetCmd(RCC_APB2Periph_USART6, ENABLE);
#endif
            // Release USART6 from reset state
            RCC_APB2PeriphResetCmd(RCC_APB2Periph_USART6, DISABLE);

#if defined (STM_UART6_REMAP)
            RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);	

            tGpio_l.GPIO_Pin = GPIO_Pin_9;				// Tx Pin is on Port G, bit 9
            GPIO_Init(GPIOG, &tGpio_l);
            GPIO_PinAFConfig(GPIOG, GPIO_PinSource9, GPIO_AF_USART6);	

            tGpio_l.GPIO_Pin = GPIO_Pin_14;				// Rx Pin is on Port G, bit 14
            GPIO_Init(GPIOG, &tGpio_l);
            GPIO_PinAFConfig(GPIOG, GPIO_PinSource14, GPIO_AF_USART6);	
#else
            RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);	

            tGpio_l.GPIO_Pin = GPIO_Pin_6;				// Tx Pin is on Port C, bit 6
            GPIO_Init(GPIOC, &tGpio_l);
            GPIO_PinAFConfig(GPIOC, GPIO_PinSource6, GPIO_AF_USART6);	

            tGpio_l.GPIO_Pin = GPIO_Pin_7;				// Rx Pin is on Port C, bit 7
            GPIO_Init(GPIOC, &tGpio_l);
            GPIO_PinAFConfig(GPIOC, GPIO_PinSource7, GPIO_AF_USART6);	
#endif

            ptUart_l = USART6;

#if defined (STM_UART6_RS485_PIN) && defined (STM_UART6_RS485_PORT)
            if(tConf_p->enRS485 == UART_enRS485_ENABLE)
            {
                KB_GPIO_InitTypeDef	tGpio_l;

                kbGPIO_ResetBitsCLK(STM_UART6_RS485_PORT, STM_UART6_RS485_PIN);		// Disable Rs485 Transmitter

                tGpio_l.GPIO_Pin = STM_UART6_RS485_PIN;
#ifdef STM_UART6_RS485_OD
                tGpio_l.GPIO_Mode = GPIO_Mode_Out_OD;
#else
                tGpio_l.GPIO_Mode = GPIO_Mode_Out_PP;
#endif
                tGpio_l.GPIO_Speed = GPIO_Speed_50MHz;
                kbGPIO_InitCLK(STM_UART6_RS485_PORT, &tGpio_l);
            }
#endif

            if( (tConf_p->cbError) || (tConf_p->cbReceive) || (tConf_p->cbTransmit) )
            {
                tNvic_l.NVIC_IRQChannel = USART6_IRQn;
                tNvic_l.NVIC_IRQChannelPreemptionPriority = 0;
                tNvic_l.NVIC_IRQChannelSubPriority = 1;
                tNvic_l.NVIC_IRQChannelCmd = ENABLE;

                NVIC_Init(&tNvic_l);
            }

        }   break;

    default:
        {
            i32uRv_l = BSPE_UART_PORT_ERR;                
            goto laError;
        }
    } 

    cbError_s[i8uPort_p] = tConf_p->cbError;
    cbTransmit_s[i8uPort_p] = tConf_p->cbTransmit;
    cbReceive_s[i8uPort_p] = tConf_p->cbReceive;

    api8uData_s[i8uPort_p] = (const INT8U *)0;
    ai16uDataLen_s[i8uPort_p] = 0;
    apenState_s[i8uPort_p] = (UART_EBufferSendState *)0;
    ai16uActIndex_s[i8uPort_p] = 0;

    // Configure Control Register CR2
    i16uTmpReg_l = ptUart_l->CR2;
    /* Clear STOP[13:12] bits */
    i16uTmpReg_l &= (INT16U)~((INT16U)USART_CR2_STOP);

    switch (tConf_p->enStopBit)
    {
    case UART_enSTOPBIT_1:
        {
            i16uTmpReg_l |= USART_StopBits_1;
        }   break;

    case UART_enSTOPBIT_2:
        {
            i16uTmpReg_l |= USART_StopBits_2;
        }   break;

    default:
        {
            i32uRv_l = BSPE_UART_STOP_BIT;
            goto laError;
        }
    }
    ptUart_l->CR2 = i16uTmpReg_l;

    /* Configure the USART Baud Rate */
    BSP_RCC_GetClocksFreq(&RCC_ClocksStatus);

    if (   i8uPort_p == UART_PORT_1
       ||  i8uPort_p == UART_PORT_6)
    {
        apbclock = RCC_ClocksStatus.PCLK2_Frequency;
    }
    else
    {
        apbclock = RCC_ClocksStatus.PCLK1_Frequency;
    }

    switch (tConf_p->enBaudRate)
    {
    case UART_enBAUD_500:       baudrate = 500;  break;
    case UART_enBAUD_600:       baudrate = 600;  break;
    case UART_enBAUD_1000:      baudrate = 1000;  break;
    case UART_enBAUD_1200:      baudrate = 1200;  break;
    case UART_enBAUD_2400:      baudrate = 2400;  break;
    case UART_enBAUD_4800:      baudrate = 4800;  break;
    case UART_enBAUD_9600:      baudrate = 9600;  break;
    case UART_enBAUD_14400:     baudrate = 14400;  break;
    case UART_enBAUD_19200:     baudrate = 19200;  break;
    case UART_enBAUD_28800:     baudrate = 28800;  break;
    case UART_enBAUD_38400:     baudrate = 38400;  break;
    case UART_enBAUD_56000:     baudrate = 56000;  break;
    case UART_enBAUD_57600:     baudrate = 57600;  break;
    case UART_enBAUD_115200:    baudrate = 115200;  break;
    case UART_enBAUD_100000:    baudrate = 100000;  break;
    case UART_enBAUD_230400:    baudrate = 230400;  break;
    case UART_enBAUD_250000:    baudrate = 250000;  break;
    default:
        {
            i32uRv_l = BSPE_UART_BAUD_RATE;
            goto laError;
        }
    }

#if CR1_OVER8 != 0
    /* Integer part computing in case Oversampling mode is 8 Samples */
            integerdivider = ((25 * apbclock) / (2 * baudrate));
#else
    /* Integer part computing in case Oversampling mode is 16 Samples */
            integerdivider = ((25 * apbclock) / (4 * baudrate));
#endif
            i16uTmpReg_l = (integerdivider / 100) << 4;

            /* Determine the fractional part */
            fractionaldivider = integerdivider - (100 * (i16uTmpReg_l >> 4));

            /* Implement the fractional part in the register */
#if CR1_OVER8 != 0
            i16uTmpReg_l |= ((((fractionaldivider * 8) + 50) / 100)) & ((uint8_t)0x07);
#else
            i16uTmpReg_l |= ((((fractionaldivider * 16) + 50) / 100)) & ((uint8_t)0x0F);
#endif

        // Achtung für Test Sensorbus bitte löschenfalls es vergessen wurde - Ralf Meeh, 29.07.2015
        //     if (baudrate == 1000)
        //     {
        //         integerdivider = 0x01d4; 
        //         fractionaldivider = 0x0C;
        //         i16uTmpReg_l = 16*fractionaldivider + (integerdivider<<4);
        //     }

        /* Write to USART BRR register */
        ptUart_l->BRR = i16uTmpReg_l;

#ifdef DMX_UART_PORT
            if (DMX_UART_PORT == i8uPort_p)
            {
                // save calculated DMX Break divider for later register setting in UART_sendBuffer() function
                i16u_DMX_BRR_BREAK_g = i16uTmpReg_l;

                // same calculation with DMX Data baudrate
                baudrate = DMX_DATA_BAUDRATE;
#if CR1_OVER8 != 0
                /* Integer part computing in case Oversampling mode is 8 Samples */
                integerdivider = ((25 * apbclock) / (2 * baudrate));
#else
                integerdivider = ((25 * apbclock) / (4 * baudrate));
#endif
                i16uTmpReg_l = (integerdivider / 100) << 4;
                fractionaldivider = integerdivider - (100 * (i16uTmpReg_l >> 4));
#if CR1_OVER8 != 0
                i16uTmpReg_l |= ((((fractionaldivider * 8) + 50) / 100)) & ((uint8_t)0x07);
#else
                i16uTmpReg_l |= ((((fractionaldivider * 16) + 50) / 100)) & ((uint8_t)0x0F);
#endif

                // save calculated DMX Data divider for later register setting in UART_sendBuffer() function
                i16u_DMX_BRR_DATA_g = i16uTmpReg_l;
            }
#endif



    // Configure Control Register CR3
    i16uTmpReg_l = 0;    

    // Do not enable error interrupt explicit. Errors are indicated, when a charcter is received

    switch (tConf_p->enFlowControl)
    {
    case UART_enFLOWCTRL_NONE:
        {
            // do nothing, all bits are already cleared
        }   break;
    default:
        {
            i32uRv_l = BSPE_UART_FLOW_CONTROL;
        }   goto laError;

    }
    ptUart_l->CR3 = i16uTmpReg_l;

    // Configure Control Register CR1
    i16uTmpReg_l = ptUart_l->CR1;    
    /* Clear OVER8, M, PCE, PS, TE and RE bits */
    i16uTmpReg_l &= (uint32_t)~((uint32_t)CR1_CLEAR_MASK);

    i16uTmpReg_l |= CR1_OVER8;			// set OVER8 bit
    i16uTmpReg_l |= USART_CR1_RE;		// Receiver Enable
    i16uTmpReg_l |= USART_CR1_TE;		// Transmitter Enable

    i16uTmpReg_l |= USART_CR1_RXNEIE;    // Receive Interrupt enable, also needed for error callback

    switch (tConf_p->enDataLen)
    {
    case UART_enDATA_LEN_8:            
        {
            switch (tConf_p->enParity)
            {
            case UART_enPARITY_NONE:            // No Parity + 8 Data -> 8 Bit -> reset M
                {
                }   break;
            case UART_enPARITY_EVEN:
                {
                    i16uTmpReg_l |= USART_WordLength_9b | USART_Parity_Even;         // Even Parity + 8 Data -> 9 Bit -> set M
                    i16uTmpReg_l |= USART_CR1_PEIE;                               // Parity Interrupt enable, needed for error callback
                }   break;
            case UART_enPARITY_ODD:
                {
                    i16uTmpReg_l |= USART_WordLength_9b | USART_Parity_Odd;         // Odd Parity  + 8 Data -> 9 Bit -> set M
                    i16uTmpReg_l |= USART_CR1_PEIE;                               // Parity Interrupt enable, needed for error callback
                }   break;
            default:
                {
                    i32uRv_l = BSPE_UART_PARITY;
                    goto laError;
                }
            }    
        }	break;

    case UART_enDATA_LEN_7:            
        {
            switch (tConf_p->enParity)
            {
            case UART_enPARITY_EVEN:
                {
                    i16uTmpReg_l |= USART_Parity_Even;         // Even Parity + 7 Data -> 8 Bit -> reset M
                    i16uTmpReg_l |= USART_CR1_PEIE;                               // Parity Interrupt enable, needed for error callback
                }   break;
            case UART_enPARITY_ODD:
                {
                    i16uTmpReg_l |= USART_Parity_Odd;         // Odd Parity  + 7 Data -> 8 Bit -> reset M
                    i16uTmpReg_l |= USART_CR1_PEIE;                               // Parity Interrupt enable, needed for error callback
                }   break;
            case UART_enPARITY_NONE:            // not possible in this implementation
            default:
                {
                    i32uRv_l = BSPE_UART_PARITY;
                    goto laError;
                }
            }    
        }	break;

    default:
        {
            i32uRv_l = BSPE_UART_DATA_LEN;
            goto laError;
        }
    }

    i16uTmpReg_l |= USART_CR1_UE;                 // Enable the UART

    ptUart_l->CR1 = i16uTmpReg_l;

    return (i32uRv_l);

    //-------------------------------------------------------------------------------------------------
laError:

    return (i32uRv_l);
}

//*************************************************************************************************
//| Function: UART_putChar
//|
//! brief.
//!
//! detailed.
//!
//!
//!
//! ingroup.
//-------------------------------------------------------------------------------------------------
INT32U UART_putChar (
                     INT8U i8uPort_p, 
                     INT8U i8uChar_p)

{
    INT32U i32uRv_l = BSPE_NO_ERROR;
    USART_TypeDef *ptUart_l = (USART_TypeDef *)0;
#if (  (defined (STM_UART1_RS485_PIN) && defined (STM_UART1_RS485_PORT))     \
    || (defined (STM_UART2_RS485_PIN) && defined (STM_UART2_RS485_PORT))     \
    || (defined (STM_UART3_RS485_PIN) && defined (STM_UART3_RS485_PORT))     \
    )  
    volatile GPIO_TypeDef *ptRs485Port_l = NULL;
    INT16U i16uRs485Bit_l = 0;
#endif

#ifdef UART_DEACTIVATE_FOR_DEBUG
    // if this macro is defined, not data will be sent and no interrupts will be generated
    // this is very helpful for debugging
    return 0;
#endif

    switch (i8uPort_p)
    {
    case UART_PORT_1:
        {
            ptUart_l = USART1;
#if (defined (STM_UART1_RS485_PIN) && defined (STM_UART1_RS485_PORT))
            ptRs485Port_l = STM_UART1_RS485_PORT;
            i16uRs485Bit_l = STM_UART1_RS485_PIN;
#endif

        }   break;

    case UART_PORT_2:
        {
            ptUart_l = USART2;
#if (defined (STM_UART2_RS485_PIN) && defined (STM_UART2_RS485_PORT))
            ptRs485Port_l = STM_UART2_RS485_PORT;
            i16uRs485Bit_l = STM_UART2_RS485_PIN;
#endif

        }	break;

    case UART_PORT_3:
        {
            ptUart_l = USART3;
#if (defined (STM_UART3_RS485_PIN) && defined (STM_UART3_RS485_PORT))
            ptRs485Port_l = STM_UART3_RS485_PORT;
            i16uRs485Bit_l = STM_UART3_RS485_PIN;
#endif
        }   break;

    case UART_PORT_4:
        ptUart_l = UART4;
        break;

    case UART_PORT_5:
        ptUart_l = UART5;
        break;

    case UART_PORT_6:
        ptUart_l = USART6;
        break;
    default:
        {
            i32uRv_l = BSPE_UART_PORT_ERR;
            goto laError;
        }
    }                

    if (ptUart_l->SR & USART_FLAG_TXE)
    {  // Register is empty -> ready to send
#if (  (defined (STM_UART1_RS485_PIN) && defined (STM_UART1_RS485_PORT))     \
    || (defined (STM_UART2_RS485_PIN) && defined (STM_UART2_RS485_PORT))     \
    || (defined (STM_UART3_RS485_PIN) && defined (STM_UART3_RS485_PORT))     \
    )  
        if (ptRs485Port_l)
        {
            ptRs485Port_l->BSRRL = i16uRs485Bit_l;
                ptUart_l->CR1 &= ~USART_Mode_Rx;           // Receiver Disable
        }

#endif

        ptUart_l->DR = (INT16U)i8uChar_p;
        ptUart_l->CR1 |= 0x0080;           // Transmit register empty Interrupt enable
    }
    else
    {
        i32uRv_l = BSPE_UART_TX_NOT_EMPTY;
    }

    return (i32uRv_l);


//-------------------------------------------------------------------------------------------------
laError:
    return (i32uRv_l);
}


//*************************************************************************************************
//| Function: UART_sendBuffer
//|
//! brief.
//!
//! detailed.
//!
//!
//!
//! ingroup.
//-------------------------------------------------------------------------------------------------
INT32U UART_sendBuffer (
                        INT8U i8uPort_p,                        //!< [in] Portnumber of UART device
                        const INT8U *pi8uData_p,                //!< [in] Pointer to a buffer of data
                        INT16U i16uDataLen_p,                   //!< [in] number of bytes to transmit
                        UART_EBufferSendState *penState_p)      //!< [out] State of the actual transmission job
                        //! \return Error Code

{
    INT32U i32uRv_l = BSPE_NO_ERROR;
    USART_TypeDef *ptUart_l = (USART_TypeDef *)0;

#if (  (defined (STM_UART1_RS485_PIN) && defined (STM_UART1_RS485_PORT))     \
    || (defined (STM_UART2_RS485_PIN) && defined (STM_UART2_RS485_PORT))     \
    || (defined (STM_UART3_RS485_PIN) && defined (STM_UART3_RS485_PORT))     \
    || (defined (STM_UART6_RS485_PIN) && defined (STM_UART6_RS485_PORT))     \
    )  
    volatile GPIO_TypeDef *ptRs485Port_l = NULL;
    INT16U i16uRs485Bit_l = 0;
#endif

#ifdef UART_DEACTIVATE_FOR_DEBUG
    // if this macro is defined, not data will be sent and no interrupts will be generated
    // this is very helpful for debugging
    return 0;
#endif

    switch (i8uPort_p)
    {
    case UART_PORT_1:
        {
            ptUart_l = USART1;
#if (defined (STM_UART1_RS485_PIN) && defined (STM_UART1_RS485_PORT))
            ptRs485Port_l = STM_UART1_RS485_PORT;
            i16uRs485Bit_l = STM_UART1_RS485_PIN;
#endif
        }   break;

    case UART_PORT_2:
        {
            ptUart_l = USART2;
#if (defined (STM_UART2_RS485_PIN) && defined (STM_UART2_RS485_PORT))
            ptRs485Port_l = STM_UART2_RS485_PORT;
            i16uRs485Bit_l = STM_UART2_RS485_PIN;
#endif
        }   break;

    case UART_PORT_3:
        {
            ptUart_l = USART3;
#if (defined (STM_UART3_RS485_PIN) && defined (STM_UART3_RS485_PORT))
            ptRs485Port_l = STM_UART3_RS485_PORT;
            i16uRs485Bit_l = STM_UART3_RS485_PIN;
#endif
        }   break;

    case UART_PORT_4:
        ptUart_l = UART4;
        break;

    case UART_PORT_5:
        ptUart_l = UART5;
        break;

    case UART_PORT_6:
        ptUart_l = USART6;
#if (defined (STM_UART6_RS485_PIN) && defined (STM_UART6_RS485_PORT))
        ptRs485Port_l = STM_UART6_RS485_PORT;
        i16uRs485Bit_l = STM_UART6_RS485_PIN;
#endif
        break;

    default:
        {
            i32uRv_l = BSPE_UART_PORT_ERR;
            goto laError;
        }
    }                

    if (ptUart_l)    
    {        
        if (i16uDataLen_p)
        {
            if (apenState_s[i8uPort_p] == NULL)    // Indicator that there is a job pending
            {
                ai16uActIndex_s[i8uPort_p] = 0;
                api8uData_s[i8uPort_p] = pi8uData_p;
                ai16uDataLen_s[i8uPort_p] = i16uDataLen_p;
                apenState_s[i8uPort_p] = penState_p;

                *apenState_s[i8uPort_p] = UART_enBSS_IN_PROGRESS;

                if (ptUart_l->SR & USART_FLAG_TXE)
                {  // Register is empty -> ready to send

#if (  (defined (STM_UART1_RS485_PIN) && defined (STM_UART1_RS485_PORT))     \
    || (defined (STM_UART2_RS485_PIN) && defined (STM_UART2_RS485_PORT))     \
    || (defined (STM_UART3_RS485_PIN) && defined (STM_UART3_RS485_PORT))     \
    || (defined (STM_UART6_RS485_PIN) && defined (STM_UART6_RS485_PORT))     \
    )  
                    if (ptRs485Port_l)
                    {
                        ptRs485Port_l->BSRRL = i16uRs485Bit_l;
                        ptUart_l->CR1 &= ~USART_Mode_Rx;           // Receiver Disable
                    }
#endif              

/** 
 * Send a DMX telegram needs a leading Break signal of 88 us.
 * Therefore instead of sending the first buffer char we switch baudrate and send a zero.
 * Switching back the baudrate is performed in the UART_genericIntHandler().
 */
#ifdef DMX_UART_PORT
                    if (DMX_UART_PORT == i8uPort_p)
                    {
                        ptUart_l->CR1 &= ~(USART_CR1_TE);       // disable transmitter
                        ptUart_l->BRR = i16u_DMX_BRR_BREAK_g;   // switch baudrate
                        //ptUart_l->CR2 &= ~(USART_CR2_STOP);   // configure 1 stop bit
                        ptUart_l->CR1 |= USART_CR1_TE;          // enable transmitter
                        // for DMX we do not transmit the first payload databyte but a zero for Break using half baudrate
                        ptUart_l->DR = 0;                       // set data zero
                        ptUart_l->CR1 |= USART_CR1_TCIE;        // Transmit register complete
                    }
                    else
                    {
                        ptUart_l->DR = api8uData_s[i8uPort_p][ai16uActIndex_s[i8uPort_p]++];
                        ptUart_l->CR1 |= USART_CR1_TXEIE;           // Transmit register empty Interrupt enable
                    }
#else
                    ptUart_l->DR = api8uData_s[i8uPort_p][ai16uActIndex_s[i8uPort_p]++];
                    ptUart_l->CR1 |= USART_CR1_TXEIE ;           // Transmit register empty Interrupt enable
#endif  //DMX_UART_PORT
                }
                else
                {  // Still busy, enable Transmitt Interrupt due to a recent init of the port
                    ptUart_l->CR1 |= USART_CR1_TXEIE;           // Transmit register empty Interrupt enable
                }
            }
            else
            {   // there is still a job pending
                i32uRv_l = BSPE_UART_BUS_JOB_PENDING;
            }
        }
        else
        {
            *penState_p = UART_enBSS_SEND_OK;
        }
    }

    return (i32uRv_l);

//-------------------------------------------------------------------------------------------------
laError:

    return (i32uRv_l);
}    





//*************************************************************************************************
//| Function: UART_uart1IntHandler
//|
//! brief.
//!
//! detailed.
//!
//!
//!
//! ingroup.
//-------------------------------------------------------------------------------------------------
void UART_uart1IntHandler (
                           void)

{
#ifdef STM_UART1_RS485_PIN
    UART_genericIntHandler(UART_PORT_1, USART1, STM_UART1_RS485_PORT, STM_UART1_RS485_PIN);
#else
    UART_genericIntHandler(UART_PORT_1, USART1, 0, 0);
#endif
}

//*************************************************************************************************
//| Function: UART_uart2IntHandler
//|
//! brief.
//!
//! detailed.
//!
//!
//!
//! ingroup.
//-------------------------------------------------------------------------------------------------
void UART_uart2IntHandler (
                           void)

{
#ifdef STM_UART2_RS485_PIN
    UART_genericIntHandler(UART_PORT_2, USART2, STM_UART2_RS485_PORT, STM_UART2_RS485_PIN);
#else
    UART_genericIntHandler(UART_PORT_2, USART2, 0, 0);
#endif
}

//*************************************************************************************************
//| Function: UART_uart3IntHandler
//|
//! brief.
//!
//! detailed.
//!
//!
//!
//! ingroup.
//-------------------------------------------------------------------------------------------------
void UART_uart3IntHandler (
                           void)

{
#ifdef STM_UART3_RS485_PIN
    UART_genericIntHandler(UART_PORT_3, USART3, STM_UART3_RS485_PORT, STM_UART3_RS485_PIN);
#else
    UART_genericIntHandler(UART_PORT_3, USART3, 0, 0);
#endif
}

//*************************************************************************************************
//| Function: UART_uart4IntHandler
//|
//! brief.
//!
//! detailed.
//!
//!
//!
//! ingroup.
//-------------------------------------------------------------------------------------------------
void UART_uart4IntHandler (
                           void)

{
    UART_genericIntHandler(UART_PORT_4, UART4, 0, 0);
}

//*************************************************************************************************
//| Function: UART_uart5IntHandler
//|
//! brief.
//!
//! detailed.
//!
//!
//!
//! ingroup.
//-------------------------------------------------------------------------------------------------
void UART_uart5IntHandler (
                           void)

{
    UART_genericIntHandler(UART_PORT_5, UART5, 0, 0);
}

//*************************************************************************************************
//| Function: UART_uart6IntHandler
//|
//! brief.
//!
//! detailed.
//!
//!
//!
//! ingroup.
//-------------------------------------------------------------------------------------------------
void UART_uart6IntHandler (
                           void)

{
#ifdef STM_UART6_RS485_PIN
    UART_genericIntHandler(UART_PORT_6, USART6, STM_UART6_RS485_PORT, STM_UART6_RS485_PIN);
#else
    UART_genericIntHandler(UART_PORT_6, USART6, 0, 0);
#endif
}

//*************************************************************************************************
//| Function: UART_genericIntHandler
//|
//! brief.
//!
//! detailed.
//!
//!
//!
//! ingroup.
//-------------------------------------------------------------------------------------------------
static void UART_genericIntHandler (INT8U i8uPort,
                             USART_TypeDef *pUART,
                             GPIO_TypeDef *pRS485Port,
                             INT16U i16uR485Pin)
{
    UART_ERecError enErr_l;
    volatile INT8U i8uChar_l;

    if (i8uPort < 0 || i8uPort >= BSP_UART_MAXPORT)
    {
        _fault_handler ();
    }

    if (pUART->SR & (USART_FLAG_PE | USART_FLAG_FE | USART_FLAG_NE | USART_FLAG_ORE))
    {
        if (cbError_s[i8uPort])
        {
            enErr_l = UART_enERR_INVALID;
            if (pUART->SR & USART_FLAG_PE)
            {
                enErr_l |= UART_enERR_PARITY;
            }
            if (pUART->SR & USART_FLAG_FE)
            {
                enErr_l |= UART_enERR_FRAME;
            }
            if (pUART->SR & USART_FLAG_NE)
            {
                enErr_l |= UART_enERR_NOISE;
            }
            if (pUART->SR & USART_FLAG_ORE)
            {
                enErr_l |= UART_enERR_OVERRUN;
            }
            cbError_s[i8uPort] (enErr_l);
        }
        i8uChar_l = (INT8U)pUART->DR;  // Read anyway for clearing the error flags
    }
    else if (pUART->SR & USART_FLAG_RXNE)
    {   // Only call receive, if there is no error
        i8uChar_l = (INT8U)pUART->DR;
        if (cbReceive_s[i8uPort])
        {
            cbReceive_s[i8uPort] (i8uChar_l);
        }
    }    

    if (   (pUART->CR1 & USART_CR1_TXEIE)  // Transmit Interrupt enabled
        && (pUART->SR & USART_FLAG_TXE) // Transmit Register empty
        ) 
    {
        TBOOL bSend_l = bFALSE;
        if (apenState_s[i8uPort])    // Indicator that there is a buffer send job pending
        {
            if (ai16uActIndex_s[i8uPort] < ai16uDataLen_s[i8uPort])
            {    // there are still bytes to send
                pUART->DR = api8uData_s[i8uPort][ai16uActIndex_s[i8uPort]++];
                bSend_l = bTRUE;
            }
            else
            {  // all Bytes send -> signal it
                *apenState_s[i8uPort] = UART_enBSS_SEND_OK;
                apenState_s[i8uPort] = NULL;   // close job
                if (cbTransmit_s[i8uPort])
                {
                    bSend_l = cbTransmit_s[i8uPort] ();  // give application chance to react on
                }
            }
        }
        else
        {   //single character send
            if (cbTransmit_s[i8uPort])
            {
                bSend_l = cbTransmit_s[i8uPort] ();
            }
        }
        if (bSend_l == bFALSE)
        {  // Interrupt flag is still set -> disable interrupt
            pUART->CR1 &= ~USART_CR1_TXEIE;           // Transmit register empty Interrupt disable
            if (pRS485Port)
            {
                pUART->CR1 |= USART_CR1_TCIE;      // Enable TC Interrupt 
            }
        }
    }


    if (   pRS485Port
        && (pUART->CR1 & USART_CR1_TCIE)  // Transmission complete interrupt enabled
        && (pUART->SR & USART_FLAG_TC) // Transmission complete flag set
        ) 
    {  
#ifdef DMX_UART_PORT
        if (*apenState_s[i8uPort] != UART_enBSS_IN_PROGRESS)
        {
            // Transmission complete -> disable RS485 Transmitter
            pRS485Port->BSRRH = i16uR485Pin;                // Disable Rs485 Transmitter
            pUART->CR1 |= USART_CR1_RE;                     // Receiver was disabled because of the echo
            pUART->SR &= ~USART_FLAG_TC;                    // Reset TC Flag
            pUART->CR1 &= ~USART_CR1_TCIE;                  // Disable TC Interrupt 
        }
        else
        {
            if (DMX_UART_PORT == i8uPort)
            {
                //DMX Break complete
                pUART->CR1 &= ~(USART_CR1_TE);    // disable transmitter
                pUART->SR &= ~USART_FLAG_TC;      // Reset TC Flag
                pUART->CR1 &= ~USART_CR1_TCIE;    // Disable TC Interrupt 
                pUART->BRR = i16u_DMX_BRR_DATA_g; // switch baudrate
                pUART->CR2 &= ~(USART_CR2_STOP);  // clear stop bits configuration
                pUART->CR2 |= USART_CR2_STOP_1;   // set 2 stop bits
                pUART->CR1 |= USART_CR1_TE;       // enable transmitter
                pUART->CR1 |= USART_CR1_TXEIE;    // Transmit register empty Interrupt enable
                pUART->DR = api8uData_s[i8uPort][ai16uActIndex_s[i8uPort]++];
                //bSend_l = bTRUE;
            }
        }
#else
        pRS485Port->BSRRH = i16uR485Pin;                // Disable Rs485 Transmitter
        pUART->CR1 |= USART_CR1_RE;                     // Receiver was disabled because of the echo
        pUART->SR &= ~USART_FLAG_TC;                    // Reset TC Flag
        pUART->CR1 &= ~USART_CR1_TCIE;                  // Disable TC Interrupt 
#endif
    }
}


//*************************************************************************************************
