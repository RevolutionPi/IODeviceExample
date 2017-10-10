/*=============================================================================================
*
*   PiSlaveApplication.h
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

#include <project.h>
#include <common_define.h>

#include <bsp/gpio/gpio.h>

#include <ModGateRS485.h>
#include <ModGateComMain.h>
#include <application.h>
#include <PiSlaveApplication.h>
#include <PiDio.h>
#include <bsp/timer/timer.h>

MODGATECOM_IDResp MODGATE_OwnID_g;     //!< ID-Data of this mGate
TBOOL bIoTimerActive_g = bFALSE;

// current values
static INT16U i16uOutput_g;                     // 0=low signal, 1=high signal
static INT8U  ai8uOutputPwmValue_s[16];         // values of pwm output
static INT16U i16uOutputStatus_s;               // 0=error on output pin (thermal shutdown, over load, open load in high side mode)
static SDioModuleStatus sDioModuleStatus_s;

static INT16U i16uInput_s;                   // 0=low signal, 1=high signal
static INT16U i16uOldInput_s = 0;
static INT32S ai32sEncoderValue_s[16];         // Encoder value for every channel
static INT32U ai32uCounterValue_s[16];         // Counter value for every channel

// configuration
static INT8U i8uPwmIncrement_s = 1;
static INT16U i16uOutputPwm_s = 0x0000;        // bitfield: 1=output is configured as PWM

static INT8U  ai8uInputMode_s[16];              // Mode of input: 0=direct, 1=counter, rising edge, 2=counter, falling edge, 3=encoder
static INT8U  i8uNumberOfCounters_s;              // number of input configurad as counter or encoder

static TBOOL    bHandleIO_s;

INT32U i32uInputError_m = 0;
INT32U i32uOutputError_m = 0;



void handleInput(void)
{
    INT16U i16uChannel_l;
    INT16U i16uRisingEdges_l;
    INT16U i16uFallingEdges_l;
    static INT8S ai8sEncoderTable_l[16] = {0,0,-1,0,0,0,0,1,1,0,0,0,0,-1,0,0};
    INT8U i8uEncoderIndex_l;
    INT16U i16uBitPos_l;

    if (PiDioReadInput(&i16uInput_s) != PIDIO_ERROR_NO_ERROR)
    {
        i32uInputError_m++;
        return;
    }

    // Mark rising edges
    i16uRisingEdges_l  = (i16uOldInput_s ^ i16uInput_s) & i16uInput_s;
    i16uFallingEdges_l = (i16uOldInput_s ^ i16uInput_s) & ~i16uInput_s;

    for (i16uChannel_l = 0; i16uChannel_l < 16; i16uChannel_l++)
    {
        i16uBitPos_l = 1 << i16uChannel_l;

        // On rising edge increase Counter for this channel
        if (ai8uInputMode_s[i16uChannel_l] == 1 && (i16uRisingEdges_l & i16uBitPos_l))
        {
            ai32uCounterValue_s[i16uChannel_l]++;
        }

        // On falling edge increase Counter for this channel
        if (ai8uInputMode_s[i16uChannel_l] == 2 && (i16uFallingEdges_l & i16uBitPos_l))
        {
            ai32uCounterValue_s[i16uChannel_l]++;
        }

        // Encoder
        if (ai8uInputMode_s[i16uChannel_l] == 3 && (i16uChannel_l % 2) == 0)    // for even channels only
        {
            // Index = INPOLD & 0x03 << 2
            i8uEncoderIndex_l = ((i16uOldInput_s & i16uBitPos_l) ? 4 : 0)           // old value from 1. channel (0, 2, 4 ...)
                              | ((i16uOldInput_s & (i16uBitPos_l << 1)) ? 8 : 0);   // old value from 2. channel (1, 3, 5 ...)
            // Index |= INPNEW & 0x03
            i8uEncoderIndex_l |= ((i16uInput_s & i16uBitPos_l) ? 1 : 0)             // new value from 1. channel (0, 2, 4 ...)
                              |  ((i16uInput_s & (i16uBitPos_l << 1)) ? 2 : 0);     // new value from 2. channel (1, 3, 5 ...)
            // Get increment/decrement from table
            ai32sEncoderValue_s[i16uChannel_l] += ai8sEncoderTable_l[i8uEncoderIndex_l];
        }
    }

    i16uOldInput_s = i16uInput_s;
}

void handleOutput(void)
{
    static INT8U i8uPwmCounter_s[16];
    INT16U i16uChannel_l;
    INT16U i16uBitPos_l;

    for (i16uChannel_l = 0; i16uChannel_l < 16; i16uChannel_l++)
    {
        i16uBitPos_l = 1 << i16uChannel_l;

        // PWM
        if (i16uOutputPwm_s & i16uBitPos_l)
        {
            i8uPwmCounter_s[i16uChannel_l] += i8uPwmIncrement_s;
            i8uPwmCounter_s[i16uChannel_l] %= 100;
            if (i8uPwmCounter_s[i16uChannel_l] < ai8uOutputPwmValue_s[i16uChannel_l])
                i16uOutput_g |= i16uBitPos_l;
            else
                i16uOutput_g &= ~i16uBitPos_l;
        }
    }

    if (PiDioWriteOutput(i16uOutput_g) != PIDIO_ERROR_NO_ERROR)
    {
        i32uOutputError_m++;
    }
}

void CbIoTimerExpired(void)
{
    TIM_CountDownReTrigger(IO_TIMER, IO_TIMER_TIME_CYCLE);
    bHandleIO_s = bTRUE;
}

void handleIO(void)
{
    if (MODGATE_OwnID_g.i16uModulType == KUNBUS_FW_DESCR_TYP_PI_DIO_14
        || MODGATE_OwnID_g.i16uModulType == KUNBUS_FW_DESCR_TYP_PI_DI_16
        )
    {
        handleInput();
        if (sDioModuleStatus_s.bitInputCommErr
            || sDioModuleStatus_s.bitInputFault
            )
        {
            LED_setLed(PIDIO_LED_INPUT_STATUS, LED_ST_RED_ON);
        }
        else if (sDioModuleStatus_s.bitInputUVL1
            || sDioModuleStatus_s.bitInputOTL
            || sDioModuleStatus_s.bitInputUVH1
            || sDioModuleStatus_s.bitInputOTh
            )
        {
            LED_setLed(PIDIO_LED_INPUT_STATUS, LED_ST_RED_BLINK_500);
        }
        else
        {
            LED_setLed(PIDIO_LED_INPUT_STATUS, LED_ST_GREEN_ON);
        }
        PiDioGetModuleStatus(&sDioModuleStatus_s);
    }

    if (MODGATE_OwnID_g.i16uModulType == KUNBUS_FW_DESCR_TYP_PI_DIO_14
        || MODGATE_OwnID_g.i16uModulType == KUNBUS_FW_DESCR_TYP_PI_DO_16
        )
    {
        handleOutput();
        PiDioGetOutputStatus(&i16uOutputStatus_s);

        if (sDioModuleStatus_s.bitOutputCommErr
            || sDioModuleStatus_s.bitOutputCRCErr
            || i16uOutputStatus_s
            || sDioModuleStatus_s.bitOutputFault
            )
        {
            LED_setLed(PIDIO_LED_OUTPUT_STATUS, LED_ST_RED_ON);
        }
        else
        {
            LED_setLed(PIDIO_LED_OUTPUT_STATUS, LED_ST_GREEN_ON);
        }
    }
}

// Get KUNBUS FW-Description type
INT16U PiSlaveAppGetModuleType()
{
    INT8U i8uVariant_l;
    INT16U i16uModulType_l = 0;

    i8uVariant_l = GPIO_ReadInputDataBit(PIIO_VARIANT_H_PORT, PIIO_VARIANT_H_PIN);
    i8uVariant_l <<= 1;
    i8uVariant_l |= GPIO_ReadInputDataBit(PIIO_VARIANT_L_PORT, PIIO_VARIANT_L_PIN);

    switch (i8uVariant_l)
    {
    case 0: // 14 Inputs, 14 Outputs
        i16uModulType_l = KUNBUS_FW_DESCR_TYP_PI_DIO_14;
        break;
    case 1:  // 16 Outputs
        i16uModulType_l = KUNBUS_FW_DESCR_TYP_PI_DO_16;
        break;
    case 2:  // 16 Inputs
        i16uModulType_l = KUNBUS_FW_DESCR_TYP_PI_DI_16;
        break;
    }
    return i16uModulType_l;
}

extern TBOOL bResponseState_g;
TBOOL bLastResponseState_g;

void PiSlaveAppSetStateLed(PISLAVEAPPLICATION_STATE eState_p)
{
    static PISLAVEAPPLICATION_STATE eState_l = PISLAVEAPPLICATION_STATE_UNKNOWN_ERROR;

    if (eState_l != eState_p)
    {
        switch (eState_p)
        {
        case PISLAVEAPPLICATION_STATE_INIT:
            LED_setLed(PIDIO_LED_POWER, LED_ST_RED_BLINK_500);
            LED_setLed(PIDIO_LED_OUTPUT_STATUS, LED_ST_GREEN_OFF);
            LED_setLed(PIDIO_LED_INPUT_STATUS, LED_ST_GREEN_OFF);
            break;

        case PISLAVEAPPLICATION_STATE_RUNNING:
            LED_setLed(PIDIO_LED_POWER, LED_ST_RED_OFF);
            if (MODGATE_OwnID_g.i16uModulType == KUNBUS_FW_DESCR_TYP_PI_DIO_14
                || MODGATE_OwnID_g.i16uModulType == KUNBUS_FW_DESCR_TYP_PI_DO_16
                )
            {
                LED_setLed(PIDIO_LED_OUTPUT_STATUS, LED_ST_GREEN_ON);
            }
            if (MODGATE_OwnID_g.i16uModulType == KUNBUS_FW_DESCR_TYP_PI_DIO_14
                || MODGATE_OwnID_g.i16uModulType == KUNBUS_FW_DESCR_TYP_PI_DI_16
                )
            {
                LED_setLed(PIDIO_LED_INPUT_STATUS, LED_ST_GREEN_ON);
            }
            break;

        case PISLAVEAPPLICATION_STATE_TIMEOUT:
        case PISLAVEAPPLICATION_STATE_UNKNOWN_ERROR:
        default:
            LED_setLed(PIDIO_LED_POWER, LED_ST_RED_ON);
        }
        eState_l = eState_p;
    }
}

void PiSlaveAppInit(BSP_TJumpBuf *ptExceptionPoint_p, void(*cbErrHandler_p)(INT32U i32uErrorCode_p,
    TBOOL bFatal_p, INT8U i8uParaCnt_p, va_list argptr_p))
{
    TIM_TCountDownTimerInit tConfTimer_l;
    SDioConfig DioConfig_l;

    PiDioInitGpios();

    // Initiate ID struct
    MODGATE_OwnID_g.i32uSerialnumber = ctKunbusFirmwareDescription_g.i32uSerialNumber;
    MODGATE_OwnID_g.i16uModulType = PiSlaveAppGetModuleType();
    MODGATE_OwnID_g.i16uHW_Revision = ctKunbusFirmwareDescription_g.i16uHwRevision;
    MODGATE_OwnID_g.i16uSW_Major = ctKunbusApplDescription_g.i8uSwMajor;
    MODGATE_OwnID_g.i16uSW_Minor = ctKunbusApplDescription_g.i16uSwMinor;
    MODGATE_OwnID_g.i32uSVN_Revision = ctKunbusApplDescription_g.i32uSvnRevision;
    MODGATE_OwnID_g.i16uFBS_InputLength = sizeof(INT16U)    // i16uInput
        + sizeof(INT16U)    // i16uOutputStatus
        + sizeof(SDioModuleStatus)
        + sizeof(INT32U) * 16; // 16 Counter inputs
    MODGATE_OwnID_g.i16uFBS_OutputLength = sizeof(INT16U) + 16*sizeof(INT8U);
    MODGATE_OwnID_g.i16uFeatureDescriptor = MODGATE_feature_RS485DataExchange;

    PiDioInitSpi();

    // set default configuration
    DioConfig_l.i16uOutputPushPull = 0;          // bitfield: 1=push-pull, 0=high side mode
    DioConfig_l.i16uOutputOpenLoadDetect = 0;    // bitfield: 1=detect open load in high side mode
    DioConfig_l.i16uOutputPWM = 0;               // bitfield: 1=generate pwm signal
    DioConfig_l.i8uOutputPWMIncrement = 1;       // [1-10] increment for pwm algorithm

    DioConfig_l.i8uInputDebounce = 0;            // 0=Off, 1=25us, 2=750us, 3=3ms, 4-255 not allowed
    DioConfig_l.i32uInputMode = 0;               // bitfield, 2 bits per channel: 00=direct, 01=counter, rising edge, 10=counter, falling edge, 11=encoder

    PiDioSetConfig(&DioConfig_l);
    PiDioWriteOutput(0);                        // set all outputs to 0

    APPL_set_allLED(0);     // turn all LEDs off

    // Init IO-Timer
    tConfTimer_l.cbTimerExpired = CbIoTimerExpired;
    tConfTimer_l.i32uTimeBase = IO_TIMER_TIME_BASE;
    TIM_initCountDownTimer(IO_TIMER, &tConfTimer_l);
}

void PiSlaveAppStart(void)
{
    if (!bIoTimerActive_g)
    {
        TIM_CountDownReTrigger(IO_TIMER, IO_TIMER_TIME_CYCLE);
        bIoTimerActive_g = bTRUE;
    }
}

void PiSlaveAppStop(void)
{
    bIoTimerActive_g = bFALSE;
    TIM_StopTimer(IO_TIMER);
}

// Set/unset termination resistor for RS485 line
void PiSlaveAppRS485Terminate(TBOOL bTerminate_p)
{
    if(bTerminate_p)
        GPIO_SetBits(PIIO_RS485TERM_PORT, PIIO_RS485TERM_PIN);
    else
        GPIO_ResetBits(PIIO_RS485TERM_PORT, PIIO_RS485TERM_PIN);
}

void PiSlaveAppRun()
{
    PiSlaveAppStart();

    if (bHandleIO_s)
    {
        bHandleIO_s = bFALSE;
        handleIO();
    }
}


void PiSlaveAppIOReq(SIOGeneric *pReq, SIOGeneric *pResp)
{
    // set error response as default
    pResp->uHeader.sHeaderTyp1.bitAddress = i8uOwnAddress_g;
    pResp->uHeader.sHeaderTyp1.bitCommand = 7;
    pResp->uHeader.sHeaderTyp1.bitIoHeaderType = 0;
    pResp->uHeader.sHeaderTyp1.bitReqResp = 1;
    pResp->uHeader.sHeaderTyp1.bitLength = 0;

    if (pReq->uHeader.sHeaderTyp1.bitIoHeaderType == 0)
    {
        if (   pReq->uHeader.sHeaderTyp1.bitCommand == IOP_TYP1_CMD_DATA
            || pReq->uHeader.sHeaderTyp1.bitCommand == IOP_TYP1_CMD_DATA2
            || pReq->uHeader.sHeaderTyp1.bitCommand == IOP_TYP1_CMD_DATA3)
        {
            if (pReq->uHeader.sHeaderTyp1.bitCommand == IOP_TYP1_CMD_DATA)
            {
                SDioRequest *pDioReq = (SDioRequest *)pReq;

                i16uOutput_g = pDioReq->i16uOutput;
            }
            else if (pReq->uHeader.sHeaderTyp1.bitCommand == IOP_TYP1_CMD_DATA2)
            {
                SDioPWMOutput *pDioReq = (SDioPWMOutput *)pReq;
                int i, j;

                i16uOutput_g = pDioReq->i16uOutput;

                j = 0;
                for (i = 0; i < 16; i++)
                {
                    if (pDioReq->i16uChannels & (1 << i))
                    {
                        if (i16uOutputPwm_s & (1 << i))
                        {
                            ai8uOutputPwmValue_s[i] = pDioReq->ai8uValue[j++];
                        }
                    }
                }
            }
            else if (pReq->uHeader.sHeaderTyp1.bitCommand == IOP_TYP1_CMD_DATA3)
            {
                SDioCounterReset *pDioReq = (SDioCounterReset *)pReq;
                int i;

                for (i = 0; i < 16; i++)
                {
                    if (pDioReq->i16uChannels & (1 << i))
                    {
                        if (ai8uInputMode_s[i] == 1 || ai8uInputMode_s[i] == 2)
                        {
                            ai32uCounterValue_s[i] = 0;
                        }
                        else if (ai8uInputMode_s[i] == 3)
                        {
                            ai32sEncoderValue_s[i] = 0;
                        }
                    }
                }
            }

            if (i8uNumberOfCounters_s == 0) // no counters
            {
                SDioResponse *pDioResp = (SDioResponse *)pResp;
                pDioResp->uHeader.sHeaderTyp1.bitCommand = IOP_TYP1_CMD_DATA;
                pDioResp->uHeader.sHeaderTyp1.bitLength = 2 * sizeof(INT16U) + sizeof(SDioModuleStatus);
                pDioResp->i16uInput = i16uInput_s;
                pDioResp->i16uOutputStatus = i16uOutputStatus_s;
                pDioResp->sDioModuleStatus = sDioModuleStatus_s;
            }
            else // add counter values
            {
                SDioCounterResponse *pDioResp = (SDioCounterResponse *)pResp;
                int i, j;

                pDioResp->uHeader.sHeaderTyp1.bitCommand = IOP_TYP1_CMD_DATA2;
                pDioResp->uHeader.sHeaderTyp1.bitLength = 2 * sizeof(INT16U) + sizeof(SDioModuleStatus) + i8uNumberOfCounters_s * sizeof(INT32U);
                pDioResp->i16uInput = i16uInput_s;
                pDioResp->i16uOutputStatus = i16uOutputStatus_s;
                pDioResp->sDioModuleStatus = sDioModuleStatus_s;
                
                // es können max. 6 Counterwerte übertragen werden, da das Telegramm max. 31 Byte Nutzdaten haben darf
                for (i = 0, j = 0; i < 16 && j < 6; i++)
                {
                    if (ai8uInputMode_s[i] == 1 || ai8uInputMode_s[i] == 2)
                    {
                        pDioResp->ai32uCounters[j++] = ai32uCounterValue_s[i];
                    }
                    else if (ai8uInputMode_s[i] == 3)
                    {
                        pDioResp->ai32uCounters[j++] = ai32sEncoderValue_s[i];
                    }
                }
            }
        }
        else if (pReq->uHeader.sHeaderTyp1.bitCommand == IOP_TYP1_CMD_CFG)
        {
            int i;
            SDioConfig *pCfgReq = (SDioConfig *)pReq;

            PiDioSetConfig(pCfgReq); // configure hardware

            i8uNumberOfCounters_s = 0;
            for (i = 0; i < 16; i++)
            {
                ai8uOutputPwmValue_s[i] = 0;
                ai32sEncoderValue_s[i] = 0;
                ai32uCounterValue_s[i] = 0;
                ai8uInputMode_s[i] = (pCfgReq->i32uInputMode >> (2 * i)) & 0x03;
                
                // inputs mit ungeradem index dürfen kein encoder sein
                if ((i % 2) == 1 && ai8uInputMode_s[i] == 3)
                    ai8uInputMode_s[i] = 0;

                if (ai8uInputMode_s[i] > 0)
                    i8uNumberOfCounters_s++;
            }

            i16uOutput_g = 0;
            i16uOutputPwm_s = pCfgReq->i16uOutputPWM;
            i8uPwmIncrement_s = pCfgReq->i8uOutputPWMIncrement;


            pResp->uHeader.sHeaderTyp1.bitCommand = IOP_TYP1_CMD_CFG;
            pResp->uHeader.sHeaderTyp1.bitLength = 0;
        }
    }
}

void PiSlaveAppHandleTimeout(void)
{
    i16uOutput_g = 0;  // set outputs to save state
}

