/*=============================================================================================
*
* BspSetJump.h
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
#ifndef BSPSETJMP_H_INC
#define BSPSETJMP_H_INC

#if defined (_MSC_VER) || defined(__NIOS_GENERIC__) || defined(__SF2_GENERIC__) || defined (__KUNBUSPI__)
  #include <setjmp.h>
  
  typedef jmp_buf BSP_TJumpBuf;

  #define bspSetJmp(x)  setjmp (x)
  #define bspLongJmp(x,y)  longjmp ((x), (y))
  
#else
  typedef INT32U BSP_TJumpBuf[12];

  #ifdef __cplusplus
  extern "C" { 
  #endif 
  
  extern INT32S bspSetJmp (BSP_TJumpBuf tJmpBuf_p);
  extern void bspLongJmp(BSP_TJumpBuf tJmpBuf_p, INT32S i32sValue_p);

  #ifdef __cplusplus
  }    
  #endif 

#endif




#ifdef __cplusplus
extern "C" { 
#endif 

#ifdef __cplusplus
}    
#endif 


#endif  // BSPSETJMP_H_INC

