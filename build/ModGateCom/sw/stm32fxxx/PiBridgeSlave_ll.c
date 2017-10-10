/*=============================================================================================
*
* PiBridgeSlave_ll.c
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

#include <PiBridgeSlave.h>

INT8U PIBS_DBG_input1A_g;
INT8U PIBS_DBG_input1B_g;
INT8U PIBS_DBG_input2_g;
INT8U PIBS_DBG_output1A_g;
INT8U PIBS_DBG_output1B_g;
INT8U PIBS_DBG_output2_g;

//*************************************************************************************************
//| Function:
//|
//! \brief
//! set output for Sniff 1A Pin
//! true: input with high impedance
//! false: output false
//!
//! \detailed
//!
//!
//!
//! \ingroup
//-------------------------------------------------------------------------------------------------
void PIBS_WriteSniff1A (
    TBOOL bSet_p)

{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Pin = PBSNIFF1A_PIN;

    if (bSet_p)
    {
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
        GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
        GPIO_Init(PBSNIFF1A_PORT, &GPIO_InitStructure);
        PIBS_DBG_output1A_g = 1;
    }
    else
    {
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
        GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
        GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
        GPIO_ResetBits(PBSNIFF1A_PORT, PBSNIFF1A_PIN);
        GPIO_Init(PBSNIFF1A_PORT, &GPIO_InitStructure);
        PIBS_DBG_output1A_g = 0;
    }
}

//*************************************************************************************************
//| Function:
//|
//! \brief
//! set output for Sniff 1B Pin
//! true: input with high impedance
//! false: output false
//!
//! \detailed
//!
//!
//!
//! \ingroup
//-------------------------------------------------------------------------------------------------
void PIBS_WriteSniff1B (
    TBOOL bSet_p)

{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Pin = PBSNIFF1B_PIN;

    if (bSet_p)
    {
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
        GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
        GPIO_Init(PBSNIFF1B_PORT, &GPIO_InitStructure);
        PIBS_DBG_output1B_g = 1;
    }
    else
    {
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
        GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
        GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
        GPIO_ResetBits(PBSNIFF1B_PORT, PBSNIFF1B_PIN);
        GPIO_Init(PBSNIFF1B_PORT, &GPIO_InitStructure);
        PIBS_DBG_output1B_g = 0;
    }
}

//*************************************************************************************************
//| Function:
//|
//! \brief
//! set output for Sniff 2 Pin
//! true: output true
//! false: input with high impedance
//!
//! \detailed
//!
//!
//!
//! \ingroup
//-------------------------------------------------------------------------------------------------
void PIBS_WriteSniff2 (
    TBOOL bSet_p)

{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Pin = PBSNIFF2_PIN;

    if (bSet_p)
    {
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
        GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
        GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
        GPIO_SetBits(PBSNIFF2_PORT, PBSNIFF2_PIN);
        GPIO_Init(PBSNIFF2_PORT, &GPIO_InitStructure);
        PIBS_DBG_output2_g = 1;
    }
    else
    {
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
        GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
        GPIO_Init(PBSNIFF2_PORT, &GPIO_InitStructure);
        PIBS_DBG_output2_g = 0;
    }
}


//*************************************************************************************************
//| Function:
//|
//! \brief
//!
//! \detailed
//!
//!
//!
//! \ingroup
//-------------------------------------------------------------------------------------------------
INT8U PIBS_ReadSniff1A (
    void)

{
    return PIBS_DBG_input1A_g = GPIO_ReadInputDataBit(PBSNIFF1A_PORT, PBSNIFF1A_PIN);
}

//*************************************************************************************************
//| Function:
//|
//! \brief
//!
//! \detailed
//!
//!
//!
//! \ingroup
//-------------------------------------------------------------------------------------------------
INT8U PIBS_ReadSniff1B (
    void)

{
    return PIBS_DBG_input1B_g = GPIO_ReadInputDataBit(PBSNIFF1B_PORT, PBSNIFF1B_PIN);
}

//*************************************************************************************************
//| Function:
//|
//! \brief
//!
//! \detailed
//!
//!
//!
//! \ingroup
//-------------------------------------------------------------------------------------------------
INT8U PIBS_ReadSniff2 (
    void)

{
    return PIBS_DBG_input2_g = GPIO_ReadInputDataBit(PBSNIFF2_PORT, PBSNIFF2_PIN);
}

