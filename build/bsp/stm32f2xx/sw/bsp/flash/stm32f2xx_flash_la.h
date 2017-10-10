/********************************************************************************
* File Name          : stm32f10x_flash_la.h
* Author             : WRD
* Date First Issued  : 07/19/2007
* Description        : additional incformations not provided by standard 
*                      include files.
 CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32F10x_FLASH_LA_H
#define __STM32F10x_FLASH_LA_H

#define CR_LOCK_Set              FLASH_CR_LOCK

/* Flash Control Register bits */
#define CR_PG_Set                ((u32)0x00000001)
#define CR_PG_Reset              ((u32)0x00001FFE) 

/* SL_FLASH_delay definition */   
#define EraseTimeout             ((uint32_t)0x00000FFF)
#define ProgramTimeout           ((uint32_t)0x0000000F)

#if 0
/* Flash Control Register bits */
#define CR_PG_Set                ((u32)0x00000001)
#define CR_PG_Reset              ((u32)0x00001FFE) 

#define CR_PER_Set               ((u32)0x00000002)
#define CR_PER_Reset             ((u32)0x00001FFD)

#define CR_MER_Set               ((u32)0x00000004)
#define CR_MER_Reset             ((u32)0x00001FFB)

#define CR_OPTPG_Set             ((u32)0x00000010)
#define CR_OPTPG_Reset           ((u32)0x00001FEF)

#define CR_OPTER_Set             ((u32)0x00000020)
#define CR_OPTER_Reset           ((u32)0x00001FDF)

#define CR_STRT_Set              ((u32)0x00000040)
							 
#define CR_LOCK_Set              ((u32)0x00000080)

/* FLASH Keys */
#define RDP_Key                  ((uint16_t)0x00A5)
#define FLASH_KEY1               ((uint32_t)0x45670123)
#define FLASH_KEY2               ((uint32_t)0xCDEF89AB)

/* SL_FLASH_delay definition */   
#define EraseTimeout             ((uint32_t)0x00000FFF)
#define ProgramTimeout           ((uint32_t)0x0000000F)
#endif

#endif /* __STM32F10x_FLASH_LA_H */
