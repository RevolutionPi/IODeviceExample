/**
******************************************************************************
* @file    EEPROM_Emulation/src/eeprom.c 
* @author  MCD Application Team
* @version V3.1.0
* @date    07/27/2009
* @brief   This file provides all the EEPROM emulation firmware functions.
******************************************************************************
* @copy
*
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
* TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
* DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
* FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
* CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*
* <h2><center>&copy; COPYRIGHT 2009 STMicroelectronics</center></h2>
*/ 
/** @addtogroup EEPROM_Emulation
* @{
*/ 

/* Includes ------------------------------------------------------------------*/
#include <common_define.h>
#include <project.h>
#include <bsp\bspConfig.h>

#if !defined(STM_WITH_EEPROM_ADDR16)
  #error For STM32F2XX only the mode STM_WITH_EEPROM_ADDR16 can be used.
#endif

#include <bsp\eeprom\eeprom.h>
#include "bsp\eeprom\eeprom_intern.h"
#include <bsp\flash\FlashDriver.h>
#include <bsp\bspError.h>


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
static INT32U BSP_EEPROM_Format (void);
static EEPROM_INTERNAL_TYPE *BSP_EEPROM_FindValidPage (INT8U i8uOperation_p);
static INT32U BSP_EEPROM_VerifyPageFullWriteVariable (INT16U i16uVirtAddress_p, INT8U i8uData_p);
static INT32U BSP_EEPROM_PageTransfer (INT16U i16uVirtAddress_p, INT8U i8uData_p);
static INT32U BSP_EEPROM_ReadVariable (INT16U i16uVirtAddress_p, INT8U* pi8uData_p);
static INT32U BSP_EEPROM_WriteVariable (INT16U i16uVirtAddress_p, INT8U i8uData_p);


/**
* @brief  Restore the pages to a known good state in case of page's status
*   corruption after a power loss.
* 
* @retval - Flash error code: on write Flash error
*         - FLASH_COMPLETE: on success
*/
void BSP_EEPROM_init (void)
{
    EEPROM_INTERNAL_TYPE iuPageStatus0_l; 
    EEPROM_INTERNAL_TYPE iuPageStatus1_l;
    INT8U     iuDataVar_l;

    BSP_ASSERT (sizeof (BSP_EEPROM_TLayout) <= MAX_EEPROM_ADDRESS, BSPE_EE_ADDRESES_OUT_OF_SPACE);

    /* Get Page0 status */
    iuPageStatus0_l = *PAGE0_BASE_ADDRESS;
    /* Get Page1 status */
    iuPageStatus1_l = *PAGE1_BASE_ADDRESS;

    if (iuPageStatus0_l == VALID_PAGE)
    {
        if (iuPageStatus1_l != ERASED)
        {
            /* Erase Page1 */
            BSP_FLASH_eraseBlock((void *)PAGE1_BASE_ADDRESS);

            /* Get Page1 status */
            iuPageStatus1_l = *PAGE1_BASE_ADDRESS;
        }
    }

    if (iuPageStatus1_l == VALID_PAGE)
    {
        if (iuPageStatus0_l != ERASED)
        {
            /* Erase Page1 */
            BSP_FLASH_eraseBlock((void *)PAGE0_BASE_ADDRESS);

            /* Get Page0 status */
            iuPageStatus0_l = *PAGE0_BASE_ADDRESS;
        }
    }

    if (iuPageStatus0_l != VALID_PAGE && iuPageStatus1_l != VALID_PAGE)
    {
        /* Erase both Page0 and Page1 and set Page0 as valid page */
        BSP_EEPROM_Format();
    }
    
    iuDataVar_l = BSP_EEPROM_readByte (BSP_EEPROM_ADDR (i8uInitMarker));
    if (iuDataVar_l != 'M')
    {   // Init EEPROM with Default values
        BSP_EEPROM_factoryReset ();
    }
}

