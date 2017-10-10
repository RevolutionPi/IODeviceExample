/*=============================================================================================
*
* bspInit.h
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
#ifndef BSPINIT_H_INC
#define BSPINIT_H_INC

#include <stdarg.h>
#include <bsp/setjmp/BspSetJmp.h>

#define BSP_ALLOC_OWNER_CAN             1


#ifdef __cplusplus
extern "C" { 
#endif 

extern void bspInit (BSP_TJumpBuf *ptExceptionPoint_p, void (*cbErrHandler_p)(INT32U i32uErrorCode_p, 
  TBOOL bFatal_p, INT8U i8uParaCnt_p, va_list argptr_p));
extern void BSP_systemReset (void);   // prozessor reset

#ifdef  __cplusplus 
} 
#endif 


#endif //BSPINIT_H_INC
