/*=============================================================================================
*
*   eepromDefaultValue.c
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

#include <bsp\eeprom\eeprom.h>

const BSP_EEPROM_TLayout BSP_EEPROM_ctDefaultValue = 
{
    .i8uRTUNodeAddr = 1,            // default Node Adresse
    .EE_i8uErrorStackIndex = 0,     // EE_i8uErrorStackIndex 
    .EE_i32uErrorStackEntry0 = 0,   // EE_i32uErrorStackEntry0 
    .EE_i32uErrorStackEntry1 = 0,   // EE_i32uErrorStackEntry1 
    .EE_i32uErrorStackEntry2 = 0,   // EE_i32uErrorStackEntry2 
    .EE_i32uErrorStackEntry3 = 0,   // EE_i32uErrorStackEntry3 
    .EE_i32uErrorStackEntry4 = 0,   // EE_i32uErrorStackEntry4 
    .EE_i32uErrorStackEntry5 = 0,   // EE_i32uErrorStackEntry5 
    .EE_i32uErrorStackEntry6 = 0,   // EE_i32uErrorStackEntry6 
    .EE_i32uErrorStackEntry7 = 0    // EE_i32uErrorStackEntry7 
};

                    
const BSP_EEPROM_TDefaultRef BSP_EEPROM_atDefaultRef[] = 
{
    {
        0,
        (void *)0,
        0
    }
};

//*************************************************************************************************
