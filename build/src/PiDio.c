/*=============================================================================================
*
*   PiDio.c
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
#include <string.h>

#include <SysLib\inc\stm32f2xx_rcc.h>
#include <SysLib\inc\stm32f2xx_spi.h>

#include <project.h>
#include <PiDio.h>

#include <common_define.h>

#include <bsp\bspConfig.h>
#include <bsp\gpio\gpio.h>
#include <bsp\spi\spi.h>
#include <bsp\led\led.h>

#include <kbUtilities.h>
#include <ModGateRS485.h>
#include <ModGateComError.h>
#include <ModGateComMain.h>
#include <IoProtocol.h>
#include <bsp\systick\systick.h>

static BSP_SPI_TRwPeriData BSP_Output_tRwPeriData_m;
static BSP_SPI_TRwPeriData BSP_Input_tRwPeriData_m;

static INT16U i16uOutputPushPull_s;          // 1=push-pull, 0=high side mode
static INT16U i16uOutputOpenLoadDetect_s;    // 1=detect open load in high side mode

static INT16U i16uOutputStatus_s;            // 0=error on output pin (thermal shutdown, over load, open load in high side mode)
static SDioModuleStatus sDioModuleStatus_s;

static INT8U crcLoop(INT8U i8uCrc_p, INT8U i8uByte_p)
{
    INT8U i8uI_l;
    for (i8uI_l = 0; i8uI_l < 8; i8uI_l++)
    {
        i8uCrc_p <<= 1;
        if (i8uCrc_p & 0x80)
            i8uCrc_p ^= 0xB7; // 0x37 with MSBit on purpose
        if (i8uByte_p & 0x80)
            i8uCrc_p ^= 1;
        i8uByte_p <<= 1;
    }
    return i8uCrc_p;
}
static INT8U crcEncode16(INT8U i8uByte1_p, INT8U i8uByte2_p)
{
    INT8U i8uSynd_l;
    i8uSynd_l = crcLoop(0x7f, i8uByte1_p);
    i8uSynd_l = crcLoop(i8uSynd_l, i8uByte2_p);
    return crcLoop(i8uSynd_l, 0x80) | 0x80;
}

static INT8U  crcEncode8(INT8U i8uByte_p)
{
    INT8U i8uI_l;
    INT8U i8uCrc_l = i8uByte_p;
    INT8U i8uPolynom_p = 0xD4; // x^5 + x^4 + x^2 + 1, shifted left to set MSB

    for (i8uI_l = 0; i8uI_l < 8; i8uI_l++)
    {
        if (i8uCrc_l & 0x80)
        {
            i8uCrc_l = i8uCrc_l ^ i8uPolynom_p;
        }
        i8uCrc_l <<= 1;
    }
    i8uCrc_l >>= 3;

    return i8uCrc_l;
}


void PiDioInitSpi()
{
    HW_SPI_CONFIGURATION tOutputHwConf_l =
    {
        .mode = HW_SPI_MODE_MASTER,
        .polarity = HW_SPI_CLOCK_POL_LOW,
        .phase = HW_SPI_CLOCK_PHASE_LEAD,
        .direction = HW_SPI_DATA_DIR_MSB,
        .nss = 0,    		// is set by software
        .bitrate = 1000000,		// 1 MHz
        .receive_cb = 0,
        .transmit_cb = 0,
        .error_cb = 0,
        .finish_cb = 0,
    };
    HW_SPI_CONFIGURATION tInputHwConf_l =
    {
        .mode = HW_SPI_MODE_MASTER,
        .polarity = HW_SPI_CLOCK_POL_LOW,
        .phase = HW_SPI_CLOCK_PHASE_LEAD,
        .direction = HW_SPI_DATA_DIR_MSB,
        .nss = 0,    		// is set by software
        .bitrate = 1000000,		// 1 MHz
        .receive_cb = 0,
        .transmit_cb = 0,
        .error_cb = 0,
        .finish_cb = 0,
    };

    BSP_Output_tRwPeriData_m.vpCsPort = PIIO_SPIOUTP_CS_PORT;
    BSP_Output_tRwPeriData_m.i16uCsPin = PIIO_SPIOUTP_CS_PIN;
    BSP_Output_tRwPeriData_m.bActiveHigh = 0;
    BSP_SPI_RWPERI_init(PIIO_SPIOUTP_PORT, &tOutputHwConf_l, &BSP_Output_tRwPeriData_m);

    BSP_Input_tRwPeriData_m.vpCsPort = PIIO_SPIINP_CS_PORT;
    BSP_Input_tRwPeriData_m.i16uCsPin = PIIO_SPIINP_CS_PIN;
    BSP_Input_tRwPeriData_m.bActiveHigh = 0;
    BSP_SPI_RWPERI_init(PIIO_SPIINP_PORT, &tInputHwConf_l, &BSP_Input_tRwPeriData_m);
}

void PiDioInitGpios()
{
    GPIO_InitTypeDef GPIO_InitStructure;

    // set gpio clocks
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC, ENABLE);

    // CONFIGoutp
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Pin = PIIO_CFGOUTP_PIN;
    GPIO_Init(PIIO_CFGOUTP_PORT, &GPIO_InitStructure);
    // Fault Output
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Pin = PIIO_FAULTOUT_PIN;
    GPIO_Init(PIIO_FAULTOUT_PORT, &GPIO_InitStructure);
    // Fault Input
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Pin = PIIO_FAULTIN_PIN;
    GPIO_Init(PIIO_FAULTIN_PORT, &GPIO_InitStructure);
    // InputFilter1
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Pin = PIIO_INPFLTR1_PIN;
    GPIO_Init(PIIO_INPFLTR1_PORT, &GPIO_InitStructure);
    // InputFilter2
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Pin = PIIO_INPFLTR2_PIN;
    GPIO_Init(PIIO_INPFLTR2_PORT, &GPIO_InitStructure);
    // RS485 Termination
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Pin = PIIO_RS485TERM_PIN;
    GPIO_Init(PIIO_RS485TERM_PORT, &GPIO_InitStructure);
    // Varianten
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Pin = PIIO_VARIANT_H_PIN;
    GPIO_Init(PIIO_VARIANT_H_PORT, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = PIIO_VARIANT_L_PIN;
    GPIO_Init(PIIO_VARIANT_L_PORT, &GPIO_InitStructure);

    // configOutp PC06 --> low --> high-side-mode
    GPIO_ResetBits(PIIO_CFGOUTP_PORT, PIIO_CFGOUTP_PIN);
}


PIDIO_ERROR PiDioSetConfig(SDioConfig *pDioConfig_p)
{
    PIDIO_ERROR eError_l = PIDIO_ERROR_NO_ERROR;
    INT8U send[6], recv[6];
    INT8U i8uHighCrc_l;
    INT8U i8uLowCrc_l;

    if (MODGATE_OwnID_g.i16uModulType == KUNBUS_FW_DESCR_TYP_PI_DIO_14
        || MODGATE_OwnID_g.i16uModulType == KUNBUS_FW_DESCR_TYP_PI_DI_16
        )
    {
        // set input debounce
        switch (pDioConfig_p->i8uInputDebounce)
        {
        case 3:
            GPIO_SetBits(PIIO_INPFLTR1_PORT, PIIO_INPFLTR1_PIN);
            GPIO_SetBits(PIIO_INPFLTR2_PORT, PIIO_INPFLTR2_PIN);
            break;
        case 2:
            GPIO_ResetBits(PIIO_INPFLTR1_PORT, PIIO_INPFLTR1_PIN);
            GPIO_SetBits(PIIO_INPFLTR2_PORT, PIIO_INPFLTR2_PIN);
            break;
        case 1:
            GPIO_SetBits(PIIO_INPFLTR1_PORT, PIIO_INPFLTR1_PIN);
            GPIO_ResetBits(PIIO_INPFLTR2_PORT, PIIO_INPFLTR2_PIN);
            break;
        case 0:
        default:
            GPIO_ResetBits(PIIO_INPFLTR1_PORT, PIIO_INPFLTR1_PIN);
            GPIO_ResetBits(PIIO_INPFLTR2_PORT, PIIO_INPFLTR2_PIN);
            break;
        }
    }

    if (MODGATE_OwnID_g.i16uModulType == KUNBUS_FW_DESCR_TYP_PI_DIO_14
        || MODGATE_OwnID_g.i16uModulType == KUNBUS_FW_DESCR_TYP_PI_DO_16
        )
    {
        i16uOutputPushPull_s = pDioConfig_p->i16uOutputPushPull;
        i16uOutputOpenLoadDetect_s = pDioConfig_p->i16uOutputOpenLoadDetect;

        // go to 16 bit config mode
        GPIO_SetBits(PIIO_CFGOUTP_PORT, PIIO_CFGOUTP_PIN);

        memset(send, 0, sizeof(send));
        memset(recv, 0, sizeof(recv));

        if (MODGATE_OwnID_g.i16uModulType == KUNBUS_FW_DESCR_TYP_PI_DIO_14)
        {
            send[0] = (i16uOutputOpenLoadDetect_s >> 8) & 0x3f;
            send[1] = (i16uOutputPushPull_s >> 8) & 0x3f;
        }
        else
        {
            send[0] = i16uOutputOpenLoadDetect_s >> 8;
            send[1] = i16uOutputPushPull_s >> 8;
        }
        send[2] = crcEncode16(send[0], send[1]);

        send[3] = i16uOutputOpenLoadDetect_s & 0xff;
        send[4] = i16uOutputPushPull_s & 0xff;
        send[5] = crcEncode16(send[3], send[4]);

        BSP_SPI_RWPERI_chipSelectEnable(&BSP_Output_tRwPeriData_m);
        spi_transceive(PIIO_SPIOUTP_PORT, send, recv, 6, bTRUE);
        BSP_SPI_RWPERI_chipSelectDisable(&BSP_Output_tRwPeriData_m);

        BSP_SPI_RWPERI_chipSelectEnable(&BSP_Output_tRwPeriData_m);
        spi_transceive(PIIO_SPIOUTP_PORT, send, recv, 6, bTRUE);
        BSP_SPI_RWPERI_chipSelectDisable(&BSP_Output_tRwPeriData_m);

        i8uHighCrc_l = crcEncode16(recv[0], recv[1]) & 0x7f;
        if (recv[2] != i8uHighCrc_l)
            eError_l = PIDIO_ERROR_CRC;
        i8uLowCrc_l = crcEncode16(recv[3], recv[4]) & 0x7f;
        if (recv[5] != i8uLowCrc_l)
            eError_l = PIDIO_ERROR_CRC;

        sDioModuleStatus_s.bitOutputCommErr = (eError_l == PIDIO_ERROR_CRC) ? 1 : 0;

        // go to 8 bit config and IO mode
        GPIO_ResetBits(PIIO_CFGOUTP_PORT, PIIO_CFGOUTP_PIN);
    }

    return eError_l;

}


PIDIO_ERROR PiDioReadInput(INT16U *pi16uInputValue_p)
{
    PIDIO_ERROR eError_l = PIDIO_ERROR_NO_ERROR;
    INT8U send[4], recv[4];
    INT8U i8uHighCrc_l;
    INT8U i8uLowCrc_l;

    memset(send, 0, sizeof(send));
    memset(recv, 0, sizeof(recv));

    BSP_SPI_RWPERI_chipSelectEnable(&BSP_Input_tRwPeriData_m);
    spi_transceive(PIIO_SPIINP_PORT, send, recv, 4, bTRUE);
    BSP_SPI_RWPERI_chipSelectDisable(&BSP_Input_tRwPeriData_m);

    i8uHighCrc_l = (recv[1] & 0xf8) >> 3;
    i8uLowCrc_l  = (recv[3] & 0xf8) >> 3;
    if (i8uHighCrc_l != crcEncode8(recv[0]))
        eError_l = PIDIO_ERROR_CRC;
    if (i8uLowCrc_l != crcEncode8(recv[2]))
        eError_l = PIDIO_ERROR_CRC;
    sDioModuleStatus_s.bitInputCommErr = (eError_l == PIDIO_ERROR_CRC) ? 1 : 0;

    sDioModuleStatus_s.bitInputUVL1 = (recv[1] & 0x04) ? 0 : 1;             // under voltage 1 on channel 0-7
    sDioModuleStatus_s.bitInputUVL2 = (recv[1] & 0x01) ? 0 : 1;             // under voltage 2 on channel 0-7
    sDioModuleStatus_s.bitInputOTL  = (recv[1] & 0x02) ? 1 : 0;             // over temperature on channel 0-7
    sDioModuleStatus_s.bitInputUVH1 = (recv[3] & 0x04) ? 0 : 1;             // under voltage 1 on channel 8-15
    sDioModuleStatus_s.bitInputUVH2 = (recv[3] & 0x01) ? 0 : 1;             // under voltage 2 on channel 8-15
    sDioModuleStatus_s.bitInputOTh  = (recv[3] & 0x02) ? 1 : 0;             // over temperature on channel 8-15
    sDioModuleStatus_s.bitInputFault = GPIO_ReadInputDataBit(PIIO_FAULTIN_PORT, PIIO_FAULTIN_PIN) ? 0 : 1;

    *pi16uInputValue_p = (recv[0] << 8) | recv[2];

    return eError_l;
}

PIDIO_ERROR PiDioWriteOutput(INT16U i16uOutputValue_p)
{
    PIDIO_ERROR eError_l = PIDIO_ERROR_NO_ERROR;
    INT8U send[6], recv[6];
    INT8U i8uHighCrc_l;
    INT8U i8uLowCrc_l;

    memset(send, 0, sizeof(send));
    memset(recv, 0, sizeof(recv));

    sDioModuleStatus_s.bitOutputFault = GPIO_ReadInputDataBit(PIIO_FAULTOUT_PORT, PIIO_FAULTOUT_PIN) ? 0 : 1;
    if (sDioModuleStatus_s.bitOutputFault)
    {
        // set all outputs to save state
        i16uOutputValue_p = 0;
    }

    if (MODGATE_OwnID_g.i16uModulType == KUNBUS_FW_DESCR_TYP_PI_DIO_14)
    {
        send[0] = i16uOutputValue_p >> 8;
        send[1] = (i16uOutputPushPull_s & 0x3fff) >> 8;
        send[2] = crcEncode16(send[0], send[1]);
    }
    else
    {
        send[0] = i16uOutputValue_p >> 8;
        send[1] = i16uOutputPushPull_s >> 8;
        send[2] = crcEncode16(send[0], send[1]);
    }

    send[3] = i16uOutputValue_p;
    send[4] = i16uOutputPushPull_s;
    send[5] = crcEncode16(send[3], send[4]);

    //BSP_SPI_RWPERI_chipSelectEnable(&BSP_Output_tRwPeriData_m);
    //spi_transceive(PIIO_SPIOUTP_PORT, send, recv, 6, bTRUE);
    //BSP_SPI_RWPERI_chipSelectDisable(&BSP_Output_tRwPeriData_m);

    BSP_SPI_RWPERI_chipSelectEnable(&BSP_Output_tRwPeriData_m);
    spi_transceive(PIIO_SPIOUTP_PORT, send, recv, 6, bTRUE);
    BSP_SPI_RWPERI_chipSelectDisable(&BSP_Output_tRwPeriData_m);

    i8uHighCrc_l = crcEncode16(recv[0], recv[1]) & 0x7f;
    if ((recv[2] & 0x7f) != i8uHighCrc_l)
        eError_l = PIDIO_ERROR_CRC;
    i8uLowCrc_l = crcEncode16(recv[3], recv[4]) & 0x7f;
    if ((recv[5] & 0x7f) != i8uLowCrc_l)
        eError_l = PIDIO_ERROR_CRC;

    i16uOutputStatus_s = (recv[0] << 8) | recv[3];  // fault info of output pins
    // recv[1] and recv[4] contains the current output values, they are not used yet

    sDioModuleStatus_s.bitOutputCommErr = (eError_l == PIDIO_ERROR_CRC) ? 1 : 0;
    sDioModuleStatus_s.bitOutputCRCErr = ((recv[2] & 0x80) || (recv[5] & 0x80)) ? 1 : 0;

    return eError_l;
}


PIDIO_ERROR PiDioGetOutputStatus(INT16U *pi16uOutputStatus_p)
{
    *pi16uOutputStatus_p = i16uOutputStatus_s;
    return PIDIO_ERROR_NO_ERROR;
}

PIDIO_ERROR PiDioGetModuleStatus(SDioModuleStatus *psDioModuleStatus_p)
{
    *psDioModuleStatus_p = sDioModuleStatus_s;
    return PIDIO_ERROR_NO_ERROR;
}

void PiDioSetStateLed()
{

}