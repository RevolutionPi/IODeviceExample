/*=============================================================================================
*
* gpio.c
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
* General Purpose I/O handling functions.
*
*/

//+=============================================================================================
//|		Include-Dateien / include files
//+=============================================================================================

#include <common_define.h>
#include <project.h>

#include <bsp/bspConfig.h>

#include <SysLib\inc\stm32f2xx_rcc.h>
#include <SysLib\inc\stm32f2xx_tim.h>
#include <SysLib\inc\stm32f2xx.h>
#include <SysLib\inc\misc.h>

#include <bsp\gpio\gpio.h>

//+=============================================================================================
//|		Globale Variablen / global variables
//+=============================================================================================

//+=============================================================================================
//|		Function:	gpio_DeInit
//+---------------------------------------------------------------------------------------------
//!		Resets GPIO to post-reset state.
//+---------------------------------------------------------------------------------------------
//|		Conditions:
//!		\pre	(pre-condition)
//!		\post	(post-condition)
//+---------------------------------------------------------------------------------------------
//|		Annotations:
//+=============================================================================================
void	gpio_DeInit(GPIO_TypeDef* GPIOx)
{
	GPIO_DeInit(GPIOx);
}


//+=============================================================================================
//|		Function:	gpio_JTAGcfg
//+---------------------------------------------------------------------------------------------
//!		Initializes JTAG interface to SWD or disables single lines / all lines.
//+---------------------------------------------------------------------------------------------
//|		Conditions:
//!		\pre	(pre-condition)
//!		\post	(post-condition)
//+---------------------------------------------------------------------------------------------
//|		Annotations:
//+=============================================================================================
void gpio_JTAGcfg( void )
{
#if defined (STM_DISABLE_JTAG_RESET)
	KB_GPIO_InitTypeDef GPIO_InitStructure;
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

	kbGPIO_InitCLK(GPIOB, &GPIO_InitStructure );
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource4, 1);
	
	//RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	//GPIO_PinRemapConfig(GPIO_Remap_SWJ_NoJTRST, ENABLE);
#endif
}

//+=============================================================================================
//|		Function:	kbGPIO_InitCLK
//+---------------------------------------------------------------------------------------------
//!		module initialisation function (detailed description)
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
//!		\warning 	-
//!		\bug 		-
//!		\see		-
//+=============================================================================================
void	kbGPIO_InitCLK (
	GPIO_TypeDef		*pGPIOx,				///< [in] GPIO port to initialize
	KB_GPIO_InitTypeDef *pGPIO_InitStruct		///< [in] Init Structure with parameters of initialisation
	)
{
	// Switch on Bridge for peripheral Clocks

	INT32U port = (INT32U)pGPIOx;
	GPIO_InitTypeDef InitStruct;

	switch( port )
	{
		case GPIOA_BASE:
		{
			port = RCC_AHB1Periph_GPIOA;
		}	break;
		case GPIOB_BASE:
		{
			port = RCC_AHB1Periph_GPIOB;
		}	break;
		case GPIOC_BASE:
		{
			port = RCC_AHB1Periph_GPIOC;
		}	break;
		case GPIOD_BASE:
		{
			port = RCC_AHB1Periph_GPIOD;
		}	break;
		case GPIOE_BASE:
		{
			port = RCC_AHB1Periph_GPIOE;
		}	break;
		case GPIOF_BASE:
		{
			port = RCC_AHB1Periph_GPIOF;
		}	break;
		case GPIOG_BASE:
		{
			port = RCC_AHB1Periph_GPIOG;
		}	break;
		case GPIOH_BASE:
		{
			port = RCC_AHB1Periph_GPIOH;
		}	break;
		case GPIOI_BASE:
		{
			port = RCC_AHB1Periph_GPIOI;
		}	break;
		
		default:
		{
			//error unknown GPIO Port
			while(1);
		}	break;
	}
	RCC_AHB1PeriphClockCmd(port, ENABLE);	

	InitStruct.GPIO_Pin   = pGPIO_InitStruct->GPIO_Pin;
	InitStruct.GPIO_Speed = pGPIO_InitStruct->GPIO_Speed;

	switch (pGPIO_InitStruct->GPIO_Mode)
	{
	case GPIO_Mode_AIN_:
		InitStruct.GPIO_Mode = GPIO_Mode_IN;
		InitStruct.GPIO_OType = GPIO_OType_PP;
		InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
		break;
	case GPIO_Mode_IN_FLOATING:
		InitStruct.GPIO_Mode = GPIO_Mode_IN;
		InitStruct.GPIO_OType = GPIO_OType_PP;
		InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
		break;
	case GPIO_Mode_IPD:
		InitStruct.GPIO_Mode = GPIO_Mode_IN;
		InitStruct.GPIO_OType = GPIO_OType_PP;
		InitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;
		break;
	case GPIO_Mode_IPU:
		InitStruct.GPIO_Mode = GPIO_Mode_IN;
		InitStruct.GPIO_OType = GPIO_OType_PP;
		InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
		break;
	case GPIO_Mode_Out_OD:
		InitStruct.GPIO_Mode = GPIO_Mode_OUT;
		InitStruct.GPIO_OType = GPIO_OType_OD;
		InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
		break;
	case GPIO_Mode_Out_PP:
		InitStruct.GPIO_Mode = GPIO_Mode_OUT;
		InitStruct.GPIO_OType = GPIO_OType_PP;
		InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
		break;
	case GPIO_Mode_AF_OD:
		InitStruct.GPIO_Mode = GPIO_Mode_AF;
		InitStruct.GPIO_OType = GPIO_OType_OD;
		InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
		break;
	case GPIO_Mode_AF_PP:
		InitStruct.GPIO_Mode = GPIO_Mode_AF;
		InitStruct.GPIO_OType = GPIO_OType_PP;
		InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
		break;
	}

	GPIO_Init( pGPIOx, &InitStruct);
}


