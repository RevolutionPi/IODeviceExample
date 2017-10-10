//+=============================================================================================
//|
//|    EEPROM with a dedicated chip, connected via SPI
//|
//+---------------------------------------------------------------------------------------------
//|
//|    File-ID:    $Id: eeprom_chip.c 9293 2015-10-01 14:30:19Z rmeeh $
//|    Location:   $URL: http://srv-kunbus03.de.pilz.local/feldbus/software/trunk/platform/bsp/sw/bsp/eeprom/eeprom_chip.c $
//|    Copyright:  KUNBUS GmbH
//|
//+=============================================================================================

#include <common_define.h>
#include <project.h>
#include <bsp\bspConfig.h>

#include <bsp\eeprom\eeprom.h>
#include <bsp\bspError.h>
#include <bsp\spi\spi.h>
#include <bsp\gpio\gpio.h>
#include <bsp\timer\timer.h>
#include <bsp\systick\systick.h>
#ifdef STM_USE_BSP_OS  
  #include <bsp\scheduler\osApi.h>
  extern BSP_OS_TMutex BSP_tSpiPeriMutex_g;
#endif    

BSP_SPI_TRwPeriData BSP_EEPROM_tRwPeriData_s;

void BSP_EEPROM_eraseAll (void);
void BSP_Wait (INT32U i32uMicroSeconds_p);

#define MAX_EEPROM_ADDRESS 2048     // 93LC86AT  serial EEPROM

//*************************************************************************************************
//| Function: eepromInit
//|
//! \brief
//!
//! \detailed
//!
//!
//!
//! \ingroup
//-------------------------------------------------------------------------------------------------
void BSP_EEPROM_init(
    void)
    
{
    const HW_SPI_CONFIGURATION tSpiConfiguration_l =
    {
        .mode			= HW_SPI_MODE_MASTER,
        .polarity		= HW_SPI_CLOCK_POL_HIGH,
        .phase			= HW_SPI_CLOCK_PHASE_TRAIL,
        .direction		= HW_SPI_DATA_DIR_MSB,
        .nss			= 0,				// is set by software
        .bitrate		= 1000000,			// 1 MHz
        .receive_cb		= 0,
        .transmit_cb	= 0,
        .error_cb		= 0,
        .finish_cb		= 0,
    };
    INT8U ai8uCmd_l[2];
    INT8U i8uData_l;

    BSP_ASSERT (sizeof (BSP_EEPROM_TLayout) <= MAX_EEPROM_ADDRESS, BSPE_EE_ADDRESES_OUT_OF_SPACE);
 
#ifdef STM_USE_BSP_OS 
    {
        TBOOL bTimeOut_l;
        
        BSP_OS_getMutex (&BSP_tSpiPeriMutex_g, -1, &bTimeOut_l);
    } 
#endif    

    BSP_EEPROM_tRwPeriData_s.vpCsPort = EEPROM_SPI_CS_PORT;
    BSP_EEPROM_tRwPeriData_s.i16uCsPin = EEPROM_SPI_CS_PIN;
    BSP_EEPROM_tRwPeriData_s.bActiveHigh = EEPROM_SPI_CS_ACTIVE_HIGH;
    
    BSP_SPI_RWPERI_init (EEPROM_SPI_PORT, &tSpiConfiguration_l, &BSP_EEPROM_tRwPeriData_s);



    // Inital Write Enable of the chip
    ai8uCmd_l[0] = 0x98;  // EWEN
    ai8uCmd_l[1] = 0x00;  

    BSP_SPI_RWPERI_prepareSpi (&BSP_EEPROM_tRwPeriData_s);
    BSP_SPI_RWPERI_chipSelectEnable (&BSP_EEPROM_tRwPeriData_s);
    spi_transceive (EEPROM_SPI_PORT, ai8uCmd_l, NULL, 2, bTRUE);
    BSP_SPI_RWPERI_chipSelectDisable (&BSP_EEPROM_tRwPeriData_s);
    BSP_Wait (2000);

#ifdef STM_USE_BSP_OS 
    BSP_OS_releaseMutex (&BSP_tSpiPeriMutex_g);
#endif    

    i8uData_l = BSP_EEPROM_readByte (BSP_EEPROM_ADDR (i8uInitMarker));
    if (i8uData_l != 'M')
    {   // Init EEPROM with Default values
        BSP_EEPROM_factoryReset ();
    }



}    


//*************************************************************************************************
//| Function: eepromReadByte
//|
//! \brief
//!
//! \detailed
//!
//!
//!
//! \ingroup
//-------------------------------------------------------------------------------------------------
INT8U BSP_EEPROM_readByte (
    INT16U i16uAddress_p)
    
