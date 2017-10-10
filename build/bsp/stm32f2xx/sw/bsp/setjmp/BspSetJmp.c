/*=============================================================================================
*
* BspSetJump.c
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
#include <common_define.h>
#include <bsp/setjmp/BspSetJmp.h>


//*************************************************************************************************
//| Function: bspSetJmp
//|
//! brief.
//!
//! detailed.
//!
//!
//!
//! ingroup.
//-------------------------------------------------------------------------------------------------
INT32S bspSetJmp (
    BSP_TJumpBuf tJmpBuf_p)      //!< [out] Buffer for 12 values 0f 32 bit width to store the context
    
{
    asm(" mrs r2, msp");
    asm(" mrs r3, psp");
    asm(" stmia	r0!, {r2, r3, r4, r5, r6, r7, r8, r9, sl, fp, ip, lr}");
    
    return (0);
}
    
//*************************************************************************************************
//| Function: bspLongJmp
//|
//! brief.
//!
//! detailed.
//!
//!
//!
//! ingroup.
//-------------------------------------------------------------------------------------------------
void	bspLongJmp(
    BSP_TJumpBuf tJmpBuf_p,   //!< [in] Buffer for 12 values 0f 32 bit width to restore the context
    INT32S i32sValue_p)      //!< [in] Return Value for the setjmp function
    
{
    asm(" ldmia	r0!, {r2, r3, r4, r5, r6, r7, r8, r9, sl, fp, ip, lr}");
    asm(" msr msp, r2");
    asm(" msr psp, r3");
    
    asm(" mov r0, r1");
    
    asm(" bx r14");
}
    
//*************************************************************************************************
