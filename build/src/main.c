/*=============================================================================================
*
*   main.c
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

#include <bsp\bspConfig.h>
#include <bsp\setjmp\BspSetJmp.h>
#include <bsp\bspInit.h>
#include "PiBridgeSlave.h"
#include <platformError.h>

//+=============================================================================================
//|		Globale Variablen / global variables
//+=============================================================================================

extern void applErrHandler (INT32U i32uErrorCode_p, TBOOL bFatal_p, INT8U i8uParaCnt_p, va_list argptr_p);
extern void emergencyOperation (void);

INT32U i32uLastError_g;
static BSP_TJumpBuf tJumpBuf_s;

//+=============================================================================================
//|		Function:	main
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
int main (void)
{
    INT32S i32sJmpRet_l;

    i32sJmpRet_l = bspSetJmp (tJumpBuf_s);
    if (i32sJmpRet_l == 0)
    {
        bspInit (&tJumpBuf_s, applErrHandler);

        platformErrorInit (&tJumpBuf_s, applErrHandler);

        PiBridgeSlaveInit(&tJumpBuf_s, applErrHandler);

        for (;;)
        {
            PiBridgeSlaveRun();
        }
    }
    else
    {   // Emergency Operation after a fatal error
        emergencyOperation ();
    }

    return (0);
}

//*************************************************************************************************
void emergencyOperation (
    void)
    
{
    while(1)
    {
    }
}    

//*************************************************************************************************
void applErrHandler (
    INT32U i32uErrorCode_p, 
    TBOOL bFatal_p, 
    INT8U i8uParaCnt_p, 
    va_list argptr_p)
{
    static TBOOL bAppErrRecursive_l = bFALSE;

    if (bAppErrRecursive_l == bFALSE)
    {
        bAppErrRecursive_l = bTRUE;
        if(bTRUE == bFatal_p)
        {
            if (i32uErrorCode_p == 0x27000014)
            {
                BSP_systemReset();
            }
            emergencyOperation();
        }
        bAppErrRecursive_l = bFALSE;
    }
}


//*************************************************************************************************