void	gpio_StructInit(GPIO_InitTypeDef* GPIO_InitStruct)
{
	GPIO_StructInit(GPIO_InitStruct);
}

INT8U	gpio_ReadInputDataBit(GPIO_TypeDef* GPIOx, INT16U GPIO_Pin)
{
	return GPIO_ReadInputDataBit(GPIOx, GPIO_Pin);
}

INT16U	gpio_ReadInputData(GPIO_TypeDef* GPIOx)
{
	return GPIO_ReadInputData(GPIOx);
}

INT8U	gpio_ReadOutputDataBit(GPIO_TypeDef* GPIOx, INT16U GPIO_Pin)
{
	return GPIO_ReadOutputDataBit(GPIOx, GPIO_Pin);
}

INT16U	gpio_ReadOutputData(GPIO_TypeDef* GPIOx)
{
	return GPIO_ReadOutputData(GPIOx);
}


//+=============================================================================================
//|		Function:	kbGPIO_SetBitsCLK
//+---------------------------------------------------------------------------------------------
//!		(brief-description).
//!     (detailed description)
//+---------------------------------------------------------------------------------------------
//|		Conditions:
//!		\pre	(pre-condition)
//!		\post	(post-condition)
//+---------------------------------------------------------------------------------------------
//|		Annotations:
//+=============================================================================================
void	kbGPIO_SetBitsCLK(GPIO_TypeDef* GPIOx, INT16U GPIO_Pin)
{
	// Switch on Bridge for peripheral Clocks
	
	INT32U port = (INT32U)GPIOx;

	switch( port )
	{
		case GPIOA_BASE:
		{
			port = RCC_AHB1Periph_GPIOA;
		}	break;
		case GPIOB_BASE:
		{
			port = RCC_AHB1Periph_GPIOB;
		}	break;
		case GPIOC_BASE:
		{
			port = RCC_AHB1Periph_GPIOC;
		}	break;
		case GPIOD_BASE:
		{
			port = RCC_AHB1Periph_GPIOD;
		}	break;
		case GPIOE_BASE:
		{
			port = RCC_AHB1Periph_GPIOE;
		}	break;
		case GPIOF_BASE:
		{
			port = RCC_AHB1Periph_GPIOF;
		}	break;
		case GPIOG_BASE:
		{
			port = RCC_AHB1Periph_GPIOG;
		}	break;
		case GPIOH_BASE:
		{
			port = RCC_AHB1Periph_GPIOH;
		}	break;
		case GPIOI_BASE:
		{
			port = RCC_AHB1Periph_GPIOI;
		}	break;
		
		default:
		{
			//error unknown GPIO Port
			while(1);
		}	break;
	}
	RCC_AHB1PeriphClockCmd(port, ENABLE);
	GPIO_SetBits(GPIOx, GPIO_Pin);
}


