/*=============================================================================================
*
*   FW_Desc.c
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
#include <project.h>

const INT32U __attribute__ ((section(".firmwareCrc"))) i32uFirmwareCrc_g = 0xCEA54A13;

extern void (* const __vector_table[])(void);
extern INT8U laStartFirmwareCrc_g;
extern INT8U laEndFirmwareCrc_g;
extern INT8U laStartFlash_g;
extern INT8U laEndFlash_g;

char *Description_g = "Compiled on 2017/10/05 16:16:37 with modified files";

const T_KUNBUS_FW_DESCR  __attribute__ ((section(".firmwareDescr"))) ctKunbusFirmwareDescription_g = 
{
    sizeof (T_KUNBUS_FW_DESCR),                 // Length of the structure
    0xffffffff,                                 // Serial number, be programmed to correct value later
    KUNBUS_FW_DESCR_TYP_PI_DIO_14,              // Kunbus internal type
    HW_REVISION,                                // Hardware revision
    {0xff, 0xff, 0xff, 0xff, 0xff, 0xff},       // MAC Addr programmed later
    (INT32U)0,                           // Entry of Firmwareupdate from application
    (INT32U)&laStartFlash_g,                    // Startaddress of application specific flash area
    (INT32U)&laEndFlash_g                       // Last address of application specific flash area
};	


const T_KUNBUS_APPL_DESCR  __attribute__ ((section(".applicationDescr"))) ctKunbusApplDescription_g = 
{
    sizeof (T_KUNBUS_APPL_DESCR),               //!< number of bytes in struct, used to determine which elements are present
    (INT32U)__vector_table,                     //!< address of vector table.
    (INT32U)&laStartFirmwareCrc_g,              //!< first address of application area for crc check
    (INT32U)&laEndFirmwareCrc_g,                //!< last address of application area for crc check
    (INT32U)&i32uFirmwareCrc_g,                 //!< address of CRC Checksum over application area
    {'K','B','_','S','W','_','R','E','V'},      ///< identification String 1 "KB_SW_REV"
    42,                                         ///< major revision number; valid numbers 0-50, other numbers reserved
    0,                                          ///< minor revision number; valid numbers 0-255, other numbers reserved
    12116                                       ///< SVN revision number $Rev: 788 $ (mainly for internal use); 
};
