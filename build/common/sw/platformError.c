/*=============================================================================================
*
* platformError.c
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
#include <platformError.h>
#include <bsp/setjmp/BspSetJmp.h>

INT32U i32uFatalError_g;
static TBOOL bPlatformErrRecursive_s = bFALSE;

static void (*cbPlatformErrHandler_s)(INT32U i32uErrorCode_p, TBOOL bFatal_p, INT8U i8uParaCnt_p, va_list argptr_p);
static BSP_TJumpBuf *ptPlatformJumpBuf_s;

//*************************************************************************************************
//| Function: platformError
//|
//! error reporting function
//!
//! internal error function. can be called from anywhere in the stack except from interrupt context.
//!
//!
//! \ingroup  cErrInt
//-------------------------------------------------------------------------------------------------
void platformError (
    INT32U i32uErrCode_p,       //!< [in] common error code, defined in \ref kbDN_Error.h
    TBOOL bFatal_p,             //!< [in] = bTRUE: fatal error, do not return to caller
                                //!       = bFALSE: no fatal error, continue program flow
    INT8U i8uParaCnt_p,         //!< [in] number of parameters in bytes
    ...)                        //!< [in] variable list of INT8U parameters
    
{
    va_list argptr_l;
  
    i32uFatalError_g = i32uErrCode_p;
    
    if (bPlatformErrRecursive_s == bFALSE)
    {
        bPlatformErrRecursive_s = bTRUE;
        if (cbPlatformErrHandler_s)
        {
            va_start (argptr_l, i8uParaCnt_p);
            cbPlatformErrHandler_s (i32uErrCode_p, bFatal_p, i8uParaCnt_p, argptr_l);     
        }
        bPlatformErrRecursive_s = bFALSE;
    }    

    if (bFatal_p == bTRUE)
    {
        if (ptPlatformJumpBuf_s)
        {
            bspLongJmp (*ptPlatformJumpBuf_s, 1);
        }    

        for (;;)
        {
        }
    }
}

//*************************************************************************************************
//| Function: platformErrorInit
//|
//! \brief
//!
//! 
//!
//!
//! \ingroup  
//-------------------------------------------------------------------------------------------------
void platformErrorInit (
    BSP_TJumpBuf *ptExceptionPoint_p, 
    void (*cbErrHandler_p)(INT32U i32uErrorCode_p, TBOOL bFatal_p, INT8U i8uParaCnt_p, va_list argptr_p))

{

    ptPlatformJumpBuf_s = ptExceptionPoint_p;
    cbPlatformErrHandler_s = cbErrHandler_p;
}


//*************************************************************************************************