/**
* @brief  Returns the last stored variable data, if found, which correspond to
*   the passed virtual address
* @param  VirtAddress: Variable virtual address
* @param  Data: Global variable contains the read variable value
* @retval Success or error status:
*           - 0: if variable was found
*           - 1: if the variable was not found
*           - NO_VALID_PAGE: if no valid page was found.
*/
INT32U BSP_EEPROM_ReadVariable (
    INT16U i16uVirtAddress_p, 
    INT8U* pi8uData_p)
{
    INT32U i32uRv_l = BSPE_EE_VAR_NOT_EXIST;
    EEPROM_INTERNAL_TYPE *piuAddress_l; 
    EEPROM_INTERNAL_TYPE *piuPageStartAddress_l;

    if (i16uVirtAddress_p > MAX_EEPROM_ADDRESS)
    {
        return (BSPE_EE_READ_ADR_NOT_EXIST);
    }

    /* Get active Page for read operation */
    piuPageStartAddress_l = BSP_EEPROM_FindValidPage(READ_FROM_VALID_PAGE);

    /* Check if there is no valid page */
    if (piuPageStartAddress_l == (EEPROM_INTERNAL_TYPE *)0)
    {
        return  (BSPE_EE_NO_VALID_PAGE);
    }

    /* Get the valid Page end Address */
    piuAddress_l = piuPageStartAddress_l + (PAGE_SIZE / sizeof (EEPROM_INTERNAL_TYPE)) - 1;

    /* Check each active page address starting from end (border is the page marker)*/
    for (;piuAddress_l > piuPageStartAddress_l; piuAddress_l--)
    {
        /* Compare the read address with the virtual address */
        if ((*piuAddress_l & EEPROM_ADDRESS_MASK) == i16uVirtAddress_p)
        {
            /* Get content of Address High which is variable value */
            *pi8uData_p = (INT8U)(*piuAddress_l >> EEPROM_ADDRESS_BITS);

            /* In case variable value is read, reset ReadStatus flag */
            i32uRv_l = BSPE_NO_ERROR;
            break;
        }
    }

    /* Return ReadStatus value: (BSPE_NO_ERROR: variable exist, BSPE_EE_VAR_NOT_EXIST: variable doesn't exist) */
    return i32uRv_l;
}

/**
* @brief  Writes/upadtes variable data in EEPROM.
* @param  VirtAddress: Variable virtual address
* @param  Data: 16 bit data to be written
* @retval Success or error status:
*           - FLASH_COMPLETE: on success
*           - PAGE_FULL: if valid page is full
*           - NO_VALID_PAGE: if no valid page was found
*           - Flash error code: on write Flash error
*/
INT32U BSP_EEPROM_WriteVariable(INT16U i16uVirtAddress_p, INT8U i8uData_p)
{
    INT32U i32uRv_l;  

    if (i16uVirtAddress_p > MAX_EEPROM_ADDRESS)
    {
        return (BSPE_EE_WRITE_ADR_NOT_EXIST);
    }

    /* Write the variable virtual address and value in the EEPROM */
    i32uRv_l = BSP_EEPROM_VerifyPageFullWriteVariable(i16uVirtAddress_p, i8uData_p);

    /* In case the EEPROM active page is full */
    if (i32uRv_l == BSPE_EE_PAGE_FULL)
    {
        /* Perform Page transfer */
        i32uRv_l = BSP_EEPROM_PageTransfer(i16uVirtAddress_p, i8uData_p);
    }

    /* Return last operation status */
    return i32uRv_l;
}