//+=============================================================================================
//|		Function:	kbGPIO_ResetBitsCLK
//+---------------------------------------------------------------------------------------------
//!		(brief-description).
//!     (detailed description)
//+---------------------------------------------------------------------------------------------
//|		Conditions:
//!		\pre	(pre-condition)
//!		\post	(post-condition)
//+---------------------------------------------------------------------------------------------
//|		Annotations:
//+=============================================================================================
void	kbGPIO_ResetBitsCLK(GPIO_TypeDef* GPIOx, INT16U GPIO_Pin)
{
	// Switch on Bridge for peripheral Clocks
	
	INT32U port = (INT32U)GPIOx;

	switch( port )
	{
		case GPIOA_BASE:
		{
			port = RCC_AHB1Periph_GPIOA;
		}	break;
		case GPIOB_BASE:
		{
			port = RCC_AHB1Periph_GPIOB;
		}	break;
		case GPIOC_BASE:
		{
			port = RCC_AHB1Periph_GPIOC;
		}	break;
		case GPIOD_BASE:
		{
			port = RCC_AHB1Periph_GPIOD;
		}	break;
		case GPIOE_BASE:
		{
			port = RCC_AHB1Periph_GPIOE;
		}	break;
		case GPIOF_BASE:
		{
			port = RCC_AHB1Periph_GPIOF;
		}	break;
		case GPIOG_BASE:
		{
			port = RCC_AHB1Periph_GPIOG;
		}	break;
		case GPIOH_BASE:
		{
			port = RCC_AHB1Periph_GPIOH;
		}	break;
		case GPIOI_BASE:
		{
			port = RCC_AHB1Periph_GPIOI;
		}	break;
		
		default:
		{
			//error unknown GPIO Port
			while(1);
		}	break;
	}
	RCC_AHB1PeriphClockCmd(port, ENABLE);
	GPIO_ResetBits(GPIOx, GPIO_Pin);
}


//+=============================================================================================
//|		Function:	kbGPIO_WriteBit
//+---------------------------------------------------------------------------------------------
//!		(brief-description).
//!     (detailed description)
//+---------------------------------------------------------------------------------------------
//|		Conditions:
//!		\pre	(pre-condition)
//!		\post	(post-condition)
//+---------------------------------------------------------------------------------------------
//|		Annotations:
//+=============================================================================================
void	kbGPIO_WriteBitCLK(GPIO_TypeDef* GPIOx, INT16U GPIO_Pin, BitAction BitVal)
{
	INT32U port = (INT32U)GPIOx;

	switch( port )
	{
		case GPIOA_BASE:
		{
			port = RCC_AHB1Periph_GPIOA;
		}	break;
		case GPIOB_BASE:
		{
			port = RCC_AHB1Periph_GPIOB;
		}	break;
		case GPIOC_BASE:
		{
			port = RCC_AHB1Periph_GPIOC;
		}	break;
		case GPIOD_BASE:
		{
			port = RCC_AHB1Periph_GPIOD;
		}	break;
		case GPIOE_BASE:
		{
			port = RCC_AHB1Periph_GPIOE;
		}	break;
		case GPIOF_BASE:
		{
			port = RCC_AHB1Periph_GPIOF;
		}	break;
		case GPIOG_BASE:
		{
			port = RCC_AHB1Periph_GPIOG;
		}	break;
		case GPIOH_BASE:
		{
			port = RCC_AHB1Periph_GPIOH;
		}	break;
		case GPIOI_BASE:
		{
			port = RCC_AHB1Periph_GPIOI;
		}	break;
		
		default:
		{
			//error unknown GPIO Port
			while(1);
		}	break;
	}
	RCC_AHB1PeriphClockCmd(port, ENABLE);
	GPIO_WriteBit(GPIOx, GPIO_Pin, BitVal);
}


//+=============================================================================================
//|		Function:	gpio_Write
//+---------------------------------------------------------------------------------------------
//!		Write to GPIO.
//+---------------------------------------------------------------------------------------------
//|		Conditions:
//!		Thread-Safe: (NO)
//!		\pre	(pre-condition)
//!		\post	(post-condition)
//+---------------------------------------------------------------------------------------------
//|		Annotations:
//+=============================================================================================
void	gpio_Write(GPIO_TypeDef* GPIOx, INT16U PortVal)
{
	GPIO_Write(GPIOx, PortVal);
}

//+=============================================================================================
//|		Aenderungsjournal
//+=============================================================================================
#ifdef __DOCGEN
/*!
@page revisions Revisions

*/
#endif
