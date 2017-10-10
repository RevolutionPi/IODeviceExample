/*=============================================================================================
*
* FlashDriver.c
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
#include <bsp/bspConfig.h>
#include <SysLib/inc/stm32f2xx.h>
#include <SysLib/inc/stm32f2xx_flash.h>
#include <bsp/flash/stm32f2xx_flash_la.h>

#include <bsp/flash/FlashDriver.h>
#include <bsp/bspError.h>

//*************************************************************************************************
//| Function: BSP_FLASH_eraseBlock
//|
//! brief.
//!
//! \details 
//!
//!
//! ingroup.
//-------------------------------------------------------------------------------------------------
void BSP_FLASH_eraseBlock (
                        const void *vpAddr_p
                        )

{
    FLASH_Status FLASHStatus = FLASH_COMPLETE;
    INT32U i32uLocked_l;
    INT32U *pI32UStart, *pI32UEnd, *pI32UPtr;
    TBOOL bErased;
    INT32U sector;

    if ((INT32U)vpAddr_p <= 0x00003fff)
    {
        sector = FLASH_Sector_0;
        pI32UStart = (INT32U *)0x00000000;
        pI32UEnd = (INT32U *)0x00003fff;
    }
    else if ((INT32U)vpAddr_p <= 0x00007fff)
    {
        sector = FLASH_Sector_1;
        pI32UStart = (INT32U *)0x00004000;
        pI32UEnd = (INT32U *)0x00007fff;
    }
    else if ((INT32U)vpAddr_p <= 0x0000bfff)
    {
        sector = FLASH_Sector_2;
        pI32UStart = (INT32U *)0x00008000;
        pI32UEnd = (INT32U *)0x0000bfff;
    }
    else if ((INT32U)vpAddr_p <= 0x0000ffff)
    {
        sector = FLASH_Sector_3;
        pI32UStart = (INT32U *)0x0000b000;
        pI32UEnd = (INT32U *)0x0000ffff;
    }
    else if ((INT32U)vpAddr_p <= 0x0001ffff)
    {
        sector = FLASH_Sector_4;
        pI32UStart = (INT32U *)0x00010000;
        pI32UEnd = (INT32U *)0x0001ffff;
    }
    else if ((INT32U)vpAddr_p <= 0x0003ffff)
    {
        sector = FLASH_Sector_5;
        pI32UStart = (INT32U *)0x00020000;
        pI32UEnd = (INT32U *)0x0003ffff;
    }
    else if ((INT32U)vpAddr_p <= 0x0005ffff)
    {
        sector = FLASH_Sector_6;
        pI32UStart = (INT32U *)0x00040000;
        pI32UEnd = (INT32U *)0x0005ffff;
    }
    else if ((INT32U)vpAddr_p <= 0x0007ffff)
    {
        sector = FLASH_Sector_7;
        pI32UStart = (INT32U *)0x00060000;
        pI32UEnd = (INT32U *)0x0007ffff;
    }
    else if ((INT32U)vpAddr_p <= 0x0009ffff)
    {
        sector = FLASH_Sector_8;
        pI32UStart = (INT32U *)0x00080000;
        pI32UEnd = (INT32U *)0x0009ffff;
    }
    else if ((INT32U)vpAddr_p <= 0x000bffff)
    {
        sector = FLASH_Sector_9;
        pI32UStart = (INT32U *)0x000a0000;
        pI32UEnd = (INT32U *)0x000bffff;
    }
    else if ((INT32U)vpAddr_p <= 0x000dffff)
    {
        sector = FLASH_Sector_10;
        pI32UStart = (INT32U *)0x000c0000;
        pI32UEnd = (INT32U *)0x000dffff;
    }
    else if ((INT32U)vpAddr_p <= 0x000fffff)
    {
        sector = FLASH_Sector_11;
        pI32UStart = (INT32U *)0x000e0000;
        pI32UEnd = (INT32U *)0x000fffff;
    }
    else
    {
        bspError(BSPE_FLASH_ERASE_ADDR_ERR, bTRUE, 0);
        return;
    }

    bErased = bTRUE;
    for (pI32UPtr = pI32UStart; bErased == bTRUE && pI32UPtr <= pI32UEnd; pI32UPtr++)
    {
        if (*pI32UPtr != 0xffffffff)
            bErased = bFALSE;
    }

    if (bErased)
    {
        // the flash page is already erased -> nothing to do
        return;
    }

    /* Get current Lock Bit value */
    i32uLocked_l = FLASH->CR & CR_LOCK_Set;

    /* Unlock the Flash Program Erase controller */
    FLASH_Unlock();

    /* Clear All pending flags */
    FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);	

    /* Erase the FLASH sector */
    FLASHStatus = FLASH_EraseSector(sector, STM_VOLTAGE_RANGE);

    /* restore flash lock state */
    if (i32uLocked_l)
    {
        FLASH_Lock();
    }

    if (FLASHStatus != FLASH_COMPLETE)
    {
        if (FLASHStatus == FLASH_ERROR_WRP)
        {
            bspError (BSPE_FLASH_LOCK_ERR, bTRUE, 0);
        }
        else
        {
            bspError (BSPE_FLASH_ERASE_ERR, bTRUE, 0);
        }
    }
}