/**
* @brief  Erases PAGE0 and PAGE1 and writes VALID_PAGE header to PAGE0
* @param  None
* @retval Status of the last operation (Flash write or erase) done during
*         EEPROM formating
*/
static INT32U BSP_EEPROM_Format(void)
{
    INT32U i32uRv_l = BSPE_NO_ERROR;
    EEPROM_INTERNAL_TYPE iuVal_l;

    /* Erase Page0 */
    BSP_FLASH_eraseBlock(PAGE0_BASE_ADDRESS);

    /* Set Page0 as valid page: Write VALID_PAGE at Page0 base address */
    iuVal_l = VALID_PAGE;
    BSP_FLASH_write (PAGE0_BASE_ADDRESS, &iuVal_l, sizeof (iuVal_l));

    /* Erase Page1 */
    BSP_FLASH_eraseBlock(PAGE1_BASE_ADDRESS);

#ifdef FACTORY_RESET_ERASES_STORE_PARA_FLASH_PAGE
    /*erase STORE_PARA blocks*/
    BSP_FLASH_eraseRange(EEPROM_STORE_PARA_START_ADDRESS, EEPROM_STORE_PARA_STOP_ADDRESS);
#endif /*FACTORY_RESET_ERASES_STORE_PARA_FLASH_PAGE*/

    /* Return Page1 erase operation status */
    return i32uRv_l;
}

/**
* @brief  Find valid Page for write or read operation
* @param  Operation: operation to achieve on the valid page.
*   This parameter can be one of the following values:
*     @arg READ_FROM_VALID_PAGE: read operation from valid page
*     @arg WRITE_IN_VALID_PAGE: write operation from valid page
* @retval Valid page number (PAGE0 or PAGE1) or NO_VALID_PAGE in case
*   of no valid page was found
*/
static EEPROM_INTERNAL_TYPE *BSP_EEPROM_FindValidPage (INT8U i8uOperation_p)
{
    EEPROM_INTERNAL_TYPE iuPageStatus0_l;
    EEPROM_INTERNAL_TYPE iuPageStatus1_l;
    EEPROM_INTERNAL_TYPE *iuRetVal_l = (EEPROM_INTERNAL_TYPE *)0;
    EEPROM_INTERNAL_TYPE *ptr;

    /* Get Page0 actual status */
    ptr = PAGE0_BASE_ADDRESS;
    iuPageStatus0_l = *ptr;

    /* Get Page1 actual status */
    ptr = PAGE1_BASE_ADDRESS;
    iuPageStatus1_l = *ptr;

    /* Write or read operation */
    switch (i8uOperation_p)
    {
    case WRITE_IN_VALID_PAGE:   /* ---- Write operation ---- */
        if (iuPageStatus1_l == VALID_PAGE)
        {
            /* Page0 receiving data */
            if (iuPageStatus0_l == RECEIVE_DATA)
            {
                iuRetVal_l = PAGE0_BASE_ADDRESS;         /* Page0 valid */
            }
            else
            {
                iuRetVal_l = PAGE1_BASE_ADDRESS;         /* Page1 valid */
            }
        }
        else if (iuPageStatus0_l == VALID_PAGE)
        {
            /* Page1 receiving data */
            if (iuPageStatus1_l == RECEIVE_DATA)
            {
                iuRetVal_l = PAGE1_BASE_ADDRESS;         /* Page1 valid */
            }
            else
            {
                iuRetVal_l = PAGE0_BASE_ADDRESS;         /* Page0 valid */
            }
        }   break;
    case READ_FROM_VALID_PAGE:  /* ---- Read operation ---- */
        if (iuPageStatus0_l == VALID_PAGE)
        {
            iuRetVal_l = PAGE0_BASE_ADDRESS;           /* Page0 valid */
        }
        else if (iuPageStatus1_l == VALID_PAGE)
        {
            iuRetVal_l = PAGE1_BASE_ADDRESS;           /* Page1 valid */
        }   break;
    default:
        break;             /* Option Invalid */
    }
    
    return (iuRetVal_l);
}

