/*=============================================================================================
*
* kbUtilities.c
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

#ifdef __KUNBUSPI_KERNEL__
#include <linux/string.h>
#else
#include <string.h>
#endif

#include "common_define.h"
#include "project.h"

#include <bsp/systick/systick.h>

#include "kbUtilities.h"


#define CRC32_POLY  0xEDB88320

#ifdef KBUT_CRC32_TABLE
static const INT32U ai32uCrc32TableSingle_s[256] = { 0x00000000L, 0x77073096L, 0xee0e612cL, 0x990951baL, 0x076dc419L, 0x706af48fL,
0xe963a535L, 0x9e6495a3L, 0x0edb8832L, 0x79dcb8a4L, 0xe0d5e91eL, 0x97d2d988L, 0x09b64c2bL, 0x7eb17cbdL,
0xe7b82d07L, 0x90bf1d91L, 0x1db71064L, 0x6ab020f2L, 0xf3b97148L, 0x84be41deL, 0x1adad47dL, 0x6ddde4ebL,
0xf4d4b551L, 0x83d385c7L, 0x136c9856L, 0x646ba8c0L, 0xfd62f97aL, 0x8a65c9ecL, 0x14015c4fL, 0x63066cd9L,
0xfa0f3d63L, 0x8d080df5L, 0x3b6e20c8L, 0x4c69105eL, 0xd56041e4L, 0xa2677172L, 0x3c03e4d1L, 0x4b04d447L,
0xd20d85fdL, 0xa50ab56bL, 0x35b5a8faL, 0x42b2986cL, 0xdbbbc9d6L, 0xacbcf940L, 0x32d86ce3L, 0x45df5c75L,
0xdcd60dcfL, 0xabd13d59L, 0x26d930acL, 0x51de003aL, 0xc8d75180L, 0xbfd06116L, 0x21b4f4b5L, 0x56b3c423L,
0xcfba9599L, 0xb8bda50fL, 0x2802b89eL, 0x5f058808L, 0xc60cd9b2L, 0xb10be924L, 0x2f6f7c87L, 0x58684c11L,
0xc1611dabL, 0xb6662d3dL, 0x76dc4190L, 0x01db7106L, 0x98d220bcL, 0xefd5102aL, 0x71b18589L, 0x06b6b51fL,
0x9fbfe4a5L, 0xe8b8d433L, 0x7807c9a2L, 0x0f00f934L, 0x9609a88eL, 0xe10e9818L, 0x7f6a0dbbL, 0x086d3d2dL,
0x91646c97L, 0xe6635c01L, 0x6b6b51f4L, 0x1c6c6162L, 0x856530d8L, 0xf262004eL, 0x6c0695edL, 0x1b01a57bL,
0x8208f4c1L, 0xf50fc457L, 0x65b0d9c6L, 0x12b7e950L, 0x8bbeb8eaL, 0xfcb9887cL, 0x62dd1ddfL, 0x15da2d49L,
0x8cd37cf3L, 0xfbd44c65L, 0x4db26158L, 0x3ab551ceL, 0xa3bc0074L, 0xd4bb30e2L, 0x4adfa541L, 0x3dd895d7L,
0xa4d1c46dL, 0xd3d6f4fbL, 0x4369e96aL, 0x346ed9fcL, 0xad678846L, 0xda60b8d0L, 0x44042d73L, 0x33031de5L,
0xaa0a4c5fL, 0xdd0d7cc9L, 0x5005713cL, 0x270241aaL, 0xbe0b1010L, 0xc90c2086L, 0x5768b525L, 0x206f85b3L,
0xb966d409L, 0xce61e49fL, 0x5edef90eL, 0x29d9c998L, 0xb0d09822L, 0xc7d7a8b4L, 0x59b33d17L, 0x2eb40d81L,
0xb7bd5c3bL, 0xc0ba6cadL, 0xedb88320L, 0x9abfb3b6L, 0x03b6e20cL, 0x74b1d29aL, 0xead54739L, 0x9dd277afL,
0x04db2615L, 0x73dc1683L, 0xe3630b12L, 0x94643b84L, 0x0d6d6a3eL, 0x7a6a5aa8L, 0xe40ecf0bL, 0x9309ff9dL,
0x0a00ae27L, 0x7d079eb1L, 0xf00f9344L, 0x8708a3d2L, 0x1e01f268L, 0x6906c2feL, 0xf762575dL, 0x806567cbL,
0x196c3671L, 0x6e6b06e7L, 0xfed41b76L, 0x89d32be0L, 0x10da7a5aL, 0x67dd4accL, 0xf9b9df6fL, 0x8ebeeff9L,
0x17b7be43L, 0x60b08ed5L, 0xd6d6a3e8L, 0xa1d1937eL, 0x38d8c2c4L, 0x4fdff252L, 0xd1bb67f1L, 0xa6bc5767L,
0x3fb506ddL, 0x48b2364bL, 0xd80d2bdaL, 0xaf0a1b4cL, 0x36034af6L, 0x41047a60L, 0xdf60efc3L, 0xa867df55L,
0x316e8eefL, 0x4669be79L, 0xcb61b38cL, 0xbc66831aL, 0x256fd2a0L, 0x5268e236L, 0xcc0c7795L, 0xbb0b4703L,
0x220216b9L, 0x5505262fL, 0xc5ba3bbeL, 0xb2bd0b28L, 0x2bb45a92L, 0x5cb36a04L, 0xc2d7ffa7L, 0xb5d0cf31L,
0x2cd99e8bL, 0x5bdeae1dL, 0x9b64c2b0L, 0xec63f226L, 0x756aa39cL, 0x026d930aL, 0x9c0906a9L, 0xeb0e363fL,
0x72076785L, 0x05005713L, 0x95bf4a82L, 0xe2b87a14L, 0x7bb12baeL, 0x0cb61b38L, 0x92d28e9bL, 0xe5d5be0dL,
0x7cdcefb7L, 0x0bdbdf21L, 0x86d3d2d4L, 0xf1d4e242L, 0x68ddb3f8L, 0x1fda836eL, 0x81be16cdL, 0xf6b9265bL,
0x6fb077e1L, 0x18b74777L, 0x88085ae6L, 0xff0f6a70L, 0x66063bcaL, 0x11010b5cL, 0x8f659effL, 0xf862ae69L,
0x616bffd3L, 0x166ccf45L, 0xa00ae278L, 0xd70dd2eeL, 0x4e048354L, 0x3903b3c2L, 0xa7672661L, 0xd06016f7L,
0x4969474dL, 0x3e6e77dbL, 0xaed16a4aL, 0xd9d65adcL, 0x40df0b66L, 0x37d83bf0L, 0xa9bcae53L, 0xdebb9ec5L,
0x47b2cf7fL, 0x30b5ffe9L, 0xbdbdf21cL, 0xcabac28aL, 0x53b39330L, 0x24b4a3a6L, 0xbad03605L, 0xcdd70693L,
0x54de5729L, 0x23d967bfL, 0xb3667a2eL, 0xc4614ab8L, 0x5d681b02L, 0x2a6f2b94L, 0xb40bbe37L, 0xc30c8ea1L,
0x5a05df1bL, 0x2d02ef8dL };

#endif

#ifdef KBUT_CRC32_SMALLTABLE
static INT32U ai32uCrc32TableSmall_s[16] =
{ 
    0x00000000,
    0x1db71064,
    0x3b6e20c8,
    0x26d930ac,
    0x76dc4190,
    0x6b6b51f4,
    0x4db26158,
    0x5005713c,
    0xedb88320,
    0xf00f9344,
    0xd6d6a3e8,
    0xcb61b38c,
    0x9b64c2b0,
    0x86d3d2d4,
    0xa00ae278,
    0xbdbdf21c,
};
#endif

#ifdef KBUT_CRC32_MULTITABLE
static TBOOL boCrc32MultiInit_s = bFALSE;
static INT32U ai32uCrc32TableMulti_s[16][256]; /*< crc table */
#endif

