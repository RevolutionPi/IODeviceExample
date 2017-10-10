/**
  ******************************************************************************
  * @file    EEPROM_Emulation/inc/eeprom.h 
  * @author  MCD Application Team
  * @version V3.1.0
  * @date    07/27/2009
  * @brief   This file contains all the functions prototypes for the EEPROM 
  *          emulation firmware library.
  ******************************************************************************
  * @copy
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2009 STMicroelectronics</center></h2>
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef EEPROM_INTERN_H_INC
#define EEPROM_INTERN_H_INC

/* Includes ------------------------------------------------------------------*/
#ifdef STM32F2XX
#include <SysLib\inc\stm32f2xx.h>
#else
#include <syslib\CM3\stm32f10x.h>
#endif


#if defined (STM_WITH_EEPROM_ADDR16)     // use 16 bit address

#define MAX_EEPROM_ADDRESS      850     /* A Maximum of 850 address values in the  EEPROM are supported. 
                                        This value could be much higher, but then the page transfer will be slower. */
#define EEPROM_ADDRESS_MASK     0x0000ffff
#define EEPROM_ADDRESS_BITS     16

typedef INT32U      EEPROM_INTERNAL_TYPE;

#elif defined(STM_WITH_EEPROM_ADDR8) // STM32F1XX  use 8 bit address

#define MAX_EEPROM_ADDRESS      254     /* A Maximum of 255 Byte EEPROM Supported */
#define EEPROM_ADDRESS_MASK     0x00ff
#define EEPROM_ADDRESS_BITS     8

typedef INT16U      EEPROM_INTERNAL_TYPE;

#else
#error "eeprom cannot be used without STM_WITH_EEPROM_ADDRXX to be defined !"
#endif









/* Exported constants --------------------------------------------------------*/
/* Define the STM32F10Xxx Flash page size depending on the used STM32 device */
#if defined (STM32F10X_LD) || defined (STM32F10X_MD)
  #define PAGE_SIZE  (INT16U)0x400  /* Page size = 1KByte */
#elif defined (STM32F10X_HD) || defined (STM32F10X_CL)
  #define PAGE_SIZE  (INT16U)0x800  /* Page size = 2KByte */
#elif defined (STM32F2XX)
	// we use the 16KByte pages for EEPROM simulation
	#define PAGE_SIZE  (INT16U)0x4000  /* Page size = 16KByte */
#endif


#if !defined (PAGE_SIZE)
    #error "Define processor category"
#endif    



/* EEPROM start address in Flash */
extern EEPROM_INTERNAL_TYPE laStartEepromSimu_g;
extern EEPROM_INTERNAL_TYPE laEndEepromSimu_g;
extern EEPROM_INTERNAL_TYPE laStartStorePara_g;
extern EEPROM_INTERNAL_TYPE laEndStorePara_g;

#define EEPROM_START_ADDRESS    (&laStartEepromSimu_g)

#ifdef FACTORY_RESET_ERASES_STORE_PARA_FLASH_PAGE
#define EEPROM_STORE_PARA_START_ADDRESS (&laStartStorePara_g)
#define EEPROM_STORE_PARA_STOP_ADDRESS  (&laEndStorePara_g)
#endif /*FACTORY_RESET_ERASES_STORE_PARA_FLASH_PAGE*/

/* Pages 0 and 1 base and end addresses */
#define PAGE0_BASE_ADDRESS      (EEPROM_START_ADDRESS)
#define PAGE1_BASE_ADDRESS      (EEPROM_START_ADDRESS + (PAGE_SIZE / sizeof (EEPROM_INTERNAL_TYPE)))

/* Used Flash pages for EEPROM emulation */
#define PAGE0                   ((EEPROM_INTERNAL_TYPE)0x00000000)
#define PAGE1                   ((EEPROM_INTERNAL_TYPE)0x00000001)

/* No valid page define */
#define NO_VALID_PAGE           ((EEPROM_INTERNAL_TYPE)0x000000AB)

/* Page status definitions */
#define ERASED                  ((EEPROM_INTERNAL_TYPE)0xFFFFFFFF)     /* PAGE is empty */
#define RECEIVE_DATA            ((EEPROM_INTERNAL_TYPE)0xEEEEEEEE)     /* PAGE is marked to receive data */
#define VALID_PAGE              ((EEPROM_INTERNAL_TYPE)0x00000000)     /* PAGE containing valid data */

/* Valid pages in read and write defines */
#define READ_FROM_VALID_PAGE    ((INT8U)0x00)
#define WRITE_IN_VALID_PAGE     ((INT8U)0x01)

/* Page full define */
#define PAGE_FULL               ((INT8U)0x80)


#endif /* EEPROM_INTERN_H_INC */

/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/