/**
* @brief  Verify if active page is full and Writes variable in EEPROM.
* @param  VirtAddress: 16 bit virtual address of the variable
* @param  Data: 16 bit data to be written as variable value
* @retval Success or error status:
*           - FLASH_COMPLETE: on success
*           - PAGE_FULL: if valid page is full
*           - NO_VALID_PAGE: if no valid page was found
*           - Flash error code: on write Flash error
*/
static INT32U BSP_EEPROM_VerifyPageFullWriteVariable(INT16U i16uVirtAddress_p, INT8U i8uData_p)
{
    EEPROM_INTERNAL_TYPE *piuAddress_l; 
    EEPROM_INTERNAL_TYPE *piuPageEndAddress_l;
    EEPROM_INTERNAL_TYPE iuAddressValue_l;

    if (i16uVirtAddress_p > MAX_EEPROM_ADDRESS)
    {
        return (BSPE_EE_READ_ADR_NOT_EXIST);
    }

    /* Get active Page for read operation */
    piuAddress_l = BSP_EEPROM_FindValidPage(WRITE_IN_VALID_PAGE);

    /* Check if there is no valid page */
    if (piuAddress_l == (EEPROM_INTERNAL_TYPE *)0)
    {
        return (BSPE_EE_NO_VALID_PAGE);
    }

    /* Get the valid Page end Address */
    piuPageEndAddress_l = piuAddress_l + (PAGE_SIZE / sizeof (EEPROM_INTERNAL_TYPE)) - 1;

    iuAddressValue_l =  (i8uData_p << EEPROM_ADDRESS_BITS) | i16uVirtAddress_p;

    piuAddress_l++;  // Skip Page marker    
    /* Check each active page address starting from beginning */
    for (;piuAddress_l <= piuPageEndAddress_l; piuAddress_l++)
    {
        /* Verify if Address High and Low contents are 0xFFFF/0xFFFFFFFF */
        if (*piuAddress_l == ERASED)
        {
            /* Set variable data */
            BSP_FLASH_write(piuAddress_l, &iuAddressValue_l, sizeof (iuAddressValue_l));
            return (BSPE_NO_ERROR);
        }
    }

    /* Return PAGE_FULL in case the valid page is full */
    return (BSPE_EE_PAGE_FULL);
}