//*************************************************************************************************
//| Function: kbUT_getCurrentMs
//|
//! reads out the current value of the tick counter (1ms)
//!
//! detailed
//!
//!
//! ingroup. Util
//-------------------------------------------------------------------------------------------------
INT32U kbUT_getCurrentMs (
    void)           //! \return tick count

{
    INT32U i32uRv_l;

    i32uRv_l = kbGetTickCount ();

    return (i32uRv_l);
}

//*************************************************************************************************
//| Function: kbUT_TimerInit
//|
//! initializes a timer variable
//!
//!
//! ingroup. Util
//-------------------------------------------------------------------------------------------------
void kbUT_TimerInit (
    kbUT_Timer *ptTimer_p)      //!< [inout] pointer to timer struct

{

    memset (ptTimer_p, 0, sizeof (kbUT_Timer));
}

//*************************************************************************************************
//| Function: kbUT_TimerStart
//|
//! starts a timer.
//!
//!
//! ingroup. Util
//-------------------------------------------------------------------------------------------------
void kbUT_TimerStart (
    kbUT_Timer *ptTimer_p,          //!< [inout] pointer to timer struct
    INT32U i32uDuration_p)          //!< [in] duration of timer to run (ms)

{

    ptTimer_p->i32uStartTime = kbUT_getCurrentMs ();
    ptTimer_p->i32uDuration = i32uDuration_p;
    ptTimer_p->bExpired = bFALSE;
    ptTimer_p->bRun = bTRUE;
}

