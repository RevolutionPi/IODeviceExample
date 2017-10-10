/*=============================================================================================
*
* mGateDecode.c
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
#include <project.h>
#include <common_define.h>

#include <bsp/gpio/gpio.h>
#include <bsp/mGateDecode/mGateDecode.h>

#if !defined (MGATE_DECODE_MS1B)
    #error "Using mGateDecode without definition of MGATE_DECODE_MS1B"
#endif    

//*************************************************************************************************
//| Function: BSP_initMgateDecode
//|
//! \brief
//!
//! \detailed
//!
//!
//!
//! \ingroup
//-------------------------------------------------------------------------------------------------
void BSP_initMgateDecode (
    void)
    
{
    KB_GPIO_InitTypeDef tInit_l;

    tInit_l.GPIO_Pin = MGATE_DECODE_MS1B_PIN;
    tInit_l.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    tInit_l.GPIO_Speed = GPIO_Speed_50MHz;
    kbGPIO_InitCLK(MGATE_DECODE_MS1B_PORT, &tInit_l);
}    

//*************************************************************************************************
//| Function: BSP_getMgateDecode
//|
//! \brief
//!
//! \detailed
//!
//!
//!
//! \ingroup
//-------------------------------------------------------------------------------------------------
INT8U BSP_getMgateDecode (
    void)
    
{
    INT8U i8uRv_l;
    
    i8uRv_l = (MGATE_DECODE_MS1B_PORT->IDR & MGATE_DECODE_MS1B_PIN) ? MGATE_MODUL_LEFT : MGATE_MODUL_RIGHT;

    return (i8uRv_l);
}

//*************************************************************************************************
    