/**
* @brief  Transfers last updated variables data from the full Page to
*   an empty one.
* @param  VirtAddress: 16 bit virtual address of the variable
* @param  Data: 16 bit data to be written as variable value
* @retval Success or error status:
*           - FLASH_COMPLETE: on success
*           - PAGE_FULL: if valid page is full
*           - NO_VALID_PAGE: if no valid page was found
*           - Flash error code: on write Flash error
*/
static INT32U BSP_EEPROM_PageTransfer (INT16U i16uVirtAddress_p, INT8U i8uData_p)
{
    INT32U i32uRv_l;
    EEPROM_INTERNAL_TYPE *piuNewPage_l;
    EEPROM_INTERNAL_TYPE *piuOldPage_l;
    EEPROM_INTERNAL_TYPE *piuValidPage_l; 
    INT16U i16uVarIdx_l;
    EEPROM_INTERNAL_TYPE iuVal_l;
    INT8U iuData_l;

    /* Get active Page for read operation */
    piuValidPage_l = BSP_EEPROM_FindValidPage(READ_FROM_VALID_PAGE);

    if (piuValidPage_l == PAGE1_BASE_ADDRESS)       /* Page1 valid */
    {
        /* New page address where variable will be moved to */
        piuNewPage_l = PAGE0_BASE_ADDRESS;

        /* Old page address where variable will be taken from */
        piuOldPage_l = PAGE1_BASE_ADDRESS;
    }
    else if (piuValidPage_l == PAGE0_BASE_ADDRESS)  /* Page0 valid */
    {
        /* New page address where variable will be moved to */
        piuNewPage_l = PAGE1_BASE_ADDRESS;

        /* Old page address where variable will be taken from */
        piuOldPage_l = PAGE0_BASE_ADDRESS;
    }
    else
    {
        return (BSPE_EE_NO_VALID_PAGE);       /* No valid Page */
    }

    /* Set the new Page status to RECEIVE_DATA status */
    iuVal_l = RECEIVE_DATA;
    BSP_FLASH_write(piuNewPage_l, &iuVal_l, sizeof (iuVal_l));

    /* Write the variable passed as parameter in the new active page */
    i32uRv_l = BSP_EEPROM_VerifyPageFullWriteVariable (i16uVirtAddress_p, i8uData_p);
    /* If program operation was failed, a Flash error code is returned */
    if (i32uRv_l != BSPE_NO_ERROR)
    {
        return (i32uRv_l);
    }

    /* Transfer process: transfer variables from old to the new active page */
    for (i16uVarIdx_l = 0; i16uVarIdx_l < MAX_EEPROM_ADDRESS; i16uVarIdx_l++)
    {
        if (i16uVarIdx_l != i16uVirtAddress_p)  /* Check each variable except the one passed as parameter */
        {
            /* Read the other last variable updates */
            i32uRv_l = BSP_EEPROM_ReadVariable (i16uVarIdx_l, &iuData_l);
            /* In case variable corresponding to the virtual address was found */
            if (i32uRv_l == BSPE_NO_ERROR)
            {
                /* Transfer the variable to the new active page */
                i32uRv_l = BSP_EEPROM_VerifyPageFullWriteVariable(i16uVarIdx_l, iuData_l);
                /* If program operation was failed, a Flash error code is returned */
                if (i32uRv_l != BSPE_NO_ERROR)
                {
                    return (i32uRv_l);
                }
            }
        }
    }

    /* Erase the old Page: Set old Page status to ERASED status */
    BSP_FLASH_eraseBlock((void *)piuOldPage_l);

    /* Set new Page status to VALID_PAGE status */
    iuVal_l = VALID_PAGE; 
    BSP_FLASH_write((void *)piuNewPage_l, &iuVal_l, sizeof (iuVal_l));

    /* if we reach this point, everything is ok */
    return (BSPE_NO_ERROR);
}

/**
* @}
*/ 

//*************************************************************************************************
//| Function: BSP_EEPROM_readByte
//|
//! brief.
//!
//! detailed.
//!
//!
//!
//! ingroup.
//-------------------------------------------------------------------------------------------------
INT8U BSP_EEPROM_readByte (
    INT16U i16uVirtAddress_p) //!< [in]  Virtual Adress inside EEPROM simulated storage
                            //! \return Value of byte; if invalid address or byte not store 0xff 
                            //! as default value is used
                            
{

    INT8U iuRv_l;

    BSP_ASSERT (i16uVirtAddress_p < sizeof (BSP_EEPROM_TLayout), BSPE_EE_RD_BYTE_ADR_OUT_OF_RANGE);

    if (BSP_EEPROM_ReadVariable(i16uVirtAddress_p, &iuRv_l) != BSPE_NO_ERROR)
    {
        iuRv_l = 0xff;
    }

    return (iuRv_l);    
}

//*************************************************************************************************
//| Function: BSP_EEPROM_readWord
//|
//! brief.
//!
//! detailed.
//!
//!
//!
//! ingroup.
//-------------------------------------------------------------------------------------------------
INT16U BSP_EEPROM_readWord (
    INT16U i16uVirtAddress_p) //!< [in]  Virtual Adress inside EEPROM simulated storage
                            //! \return Value of DW; if invalid address or byte not store 0xffffffff 
                            //! as default value is used
                            
{
    INT16U i;
    INT16U i16uData_l;

    BSP_ASSERT (i16uVirtAddress_p < sizeof (BSP_EEPROM_TLayout), BSPE_EE_RD_WORD_ADR_OUT_OF_RANGE);

    // Read the 2 bytes of the Word
    for (i = 0; i < 2; i++)
    {
        if (BSP_EEPROM_ReadVariable(i16uVirtAddress_p + i, ((INT8U *)&i16uData_l) + i) != BSPE_NO_ERROR)
        {
            return (0xffff);
        }
    }

    return i16uData_l; // assumes little endian !
}


