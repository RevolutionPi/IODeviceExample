/******************** (C) COPYRIGHT 2007 STMicroelectronics ********************
* File Name          : stm32f10x_vector.c
* Author             : MCD Application Team
* Date First Issued  : 02/19/2007
* Description        : This file contains the vector table for STM32F10x
********************************************************************************
* History:
* 02/19/2007: V0.1
********************************************************************************
* THE PRESENT SOFTWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH SOFTWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include <common_define.h>
#include <project.h>
#include <bsp\bspConfig.h>

#include <SysLib\inc\stm32f2xx.h>
#include <bsp\systick\systick.h>
#if defined (STM_WITH_CAN1) || defined (STM_WITH_CAN2)
#include <bsp\can\CanDriverIntern.h>
#endif

#include <bsp\spi\spi.h>
#include <bsp\uart\uart_intern.h>
#include <bsp\timer\timer_intern.h>

#if defined (STM_WITH_ETH)
    extern void BSP_ETH_STM32_IRQHandler (void);
#endif


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/
extern INT32U _etext;
/* start address for the initialization values of the .data section. 
defined in linker script */
extern INT32U _sidata;

/* start address for the .data section. defined in linker script */    
extern INT32U _sdata;

/* end address for the .data section. defined in linker script */    
extern INT32U _edata;

/* start address for the .bss section. defined in linker script */
extern INT32U _sbss;

/* end address for the .bss section. defined in linker script */      
extern INT32U _ebss;

/* init value for the stack pointer. defined in linker script */
extern INT8U _estack;  
extern INT8U laStartRamTest_g;
extern INT8U laEndRamTest_g;
extern INT32U laStartFirmwareCrc_g;
extern INT32U laEndFirmwareCrc_g;

TBOOL bInitErr_RamTest_g;       //!< indicates an initial RAM test error to the application
TBOOL bInitErr_CrcTest_g;       //!< indicates an initial CRC error over the firmware
TBOOL bInitErr_SerTest_g;       //!< indicates an Serial number error
extern INT32U i32uFirmwareCrc_g;

/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
void Reset_Handler(void);
void __Init_Data(void);
/* Private functions ---------------------------------------------------------*/

extern int  main                (void);

extern void SPI_Spi1IntHandler  (void);
extern void SPI_Spi2IntHandler  (void);
extern void SPI_Spi3IntHandler  (void);


void _default_handler (void)
{
    volatile    INT32U      active_irq;

    __disable_irq();

    active_irq = *(INT32U*)0xE000ED04; ///< NVIC interrupt control state register bit[8..0] = active irq

    active_irq = active_irq;

    while(1)
    {

    }
}

void _fault_handler (void)
{   
	

    asm volatile (
        "TST    lr, #0x4\n"
        "ITTEE  eq\n"
        "MRSEQ  r7, msp\n"
        "LDREQ  r7, [r7, #24]\n"
        "MRSNE  r7, psp\n"
        "LDRNE  r7, [r7, #24]\n"
        "MOV    r0, r7");

    //UFSR_BFSR_MFSR = *(INT32U*)0xE000ED28;  ///< Usage Fault status register [31:16]
    ///< Bus Fault Status register [15:8]
    ///< MemManage Fault status register [7:0]

    asm volatile (
        "movw   r7, #0xed28\n"
        "movt   r7, #0xe000\n"
        "ldr    r1, [r7, #0]\n");

    //HFSR = *(INT32U*)0xE000ED2C;            ///< Hard Fault status register
    asm volatile (
        "movw   r7, #0xed2c\n"
        "ldr    r2, [r7, #0]\n");

    //DFSR = *(INT32U*)0xE000ED30;            ///< debug Fault status register
    asm volatile (
        "movw   r7, #0xed30\n"
        "ldr    r3, [r7, #0]\n");
    //AFSR = *(INT32U*)0xE000ED3C;            ///< Aux Fault status register
    asm volatile (
        "movw   r7, #0xed3c\n"
        "ldr    r4, [r7, #0]\n");

    while(1)
    {

    }
}