//*************************************************************************************************
//| Function: kbUT_TimerRunning
//|
//! checks if a timer is actually running
//!
//!
//! ingroup. Util
//-------------------------------------------------------------------------------------------------
TBOOL kbUT_TimerRunning (
    kbUT_Timer *ptTimer_p)      //!< [in] pointer to timer struct
                                //! \return = bTRUE: timer is actual running, = bFALSE: timer is
                                //! either not started or expired.

{
    INT32U i32uTimeDiff_l;

    if (ptTimer_p->bRun == bTRUE)
    {
        i32uTimeDiff_l = (kbUT_getCurrentMs () - ptTimer_p->i32uStartTime);
        if (i32uTimeDiff_l >= ptTimer_p->i32uDuration)
        {  // Timer expired
            ptTimer_p->bExpired = bTRUE;
            ptTimer_p->bRun = bFALSE;
        }
    }

    return (ptTimer_p->bRun);
}

//*************************************************************************************************
//| Function: kbUT_TimerExpired
//|
//! checks if a timer is expired.
//!
//! Tests if the Timer just expires. Return value is true only for one call. Can be used to
//! check the timer cyclically, but do an action only once.
//!
//! ingroup. Util
//-------------------------------------------------------------------------------------------------
TBOOL kbUT_TimerExpired (
    kbUT_Timer *ptTimer_p)

{
    INT32U i32uTimeDiff_l;
    TBOOL bRv_l = bFALSE;

    if (ptTimer_p->bRun == bTRUE)
    {
        i32uTimeDiff_l = (kbUT_getCurrentMs () - ptTimer_p->i32uStartTime);
        if (i32uTimeDiff_l >= ptTimer_p->i32uDuration)
        {  // Timer expired
            ptTimer_p->bExpired = bTRUE;
            ptTimer_p->bRun = bFALSE;
            bRv_l = bTRUE;
        }
    }

    bRv_l = ptTimer_p->bExpired;

    ptTimer_p->bExpired = bFALSE; // Reset Flag, so that it is only once TRUE

    return (bRv_l);

}

//*************************************************************************************************
//| Function: kbUT_TimeElapsed
//|
//! calculates the time since the timer was started
//!
//! if a timer is not started or it is expired, zero is given back
//!
//!
//! ingroup. Util
//-------------------------------------------------------------------------------------------------
INT32U kbUT_TimeElapsed (
    kbUT_Timer *ptTimer_p)      //!< [in] pointer to timer
                                //! \return time elapsed since timer started in ms

{

    INT32U i32uTimeDiff_l = 0;

    if (ptTimer_p->bRun == bTRUE)
    {
        i32uTimeDiff_l = (kbUT_getCurrentMs () - ptTimer_p->i32uStartTime);
    }

    return (i32uTimeDiff_l);
}

