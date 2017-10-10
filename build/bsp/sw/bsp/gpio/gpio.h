/*=============================================================================================
*
* gpio.h
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
*
* General Purpose I/O handling functions.
*
* These functions are designed to handle GPIO Ports and Pins.
*/
#ifndef _GPIO_H_
#define	_GPIO_H_

#ifdef __cplusplus
extern "C" {
#endif

//+=============================================================================================
//|		Include-Dateien / include files
//+=============================================================================================

#if defined (STM32F2XX)
  #include <SysLib\inc\stm32f2xx_gpio.h>
#elif defined (STM32F30X)
  #include <syslib\inc\stm32f30x_gpio.h>
#elif defined (STM32F40_41xxx) || defined (STM32F427_437xx) || defined (STM32F429_439xx) || defined (STM32F401xx)
  #include <syslib\inc\stm32f4xx_gpio.h>
#elif defined __SF2_GENERIC__
  #include <drivers\mss_gpio\mss_gpio.h>
#else
  #include <SysLib\inc\stm32f10x_gpio.h>
#endif

#if defined	(STM_WITH_GPIO)
#else
#error "Define STM_WITH_GPIO in order to use GPIO BSP !"
#endif


//+=============================================================================================
//|		Konstanten / constants
//+=============================================================================================

#if !defined(__STM32F10x_GPIO_H) && !defined(__STM32F2xx_GPIO_H)
#define GPIO_Pin_0                 ((uint16_t)0x0001)  ///< Pin 0 selected
#define GPIO_Pin_1                 ((uint16_t)0x0002)  ///< Pin 1 selected
#define GPIO_Pin_2                 ((uint16_t)0x0004)  ///< Pin 2 selected
#define GPIO_Pin_3                 ((uint16_t)0x0008)  ///< Pin 3 selected
#define GPIO_Pin_4                 ((uint16_t)0x0010)  ///< Pin 4 selected
#define GPIO_Pin_5                 ((uint16_t)0x0020)  ///< Pin 5 selected
#define GPIO_Pin_6                 ((uint16_t)0x0040)  ///< Pin 6 selected
#define GPIO_Pin_7                 ((uint16_t)0x0080)  ///< Pin 7 selected
#define GPIO_Pin_8                 ((uint16_t)0x0100)  ///< Pin 8 selected
#define GPIO_Pin_9                 ((uint16_t)0x0200)  ///< Pin 9 selected
#define GPIO_Pin_10                ((uint16_t)0x0400)  ///< Pin 10 selected
#define GPIO_Pin_11                ((uint16_t)0x0800)  ///< Pin 11 selected
#define GPIO_Pin_12                ((uint16_t)0x1000)  ///< Pin 12 selected
#define GPIO_Pin_13                ((uint16_t)0x2000)  ///< Pin 13 selected
#define GPIO_Pin_14                ((uint16_t)0x4000)  ///< Pin 14 selected
#define GPIO_Pin_15                ((uint16_t)0x8000)  ///< Pin 15 selected
#define GPIO_Pin_All               ((uint16_t)0xFFFF)  ///< All pins selected
#endif

//+=============================================================================================
//|		Typen / types
//+=============================================================================================
#ifdef __STM32F10x_GPIO_H

typedef enum _GPIO_Port
{
	GPIO_A = GPIOA_BASE,
	GPIO_B = GPIOB_BASE,
	GPIO_C = GPIOC_BASE,
	GPIO_D = GPIOD_BASE,
	GPIO_E = GPIOE_BASE,
	GPIO_F = GPIOF_BASE,
	GPIO_G = GPIOG_BASE
} GPIO_Port;

typedef GPIOMode_TypeDef  KB_GPIOMode_TypeDef;
typedef GPIOSpeed_TypeDef KB_GPIOSpeed_TypeDef;
typedef GPIO_InitTypeDef  KB_GPIO_InitTypeDef;

#else

typedef enum _KB_GPIOMode_TypeDef
{ GPIO_Mode_AIN_ = 0x0,
  GPIO_Mode_IN_FLOATING = 0x04,
  GPIO_Mode_IPD = 0x28,
  GPIO_Mode_IPU = 0x48,
  GPIO_Mode_Out_OD = 0x14,
  GPIO_Mode_Out_PP = 0x10,
  GPIO_Mode_AF_OD = 0x1C,
  GPIO_Mode_AF_PP = 0x18
} KB_GPIOMode_TypeDef;

typedef GPIOSpeed_TypeDef KB_GPIOSpeed_TypeDef;

typedef struct _GPIO_InitTypeDef
{
  uint16_t GPIO_Pin;
  KB_GPIOSpeed_TypeDef GPIO_Speed;
  KB_GPIOMode_TypeDef GPIO_Mode;
} KB_GPIO_InitTypeDef;

/*
typedef enum _GPIO_Port
{
	GPIOA = GPIOA_BASE,
	GPIOB = GPIOB_BASE,
	GPIOC = GPIOC_BASE,
	GPIOD = GPIOD_BASE,
	GPIOE = GPIOE_BASE,
	GPIOF = GPIOF_BASE,
	GPIOG = GPIOG_BASE
} GPIO_Port;
*/

#endif
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

///	\defgroup	kb_gpio_handlers
/// @{
void	gpio_DeInit(GPIO_TypeDef* GPIOx);
void	kbGPIO_InitCLK(GPIO_TypeDef* GPIOx, KB_GPIO_InitTypeDef* GPIO_InitStruct);
void	gpio_StructInit(GPIO_InitTypeDef* GPIO_InitStruct);
INT8U	gpio_ReadInputDataBit(GPIO_TypeDef* GPIOx, INT16U GPIO_Pin);
INT16U	gpio_ReadInputData(GPIO_TypeDef* GPIOx);
INT8U	gpio_ReadOutputDataBit(GPIO_TypeDef* GPIOx, INT16U GPIO_Pin);
INT16U	gpio_ReadOutputData(GPIO_TypeDef* GPIOx);
void	kbGPIO_SetBitsCLK(GPIO_TypeDef* GPIOx, INT16U GPIO_Pin);
void	kbGPIO_ResetBitsCLK(GPIO_TypeDef* GPIOx, INT16U GPIO_Pin);
void	kbGPIO_WriteBitCLK(GPIO_TypeDef* GPIOx, INT16U GPIO_Pin, BitAction BitVal);
void	gpio_Write(GPIO_TypeDef* GPIOx, INT16U PortVal);
void	gpio_PinLockConfig(GPIO_TypeDef* GPIOx, INT16U GPIO_Pin);
void	gpio_JTAGcfg( void );

#ifndef STM32F2XX
void	gpio_AFIODeInit(void);
void	gpio_EventOutputConfig(INT8U GPIO_PortSource, INT8U GPIO_PinSource);
void	gpio_EventOutputCmd(FunctionalState NewState);
void	gpio_PinRemapConfig(INT32U GPIO_Remap, FunctionalState NewState);
void	gpio_EXTILineConfig(INT8U GPIO_PortSource, INT8U GPIO_PinSource);
void	gpio_ETH_MediaInterfaceConfig(INT32U GPIO_ETH_MediaInterface);
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


#endif//_GPIO_H_