//*************************************************************************************************
//| Function: BSP_FLASH_eraseRange
//|
//! brief.
//!
//! \details 
//!
//!
//! ingroup.
//-------------------------------------------------------------------------------------------------
void BSP_FLASH_eraseRange (
                        const void *pvStartAddr_p,
                        const void *pvEndAddr_p
                        )

{
    const INT8U* pi8uStart_l;
    const INT8U* pi8uEnd_l;

    pi8uStart_l = pvStartAddr_p;
    pi8uEnd_l = pvEndAddr_p;

    for(;pi8uStart_l <= pi8uEnd_l;)
    {
        BSP_FLASH_eraseBlock (pi8uStart_l);

        if ((INT32U)pi8uStart_l <= 0x0000ffff)
            pi8uStart_l += 0x00004000;	// 16 KByte 
        else if ((INT32U)pi8uStart_l < 0x0001ffff)
            pi8uStart_l += 0x00010000;	// 64 KByte
        else
            pi8uStart_l += 0x00020000;	// 128 KByte
    }

}

//*************************************************************************************************
//| Function: BSP_FLASH_write
//|
//! brief.
//!
//! \details 
//!
//!
//! ingroup.
//-------------------------------------------------------------------------------------------------
void BSP_FLASH_write (
                   void *pvDest_p,          //!< [in] Destination address, must point to flash area and 
                                            //!       have an alignment of 2, space to write must be 
                                            //!       big enough for word writes. (Done through proper alignment) 
                   const void *pvSrc_p,     //!< [in] Source address
                   INT16U i16uLen_p)        //!< [in] Length of data in bytes

{

    INT32U i32uLocked_l;
    FLASH_Status FLASHStatus;
    FLASHStatus = FLASH_COMPLETE;
    INT8U *pi8uSrc_l;
    INT16U *pi16uDest_l;
    INT16U i16uData_l;
    INT16U i16uWordCnt_l;
    INT16U i16uInd_l;


    if (((INT32U)pvDest_p) & 0x00000001)
    {  // alignment error for destination
        bspError (BSPE_FLASH_DEST_ALIGN, bTRUE, 0);
    }

    /* Get current Lock Bit value */
    i32uLocked_l = FLASH->CR & CR_LOCK_Set;

    /* Unlock the Flash Program Erase controller */
    FLASH_Unlock();

    /* Clear All pending flags */
    FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);	

    /* Start conditions */
    pi8uSrc_l = (INT8U *)pvSrc_p;
    pi16uDest_l = (INT16U *)pvDest_p;
    i16uWordCnt_l = i16uLen_p >> 1;  

    for (i16uInd_l = 0; i16uInd_l < i16uWordCnt_l; i16uInd_l++)
    {
        i16uData_l = ((INT16U)pi8uSrc_l[0]) | (((INT16U)pi8uSrc_l[1]) << 8);   // avoid problems with odd alignment of src
        FLASHStatus = FLASH_ProgramHalfWord ((INT32U)pi16uDest_l, i16uData_l);

        /* Check the correctness of written data */
        if (   (FLASHStatus != FLASH_COMPLETE)
            || (*pi16uDest_l != i16uData_l)
            ) 
        {
            if (FLASHStatus == FLASH_ERROR_WRP)
            {
                bspError (BSPE_FLASH_LOCK_ERR, bTRUE, 0);
            }
            else
            {
                bspError (BSPE_FLASH_PROG_ERR, bTRUE, 0);
            }    
            break;
        }
        pi8uSrc_l += 2;
        pi16uDest_l++;
    }

    if (i16uLen_p & 0x0001)
    {   // Program last byte if it is a odd number of bytes to program
        i16uData_l = 0xff00 | *pi8uSrc_l;
        FLASHStatus = FLASH_ProgramHalfWord ((INT32U)pi16uDest_l, i16uData_l);

        /* Check the correctness of written data */
        if (   (FLASHStatus != FLASH_COMPLETE)
            || (*pi16uDest_l != i16uData_l)
            ) 
        {
            if (FLASHStatus == FLASH_ERROR_WRP)
            {
                bspError (BSPE_FLASH_LOCK_ERR, bTRUE, 0);
            }
            else
            {
                bspError (BSPE_FLASH_PROG_ERR, bTRUE, 0);
            }    
        }
    }


    /* restore flash lock state */
    if (i32uLocked_l)
    {
        FLASH_Lock();
    }
}    

//*************************************************************************************************