//+=============================================================================================
//|     Function:   __vector_table
//+---------------------------------------------------------------------------------------------
//!     STM32F10x Vector Table entries.
//!     (detailed description)
//+---------------------------------------------------------------------------------------------
//|     Conditions:
//!     \pre    (none)
//!     \post   (none)
//+---------------------------------------------------------------------------------------------
//|     Annotations:
//+=============================================================================================
__attribute__ ((section(".isr_vector"))) void (* const __vector_table[])(void)  =

{
    (void (* const)())&_estack,         //   Stack Top, defined in linkerscript
    Reset_Handler,
    _fault_handler,                     // -14 NMIException
    _fault_handler,                     // -13 HardFaultException
    _fault_handler,                     // -12 MemManageException
    _fault_handler,                     // -11 BusFaultException
    _fault_handler,                     // -10 UsageFaultException
    _fault_handler,                     // -9 Reserved
    _fault_handler,                     // -8 Reserved
    _fault_handler,                     // -7 Reserved
    _fault_handler,                     // -6 Reserved
    _fault_handler,                     // -5 SVCHandler
    _fault_handler,                     // -4 DebugMonitor
    _fault_handler,                     // -3 Reserved
    _fault_handler,                     // -2 PendSVC
    SysTickHandler,                     // -1 SysTickHandler
    _default_handler,                   // 0 WWDG_IRQHandler
    _default_handler,                   // 1 PVD_IRQHandler
    _default_handler,                   // 2 TAMPER_IRQHandler
    _default_handler,                   // 3 RTC_IRQHandler
    _default_handler,                   // 4 FLASH_IRQHandler
    _default_handler,                   // 5 RCC_IRQHandler
#if defined (STM_WITH_EXTI)
    EXTI0_IRQHandler,                   // 6 EXTI0_IRQHandler
    EXTI1_IRQHandler,                   // 7 EXTI1_IRQHandler
    EXTI2_IRQHandler,                   // 8 EXTI2_IRQHandler
    EXTI3_IRQHandler,                   // 9 EXTI3_IRQHandler
    EXTI4_IRQHandler,                   // 10 EXTI4_IRQHandler
#else
    _default_handler,                   // 6 EXTI0_IRQHandler
    _default_handler,                   // 7 EXTI1_IRQHandler 
    _default_handler,                   // 8 EXTI2_IRQHandler
    _default_handler,                   // 9 EXTI3_IRQHandler 
    _default_handler,                   // 10 EXTI4_IRQHandler 
#endif
#if defined (STM_WITH_DMA)
    DMA1Stream0_IRQHandler,             // 11 DMAStream0_IRQHandler
    DMA1Stream1_IRQHandler,             // 12 DMAStream1_IRQHandler
    DMA1Stream2_IRQHandler,             // 13 DMAStream2_IRQHandler
    DMA1Stream3_IRQHandler,             // 14 DMAStream3_IRQHandler
    DMA1Stream4_IRQHandler,             // 15 DMAStream4_IRQHandler
    DMA1Stream5_IRQHandler,             // 16 DMAStream5_IRQHandler
    DMA1Stream6_IRQHandler,             // 17 DMAStream6_IRQHandler
#else
    _default_handler,                   // 11 DMAStream0_IRQHandler
    _default_handler,                   // 12 DMAStream1_IRQHandler
    _default_handler,                   // 13 DMAStream2_IRQHandler
    _default_handler,                   // 14 DMAStream3_IRQHandler
    _default_handler,                   // 15 DMAStream4_IRQHandler
    _default_handler,                   // 16 DMAStream5_IRQHandler
    _default_handler,                   // 17 DMAStream6_IRQHandler
#endif
    _default_handler,                   // 18 ADC_IRQHandler
#if defined (STM_WITH_CAN1)
    CAN_Can1TxIntHandler,               // 19 USB_HP_CAN_TX_IRQHandler
    CAN_Can1Rx0IntHandler,              // 20 USB_LP_CAN_RX0_IRQHandler
    CAN_Can1Rx1IntHandler,              // 21 CAN_RX1_IRQHandler
    CAN_Can1EsrIntHandler,              // 22 CAN_SCE_IRQHandler
#else
    _default_handler,                   // 19 USB_HP_CAN_TX_IRQHandler
    _default_handler,                   // 20 USB_LP_CAN_RX0_IRQHandler
    _default_handler,                   // 21 CAN_RX1_IRQHandler
    _default_handler,                   // 22 CAN_SCE_IRQHandler
#endif
#if defined (STM_WITH_EXTI)
    EXTI9_5_IRQHandler,                 // 23 EXTI9_5_IRQHandler
#else    
    _default_handler,                   // 23 EXTI9_5_IRQHandler
#endif

    _default_handler,                   // 24 TIM1_BRK_IRQHandler
#if defined (STM_WITH_COUNT_DOWN_TIMER)
    TIM1_intHandler,                    // 25 TIM1_UP_IRQHandler
#else
    _default_handler,                   // 25 TIM1_UP_IRQHandler
#endif
    _default_handler,                   // 26 TIM1_TRG_CCUP_IRQHandler
    _default_handler,                   // 27 TIM1_CC_IRQHandler

#if defined (BSP_SCHED_USE_TIM2)    
    BSP_SCHED_intHandlerTimer,          // 28 TIM2_IRQHandler
#else
#if defined (STM_WITH_COUNT_DOWN_TIMER)
    TIM2_intHandler,                    // 28 TIM2_IRQHandler
#else
    _default_handler,                   // 28 TIM2_IRQHandler
#endif
#endif

#if defined (BSP_SCHED_USE_TIM3)    
    BSP_SCHED_intHandlerTimer,          // 29 TIM3_IRQHandler
#else
#if defined (STM_WITH_COUNT_DOWN_TIMER)
    TIM3_intHandler,                    // 29 TIM3_IRQHandler
#else
    _default_handler,                   // 29 TIM3_IRQHandler
#endif  
#endif

#if defined (BSP_SCHED_USE_TIM4)    
    BSP_SCHED_intHandlerTimer,          // 30 TIM2_IRQHandler
#else
#if defined (STM_WITH_COUNT_DOWN_TIMER)
    TIM4_intHandler,                    // 30 TIM4_IRQHandler
#else
    _default_handler,                   // 30 TIM4_IRQHandler
#endif
#endif  

#if defined (STM_WITH_I2C)
    i2c_I2C1EVIntHandler,               // 31 I2C1_EV_IRQHandler
    i2c_I2C1ERIntHandler,               // 32 I2C1_ER_IRQHandler
    i2c_I2C2EVIntHandler,               // 33 I2C1_EV_IRQHandler
    i2c_I2C2ERIntHandler,               // 34 I2C1_ER_IRQHandler
#else
    _default_handler,                   // 31 I2C1_EV_IRQHandler
    _default_handler,                   // 32 I2C1_ER_IRQHandler
    _default_handler,                   // 33 I2C2_EV_IRQHandler
    _default_handler,                   // 34 I2C2_ER_IRQHandler
#endif
#if defined (STM_WITH_SPI)
    SPI_Spi1IntHandler,                 // 35 SPI1_IRQHandler
    SPI_Spi2IntHandler,                 // 36 SPI2_IRQHandler
#else
    _default_handler,                   // 35 SPI1_IRQHandler
    _default_handler,                   // 36 SPI2_IRQHandler
#endif
#if defined (STM_WITH_UART)
    UART_uart1IntHandler,               // 37 USART1_IRQHandler
    UART_uart2IntHandler,               // 38 USART2_IRQHandler
    UART_uart3IntHandler,               // 39 USART3_IRQHandler
#else
    _default_handler,                   // 37 USART1_IRQHandler
    _default_handler,                   // 38 USART2_IRQHandler
    _default_handler,                   // 39 USART3_IRQHandler
#endif
#if defined (STM_WITH_EXTI)
    EXTI15_10_IRQHandler,               // 40 EXTI15_10_IRQHandler
#else
    _default_handler,                   // 40 EXTI15_10_IRQHandler
#endif
    _default_handler,                   // 41 RTCAlarm_IRQHandler
    _default_handler,                   // 42 USBWakeUp_IRQHandler
    _default_handler,                   // 43 TIM8_BRK_TIM12
    _default_handler,                   // 44 TIM8_UP_TIM13
    _default_handler,                   // 45 TIM8_TRG_COM_TIM14
    _default_handler,                   // 46 TIM8_CC
#if defined (STM_WITH_DMA)
    DMA1Stream7_IRQHandler,              // 47 DMA1_Stream7
#else
    _default_handler,                   // 47 DMA1_Stream7
#endif
    _default_handler,                   // 48 FSMC
    _default_handler,                   // 49 SDIO
#if defined (STM_WITH_COUNT_DOWN_TIMER)
    TIM5_intHandler,                    // 50 TIM5
#else
    _default_handler,                   // 50 TIM5
#endif
#ifdef STM_WITH_SPI 
    SPI_Spi3IntHandler,                 // 51 SPI3_IRQHandler
#else
    _default_handler,                   // 51
#endif
#if defined (STM_WITH_UART)
    UART_uart4IntHandler,               // 52 UART4_IRQHandler
    UART_uart5IntHandler,               // 53 UART5_IRQHandler
#else
    _default_handler,                   // 52 UART4_IRQn          / UART4 global Interrupt
    _default_handler,                   // 53 UART5_IRQn          / UART5 global Interrupt
#endif
    _default_handler,                   // 54 TIM6_IRQn           / TIM6 global Interrupt
    _default_handler,                   // 55 TIM7_IRQn           / TIM7 global Interrupt
#if defined (STM_WITH_DMA)
    DMA2Stream0_IRQHandler,             // 56 DMA2_Stream0_IRQn  / DMA2 Channel 1 global Interrupt
    DMA2Stream1_IRQHandler,             // 57 DMA2_Stream1_IRQn  / DMA2 Channel 2 global Interrupt
    DMA2Stream2_IRQHandler,             // 58 DMA2_Stream2_IRQn  / DMA2 Channel 3 global Interrupt
    DMA2Stream3_IRQHandler,             // 59 DMA2_Stream3_IRQn  / DMA2 Channel 4 global Interrupt
    DMA2Stream4_IRQHandler,             // 60 DMA2_Stream4_IRQn  / DMA2 Channel 5 global Interrupt
#else
    _default_handler,                   // 56 DMA2_Stream0_IRQn  / DMA2 Channel 1 global Interrupt
    _default_handler,                   // 57 DMA2_Stream1_IRQn  / DMA2 Channel 2 global Interrupt
    _default_handler,                   // 58 DMA2_Stream2_IRQn  / DMA2 Channel 3 global Interrupt
    _default_handler,                   // 59 DMA2_Stream3_IRQn  / DMA2 Channel 4 global Interrupt
    _default_handler,                   // 60 DMA2_Stream4_IRQn  / DMA2 Channel 5 global Interrupt
#endif
#if defined (STM_WITH_ETH)
    BSP_ETH_STM32_IRQHandler,           // 61 ETH_IRQn       / Ethernet global Interrupt
#else
    _default_handler,                   // 61 ETH_IRQn       / Ethernet global Interrupt
#endif
    _default_handler,                   // 62 ETH_WKUP_IRQn  / Ethernet Wakeup through EXTI line Interrupt
#ifdef STM_WITH_CAN2
    CAN_Can2TxIntHandler,               // 63 CAN2_TX_IRQn   / CAN2 TX Interrupt
    CAN_Can2Rx0IntHandler,              // 64 CAN2_RX0_IRQn  / CAN2 RX0 Interrupt
    CAN_Can2Rx1IntHandler,              // 65 CAN2_RX1_IRQn  / CAN2 RX1 Interrupt
    CAN_Can2EsrIntHandler,              // 66 CAN2_SCE_IRQn  / CAN2 SCE Interrupt
#else
    _default_handler,                   // 63 CAN2_TX_IRQn   / CAN2 TX Interrupt
    _default_handler,                   // 64 CAN2_RX0_IRQn  / CAN2 RX0 Interrupt
    _default_handler,                   // 65 CAN2_RX1_IRQn  / CAN2 RX1 Interrupt
    _default_handler,                   // 66 CAN2_SCE_IRQn  / CAN2 SCE Interrupt
#endif
    _default_handler,                   // 67 OTG_FS_IRQn    / USB OTG FS global Interrupt
#if defined (STM_WITH_DMA)
    DMA2Stream5_IRQHandler,             // 68 DMA2_Stream5_IRQn
    DMA2Stream6_IRQHandler,             // 69 DMA2_Stream6_IRQn
    DMA2Stream7_IRQHandler,             // 70 DMA2_Stream7_IRQn
#else
    _default_handler,                   // 68 DMA2_Stream5_IRQn
    _default_handler,                   // 69 DMA2_Stream6_IRQn
    _default_handler,                   // 70 DMA2_Stream7_IRQn
#endif
#if defined (STM_WITH_UART) 
    UART_uart6IntHandler,               // 71 USART6_IRQHandler
#else
    _default_handler,                   // 71 USART6
#endif
    _default_handler,                   // 72 I2C3_EV
    _default_handler,                   // 73 I2C3_ER
    _default_handler,                   // 74 OTG_HS_EP1_OUT
    _default_handler,                   // 75 OTG_HS_EP1_IN
    _default_handler,                   // 76 OTG_HS_WKUP
    _default_handler,                   // 77 OTG_HS
    _default_handler,                   // 78 DCMI
    _default_handler,                   // 79 CRYP
    _default_handler,                   // 80 HASH_RNG
};