{
    INT8U ai8uCmd_l[3];
    INT8U ai8uRecVal_l[3];
    INT8U i8uRv_l;

    BSP_ASSERT (i16uAddress_p < sizeof (BSP_EEPROM_TLayout), BSPE_EE_RD_BYTE_ADR_OUT_OF_RANGE);

    ai8uCmd_l[0] = 0xc0 | ((i16uAddress_p >> 6) & 0x1f);  // READ
    ai8uCmd_l[1] = (i16uAddress_p << 2);  
    ai8uCmd_l[2] = 0x00;  

#ifdef STM_USE_BSP_OS 
    {
        TBOOL bTimeOut_l;
        
        BSP_OS_getMutex (&BSP_tSpiPeriMutex_g, -1, &bTimeOut_l);
    } 
#endif    

    BSP_SPI_RWPERI_prepareSpi (&BSP_EEPROM_tRwPeriData_s);
    BSP_SPI_RWPERI_chipSelectEnable (&BSP_EEPROM_tRwPeriData_s);
    spi_transceive (EEPROM_SPI_PORT, ai8uCmd_l, ai8uRecVal_l, 3, bTRUE);
    BSP_SPI_RWPERI_chipSelectDisable (&BSP_EEPROM_tRwPeriData_s);

#ifdef STM_USE_BSP_OS 
    BSP_OS_releaseMutex (&BSP_tSpiPeriMutex_g);
#endif    

    i8uRv_l = (ai8uRecVal_l[1] << 7) |(ai8uRecVal_l[2] >> 1);
    return (i8uRv_l);
}    

//*************************************************************************************************
//| Function: eepromWriteByte
//|
//! \brief
//!
//! \detailed
//!
//!
//!
//! \ingroup
//-------------------------------------------------------------------------------------------------
void BSP_EEPROM_writeByte (
    INT16U i16uAddress_p, 
    INT8U i8uData_p)
    
{

    INT8U ai8uCmd_l[3];
    INT16U i16uInd_l;
    INT8U i8uMask_l;
    INT32U i32uStartTime_l;
    
    GPIO_TypeDef *ptSckPort_l = (GPIO_TypeDef *)BSP_EEPROM_tRwPeriData_s.vpSckPort;
    INT16U i16uSckPin_l = BSP_EEPROM_tRwPeriData_s.i16uSckPin;
    GPIO_TypeDef *ptMosiPort_l = (GPIO_TypeDef *)BSP_EEPROM_tRwPeriData_s.vpMosiPort;
    INT16U i16uMosiPin_l = BSP_EEPROM_tRwPeriData_s.i16uMosiPin;
    GPIO_TypeDef *ptMisoPort_l = (GPIO_TypeDef *)BSP_EEPROM_tRwPeriData_s.vpMisoPort;
    INT16U i16uMisoPin_l = BSP_EEPROM_tRwPeriData_s.i16uMisoPin;

    BSP_ASSERT (i16uAddress_p < sizeof (BSP_EEPROM_TLayout), BSPE_EE_WR_BYTE_ADR_OUT_OF_RANGE);

#ifdef STM_USE_BSP_OS 
    {
        TBOOL bTimeOut_l;
        
        BSP_OS_getMutex (&BSP_tSpiPeriMutex_g, -1, &bTimeOut_l);
    } 
#endif    


    BSP_SPI_RWPERI_spiDisable (&BSP_EEPROM_tRwPeriData_s);

    ai8uCmd_l[0] = 0xa0 | ((i16uAddress_p >> 6) & 0x1f);  // WRITE
    ai8uCmd_l[1] = (i16uAddress_p << 2) | (i8uData_p >> 6);  
    ai8uCmd_l[2] = (i8uData_p << 2);  

    BSP_SPI_RWPERI_chipSelectEnable (&BSP_EEPROM_tRwPeriData_s);
    i8uMask_l = 0x80;
    for (i16uInd_l = 0; i16uInd_l < 22; i16uInd_l++)
    {
        if (ai8uCmd_l[i16uInd_l / 8] & i8uMask_l)
        {
            GPIO_SetBits (ptMosiPort_l, i16uMosiPin_l);
        }
        else
        {
            GPIO_ResetBits (ptMosiPort_l, i16uMosiPin_l);
        }
        BSP_Wait (2);
        i8uMask_l >>= 1;
        if (i8uMask_l == 0)
        {
            i8uMask_l = 0x80;
        }
        

        GPIO_SetBits (ptSckPort_l, i16uSckPin_l);
        BSP_Wait (2);
        GPIO_ResetBits (ptSckPort_l, i16uSckPin_l);
        BSP_Wait (2);
    }
    
    BSP_SPI_RWPERI_chipSelectDisable (&BSP_EEPROM_tRwPeriData_s);
    BSP_Wait (2);
    
    i32uStartTime_l = kbGetTickCount ();
    BSP_SPI_RWPERI_chipSelectEnable (&BSP_EEPROM_tRwPeriData_s);
    while (GPIO_ReadInputDataBit (ptMisoPort_l, i16uMisoPin_l) == Bit_RESET)
    {
        if ((kbGetTickCount () - i32uStartTime_l) > 10)
        {       // maximum wait time due to data sheet is 5 ms
            bspError (BSPE_EE_WRITE_READY_TIME_OUT, bTRUE, 0);
        }
    }
    
    BSP_SPI_RWPERI_chipSelectDisable (&BSP_EEPROM_tRwPeriData_s);
    BSP_Wait (2);
    
    BSP_SPI_RWPERI_spiEnable (&BSP_EEPROM_tRwPeriData_s);
    
#ifdef STM_USE_BSP_OS 
    BSP_OS_releaseMutex (&BSP_tSpiPeriMutex_g);
#endif    
    
    
}    