//*************************************************************************************************
//| Function: BSP_EEPROM_readDWord
//|
//! brief.
//!
//! detailed.
//!
//!
//!
//! ingroup.
//-------------------------------------------------------------------------------------------------
INT32U BSP_EEPROM_readDWord (
    INT16U i16uVirtAddress_p) //!< [in]  Virtual Adress inside EEPROM simulated storage
                            //! \return Value of DW; if invalid address or byte not store 0xffffffff 
                            //! as default value is used
                            
{
    INT16U i;
    INT32U i32uData_l;

    BSP_ASSERT (i16uVirtAddress_p < sizeof (BSP_EEPROM_TLayout), BSPE_EE_RD_DWORD_ADR_OUT_OF_RANGE);

    // Read the 4 bytes of the DW
    for (i = 0; i < 4; i++)
    {
        if (BSP_EEPROM_ReadVariable(i16uVirtAddress_p + i, ((INT8U *)&i32uData_l) + i) != BSPE_NO_ERROR)
        {
            return (0xffffffff);
        }
    }

    return i32uData_l; // assumes little endian !
}

//*************************************************************************************************
//| Function: BSP_EEPROM_writeByte
//|
//! brief.
//!
//! detailed.
//!
//!
//!
//! ingroup.
//-------------------------------------------------------------------------------------------------
void BSP_EEPROM_writeByte (
    INT16U i16uVirtAddress_p,     //!< [in] Virtual Adress inside EEPROM simulated storage
    INT8U i8uData_p)            //!< [in] Data to write
                                //! \return Error Code
{
    INT32U i32uErr_l;
    INT8U i8uDataStored_l;
    
    BSP_ASSERT (i16uVirtAddress_p < sizeof (BSP_EEPROM_TLayout), BSPE_EE_WR_BYTE_ADR_OUT_OF_RANGE);

    i32uErr_l = BSP_EEPROM_ReadVariable(i16uVirtAddress_p, &i8uDataStored_l);
    if (   (i32uErr_l != BSPE_NO_ERROR)
        || (i8uDataStored_l != i8uData_p)
       ) 
    {
        i32uErr_l = BSP_EEPROM_WriteVariable(i16uVirtAddress_p, i8uData_p);
        BSP_ASSERT (i32uErr_l == BSPE_NO_ERROR, i32uErr_l);
    }    
    
}                                


//*************************************************************************************************
//| Function: BSP_EEPROM_writeDWord
//|
//! brief.
//!
//! detailed.
//!
//!
//!
//! ingroup.
//-------------------------------------------------------------------------------------------------
void BSP_EEPROM_writeWord (
    INT16U i16uVirtAddress_p,     //!< [in] Virtual Adress inside EEPROM simulated storage
    INT16U i16uData_p)          //!< [in] Data to write
                                //! \return Error Code
{
    // Note that the DW is stored in little endian format, i.e.
    // The LSB is stored first (at the lower eeprom address).

    INT32U i32uErr_l;
    INT16U i16uDataStored_l;
    INT16U i;
    
    BSP_ASSERT (i16uVirtAddress_p < sizeof (BSP_EEPROM_TLayout), BSPE_EE_WR_WORD_ADR_OUT_OF_RANGE);

    i16uDataStored_l = BSP_EEPROM_readWord (i16uVirtAddress_p);
    if (i16uDataStored_l != i16uData_p)
    {
        for (i = 0; i < 2; i++)
        {
            i32uErr_l = BSP_EEPROM_WriteVariable (i16uVirtAddress_p + i, (INT8U)(i16uData_p >> (8 * i) & 0xff));
            BSP_ASSERT (i32uErr_l == BSPE_NO_ERROR, i32uErr_l);
        }
    }
}

