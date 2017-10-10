/*=============================================================================================
*
*   eepromLayout.h
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
#ifndef EEPROMLAYOUT_C_INC
#define EEPROMLAYOUT_C_INC

#include <project.h>
#include <common_define.h>


typedef 
#include <COMP_packBegin.h>     
struct BSP_EEPROM_StrLayout
{
    INT8U i8uInitMarker;            // System variable, used for eeprom initialization
    INT8U i8uRTUNodeAddr;

    INT8U  EE_i8uErrorStackIndex;        // Error Stack Index 
    INT32U EE_i32uErrorStackEntry0;      // Error Stack Entry 0 
    INT32U EE_i32uErrorStackEntry1;      // Error Stack Entry 1 
    INT32U EE_i32uErrorStackEntry2;      // Error Stack Entry 2 
    INT32U EE_i32uErrorStackEntry3;      // Error Stack Entry 3 
    INT32U EE_i32uErrorStackEntry4;      // Error Stack Entry 4 
    INT32U EE_i32uErrorStackEntry5;      // Error Stack Entry 5 
    INT32U EE_i32uErrorStackEntry6;      // Error Stack Entry 6 
    INT32U EE_i32uErrorStackEntry7;      // Error Stack Entry 7 
} 
#include <COMP_packEnd.h>     
BSP_EEPROM_TLayout;

#endif // EEPROMLAYOUT_C_INC
//*************************************************************************************************