//*************************************************************************************************
//| Function: eepromReadDWord
//|
//! \brief
//!
//! \detailed
//!
//!
//!
//! \ingroup
//-------------------------------------------------------------------------------------------------
INT16U BSP_EEPROM_readWord (
    INT16U i16uAddress_p)
    
{
    INT8U ai8uCmd_l[4];
    INT8U ai8uRecVal_l[4];
    INT8U i8uDataL_l;
    INT8U i8uDataH_l;
    INT16U i16uRv_l;

    BSP_ASSERT (i16uAddress_p < (sizeof (BSP_EEPROM_TLayout) - 1), BSPE_EE_RD_WORD_ADR_OUT_OF_RANGE);

    ai8uCmd_l[0] = 0xc0 | ((i16uAddress_p >> 6) & 0x1f);  // READ
    ai8uCmd_l[1] = (i16uAddress_p << 2);  
    ai8uCmd_l[2] = 0;  
    ai8uCmd_l[3] = 0;  

#ifdef STM_USE_BSP_OS 
    {
        TBOOL bTimeOut_l;
        
        BSP_OS_getMutex (&BSP_tSpiPeriMutex_g, -1, &bTimeOut_l);
    } 
#endif    

    BSP_SPI_RWPERI_prepareSpi (&BSP_EEPROM_tRwPeriData_s);
    BSP_SPI_RWPERI_chipSelectEnable (&BSP_EEPROM_tRwPeriData_s);
    spi_transceive (EEPROM_SPI_PORT, ai8uCmd_l, ai8uRecVal_l, 4, bTRUE);
    BSP_SPI_RWPERI_chipSelectDisable (&BSP_EEPROM_tRwPeriData_s);

#ifdef STM_USE_BSP_OS 
    BSP_OS_releaseMutex (&BSP_tSpiPeriMutex_g);
#endif    

    i8uDataL_l = (ai8uRecVal_l[1] << 7) |(ai8uRecVal_l[2] >> 1);
    i8uDataH_l = (ai8uRecVal_l[2] << 7) |(ai8uRecVal_l[3] >> 1);

    i16uRv_l = (i8uDataH_l << 8) | (i8uDataL_l);

    return (i16uRv_l);
}    

//*************************************************************************************************
//| Function: eepromWriteDWord
//|
//! \brief
//!
//! \detailed
//!
//!
//!
//! \ingroup
//-------------------------------------------------------------------------------------------------
void BSP_EEPROM_writeWord  (
    INT16U i16uAddress_p, 
    INT16U i16uData_p)
    
{
    BSP_ASSERT (i16uAddress_p < (sizeof (BSP_EEPROM_TLayout) - 1), BSPE_EE_WR_WORD_ADR_OUT_OF_RANGE);

    BSP_EEPROM_writeByte (i16uAddress_p, (INT8U)i16uData_p);
    BSP_EEPROM_writeByte (i16uAddress_p + 1, (INT8U)(i16uData_p >> 8));
}    

//*************************************************************************************************
//| Function: eepromReadDWord
//|
//! \brief
//!
//! \detailed
//!
//!
//!
//! \ingroup
//-------------------------------------------------------------------------------------------------
INT32U BSP_EEPROM_readDWord (
    INT16U i16uAddress_p)
    