//*************************************************************************************************
//| Function: BSP_EEPROM_writeDWord
//|
//! brief.
//!
//! detailed.
//!
//!
//!
//! ingroup.
//-------------------------------------------------------------------------------------------------
void BSP_EEPROM_writeDWord (
    INT16U i16uVirtAddress_p,     //!< [in] Virtual Adress inside EEPROM simulated storage
    INT32U i32uData_p)          //!< [in] Data to write
                                //! \return Error Code
{
    // Note that the DW is stored in little endian format, i.e.
    // The LSB is stored first (at the lower eeprom address).

    INT32U i32uErr_l;
    INT32U i32uDataStored_l;
    INT16U i;
    
    BSP_ASSERT (i16uVirtAddress_p < sizeof (BSP_EEPROM_TLayout), BSPE_EE_WR_DWORD_ADR_OUT_OF_RANGE);

    i32uDataStored_l = BSP_EEPROM_readDWord (i16uVirtAddress_p);
    if (i32uDataStored_l != i32uData_p)
    {
        for (i=0;i<4;i++)
        {
            i32uErr_l = BSP_EEPROM_WriteVariable (i16uVirtAddress_p + i, (INT8U)(i32uData_p >> (8 * i) & 0xff));
            BSP_ASSERT (i32uErr_l == BSPE_NO_ERROR, i32uErr_l);
        }
    }
}

//*************************************************************************************************
//| Function: BSP_EEPROM_getDefaultData
//|
//! \brief
//!
//!
//!
//!
//!
//! \ingroup
//-------------------------------------------------------------------------------------------------
INT8U BSP_EEPROM_getDefaultData (
    INT16U i16uVirtAddress_p)
    
{
    INT16U i16uInd_l;
    INT8U *pi8uData_l;

    BSP_ASSERT (i16uVirtAddress_p < sizeof (BSP_EEPROM_TLayout), BSPE_EE_DEF_DATA_ADR_OUT_OF_RANGE);

    for (i16uInd_l = 0; BSP_EEPROM_atDefaultRef[i16uInd_l].vpSrc != ((void *)0); i16uInd_l++)
    {
        const BSP_EEPROM_TDefaultRef *ptRef_l = &BSP_EEPROM_atDefaultRef[i16uInd_l]; 
    
        if (   (i16uVirtAddress_p >= ptRef_l->i16uEepromAddr)
            && (i16uVirtAddress_p < ptRef_l->i16uEepromAddr + ptRef_l->i16uLen)
           ) 
        {
            pi8uData_l = (INT8U *)ptRef_l->vpSrc + i16uVirtAddress_p - ptRef_l->i16uEepromAddr;
            return (*pi8uData_l);
        }   
    }
    
    pi8uData_l = (INT8U *)&BSP_EEPROM_ctDefaultValue + i16uVirtAddress_p;
    return (*pi8uData_l);
}    


//*************************************************************************************************
//| Function: BSP_EEPROM_factoryReset
//|
//! \brief
//!
//!
//!
//!
//!
//! \ingroup
//-------------------------------------------------------------------------------------------------
void BSP_EEPROM_factoryReset (
    void)
    
{

    INT16U i16uInd_l;
    INT8U i8uData_l;
    INT32U i32uErr_l;
 
    i32uErr_l = BSP_EEPROM_Format ();
    BSP_ASSERT (i32uErr_l == BSPE_NO_ERROR, BSPE_EE_FORMAT_FACTORY_RESET);
 
    for (i16uInd_l = 0; i16uInd_l < sizeof (BSP_EEPROM_TLayout); i16uInd_l++)
    {
        i8uData_l = BSP_EEPROM_getDefaultData (i16uInd_l);  
        BSP_EEPROM_writeByte (i16uInd_l, i8uData_l);  
    }
    BSP_EEPROM_writeByte (BSP_EEPROM_ADDR (i8uInitMarker), 'M');
}    

//*************************************************************************************************



/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/
