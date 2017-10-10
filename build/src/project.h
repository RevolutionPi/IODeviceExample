/*=============================================================================================
*
* project.h
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
#ifndef _PROJECT_H_
#define	_PROJECT_H_

#ifdef __cplusplus
extern "C" {
#endif

//+=============================================================================================
//|		Include-Dateien / include files
//+=============================================================================================

#include <common_define.h>

//+=============================================================================================
//|		Konstanten / constants
//+=============================================================================================

#define HW_REVISION             1

#define PI_IO_PROTOCOL

#define kbCOM_LITTLE_ENDIAN     1

// BSP Configuration
#define STM_CLOCK_25_120

#define STM_VOLTAGE_RANGE           VoltageRange_3

#define STM32F2XX
#define STM_WITH_GPIO
#define STM_WITH_EEPROM
#define STM_WITH_EEPROM_ADDR16
#define STM_WITH_SPI
#define STM_WITH_COUNT_DOWN_TIMER

// Uart for mGate RS485-Communication
#define STM_UART3_RS485_PIN         GPIO_Pin_12 // Transmit enable pin
#define STM_UART3_RS485_PORT        GPIOC       // Transmit enable port
#define MODGATE_RS485_UART          UART_PORT_3 // Rx/Tx lines
#define STM_UART3_REMAP_PARTIAL

#define STM_WITH_UART

// PI IO-Module
#define PIIO_SPIOUTP_PORT           1
#define PIIO_SPIOUTP_CS_PORT        GPIOB           // SPI Output
#define PIIO_SPIOUTP_CS_PIN         GPIO_Pin_12
#define PIIO_SPIINP_PORT            0
#define PIIO_SPIINP_CS_PORT         GPIOA           // SPI Input
#define PIIO_SPIINP_CS_PIN          GPIO_Pin_4
#define PIIO_CFGOUTP_PORT           GPIOC         // Config Output
#define PIIO_CFGOUTP_PIN            GPIO_Pin_6
#define PIIO_FAULTOUT_PORT          GPIOB        // Fault Output
#define PIIO_FAULTOUT_PIN           GPIO_Pin_0
#define PIIO_FAULTIN_PORT           GPIOB         // Fault Input
#define PIIO_FAULTIN_PIN            GPIO_Pin_1
#define PIIO_INPFLTR1_PORT          GPIOA        // Fault Input Filter 1
#define PIIO_INPFLTR1_PIN           GPIO_Pin_11
#define PIIO_INPFLTR2_PORT          GPIOA        // Fault Input Filter 2
#define PIIO_INPFLTR2_PIN           GPIO_Pin_12
#define PIIO_RS485TERM_PORT         GPIOC       // RS485 Termination
#define PIIO_RS485TERM_PIN          GPIO_Pin_9
#define PIIO_VARIANT_H_PORT         GPIOC       // PiDio-Variante High-Bit
#define PIIO_VARIANT_H_PIN          GPIO_Pin_8
#define PIIO_VARIANT_L_PORT         GPIOC       // PiDio-Variante Low-Bit
#define PIIO_VARIANT_L_PIN          GPIO_Pin_7

#define PBSNIFF1A_PORT              GPIOA       // PiBridge Sniffer 1A
#define PBSNIFF1A_PIN               GPIO_Pin_8
#define PBSNIFF1B_PORT              GPIOA       // PiBridge Sniffer 1B
#define PBSNIFF1B_PIN               GPIO_Pin_10
#define PBSNIFF2_PORT               GPIOA        // PiBridge Sniffer 2
#define PBSNIFF2_PIN                GPIO_Pin_9

// Interrupt priorities
#define STM_INTERRUPT_PRIORITYGROUP NVIC_PriorityGroup_4
#define STM_INTERRUPT_PREEMPTIONPRIORITY_TIMER1 1
#define STM_INTERRUPT_SUBPRIORITY_TIMER1 1
#define STM_INTERRUPT_PREEMPTIONPRIORITY_TIMER2 1
#define STM_INTERRUPT_SUBPRIORITY_TIMER2 1
#define STM_INTERRUPT_PREEMPTIONPRIORITY_TIMER3 1
#define STM_INTERRUPT_SUBPRIORITY_TIMER3 1
#define STM_INTERRUPT_PREEMPTIONPRIORITY_TIMER4 1
#define STM_INTERRUPT_SUBPRIORITY_TIMER4 1
#define STM_INTERRUPT_PREEMPTIONPRIORITY_TIMER5 1
#define STM_INTERRUPT_SUBPRIORITY_TIMER5 1
#define STM_INTERRUPT_PREEMPTIONPRIORITY_ETH 1
#define STM_INTERRUPT_SUBPRIORITY_ETH 0
#define STM_INTERRUPT_PREEMPTIONPRIORITY_SPI1 0
#define STM_INTERRUPT_SUBPRIORITY_SPI1 1
#define STM_INTERRUPT_PREEMPTIONPRIORITY_SPI2 0
#define STM_INTERRUPT_SUBPRIORITY_SPI2 1
#define STM_INTERRUPT_PREEMPTIONPRIORITY_SPI3 0
#define STM_INTERRUPT_SUBPRIORITY_SPI3 1

// mGate-Decodierung left/right module
#define MGATE_DECODE_MS1B
#define MGATE_DECODE_MS1B_PORT              PBSNIFF1B_PORT
#define MGATE_DECODE_MS1B_PIN               PBSNIFF1B_PIN

#define RS485_STATE_TIMER                   TIM_TIMER1      // Use Timer 1 for mGate RS485 state machine 

// Timer for input and output signals
#define IO_TIMER_TIME_BASE         1               // Time base in µs
#define IO_TIMER_TIME_CYCLE        250             // Cyclic time 
#define IO_TIMER                   TIM_TIMER3      // Use Timer 3

// LED definitions
#define STM_WITH_LED
#define STM_LED_GPIO

#define BSP_FATAL_ERROR_LED                 0
#define LED_MAX_DUAL_LED                    3
#define PIDIO_LED_POWER                     0 // Power-LED
#define PIDIO_LED_OUTPUT_STATUS             1
#define PIDIO_LED_INPUT_STATUS              2

#define LED_0_R_PORT    GPIOA
#define LED_0_R_PIN     GPIO_Pin_1

#define LED_1_G_PORT    GPIOA
#define LED_1_G_PIN     GPIO_Pin_2
#define LED_1_R_PORT    GPIOA
#define LED_1_R_PIN     GPIO_Pin_3

#define LED_2_G_PORT    GPIOC
#define LED_2_G_PIN     GPIO_Pin_0
#define LED_2_R_PORT    GPIOC
#define LED_2_R_PIN     GPIO_Pin_1






//+=============================================================================================
//|		Typen / types
//+=============================================================================================

//typedef 

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


#endif//_PROJECT_H_