{
    INT8U ai8uCmd_l[6];
    INT8U ai8uRecVal_l[6];
    INT8U i8uDataLL_l;
    INT8U i8uDataLH_l;
    INT8U i8uDataHL_l;
    INT8U i8uDataHH_l;
    INT32U i32uRv_l;

    BSP_ASSERT (i16uAddress_p < (sizeof (BSP_EEPROM_TLayout) - 3), BSPE_EE_RD_DWORD_ADR_OUT_OF_RANGE);

    ai8uCmd_l[0] = 0xc0 | ((i16uAddress_p >> 6) & 0x1f);  // READ
    ai8uCmd_l[1] = (i16uAddress_p << 2);  
    ai8uCmd_l[2] = 0;  
    ai8uCmd_l[3] = 0;  
    ai8uCmd_l[4] = 0;  
    ai8uCmd_l[5] = 0;  

#ifdef STM_USE_BSP_OS 
    {
        TBOOL bTimeOut_l;
        
        BSP_OS_getMutex (&BSP_tSpiPeriMutex_g, -1, &bTimeOut_l);
    } 
#endif    

    BSP_SPI_RWPERI_prepareSpi (&BSP_EEPROM_tRwPeriData_s);
    BSP_SPI_RWPERI_chipSelectEnable (&BSP_EEPROM_tRwPeriData_s);
    spi_transceive (EEPROM_SPI_PORT, ai8uCmd_l, ai8uRecVal_l, 6, bTRUE);
    BSP_SPI_RWPERI_chipSelectDisable (&BSP_EEPROM_tRwPeriData_s);

    i8uDataLL_l = (ai8uRecVal_l[1] << 7) |(ai8uRecVal_l[2] >> 1);
    i8uDataLH_l = (ai8uRecVal_l[2] << 7) |(ai8uRecVal_l[3] >> 1);
    i8uDataHL_l = (ai8uRecVal_l[3] << 7) |(ai8uRecVal_l[4] >> 1);
    i8uDataHH_l = (ai8uRecVal_l[4] << 7) |(ai8uRecVal_l[5] >> 1);

#ifdef STM_USE_BSP_OS 
    BSP_OS_releaseMutex (&BSP_tSpiPeriMutex_g);
#endif    

    i32uRv_l = (i8uDataHH_l << 24) | (i8uDataHL_l << 16) | (i8uDataLH_l << 8) | (i8uDataLL_l);

    return (i32uRv_l);
}    

//*************************************************************************************************
//| Function: eepromWriteDWord
//|
//! \brief
//!
//! \detailed
//!
//!
//!
//! \ingroup
//-------------------------------------------------------------------------------------------------
void BSP_EEPROM_writeDWord  (
    INT16U i16uAddress_p, 
    INT32U i32uData_p)
    
{
    BSP_ASSERT (i16uAddress_p < (sizeof (BSP_EEPROM_TLayout) - 3), BSPE_EE_WR_DWORD_ADR_OUT_OF_RANGE);

    BSP_EEPROM_writeByte (i16uAddress_p, (INT8U)i32uData_p);
    BSP_EEPROM_writeByte (i16uAddress_p + 1, (INT8U)(i32uData_p >> 8));
    BSP_EEPROM_writeByte (i16uAddress_p + 2, (INT8U)(i32uData_p >> 16));
    BSP_EEPROM_writeByte (i16uAddress_p + 3, (INT8U)(i32uData_p >> 24));
}    

//*************************************************************************************************
//| Function: BSP_EEPROM_getDefaultData
//|
//! \brief
//!
//! \detailed
//!
//!
//!
//! \ingroup
//-------------------------------------------------------------------------------------------------
INT8U BSP_EEPROM_getDefaultData (
    INT16U i16uAddress_p)
    
{
    INT16U i16uInd_l;
    INT8U *pi8uData_l;

    BSP_ASSERT (i16uAddress_p < sizeof (BSP_EEPROM_TLayout), BSPE_EE_DEF_DATA_ADR_OUT_OF_RANGE);

    for (i16uInd_l = 0; BSP_EEPROM_atDefaultRef[i16uInd_l].vpSrc != ((void *)0); i16uInd_l++)
    {
        const BSP_EEPROM_TDefaultRef *ptRef_l = &BSP_EEPROM_atDefaultRef[i16uInd_l]; 
    
        if (   (i16uAddress_p >= ptRef_l->i16uEepromAddr)
            && (i16uAddress_p < ptRef_l->i16uEepromAddr + ptRef_l->i16uLen)
           ) 
        {
            pi8uData_l = (INT8U *)ptRef_l->vpSrc + i16uAddress_p - ptRef_l->i16uEepromAddr;
            return (*pi8uData_l);
        }   
    }
    
    pi8uData_l = (INT8U *)&BSP_EEPROM_ctDefaultValue + i16uAddress_p;
    return (*pi8uData_l);
}    


