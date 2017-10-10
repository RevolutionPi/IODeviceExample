/*=============================================================================================
*
* PiSlaveApplication.h
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

#ifndef PISLAVEAPPLICATION_H_INC
#define PISLAVEAPPLICATION_H_INC

#include <stdarg.h>
#include <bsp/setjmp/BspSetJmp.h>

// some modules (mGate) have a red/green double led
// some modules (piDIO) have a red led which is combined with a green one. 
// This means only the red one can be turned on and off, the green one is on, if the red one is off
typedef enum                                
{                                           // mGate                piDIO
    PISLAVEAPPLICATION_STATE_INIT,          // red blink            red blink 1hz
    PISLAVEAPPLICATION_STATE_RUNNING,       // green solid          red off -> green solid
    PISLAVEAPPLICATION_STATE_TIMEOUT,       // red blink            red solid
    PISLAVEAPPLICATION_STATE_UNKNOWN_ERROR, // red solid            red solid
} PISLAVEAPPLICATION_STATE;

typedef
#include <COMP_packBegin.h>
struct 
{
    INT32U  i32uSerialnumber;
    INT16U  i16uModulType;           // see common_define.h
    INT16U  i16uHW_Revision;
    INT16U  i16uSW_Major;
    INT16U  i16uSW_Minor;
    INT32U  i32uSVN_Revision;
    INT16U  i16uInputLength;
    INT16U  i16uOutputLength;
}
#include <COMP_packEnd.h> 
PISLAVEAPPLICATION_ID;

void PiSlaveAppInit(BSP_TJumpBuf *ptExceptionPoint_p, void(*cbErrHandler_p)(INT32U i32uErrorCode_p,
    TBOOL bFatal_p, INT8U i8uParaCnt_p, va_list argptr_p));
INT16U PiSlaveAppGetModuleType(void);
void PiSlaveAppSetStateLed(PISLAVEAPPLICATION_STATE eState_p);
void PiSlaveAppRS485Terminate(TBOOL bTerminate_p);

void PiSlaveAppRun(void);

#ifdef PI_IO_PROTOCOL
void PiSlaveAppIOReq(SIOGeneric *pReq, SIOGeneric *pResp);
void PiSlaveAppHandleTimeout(void);
#endif
#ifdef __cplusplus
extern "C" {
#endif

#ifdef  __cplusplus 
} 
#endif 


#endif //PISLAVEAPPLICATION_H_INC