//*************************************************************************************************
//| Function: kbUT_TimerInUse
//|
//! checks if a timer is actually running and the expired flag has not been reset.
//! if false is returned, the timer can be started again.
//!
//! ingroup. Util
//-------------------------------------------------------------------------------------------------
TBOOL kbUT_TimerInUse (
    kbUT_Timer *ptTimer_p)      //!< [in] pointer to timer struct
                                //! \return = bTRUE: timer is actual running, = bFALSE: timer is
                                //! either not started or expired and can be restarted now

{
  return (ptTimer_p->bRun || ptTimer_p->bExpired);
}


#ifdef KBUT_CRC32_MULTITABLE
/***********************************************************************************/
/*!
* @brief Generate CRC32 Tables
*
* This function is used to generate the crc tables. There are multiple tables
* generated, which is needed by the used algorithm.
*
************************************************************************************/
void KbUT_Crc32MakeTable()
{
    INT32U i, j;

    for (i = 0; i <= 0xFF; i++)
    {
        INT32U crc = i;
        for (j = 0; j < 8; j++)
            crc = (crc >> 1) ^ ((crc & 1) * CRC32_POLY);
        ai32uCrc32TableMulti_s[0][i] = crc;
    }

    for (i = 0; i <= 0xFF; i++)
    {   // for Slicing-by-4 and Slicing-by-8
        ai32uCrc32TableMulti_s[1][i] = (ai32uCrc32TableMulti_s[0][i] >> 8) ^ ai32uCrc32TableMulti_s[0][ai32uCrc32TableMulti_s[0][i] & 0xFF];
        ai32uCrc32TableMulti_s[2][i] = (ai32uCrc32TableMulti_s[1][i] >> 8) ^ ai32uCrc32TableMulti_s[0][ai32uCrc32TableMulti_s[1][i] & 0xFF];
        ai32uCrc32TableMulti_s[3][i] = (ai32uCrc32TableMulti_s[2][i] >> 8) ^ ai32uCrc32TableMulti_s[0][ai32uCrc32TableMulti_s[2][i] & 0xFF];

        // only Slicing-by-8
        ai32uCrc32TableMulti_s[4][i] = (ai32uCrc32TableMulti_s[3][i] >> 8) ^ ai32uCrc32TableMulti_s[0][ai32uCrc32TableMulti_s[3][i] & 0xFF];
        ai32uCrc32TableMulti_s[5][i] = (ai32uCrc32TableMulti_s[4][i] >> 8) ^ ai32uCrc32TableMulti_s[0][ai32uCrc32TableMulti_s[4][i] & 0xFF];
        ai32uCrc32TableMulti_s[6][i] = (ai32uCrc32TableMulti_s[5][i] >> 8) ^ ai32uCrc32TableMulti_s[0][ai32uCrc32TableMulti_s[5][i] & 0xFF];
        ai32uCrc32TableMulti_s[7][i] = (ai32uCrc32TableMulti_s[6][i] >> 8) ^ ai32uCrc32TableMulti_s[0][ai32uCrc32TableMulti_s[6][i] & 0xFF];

        // only Slicing-by-16
        ai32uCrc32TableMulti_s[8][i] = (ai32uCrc32TableMulti_s[7][i] >> 8) ^ ai32uCrc32TableMulti_s[0][ai32uCrc32TableMulti_s[7][i] & 0xFF];
        ai32uCrc32TableMulti_s[9][i] = (ai32uCrc32TableMulti_s[8][i] >> 8) ^ ai32uCrc32TableMulti_s[0][ai32uCrc32TableMulti_s[8][i] & 0xFF];
        ai32uCrc32TableMulti_s[10][i] = (ai32uCrc32TableMulti_s[9][i] >> 8) ^ ai32uCrc32TableMulti_s[0][ai32uCrc32TableMulti_s[9][i] & 0xFF];
        ai32uCrc32TableMulti_s[11][i] = (ai32uCrc32TableMulti_s[10][i] >> 8) ^ ai32uCrc32TableMulti_s[0][ai32uCrc32TableMulti_s[10][i] & 0xFF];
        ai32uCrc32TableMulti_s[12][i] = (ai32uCrc32TableMulti_s[11][i] >> 8) ^ ai32uCrc32TableMulti_s[0][ai32uCrc32TableMulti_s[11][i] & 0xFF];
        ai32uCrc32TableMulti_s[13][i] = (ai32uCrc32TableMulti_s[12][i] >> 8) ^ ai32uCrc32TableMulti_s[0][ai32uCrc32TableMulti_s[12][i] & 0xFF];
        ai32uCrc32TableMulti_s[14][i] = (ai32uCrc32TableMulti_s[13][i] >> 8) ^ ai32uCrc32TableMulti_s[0][ai32uCrc32TableMulti_s[13][i] & 0xFF];
        ai32uCrc32TableMulti_s[15][i] = (ai32uCrc32TableMulti_s[14][i] >> 8) ^ ai32uCrc32TableMulti_s[0][ai32uCrc32TableMulti_s[14][i] & 0xFF];
    }
    boCrc32MultiInit_s = bTRUE;
}
#endif


