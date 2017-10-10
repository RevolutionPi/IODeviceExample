/*=============================================================================================
*
* platformError.h
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
#ifndef PLATFORMERROR_H_INC
#define PLATFORMERROR_H_INC

#include <stdarg.h>
#include <bsp\setjmp\BspSetJmp.h>

//+=============================================================================================
//|     Konstanten / constants
//+=============================================================================================

#define KB_NO_ERROR                             0x00000000    //!< Default Value, no Error, everything ok.

// base addresses for the different packages
#define KB_ERR_BASE_BSP                         0x00000000
#define KB_ERR_BASE_Utility                     0x01000000
#define KB_ERR_BASE_Platform                    0x02000000

#define KB_ERR_BASE_CANopen                     0x11000000
#define KB_ERR_BASE_DeviceNet                   0x12000000
#define KB_ERR_BASE_Profibus                    0x13000000
#define KB_ERR_BASE_CC_Link                     0x14000000

#define KB_ERR_BASE_EthernetIP                  0x21000000
#define KB_ERR_BASE_EtherCat                    0x22000000
#define KB_ERR_BASE_CIP                         0x23000000
#define KB_ERR_BASE_SercosIII                   0x24000000
#define KB_ERR_BASE_PROFINET_IO                 0x25000000
#define KB_ERR_BASE_Powerlink                   0x26000000
#define KB_ERR_BASE_MODBUS                      0x27000000
#define KB_ERR_BASE_PROFINET_TPS1               0x28000000

#define KB_ERR_BASE_PNOZcom                     0x31000000
#define KB_ERR_BASE_IC                          0x32000000
#define KB_ERR_BASE_MiniMultiCOM                0x33000000
#define KB_ERR_BASE_COMS                        0x34000000
#define KB_ERR_BASE_B0_COM                      0x35000000
#define KB_ERR_BASE_EtherCAT_Master             0x36000000

// RegTest Tool MMSerNumProg
#define CMN_RTC_MMSIM_CAN_INIT                  0x02000000      //!< Can Controller initialization failed
#define CMN_RTC_MMSIM_CAN_INIT2                 0x02000001      //!< Can Controller initialization failed
#define CMN_RTC_MMSIM_INTERN1                   0x02000002      //!< default reached
#define CMN_RTC_MMSIM_INTERN2                   0x02000003      //!< default reached
#define CMN_RTC_MMSIM_INTERN3                   0x02000004      //!< default reached
#define CMN_RTC_MMSIM_CAN_MEM1                  0x02000005      //!< no mem
#define CMN_RTC_MMSIM_INTERN4                   0x02000006      //!< 

#define CMN_RTC_CCD_CAN_MEM1                    0x02000007      //!< no mem
#define CMN_RTC_PET_CAN_MEM1                    0x02000008      //!< no mem
#define CMN_RTC_PB0S_CAN_MEM1                   0x02000009      //!< no mem
#define CMN_RTC_PB0S_INTERN4                    0x0200000a      //!< no mem

// SPI Master/Slave
#define CMN_SSC_OUTPUTBUF_UNDRFLW               0x02050000    //!< Output Buffer to small
#define CMN_SSC_OLEN_FB_OLEN_FAIL               0x02050001    //!< fb_olen > olen
#define CMN_SSC_ILEN_FB_ILEN_FAIL               0x02050002    //!< fb_ilen > ilen

// DPR
#define CMN_DPR_MBOX_INTERN1                    0x02060101      //!< mailbox does not support fragmented packets yet
#define CMN_DPR_MBOX_UNKNOWN_TYPE               0x02060102      //!< unknown packet type in mailbox 

#define CMN_DPR_ERROR_BASE                      0x0206f000      //!< Base for error codes sent to the base board
#define CMN_DPR_ERROR_END                       0x0206ffff      //!< End of Base for error codes sent to the base board

// Script interpreter
#define CMN_SC_SERIALIZE_TRACE                  0x02070000      //!< script, serialize trace variable
#define CMN_SC_SERIALIZE_TRACE_INCONSISTENT     0x02070001      //!< script, serialize trace is inconsistent
#define CMN_SC_SERIALIZE_TRACE_TOO_SHORT        0x02070002      //!< script, serialize trace shorter than longest record
#define CMN_SC_UNKNOWN_BAUD_RATE                0x02070003      //!< baud rate has unknown value
#define CMN_SC_UNKNOWN_PARITY                   0x02070004      //!< parity has unknown value
#define CMN_SC_UNKNOWN_STOP_BITS                0x02070005      //!< stop bits has unknown value
#define CMN_SC_UNKNOWN_DATA_LEN                 0x02070006      //!< data len has unknown value
#define CMN_SC_UNKNOWN_RS485_ENABLE             0x02070007      //!< rs485 enable has unknown value
#define CMN_SC_UART_INIT_ERROR                  0x02070008      //!< UART_init failed
#define CMN_SC_STOP_CMD                         0x02070009      //!< unknown oder stop cmd in script
#define CMN_SC_PREPARE_CMD_ERROR                0x0207000a      //!< error in SC_prepareNextCMD
#define CMN_SC_CONFIG_PORT_NOT_ALLOWED          0x0207000b      //!< cmd 'configPort' not allowed

// SSC
#define CMN_SSC_ERR_INTERN1                     0x02080000      //!< invalid SPI Port used

//+=============================================================================================
//|     Makros / macros
//+=============================================================================================

#ifdef __cplusplus
extern "C" { 
#endif 

//+=============================================================================================
//|     Globale Variablen / global variables
//+=============================================================================================

//+=============================================================================================
//|     Prototypen / prototypes
//+=============================================================================================

void platformError (INT32U i32uErrCode_p, TBOOL bFatal_p, INT8U i8uAnzPara_p, ...);
void platformErrorInit (BSP_TJumpBuf *ptExceptionPoint_p, void (*cbErrHandler_p)(INT32U i32uErrorCode_p, 
  TBOOL bFatal_p, INT8U i8uParaCnt_p, va_list argptr_p));

#ifdef  __cplusplus 
} 
#endif 


#endif // PLATFORMERROR_H_INC