//+=============================================================================================
//|     Function:   Reset_Handler
//+---------------------------------------------------------------------------------------------
//!     STM32xxxx reset handler function.
//!     This is the code that gets called when the processor first starts execution following a 
//!     reset event. Only the absolutely necessary set is performed, after which the application
//!     supplied main() routine is called. 
//+---------------------------------------------------------------------------------------------
//|     Conditions:
//!     \pre    (pre-condition)
//!     \post   (post-condition)
//+---------------------------------------------------------------------------------------------
//|     Annotations:
//+=============================================================================================
void Reset_Handler(void)
{
    register TBOOL bError_l asm ("r8");

#if !defined (STM32_NORAMTEST)
    register INT8U *pi8uMem_l asm ("r9");
    register INT8U i8uInd_l asm ("r10");
    register INT8U i8uOldPattern_l asm ("r11");
    const INT8U ai8uPattern_l[] = {0x55, 0xcc, 0x33, 0xf0, 0x0f, 0xcd};

    //INT32U *pi32uMem_l;  // Used for CRC-Calculation

    // perform a RAM test and initialize the ram at last to 0xcd pattern
    bError_l = bFALSE;
    for (pi8uMem_l = &laStartRamTest_g; pi8uMem_l <= &laEndRamTest_g; pi8uMem_l++)
    {
        *pi8uMem_l = 0xaa;  
    }
    i8uOldPattern_l = 0xaa;


    for (i8uInd_l = 0; bError_l == bFALSE && i8uInd_l < sizeof (ai8uPattern_l); i8uInd_l++)
    {
        for (pi8uMem_l = &laStartRamTest_g; pi8uMem_l <= &laEndRamTest_g; pi8uMem_l++)
        {
            if (*pi8uMem_l != i8uOldPattern_l)
            {   // Error
                bError_l = bTRUE;
                break;
            }
            *pi8uMem_l = ai8uPattern_l[i8uInd_l];  
        }
        i8uOldPattern_l = ai8uPattern_l[i8uInd_l];
    }    



    /* restore original stack pointer */
    asm(" LDR r0, =_estack");
    asm(" MSR msp, r0");
#endif

    /* Initialize data and bss */
    __Init_Data();


    // save the result of the RAM test in a global variable
    bInitErr_RamTest_g = bError_l;      

    bInitErr_CrcTest_g = bFALSE;

    /* Call the application's entry point.*/
    main();

    /* Should never be reached */
    while (1)
    {

    }
}

/**
* @brief  initializes data and bss sections
*/

void __Init_Data(void)
{
    INT32U *pulSrc; 
    INT32U *pulDest;

    /* Copy the data segment initializers from flash to SRAM */
    pulSrc = (INT32U *)&_sidata;

    for(pulDest = (INT32U *)&_sdata; pulDest < (INT32U *)&_edata; )
    {
        *(pulDest++) = *(pulSrc++);
    }
    /* Zero fill the bss segment. */
    for(pulDest = (INT32U *)&_sbss; pulDest < (INT32U *)&_ebss; )
    {
        *(pulDest++) = 0;
    }
}

/******************* (C) COPYRIGHT 2007 STMicroelectronics *****END OF FILE****/