//*************************************************************************************************
//| Function: kbUT_crc32
//|
//! calculates a 32Bit CRC over a data block
//!
//! The Polynom is the Ethernet Polynom  0xEDB88320
//!
//!
//! ingroup. Util
//-------------------------------------------------------------------------------------------------
void kbUT_crc32 (
    INT8U *pi8uData_p,        //!< [in] pointer to data
    INT32U i32uCnt_p,         //!< [in] number of bytes
    INT32U *pi32uCrc_p)       //!< [inout] CRC sum and inital value

{

#if defined(KBUT_CRC32_TABLE)
    /* algorithm with single lookup table for faster computation */
    INT32U i32uCrc_l = *pi32uCrc_p;
    INT8U *pi8uData_l = pi8uData_p;
    INT32U i32uLength_l = i32uCnt_p;

    while (i32uLength_l-- != 0)
    {
        INT8U i8uPos;

        i8uPos = i32uCrc_l ^ *pi8uData_l++;
        i32uCrc_l = (i32uCrc_l >> 8) ^ ai32uCrc32TableSingle_s[i8uPos];
    }

    *pi32uCrc_p = i32uCrc_l;
#elif defined(KBUT_CRC32_SMALLTABLE)
    /* algorithem with 4 bit lookukp table */
    INT32U i32uCrc_l = *pi32uCrc_p;
    INT32U i32uPos;

    for (i32uPos = 0; i32uPos < i32uCnt_p; i32uPos++)
    {
        INT8U i8uCurByte = pi8uData_p[i32uPos];
        INT8U i8uCurNib;
        INT8U i8uPos;

        i8uCurNib = i8uCurByte;
        i8uPos = (i32uCrc_l ^ i8uCurNib) & 0x0F;
        i32uCrc_l = (i32uCrc_l >> 4) ^ ai32uCrc32TableSmall_s[i8uPos];

        i8uCurNib = (i8uCurByte >> 4);
        i8uPos = (i32uCrc_l ^ i8uCurNib) & 0x0F;
        i32uCrc_l = (i32uCrc_l >> 4) ^ ai32uCrc32TableSmall_s[i8uPos];
    }

    *pi32uCrc_p = i32uCrc_l;

#elif defined(KBUT_CRC32_MULTITABLE)
    /* algorithm with multiple tables for fast computation on big blocks */
    INT32U i32uCrc_l = *pi32uCrc_p;
    INT32U i32uLength_l = i32uCnt_p;
    const INT32U *pi32uBuf = (const INT32U *)pi8uData_p;
    const INT8U *pi8uBuf;

    if (boCrc32MultiInit_s == bFALSE)
    {
        KbUT_Crc32MakeTable();
    }

    const INT32S BytesAtOnce = 16;
    while (i32uLength_l >= BytesAtOnce)
    {
#ifdef kbCOM_BIG_ENDIAN
        INT32U one = *pi32uBuf++ ^ SWAP_ENDIAN_32_R(i32uCrc_l);
        INT32U two = *pi32uBuf++;
        INT32U three = *pi32uBuf++;
        INT32U four = *pi32uBuf++;

        i32uCrc_l = ai32uCrc32TableMulti_s[0][four & 0xFF] ^
            ai32uCrc32TableMulti_s[1][(four >> 8) & 0xFF] ^
            ai32uCrc32TableMulti_s[2][(four >> 16) & 0xFF] ^
            ai32uCrc32TableMulti_s[3][(four >> 24) & 0xFF] ^
            ai32uCrc32TableMulti_s[4][three & 0xFF] ^
            ai32uCrc32TableMulti_s[5][(three >> 8) & 0xFF] ^
            ai32uCrc32TableMulti_s[6][(three >> 16) & 0xFF] ^
            ai32uCrc32TableMulti_s[7][(three >> 24) & 0xFF] ^
            ai32uCrc32TableMulti_s[8][two & 0xFF] ^
            ai32uCrc32TableMulti_s[9][(two >> 8) & 0xFF] ^
            ai32uCrc32TableMulti_s[10][(two >> 16) & 0xFF] ^
            ai32uCrc32TableMulti_s[11][(two >> 24) & 0xFF] ^
            ai32uCrc32TableMulti_s[12][one & 0xFF] ^
            ai32uCrc32TableMulti_s[13][(one >> 8) & 0xFF] ^
            ai32uCrc32TableMulti_s[14][(one >> 16) & 0xFF] ^
            ai32uCrc32TableMulti_s[15][(one >> 24) & 0xFF];
#else
        INT32U one = *pi32uBuf++ ^ i32uCrc_l;
        INT32U two = *pi32uBuf++;
        INT32U three = *pi32uBuf++;
        INT32U four = *pi32uBuf++;

        i32uCrc_l = ai32uCrc32TableMulti_s[0][(four >> 24) & 0xFF] ^
            ai32uCrc32TableMulti_s[1][(four >> 16) & 0xFF] ^
            ai32uCrc32TableMulti_s[2][(four >> 8) & 0xFF] ^
            ai32uCrc32TableMulti_s[3][four & 0xFF] ^
            ai32uCrc32TableMulti_s[4][(three >> 24) & 0xFF] ^
            ai32uCrc32TableMulti_s[5][(three >> 16) & 0xFF] ^
            ai32uCrc32TableMulti_s[6][(three >> 8) & 0xFF] ^
            ai32uCrc32TableMulti_s[7][three & 0xFF] ^
            ai32uCrc32TableMulti_s[8][(two >> 24) & 0xFF] ^
            ai32uCrc32TableMulti_s[9][(two >> 16) & 0xFF] ^
            ai32uCrc32TableMulti_s[10][(two >> 8) & 0xFF] ^
            ai32uCrc32TableMulti_s[11][two & 0xFF] ^
            ai32uCrc32TableMulti_s[12][(one >> 24) & 0xFF] ^
            ai32uCrc32TableMulti_s[13][(one >> 16) & 0xFF] ^
            ai32uCrc32TableMulti_s[14][(one >> 8) & 0xFF] ^
            ai32uCrc32TableMulti_s[15][one & 0xFF];
#endif
        i32uLength_l -= BytesAtOnce;
    }

    pi8uBuf = (const INT8U *)pi32uBuf;
    while (i32uLength_l-- != 0)
    {
        i32uCrc_l = (i32uCrc_l >> 8) ^ ai32uCrc32TableMulti_s[0][(i32uCrc_l & 0xFF) ^ *pi8uBuf++];
    }

    *pi32uCrc_p = i32uCrc_l;
#else
    INT32U i32uCrc_l = *pi32uCrc_p;
    INT32U i;
    INT16U j;

    for (i = 0; i < i32uCnt_p; i++)
    {
        i32uCrc_l ^= (INT32U)pi8uData_p[i];
        for (j = 0; j < 8; j++)
        {
            if (i32uCrc_l & 0x00000001)
            {
                i32uCrc_l = (i32uCrc_l >> 1) ^ CRC32_POLY;
            }
            else
            {
                i32uCrc_l >>= 1;
            }
        }
    }

    *pi32uCrc_p = i32uCrc_l;
#endif
}

