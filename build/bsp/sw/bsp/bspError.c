/*=============================================================================================
*
* bspError.c
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
#include <stdarg.h>

#include <common_define.h>
#include <project.h>
#include <bsp/bspConfig.h>
#include <bsp/led/Led.h>

#include <bsp/bspError.h>
#include <bsp/setjmp/BspSetJmp.h>

static BSP_TJumpBuf *ptJumpBuf_s = (BSP_TJumpBuf *)0;
static void (*cbErrHandler_s)(INT32U i32uErrorCode_p, TBOOL bFatal_p, INT8U i8uParaCnt_p, va_list argptr_p);
//*************************************************************************************************
//| Function: bspError
//|
//! error reporting function
//!
//! internal error function. can be called from anywhere in the stack except from interrupt context.
//!
//!
//! ingroup.  ErrInt
//-------------------------------------------------------------------------------------------------
void bspError (
    INT32U i32uErrCode_p,       //!< [in] common error code, defined in \ref kbError.h
    TBOOL bFatal_p,             //!< [in] = bTRUE: fatal error, do not return to caller
                                //!       = bFALSE: no fatal error, continue program flow
    INT8U i8uParaCnt_p,         //!< [in] number of parameters in bytes
    ...)                        //!< [in] variable list of INT8U parameters

{
    va_list argptr_l;
    BSP_TJumpBuf *ptJumpBuf_l = NULL;

    if (cbErrHandler_s)
    {
        va_start (argptr_l, i8uParaCnt_p);
        cbErrHandler_s (i32uErrCode_p, bFatal_p, i8uParaCnt_p, argptr_l);
    }


    if (bFatal_p == bTRUE)
    {
#if defined (BSP_FATAL_ERROR_LED)
        LED_setLed(BSP_FATAL_ERROR_LED, LED_ST_RED_ON);
#endif

        if (ptJumpBuf_l == NULL)
        {
            ptJumpBuf_l = ptJumpBuf_s;
        }

        if (ptJumpBuf_l)
        {
            bspLongJmp (*ptJumpBuf_l, 1);
        }

        for (;;)
        {
        }
    }
}

//*************************************************************************************************
//| Function: bspSetExceptionPoint
//|
//! set a point where the stack can return in case of a fatal error
//!
//! simulates an exception in C++. Goes to the "catch" block
//!
//!
//!
//! ingroup. Intf
//-------------------------------------------------------------------------------------------------
void bspSetExceptionPoint (
    BSP_TJumpBuf *ptJumpBuf_p)      //!< [in] Pointer to a Jump Buf from setjmp ()

{
    ptJumpBuf_s = ptJumpBuf_p;
}

//*************************************************************************************************
//| Function: bspGetExceptionPoint
//|
//! get the point where the stack can return in case of a fatal error
//!
//!
//! ingroup. Intf
//-------------------------------------------------------------------------------------------------
BSP_TJumpBuf *bspGetExceptionPoint (void)
{
    return ptJumpBuf_s;
}

//*************************************************************************************************
//| Function: bspRegisterErrorHandler
//|
//! register an application specific error handler
//!
//! this application specific handler will be invoked with each occurence of a stack error
//!
//!
//!
//! ingroup. Intf
//-------------------------------------------------------------------------------------------------
void bspRegisterErrorHandler (
    void (*cbErrHandler_p)(INT32U i32uErrorCode_p, TBOOL bFatal_p, INT8U i8uParaCnt_p, va_list argptr_p))
       //!< [in] Pointer to application specific error handler
{

    cbErrHandler_s = cbErrHandler_p;
}

//*************************************************************************************************