//*************************************************************************************************
//| Function: BSP_EEPROM_factoryReset
//|
//! \brief
//!
//! \detailed
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
    
    BSP_EEPROM_eraseAll ();
 
    for (i16uInd_l = 0; i16uInd_l < sizeof (BSP_EEPROM_TLayout); i16uInd_l++)
    {
        i8uData_l = BSP_EEPROM_getDefaultData (i16uInd_l);  
        BSP_EEPROM_writeByte (i16uInd_l, i8uData_l);  
    }
    BSP_EEPROM_writeByte (BSP_EEPROM_ADDR (i8uInitMarker), 'M');
}    

//*************************************************************************************************
//| Function: BSP_EEPROM_eraseAll
//|
//! \brief
//!
//! \detailed
//!
//!
//!
//! \ingroup
//-------------------------------------------------------------------------------------------------
void BSP_EEPROM_eraseAll (
    void)    
{

    INT8U ai8uCmd_l[2];
    INT16U i16uInd_l;
    INT8U i8uMask_l;
    INT32U i32uStartTime_l;

    GPIO_TypeDef *ptSckPort_l = (GPIO_TypeDef *)BSP_EEPROM_tRwPeriData_s.vpSckPort;
    INT16U i16uSckPin_l = BSP_EEPROM_tRwPeriData_s.i16uSckPin;
    GPIO_TypeDef *ptMosiPort_l = (GPIO_TypeDef *)BSP_EEPROM_tRwPeriData_s.vpMosiPort;
    INT16U i16uMosiPin_l = BSP_EEPROM_tRwPeriData_s.i16uMosiPin;
    GPIO_TypeDef *ptMisoPort_l = (GPIO_TypeDef *)BSP_EEPROM_tRwPeriData_s.vpMisoPort;
    INT16U i16uMisoPin_l = BSP_EEPROM_tRwPeriData_s.i16uMisoPin;

#ifdef STM_USE_BSP_OS 
    {
        TBOOL bTimeOut_l;
        
        BSP_OS_getMutex (&BSP_tSpiPeriMutex_g, -1, &bTimeOut_l);
    } 
#endif    

    BSP_SPI_RWPERI_spiDisable (&BSP_EEPROM_tRwPeriData_s);

    ai8uCmd_l[0] = 0x90;   // ERAL
    ai8uCmd_l[1] = 0;  

    BSP_SPI_RWPERI_chipSelectEnable (&BSP_EEPROM_tRwPeriData_s);
    i8uMask_l = 0x80;
    for (i16uInd_l = 0; i16uInd_l < 14; i16uInd_l++)
    {
        if (ai8uCmd_l[i16uInd_l / 8] & i8uMask_l)
        {
            GPIO_SetBits (ptMosiPort_l, i16uMosiPin_l);
        }
        else
        {
            GPIO_ResetBits (ptMosiPort_l, i16uMosiPin_l);
        }
        BSP_Wait (2);
        i8uMask_l >>= 1;
        if (i8uMask_l == 0)
        {
            i8uMask_l = 0x80;
        }
        

        GPIO_SetBits (ptSckPort_l, i16uSckPin_l);
        BSP_Wait (2);
        GPIO_ResetBits (ptSckPort_l, i16uSckPin_l);
        BSP_Wait (2);
    }
    
    BSP_SPI_RWPERI_chipSelectDisable (&BSP_EEPROM_tRwPeriData_s);
    BSP_Wait (2);
    
    i32uStartTime_l = kbGetTickCount ();
    BSP_SPI_RWPERI_chipSelectEnable (&BSP_EEPROM_tRwPeriData_s);
    while (GPIO_ReadInputDataBit (ptMisoPort_l, i16uMisoPin_l) == Bit_RESET)
    {
        if ((kbGetTickCount () - i32uStartTime_l) > 10)
        {       // maximum wait time due to data sheet is 5 ms
            bspError (BSPE_EE_WRITE_READY_TIME_OUT, bTRUE, 0);
        }
    }
    
    BSP_SPI_RWPERI_chipSelectDisable (&BSP_EEPROM_tRwPeriData_s);
    BSP_Wait (2);
    
    BSP_SPI_RWPERI_spiEnable (&BSP_EEPROM_tRwPeriData_s);

#ifdef STM_USE_BSP_OS 
    BSP_OS_releaseMutex (&BSP_tSpiPeriMutex_g);
#endif    

}    

//*************************************************************************************************