//*************************************************************************************************
//| Function: kbUT_uitoa
//|
//! Converts an unsigned integer into a string.
//! Returns true in case of success.
//!
//! ingroup. Util
//-------------------------------------------------------------------------------------------------
TBOOL kbUT_uitoa(INT32U p_value, INT8U* p_string, INT8U p_radix)
{
  INT8U* stringPtr  = p_string;
  INT8U* stringPtr1 = p_string;
  INT8U  tmp_char;
  INT32U oldValue;

  // Check base
  if (p_radix != 2 && p_radix != 10 && p_radix != 16)
  {
    // Error, invalid base
    return bFALSE;
  }

  do
  {
    oldValue = p_value;
    p_value /= p_radix;
    *stringPtr++ = "0123456789abcdef" [oldValue - p_value * p_radix];
  } while ( p_value );

  *stringPtr-- = '\0';

  // Reverse string
  while(stringPtr1 < stringPtr)
  {
    // Swap chars
    tmp_char = *stringPtr;
    *stringPtr--   = *stringPtr1;
    *stringPtr1++  = tmp_char;
  }

  return bTRUE;
}

//*************************************************************************************************
//| Function: kbUT_atoi
//|
//! Converts a string into an integer.
//! If the string starts with '0x', it is interpreted an hexadecimal number.
//! The function is able to read negative numbers, they are returned as two's complement.
//!
//!
//! ingroup. Util
//-------------------------------------------------------------------------------------------------
unsigned long kbUT_atoi(const char *start, int *success)
{
    unsigned long val = 0;         /* value we're accumulating */
    unsigned long base = 10;
    unsigned long max;
    int neg=0;              /* set to true if we see a minus sign */
    const char *s = start;

    *success = 1;

    /* skip whitespace */
    while (*s==' ' || *s=='\t') {
        s++;
    }

    /* check for sign */
    if (*s=='-') {
        neg=1;
        s++;
    } else if (*s=='0') {
        s++;
        if (*s=='x' || *s=='X') {
            base = 16;
            s++;
        }
    } else if (*s=='+') {
        s++;
    }

    max = 0xffffffff / base;

    /* process each digit */
    while (*s) {
        unsigned digit = 0;
        if (*s >= '0' && *s <= '9')
        {
            digit = *s - '0';
        }
        else if (base == 16 && *s >= 'a' && *s <= 'f')
        {
            digit = *s - 'a' + 10;
        }
        else if (base == 16 && *s >= 'A' && *s <= 'F')
        {
            digit = *s - 'A' + 10;
        }
        else
        {
            // unknown character -> stop conversion
            break;
        }
        if (val > max)
            *success = 0; // overflow

        /* shift the number over and add in the new digit */
        val = val*base + digit;

        /* look at the next character */
        s++;
    }

    if (s == start)
    {
        // not a single character has been converted, this is not successful
        *success = 0;
        return val;
    }

    /* handle negative numbers */
    if (neg) {
        val = ~(val-1);
        return val;
    }

    /* done */
    return val;
}

//*************************************************************************************************
//| Function: kbUT_itoa
//|
//! Converts an signed or unsigned integer into a string.
//! If radix is less than 0, val is interpreted as signed value,
//! otherwise, val is interpreted as unsigned value,
//!
//! Returns a pointer to a static buffer. The content of the buffer is only valid
//! until the next call of the function.
//! ingroup. Util
//-------------------------------------------------------------------------------------------------
char *kbUT_itoa(INT32U val, INT16S radix, INT16U len)
{
  static char buffer[13];
  unsigned int i = 0, j;
  char sign = 0;

  if (radix < 0)
  {
    radix = -radix;
    if ((INT32S)val < 0)
    {
      val = ~(val-1);
      sign = 1;
    }
  }
  if (radix > 16 || radix <= 7)
  {
    return 0;
  }

  while (val > 0)
  {
    j = val % radix;
    val = val / radix;
    if (j < 10)
      buffer[i++] = (char) ('0' + j);
    else
      buffer[i++] = (char) ('a' + j - 10);
  }
  if (i == 0)
    buffer[i++] = '0';
  if (sign)
    buffer[i++] = '-';

  while (i<len && i < sizeof(buffer)-1)
  {
    buffer[i++] = ' ';
  }

  buffer[i] = 0;

  // Reverse string
  i--;
  j = 0;
  while(j < i)
  {
    // Swap chars
    sign = buffer[j];
    buffer[j] = buffer[i];
    buffer[i] = sign;
    j++;
    i--;
  }
  return buffer;
}


//*************************************************************************************************

