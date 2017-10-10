/*=============================================================================================
*
* ModGateRS485.c
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

#include <stddef.h>
#include <project.h>
#include <common_define.h>
#include <kbAlloc.h>
#include <bsp/led/Led.h>
#include <bsp/uart/uart.h>
#include <bsp/timer/timer.h>
#if defined STM_WITH_EEPROM
#include <bsp/eeprom/eeprom.h>
#endif
#include <bsp/bspError.h>
#include <bsp/mGateDecode/mGateDecode.h>
#include <ModGateComMain.h>
#include <ModGateComError.h>
#include <ModGateRS485.h>
#include <IoProtocol.h>

#ifdef __KUNBUSPI_KERNEL__
#include <linux/kernel.h>
#include <linux/string.h>
#elif defined(__KUNBUSPI__)
#include <string.h>
#include <PiSlaveApplication.h>
#elif defined(__STM32GENERIC__) || defined (__TIAM335XGENERIC__)
#include <string.h>
#include <application.h>
#endif

#if defined(PIAPI_USE_BRIDGE) && (PIAPI_USE_BRIDGE == 1)
    #include <PiApi/PIAPI_bridge.h>
#endif
#ifdef NEED_RS485_RAPI_MESSAGE
    #include <PiApi/PIAPI_remote.h>
#endif

#ifdef NEED_RS485_SCRIPT_MESSAGE
#include <scDbgIF.h>
#endif
#include <kbUtilities.h>
#include <PiSlaveApplication.h>

static TBOOL IOProtocolCheckFrame(SIOGeneric *sFrame_p);
static INT8U Crc8(INT8U *pi8uFrame_p, INT16U i16uLen_p);


#define STATE_TIMER_TIME_BASE 100   // (100 us) State timer time base in us 
#define WAIT_SILENCE_TIME      40   // (4 ms) silence wait time
#define END_FRAME_TIME         20   // (2 ms) end of frame detection wait time
#define WAIT_RESPONSE_TIME  50000   // (5 s) wait time for answer
#define IOPROTOCOL_TIMEOUT   2000   // (100 ms) timeout to detect failed io communication, e.g. stopped driver on piCore
#ifdef PI_IO_PROTOCOL
static INT32U i32uLastPacketTick_s;
#endif

#define MAX_TELEGRAM_HEAP_MEM 20

#define MAX_TELEG_BUFFER_LIST 10

typedef enum
{
    eAdrPc    = 0,
    eAdrLeft  = 1,
    eAdrRight = 2,
} ERs485Adress;

typedef enum
{
    eRs485StateIdle            = 1,  // Wait for first received character of frame or send request
    eRs485StateWaitSilence     = 2,  // Wait until 4 ms silence was detected
    eRs485StateReceive         = 3,  // Wait for the rest of the frame
    eRs485StateSend            = 4,  // Sending of telegram -> discard all incomming traffic
    eRs485StateWaitResponse    = 5,  // Wait for response after telegram is sent
    eRs485StateSendResponse    = 6,  // Received frame complete
    eRs485StateResponseAnalyze = 7,  // Answer received
    eRs485StateResponseDiscard = 8,  // No answer
    eRs485StateReceiveResponse = 9,  // Wait for the response frame
} ERs485State;

typedef struct
{
    SRs485Telegram *aptBuffer[MAX_TELEG_BUFFER_LIST]; // ring buffer
    INT8U i16uHead;                                        // next free index for insert
    INT8U i16uTail;                                        // next index to remove
} STelegBuffer;

STelegBuffer suSendList_g;
STelegBuffer suReceiveList_g;

void CbReceive(INT8U i8uChar_p);
TBOOL CbTransmit(void);
void CbError(UART_ERecError enRecError_p);
void CbTimerExpired(void);

ERs485State  eRs485State_g;
EIoProtocolState eIoProtocolState_g;
TBOOL bResponseState_g = bTRUE;           //!< Response to RS485 telegrams
TBOOL bConfigurationComplete_g = bFALSE;  //!< Configuration from PiBridge master is completed
SRs485Telegram *ptRecvBuffer_g;           //!< Receive Buffer for current Telegram
SRs485Telegram *ptSendBuffer_g;           //!< Send Buffer for current Telegram
SIOGeneric sIoProtocolRecvFrame_g;
TBOOL bNewFrameReceived_g;
SIOGeneric sIoProtocolSendFrame_g;

INT16U i16uRecBufInd_g;
UART_ERecError eUartError_g;
UART_EBufferSendState eSendState_g = UART_enBSS_SEND_OK;
TBOOL bUpdateMode_g = bFALSE;
INT16U i16uSendSequNr_g = 0;
SRs485Telegram suResponseTelegram_g;

INT8U i8uOwnAddress_g;
void *pvRs485TelegHeapHandle_g;
static SRs485Telegram asuArgHeapMem_s[MAX_TELEGRAM_HEAP_MEM];

ERs485Protocol eRs485Protocol_g;
int eRs485Protocol_cnt;

#if defined(NEED_RS485_RAPI_MESSAGE) && (PIAPI_USE_BRIDGE == 0)
static TBOOL bRAPIResponsePending_s = bFALSE;
#endif

#define AGENT_TESTING_FAKE_ERROR_CODE 0x12345

//*************************************************************************************************
//| Function: ReceiveBufferInit
//|
//! \brief
//!
//! \detailed
//!
//!
//!
//! \ingroup
//-------------------------------------------------------------------------------------------------
void ReceiveBufferInit()
{
    if (ptRecvBuffer_g == NULL)
    {
        ptRecvBuffer_g = kbUT_malloc (pvRs485TelegHeapHandle_g, 5, sizeof (SRs485Telegram));
    }
    i16uRecBufInd_g = 0;
}

//*************************************************************************************************
//| Function: GateProtocolReceive
//|
//! \brief
//!
//! \detailed
//!
//!
//!
//! \ingroup
//-------------------------------------------------------------------------------------------------
static void GateProtocolReceive(INT8U i8uChar_p)
{
    switch (eRs485State_g)
    {
    case eRs485StateIdle:
    case eRs485StateWaitSilence:
    case eRs485StateWaitResponse:
    {
        ReceiveBufferInit();
        eRs485State_g = eRs485StateReceive;
    }   break;
    default:
    {
    }   break;
    }

    switch (eRs485State_g)
    {
    case eRs485StateReceive:
    case eRs485StateReceiveResponse:
    {
        if (i16uRecBufInd_g < sizeof(SRs485Telegram))
        {
            // insert byte
            ((INT8U *)ptRecvBuffer_g)[i16uRecBufInd_g++] = i8uChar_p;

            // Retrigger Timer to detect end of frame
            TIM_CountDownReTrigger(RS485_STATE_TIMER, END_FRAME_TIME);
        }
        else
        {
            // frame is too long -> discard it as a whole
            i16uRecBufInd_g = 0;
            eRs485State_g = eRs485StateIdle;
        }
    }    break;
    default:
    {
    }   break;
    }
}

//*************************************************************************************************
//| Function: IoProtocolReceive
//|
//! \brief
//!
//! \detailed
//!
//!
//!
//! \ingroup
//-------------------------------------------------------------------------------------------------
static void IoProtocolReceive(INT8U i8uChar_p)
{
    switch (eIoProtocolState_g)
    {
    case eIoProtocolIdle:
    case eIoProtocolTimeout:
    {
        i16uRecBufInd_g = 0;
        eIoProtocolState_g = eIoProtocolReceive;
        TIM_CountDownReTrigger(RS485_STATE_TIMER, IOPROTOCOL_TIMEOUT);
    }   break;
    default:
    {
    }   break;
    }

    if (i16uRecBufInd_g < IOPROTOCOL_HEADER_LENGTH)
    {
        sIoProtocolRecvFrame_g.uHeader.ai8uHeader[i16uRecBufInd_g] = i8uChar_p;
        i16uRecBufInd_g++;
    }
    else
    {

        if ((i16uRecBufInd_g - IOPROTOCOL_HEADER_LENGTH) == sIoProtocolRecvFrame_g.uHeader.sHeaderTyp1.bitLength)
        {
            sIoProtocolRecvFrame_g.ai8uData[i16uRecBufInd_g - IOPROTOCOL_HEADER_LENGTH] = i8uChar_p;
            if (IOProtocolCheckFrame(&sIoProtocolRecvFrame_g) == bTRUE)
            {
                if (sIoProtocolRecvFrame_g.uHeader.sHeaderTyp2.bitIoHeaderType == 1    // header type 2
                && sIoProtocolRecvFrame_g.uHeader.sHeaderTyp2.bitCommand == IOP_TYP2_CMD_GOTO_GATE_PROTOCOL)
                {
                    // go back to gate protocol, do not send a response
                    ModGateRs485SetProtocol(eGateProtocol);
                    i16uRecBufInd_g = 0;
                    return;
                }
#ifdef PI_IO_PROTOCOL
                i32uLastPacketTick_s = kbUT_getCurrentMs();
                bNewFrameReceived_g = bTRUE;
#endif
            }

            eIoProtocolState_g = eIoProtocolIdle;

            // Retrigger Timer to detect timeout between packets
            TIM_CountDownReTrigger(RS485_STATE_TIMER, IOPROTOCOL_TIMEOUT);
        }
        else
        {
            sIoProtocolRecvFrame_g.ai8uData[i16uRecBufInd_g - IOPROTOCOL_HEADER_LENGTH] = i8uChar_p;
            i16uRecBufInd_g++;
        }
    }
}

//*************************************************************************************************
//| Function: CbReceive
//|
//! \brief
//!
//! \detailed
//!
//!
//!
//! \ingroup
//-------------------------------------------------------------------------------------------------
void CbReceive(INT8U i8uChar_p)
{
    switch (eRs485Protocol_g)
    {
    case eGateProtocol:
    case eIoConfig:
        GateProtocolReceive(i8uChar_p);
        break;
    case eIoProtocol:
        IoProtocolReceive(i8uChar_p);
        break;
    }
}

//*************************************************************************************************
//| Function: CbTransmit
//|
//! \brief
//!
//! \detailed
//!
//!
//!
//! \ingroup
//-------------------------------------------------------------------------------------------------
TBOOL CbTransmit(void)
{
    if (bUpdateMode_g)
    {
#if defined(KUNBUS_FW_DESCR_TYP_OF_PRODUCT) && KUNBUS_FW_DESCR_TYP_OF_PRODUCT == KUNBUS_FW_DESCR_TYP_MG_SERCOS3
        volatile INT32U i32uLoopCount = 0;
        while (i32uLoopCount++ < 500000)
        {
            // make sure last byte has been sent via UART
        }
#endif
        // Enter update mode
        ((void (*)(void))(ctKunbusFirmwareDescription_g.i32uFwuEntryAddr)) ();
    }
    return (bFALSE);
}

//*************************************************************************************************
//| Function: CbError
//|
//! \brief
//!
//! \detailed
//!
//!
//!
//! \ingroup
//-------------------------------------------------------------------------------------------------
void CbError(UART_ERecError enRecError_p)
{
    eUartError_g = enRecError_p;

    // Discard Telegram, if in Telegram receive state
    switch (eRs485Protocol_g)
    {
    case eGateProtocol:
    case eIoConfig:
        switch (eRs485State_g)
        {
        case eRs485StateReceive:
        {
            i16uRecBufInd_g = 0;
            eRs485State_g = eRs485StateIdle;
        }   break;
        default:  // do nothing
            break;
        }
        break;

    case eIoProtocol:
        // do nothing eIoProtocolState_g = eIoProtocolIdle;
        break;
    }
}

//*************************************************************************************************
//| Function: CbTimerExpired
//|
//! \brief
//!
//! \detailed
//!
//!
//!
//! \ingroup
//-------------------------------------------------------------------------------------------------
void CbTimerExpired(void)
{
    switch (eRs485Protocol_g)
    {
    case eGateProtocol:
    case eIoConfig:
        switch (eRs485State_g)
        {
        case eRs485StateIdle:
            break;
        case eRs485StateWaitSilence:
            eRs485State_g = eRs485StateSend;
            break;
        case eRs485StateReceive:
            eRs485State_g = eRs485StateSendResponse;
            break;
        case eRs485StateSend:
            break;
        case eRs485StateWaitResponse:
            // no answer received
            eRs485State_g = eRs485StateResponseDiscard;
            break;
        case eRs485StateSendResponse:
            break;
        case eRs485StateResponseAnalyze:
            break;
        case eRs485StateReceiveResponse:
            // response frame complete
            eRs485State_g = eRs485StateResponseAnalyze;
            break;
        default:  // Do nothing
            break;
        }
        break;

    case eIoProtocol:
        // timeout in communication
        eIoProtocolState_g = eIoProtocolTimeout;
        break;
    }
}

//*************************************************************************************************
//| Function: Crc8
//|
//! \brief
//!
//! \detailed
//!
//!
//!
//! \ingroup
//-------------------------------------------------------------------------------------------------
static INT8U Crc8 (
    INT8U *pi8uFrame_p,
    INT16U i16uLen_p)
{
    INT8U i8uRv_l = 0;

    while (i16uLen_p--)
    {
        i8uRv_l = i8uRv_l ^ pi8uFrame_p[i16uLen_p];
    }
    return i8uRv_l;
}


//*************************************************************************************************
//| Function: SetAnswerDefaultValues
//|
//! \brief
//!
//! \detailed
//!
//!
//!
//! \ingroup
//-------------------------------------------------------------------------------------------------
void SetAnswerDefaultValues(SRs485Telegram *psuSendTelegram_p, SRs485Telegram *psuRecvTelegram_p)
{
    psuSendTelegram_p->i8uDstAddr = psuRecvTelegram_p->i8uSrcAddr;
    psuSendTelegram_p->i8uSrcAddr = psuRecvTelegram_p->i8uDstAddr;
    psuSendTelegram_p->i16uCmd = psuRecvTelegram_p->i16uCmd;
    psuSendTelegram_p->i16uSequNr = psuRecvTelegram_p->i16uSequNr;
    psuSendTelegram_p->i8uDataLen = 0;
    memset(psuSendTelegram_p->ai8uData, 0, sizeof(psuSendTelegram_p->ai8uData));
}

//*************************************************************************************************
//| Function: SetAnswerErrorValues
//|
//! \brief
//!
//! \detailed
//!
//!
//!
//! \ingroup
//-------------------------------------------------------------------------------------------------
void SetAnswerErrorValues(SRs485Telegram *psuSendTelegram_p, EModGateComError eModGateComError_p)
{
    psuSendTelegram_p->i16uCmd |= MODGATE_RS485_COMMAND_ANSWER_ERROR;
    psuSendTelegram_p->i8uDataLen = 4;
    memcpy(psuSendTelegram_p->ai8uData, &eModGateComError_p, 4);
}

//*************************************************************************************************
//| Function: DispatchCmdPing
//|
//! \brief
//!
//! \detailed
//!
//!
//!
//! \ingroup
//-------------------------------------------------------------------------------------------------
void DispatchCmdPing(
    SRs485Telegram *psuRecvTelegram_p,
    SRs485Telegram *psuSendTelegram_p
    )
{
    EModGateComError eError_l = MODGATECOM_NO_ERROR;
    (void)psuRecvTelegram_p;    // ignore parameter

    if(eError_l == MODGATECOM_NO_ERROR)
    {
        psuSendTelegram_p->i16uCmd |= MODGATE_RS485_COMMAND_ANSWER_OK;
    }
    else
    {
        SetAnswerErrorValues(psuSendTelegram_p, eError_l);
    }
}

//*************************************************************************************************
//| Function: DispatchCmdSetFwUpdateMode
//|
//! \brief
//!
//! \detailed
//!
//!
//!
//! \ingroup
//-------------------------------------------------------------------------------------------------
void DispatchCmdSetFwUpdateMode(
    SRs485Telegram *psuRecvTelegram_p,
    SRs485Telegram *psuSendTelegram_p
    )
{
    EModGateComError eError_l = MODGATECOM_NO_ERROR;

    (void)psuRecvTelegram_p;    // ignore parameter

    if(eError_l == MODGATECOM_NO_ERROR)
    {
        bUpdateMode_g = bTRUE;
        psuSendTelegram_p->i16uCmd |= MODGATE_RS485_COMMAND_ANSWER_OK;
    }
    else
    {
        SetAnswerErrorValues(psuSendTelegram_p, eError_l);
    }
}

//*************************************************************************************************
//| Function: DispatchCmdGetDeviceInfo
//|
//! \brief
//!
//! \detailed
//!
//!
//!
//! \ingroup
//-------------------------------------------------------------------------------------------------
TBOOL DispatchCmdGetDeviceInfo(
    SRs485Telegram *psuRecvTelegram_p,
    SRs485Telegram *psuSendTelegram_p
    )
{
    (void)psuRecvTelegram_p;    // ignore parameter
    extern MODGATECOM_IDResp MODGATE_OwnID_g;
    EModGateComError eError_l = MODGATECOM_NO_ERROR;
    TBOOL bRv_l = bTRUE;

    psuSendTelegram_p->i8uDataLen = sizeof(MODGATECOM_IDResp);
    memcpy(psuSendTelegram_p->ai8uData, &MODGATE_OwnID_g, psuSendTelegram_p->i8uDataLen);
    if(eError_l == MODGATECOM_NO_ERROR)
    {
        psuSendTelegram_p->i16uCmd |= MODGATE_RS485_COMMAND_ANSWER_OK;
    }
    else
    {
        SetAnswerErrorValues(psuSendTelegram_p, eError_l);
    }
    return bRv_l;
}

//*************************************************************************************************
//| Function: DispatchCmdFactoryReset
//|
//! \brief
//!
//! \detailed
//!
//!
//!
//! \ingroup
//-------------------------------------------------------------------------------------------------
void DispatchCmdFactoryReset(
    SRs485Telegram *psuRecvTelegram_p,
    SRs485Telegram *psuSendTelegram_p
    )
{
    (void)psuRecvTelegram_p;    // ignore parameter
#if defined STM_WITH_EEPROM
    BSP_EEPROM_factoryReset();
#endif
    psuSendTelegram_p->i16uCmd |= MODGATE_RS485_COMMAND_ANSWER_OK;
}

//*************************************************************************************************
//| Function: DispatchCmdSetLEDGreen
//|
//! \brief
//!
//! \detailed
//!
//!
//!
//! \ingroup
//-------------------------------------------------------------------------------------------------
void DispatchCmdSetLEDGreen(
    SRs485Telegram *psuRecvTelegram_p,
    SRs485Telegram *psuSendTelegram_p
    )
{
    EModGateComError eError_l = MODGATECOM_NO_ERROR;
    (void)psuRecvTelegram_p;    // ignore parameter

    if(eError_l == MODGATECOM_NO_ERROR)
    {
        INT8U i8uLEDgreen = 1;
        APPL_set_allLED(i8uLEDgreen);
        psuSendTelegram_p->i16uCmd |= MODGATE_RS485_COMMAND_ANSWER_OK;
    }
    else
    {
        SetAnswerErrorValues(psuSendTelegram_p, eError_l);
    }
}

//*************************************************************************************************
//| Function: DispatchCmdSetLEDRed
//|
//! \brief
//!
//! \detailed
//!
//!
//!
//! \ingroup
//-------------------------------------------------------------------------------------------------
void DispatchCmdSetLEDRed(
    SRs485Telegram *psuRecvTelegram_p,
    SRs485Telegram *psuSendTelegram_p
    )
{
    EModGateComError eError_l = MODGATECOM_NO_ERROR;
    (void)psuRecvTelegram_p;    // ignore parameter

    if(eError_l == MODGATECOM_NO_ERROR)
    {
        INT8U i8uLEDred = 2;
        APPL_set_allLED(i8uLEDred);
        psuSendTelegram_p->i16uCmd |= MODGATE_RS485_COMMAND_ANSWER_OK;
    }
    else
    {
        SetAnswerErrorValues(psuSendTelegram_p, eError_l);
    }
}


// only needed for CANopen Master
//*************************************************************************************************
//| Function: DispatchCmdRAPIMessage
//|
//! \brief
//!
//! \detailed
//!
//!
//!
//! \ingroup
//-------------------------------------------------------------------------------------------------
#if (PIAPI_USE_BRIDGE == 1) 
void DispatchRspRAPIMessage(
    SRs485Telegram *psuRecvTelegram_p,
    SRs485Telegram *psuSendTelegram_p
    )
{
    (void)psuSendTelegram_p;    // ignore parameter
    kbRAPI_ForwardToETH(psuRecvTelegram_p->ai8uData, psuRecvTelegram_p->i8uDataLen);
}
#endif
#ifdef NEED_RS485_RAPI_MESSAGE
TBOOL DispatchCmdRAPIMessage(
    SRs485Telegram *psuRecvTelegram_p,
    SRs485Telegram *psuSendTelegram_p
    )
{
    EModGateComError eError_l = MODGATECOM_NO_ERROR;
    INT16U i16uI;
    TBOOL bResult_l;

    if (bRAPIResponsePending_s == bFALSE)
    {
        // Insert rapi message
        for (i16uI = 0; i16uI < psuRecvTelegram_p->i8uDataLen; i16uI++)
        {
            kbRAPI_ReadChar(psuRecvTelegram_p->ai8uData[i16uI]);
        }

        bRAPIResponsePending_s = bTRUE;
    }
    
    // Get rapi answer message
    bResult_l = kbRAPI_CopySendBuffer(psuSendTelegram_p->ai8uData, &psuSendTelegram_p->i8uDataLen, MAX_TELEGRAM_DATA_SIZE);

    if (bResult_l == bTRUE)
    {
        if (eError_l == MODGATECOM_NO_ERROR)
        {
            psuSendTelegram_p->i16uCmd |= MODGATE_RS485_COMMAND_ANSWER_OK;
        }
        else
        {
            SetAnswerErrorValues(psuSendTelegram_p, eError_l);
        }

        bRAPIResponsePending_s = bFALSE;
    }

    return bResult_l;
}

#endif


//*************************************************************************************************
//| Function: DispatchCmdGetErrorLog
//|
//! \brief
//!
//! \detailed
//!
//!
//!
//! \ingroup
//-------------------------------------------------------------------------------------------------
void DispatchCmdGetErrorLog(
    SRs485Telegram *psuRecvTelegram_p,
    SRs485Telegram *psuSendTelegram_p
    )
{
    (void)psuRecvTelegram_p;    // ignore parameter
#ifdef PI_MGATE_MODULE
    EModGateComError eError_l = MODGATECOM_NO_ERROR;

    psuSendTelegram_p->i8uDataLen = 32;

    MODGATECOM_getErrorLogEEPROM((INT32U*)psuSendTelegram_p->ai8uData);

    if(eError_l == MODGATECOM_NO_ERROR)
    {
        psuSendTelegram_p->i16uCmd |= MODGATE_RS485_COMMAND_ANSWER_OK;
    }
    else
#else
    EModGateComError eError_l = MODGATECOM_ERROR_CMD_NOT_SUPPORTED;
#endif //PI_MGATE_MODULE
    {
        SetAnswerErrorValues(psuSendTelegram_p, eError_l);
    }
}

#ifdef NEED_RS485_SCRIPT_MESSAGE
// only needed for serial modgate
//*************************************************************************************************
//| Function: DispatchCmdScriptMessage
//|
//! \brief
//!
//! \detailed
//!
//!
//!
//! \ingroup
//-------------------------------------------------------------------------------------------------
void DispatchCmdScriptMessage(
    SRs485Telegram *psuRecvTelegram_p,
    SRs485Telegram *psuSendTelegram_p
    )
{
    EModGateComError eError_l = MODGATECOM_NO_ERROR;
    INT16U i16uI;

    // Insert script message
    for(i16uI = 0; i16uI < psuRecvTelegram_p->i8uDataLen; i16uI++)
    {
        SC_Debug_ReadChar(psuRecvTelegram_p->ai8uData[i16uI]);
    }
    // Get script answer message
    psuSendTelegram_p->i8uDataLen = 4 + scDbgIF_CopySendBuffer(psuSendTelegram_p->ai8uData, MAX_TELEGRAM_DATA_SIZE);

    if(eError_l == MODGATECOM_NO_ERROR)
    {
        psuSendTelegram_p->i16uCmd |= MODGATE_RS485_COMMAND_ANSWER_OK;
    }
    else
    {
        SetAnswerErrorValues(psuSendTelegram_p, eError_l);
    }
}
#endif

#ifdef MODGATE_AGENT_ENABLE
//*************************************************************************************************
//| Function: DispatchCmdAgentCode
//|
//! \brief
//!
//! \detailed
//!
//!
//!
//! \ingroup
//-------------------------------------------------------------------------------------------------
void DispatchCmdAgentCode(
    SRs485Telegram *psuRecvTelegram_p,
    SRs485Telegram *psuSendTelegram_p
    )
{
    EModGateComError eError_l = MODGATECOM_NO_ERROR;
    INT16U i16uAgentCode_l = 0;

    psuSendTelegram_p->i8uDataLen = 32;

    if(psuRecvTelegram_p->i8uDataLen == 2)
    {
        i16uAgentCode_l = psuRecvTelegram_p->ai8uData[1] << 8 | psuRecvTelegram_p->ai8uData[0];

        switch(i16uAgentCode_l)
        {
            case MODGATE_AGENT_CODE_HARDFAULT:
            {
                bspError (AGENT_TESTING_FAKE_ERROR_CODE, bTRUE, 0);
            }   break;
        }
    }
    else
    {
        eError_l = MODGATECOM_ERROR_UNKNOWN;
    }


    if(eError_l == MODGATECOM_NO_ERROR)
    {
        psuSendTelegram_p->i16uCmd |= MODGATE_RS485_COMMAND_ANSWER_OK;
    }
    else
    {
        SetAnswerErrorValues(psuSendTelegram_p, eError_l);
    }
}
#endif  //MODGATE_AGENT_ENABLE

static TBOOL DispatchCmdPiIoSetModuleId(
    SRs485Telegram *psuRecvTelegram_p,
    SRs485Telegram *psuSendTelegram_p
    )
{
    EModGateComError eError_l = MODGATECOM_NO_ERROR;
    TBOOL bRv_l = bTRUE;

    if (eError_l == MODGATECOM_NO_ERROR)
    {
        if (ModGateRs485GetResponseState() && i8uOwnAddress_g == 0)
        {
            i8uOwnAddress_g = psuRecvTelegram_p->i8uDstAddr;
            bConfigurationComplete_g = bTRUE;
            psuSendTelegram_p->i16uCmd |= MODGATE_RS485_COMMAND_ANSWER_OK;
        }
        else
        {
            psuSendTelegram_p->i16uCmd |= MODGATE_RS485_COMMAND_ANSWER_ERROR;
        }
    }
    else
    {
        SetAnswerErrorValues(psuSendTelegram_p, eError_l);
    }
    return bRv_l;
}

static TBOOL DispatchCmdPiIoSetTermination(
    SRs485Telegram *psuRecvTelegram_p,
    SRs485Telegram *psuSendTelegram_p
    )
{
    EModGateComError eError_l = MODGATECOM_NO_ERROR;
    TBOOL bRv_l = bTRUE;

    if (eError_l == MODGATECOM_NO_ERROR)
    {
        psuSendTelegram_p->i16uCmd |= MODGATE_RS485_COMMAND_ANSWER_OK;
        PiSlaveAppRS485Terminate((psuRecvTelegram_p->ai8uData[0] == 0) ? bTRUE : bFALSE);
    }
    else
    {
        SetAnswerErrorValues(psuSendTelegram_p, eError_l);
    }
    return bRv_l;
}

static TBOOL DispatchCmdPiIoConfigure(
    SRs485Telegram *psuRecvTelegram_p,
    SRs485Telegram *psuSendTelegram_p
    )
{
    EModGateComError eError_l = MODGATECOM_NO_ERROR;
    TBOOL bRv_l = bTRUE;

    if (eError_l == MODGATECOM_NO_ERROR)
    {
        psuSendTelegram_p->i16uCmd |= MODGATE_RS485_COMMAND_ANSWER_OK;
    }
    else
    {
        SetAnswerErrorValues(psuSendTelegram_p, eError_l);
    }
    return bRv_l;
}

static TBOOL DispatchCmdPiIoStartDataExchange(
    SRs485Telegram *psuRecvTelegram_p,
    SRs485Telegram *psuSendTelegram_p
    )
{
    ModGateRs485SetProtocol(eIoProtocol);   // switch to IO protocol
    return bFALSE;      // do not send a response
}

//*************************************************************************************************
//| Function: DispatchCmd
//|
//! \brief
//!
//! \detailed
//!
//!
//!
//! \ingroup
//-------------------------------------------------------------------------------------------------
TBOOL DispatchCmd(SRs485Telegram *psuRecvTelegram_p, SRs485Telegram *psuSendTelegram_p)
{
    TBOOL bResult_l = bTRUE;

    SetAnswerDefaultValues(psuSendTelegram_p, psuRecvTelegram_p);

    switch(psuRecvTelegram_p->i16uCmd)
    {
    case eCmdPing:
        DispatchCmdPing(psuRecvTelegram_p, psuSendTelegram_p);
        break;
    case eCmdSetFwUpdateMode:
        DispatchCmdSetFwUpdateMode(psuRecvTelegram_p, psuSendTelegram_p);
        break;
    case eCmdResetModule:
    case eCmdGetFwInfo:
    case eCmdReadFwFlash:
    case eCmdWriteFwFlash:
    case eCmdEraseFwFlash:
    case eCmdWriteSerialNumber:
    case eCmdWriteMacAddr:
        // These commands are only available in update mode
        SetAnswerErrorValues(psuSendTelegram_p, MODGATECOM_ERROR_NO_UPDATE_MODE);
        break;
    case eCmdGetDeviceInfo:
        bResult_l = DispatchCmdGetDeviceInfo(psuRecvTelegram_p, psuSendTelegram_p);
        break;
    case eCmdFactoryReset:
        DispatchCmdFactoryReset(psuRecvTelegram_p, psuSendTelegram_p);
        break;
    case eCmdSetLEDGreen:
        DispatchCmdSetLEDGreen(psuRecvTelegram_p, psuSendTelegram_p);
        break;
    case eCmdSetLEDRed:
        DispatchCmdSetLEDRed(psuRecvTelegram_p, psuSendTelegram_p);
        break;
#if (PIAPI_USE_BRIDGE == 1)
    case (eCmdRAPIMessage | MODGATE_RS485_COMMAND_ANSWER_OK) :
        DispatchRspRAPIMessage(psuRecvTelegram_p, psuSendTelegram_p);
        bResult_l = bFALSE; // do not send a response
        break;
#endif    
#ifdef NEED_RS485_RAPI_MESSAGE
    case eCmdRAPIMessage:
        bResult_l = DispatchCmdRAPIMessage(psuRecvTelegram_p, psuSendTelegram_p);
        break;
#endif
    case eCmdGetErrorLog:
        DispatchCmdGetErrorLog(psuRecvTelegram_p, psuSendTelegram_p);
        break;
#ifdef NEED_RS485_SCRIPT_MESSAGE
    case eCmdScriptMessage:
        DispatchCmdScriptMessage(psuRecvTelegram_p, psuSendTelegram_p);
        break;
#endif
#ifdef MODGATE_AGENT_ENABLE
    case eCmdAgentCode:
        DispatchCmdAgentCode(psuRecvTelegram_p, psuSendTelegram_p);
        break;
#endif
    case eCmdPiIoSetAddress:
        bResult_l = DispatchCmdPiIoSetModuleId(psuRecvTelegram_p, psuSendTelegram_p);
        break;
    case eCmdPiIoSetTermination:
        bResult_l = DispatchCmdPiIoSetTermination(psuRecvTelegram_p, psuSendTelegram_p);
        break;
    case eCmdPiIoConfigure:
        bResult_l = DispatchCmdPiIoConfigure(psuRecvTelegram_p, psuSendTelegram_p);
        break;
    case eCmdPiIoStartDataExchange:
        bResult_l = DispatchCmdPiIoStartDataExchange(psuRecvTelegram_p, psuSendTelegram_p);
        break;
    default:
        SetAnswerErrorValues(psuSendTelegram_p, MODGATECOM_ERROR_CMD_NOT_SUPPORTED);
        break;
    }
    psuSendTelegram_p->i8uSrcAddr = i8uOwnAddress_g;
    psuSendTelegram_p->ai8uData[psuSendTelegram_p->i8uDataLen] = Crc8((INT8U*)psuSendTelegram_p, psuSendTelegram_p->i8uDataLen + RS485_HDRLEN);

    return bResult_l;
}

//*************************************************************************************************
//| Function: ResponseAnalyze
//|
//! \brief
//!
//! \detailed
//!
//!
//!
//! \ingroup
//-------------------------------------------------------------------------------------------------
INT32U ResponseAnalyze(SRs485Telegram *psuRecvBuf_p)
{
    INT32U i32uError_l = MODGATECOM_NO_ERROR;

    // Test sequence number
    if(ptSendBuffer_g->i16uSequNr != psuRecvBuf_p->i16uSequNr)
    {
        i32uError_l = MODGATECOM_ERROR_RESP_SEQUENCE;
        goto laError;
    }

    // Test command
    if(ptSendBuffer_g->i16uCmd != (psuRecvBuf_p->i16uCmd & MODGATE_RS485_COMMAND_ANSWER_FILTER))
    {
        i32uError_l = MODGATECOM_ERROR_RESP_CMDNO;
        goto laError;
    }

laError:
    return i32uError_l;
}

//*************************************************************************************************
//| Function: GateProtocolCheckFrame
//|
//! \brief
//!
//! \detailed
//!
//!
//!
//! \ingroup
//-------------------------------------------------------------------------------------------------
static TBOOL GateProtocolCheckFrame(TBOOL *pbResponse)

{
    INT8U i8uCrcCalc_l;
    INT8U i8uCrcFrame_l;
    TBOOL bRv_l = bFALSE;
    *pbResponse = bFALSE;

    if ( i16uRecBufInd_g >= RS485_HDRLEN + 1
      && i16uRecBufInd_g >= RS485_HDRLEN + ptRecvBuffer_g->i8uDataLen + 1)    // check that the whole telegram has been received
    {
        if ((ptRecvBuffer_g->i16uCmd & MODGATE_RS485_COMMAND_ANSWER) == 0) // do not care about response packets
        {
            i8uCrcCalc_l = Crc8((INT8U*)ptRecvBuffer_g, ptRecvBuffer_g->i8uDataLen + RS485_HDRLEN);
            i8uCrcFrame_l = ptRecvBuffer_g->ai8uData[ptRecvBuffer_g->i8uDataLen];

            if (i8uCrcCalc_l == i8uCrcFrame_l)
            {
                // Frame is consistent -> check Address now
                if (ptRecvBuffer_g->i16uCmd == eCmdGetDeviceInfo ||
                    ptRecvBuffer_g->i16uCmd == eCmdPiIoConfigure ||
                    ptRecvBuffer_g->i16uCmd == eCmdPiIoSetAddress ||
                    ptRecvBuffer_g->i16uCmd == eCmdPiIoStartDataExchange)
                {
                    // This is a configuration telegram from PiBridgeMaster, address should not be checked
                    bRv_l = bTRUE;
                }
                else if (
                    ptRecvBuffer_g->i8uDstAddr == BSP_getMgateDecode()
                    || ptRecvBuffer_g->i8uDstAddr == i8uOwnAddress_g
                    || ptRecvBuffer_g->i8uDstAddr == MODGATE_RS485_BROADCAST_ADDR)
                {   // Frame is valid and for us
                    bRv_l = bTRUE;
                    if (ptRecvBuffer_g->i8uDstAddr == i8uOwnAddress_g)
                    {
                        // if the packet is directly addressed, send a response even if ModGateRs485GetResponseState() returns false
                        *pbResponse = bTRUE;
                    }
                }
            }
        }
    }
    return (bRv_l);
}

//*************************************************************************************************
//| Function: IOProtocolCheckFrame
//|
//! \brief
//!
//! \detailed
//!
//!
//!
//! \ingroup
//-------------------------------------------------------------------------------------------------
static TBOOL IOProtocolCheckFrame(
    SIOGeneric *sFrame_p
    )
{
    if (sFrame_p->uHeader.sHeaderTyp1.bitIoHeaderType == 0)
    {   // Header Type 1
        if (sFrame_p->uHeader.sHeaderTyp1.bitAddress == i8uOwnAddress_g)
        {
            INT8U i8uCRC = Crc8((INT8U*)sFrame_p, (INT16U)sFrame_p->uHeader.sHeaderTyp1.bitLength + IOPROTOCOL_HEADER_LENGTH);
            if (sFrame_p->ai8uData[sFrame_p->uHeader.sHeaderTyp1.bitLength] == i8uCRC)
            {
                return bTRUE;
            }
        }
    }
    else
    {   // Header Type 2
        INT8U i8uCRC = Crc8((INT8U*)sFrame_p, (INT16U)sFrame_p->uHeader.sHeaderTyp2.bitLength + IOPROTOCOL_HEADER_LENGTH);
        if (sFrame_p->ai8uData[sFrame_p->uHeader.sHeaderTyp2.bitLength] == i8uCRC)
        {
            return bTRUE;
        }
    }

    return bFALSE;
}


//*************************************************************************************************
//| Function: ModGateRs485InsertSendTelegram
//|
//! \brief
//!
//! \detailed
//!
//!
//!
//! \ingroup
//-------------------------------------------------------------------------------------------------
void ModGateRs485InsertSendTelegram(SRs485Telegram *ptSendTelegram_p)
{
    INT16U i16uNextHead_l;

    ptSendTelegram_p->i8uSrcAddr = BSP_getMgateDecode();
    ptSendTelegram_p->i8uDstAddr = (ptSendTelegram_p->i8uSrcAddr == MGATE_MODUL_LEFT) ? MGATE_MODUL_RIGHT : MGATE_MODUL_LEFT;
    ptSendTelegram_p->i16uSequNr = i16uSendSequNr_g++;
    ptSendTelegram_p->ai8uData[ptSendTelegram_p->i8uDataLen] =
        Crc8 ((INT8U *)ptSendTelegram_p, ptSendTelegram_p->i8uDataLen + RS485_HDRLEN);

    if (suSendList_g.i16uHead < MAX_TELEG_BUFFER_LIST - 1)
    {
        i16uNextHead_l = suSendList_g.i16uHead + 1;
    }
    else
    {
        i16uNextHead_l = 0;
    }

    if (i16uNextHead_l != suSendList_g.i16uTail)
    {
        suSendList_g.aptBuffer[suSendList_g.i16uHead] = ptSendTelegram_p;
        suSendList_g.i16uHead = i16uNextHead_l;
    }
    else
    {
        kbUT_free (ptSendTelegram_p);
    }
}

//*************************************************************************************************
//| Function: ModGateRs485SetProtocol
//|
//! \brief
//!
//! \detailed
//!
//!
//!
//! \ingroup
//-------------------------------------------------------------------------------------------------
void ModGateRs485SetProtocol(ERs485Protocol eRs485Protocol_p)
{
    eRs485Protocol_g = eRs485Protocol_p;
    eRs485Protocol_cnt++;
    i16uRecBufInd_g = 0;
    switch (eRs485Protocol_g)
    {
    case eGateProtocol:
    case eIoConfig:
        eRs485State_g = eRs485StateIdle;
        break;
    case eIoProtocol:
        eIoProtocolState_g = eIoProtocolIdle;
        break;
    }
}

//*************************************************************************************************
//| Function: ModGateRs485GetProtocol
//|
//! \brief
//!
//! \detailed
//!
//!
//!
//! \ingroup
//-------------------------------------------------------------------------------------------------
ERs485Protocol ModGateRs485GetProtocol(void)
{
    return eRs485Protocol_g;
}

//*************************************************************************************************
//| Function: ModGateRs485Init
//|
//! \brief
//!
//! \detailed
//!
//!
//!
//! \ingroup
//-------------------------------------------------------------------------------------------------
INT32U ModGateRs485Init(void)
{
    UART_TConfig suUartConfig_l;
    TIM_TCountDownTimerInit tConfTimer_l;
    INT32U i32uRv_l = MODGATECOM_NO_ERROR;

    // Init Heap
    pvRs485TelegHeapHandle_g = kbUT_initHeap ((INT8U*)asuArgHeapMem_s, sizeof(asuArgHeapMem_s));

    // Init left-/right-mGate decoder
    BSP_initMgateDecode();

    // Init Sendlist
    suSendList_g.i16uHead = suSendList_g.i16uTail = 0;
    // Init Receive buffer
    ReceiveBufferInit();

    // Initial status
    eRs485State_g = eRs485StateIdle;
    eIoProtocolState_g = eIoProtocolIdle;

    // Begin with Config Protocol
    ModGateRs485SetProtocol(eIoConfig);

    // Init Uart
    suUartConfig_l.enBaudRate = UART_enBAUD_115200;
    suUartConfig_l.enParity = UART_enPARITY_EVEN;
    suUartConfig_l.enStopBit = UART_enSTOPBIT_1;
    suUartConfig_l.enDataLen = UART_enDATA_LEN_8;
    suUartConfig_l.enFlowControl = UART_enFLOWCTRL_NONE;
    suUartConfig_l.enRS485 = UART_enRS485_ENABLE;
    suUartConfig_l.cbReceive = CbReceive;
    suUartConfig_l.cbTransmit = CbTransmit;
    suUartConfig_l.cbError = CbError;
    if(UART_init (MODGATE_RS485_UART, &suUartConfig_l) != BSPE_NO_ERROR)
    {
        i32uRv_l = MODGATECOM_ERROR_RS485_INIT;
        goto laError;
    }

    // Init Timer
    tConfTimer_l.cbTimerExpired = CbTimerExpired;
    tConfTimer_l.i32uTimeBase = STATE_TIMER_TIME_BASE;
    TIM_initCountDownTimer (RS485_STATE_TIMER, &tConfTimer_l);

    return i32uRv_l;

laError:
    return i32uRv_l;
}

TBOOL ModGateRs485GetResponseState(void)
{
    return bResponseState_g;
}

void ModGateRs485SetResponseState(TBOOL bResponseState_p)
{
    PiSlaveAppRS485Terminate(bResponseState_p);
    bResponseState_g = bResponseState_p;
}

//*************************************************************************************************
//| Function: ModGateRs485Run
//|
//! \brief
//!
//! \detailed
//!
//!
//!
//! \ingroup
//-------------------------------------------------------------------------------------------------
void ModGateRs485Run(void)
{
    static SRs485Telegram suSendTelegram_s;
    TBOOL bSendResponse;

    switch (eRs485Protocol_g)
    {
    case eGateProtocol:
    case eIoConfig:

        switch (eRs485State_g)
        {
        case eRs485StateIdle:
        {
            if (ptSendBuffer_g)
            {
                kbUT_free(ptSendBuffer_g);
                ptSendBuffer_g = NULL;
            }

            // look for send request
            if (suSendList_g.i16uHead != suSendList_g.i16uTail)
            {
                // send request found
                ptSendBuffer_g = suSendList_g.aptBuffer[suSendList_g.i16uTail];
                if (suSendList_g.i16uTail < MAX_TELEG_BUFFER_LIST - 1)
                {
                    suSendList_g.i16uTail++;
                }
                else
                {
                    suSendList_g.i16uTail = 0;
                }
                eRs485State_g = eRs485StateWaitSilence;
                TIM_CountDownReTrigger(RS485_STATE_TIMER, WAIT_SILENCE_TIME);
            }
        }   break;
        case eRs485StateWaitSilence:
            break;
        case eRs485StateReceive:
            break;
        case eRs485StateSend:
            if (ptSendBuffer_g)
            {
                // send request found
                UART_sendBuffer(MODGATE_RS485_UART, (INT8U *)ptSendBuffer_g, ptSendBuffer_g->i8uDataLen + RS485_HDRLEN + 1, &eSendState_g);

                eRs485State_g = eRs485StateWaitResponse;
                TIM_CountDownReTrigger(RS485_STATE_TIMER, WAIT_RESPONSE_TIME);
            }
            break;
        case eRs485StateWaitResponse:
            break;
        case eRs485StateSendResponse:
            if (GateProtocolCheckFrame(&bSendResponse) == bTRUE)
            {
                if (DispatchCmd(ptRecvBuffer_g, &suSendTelegram_s) == bTRUE)
                {
                    // Send response
                    if (ModGateRs485GetResponseState() || bSendResponse == bTRUE)
                    {
                        UART_sendBuffer(MODGATE_RS485_UART, (INT8U*)&suSendTelegram_s, suSendTelegram_s.i8uDataLen + RS485_HDRLEN + 1, &eSendState_g);
                    }
                    eRs485State_g = eRs485StateIdle;
                }
            }
            else
            {
                eRs485State_g = eRs485StateIdle;
            }
            break;
        case eRs485StateResponseAnalyze:
            // Receive of response frame is complete, check the answer
            ResponseAnalyze(ptRecvBuffer_g);
            eRs485State_g = eRs485StateIdle;
            break;
        case eRs485StateResponseDiscard:
            eRs485State_g = eRs485StateIdle;
            break;
        case eRs485StateReceiveResponse:
            break;
        }
        break;

    case eIoProtocol:
#ifdef PI_IO_PROTOCOL
        if (bNewFrameReceived_g)
        {
            bNewFrameReceived_g = bFALSE;

            PiSlaveAppIOReq(&sIoProtocolRecvFrame_g, &sIoProtocolSendFrame_g);

            sIoProtocolSendFrame_g.ai8uData[sIoProtocolSendFrame_g.uHeader.sHeaderTyp1.bitLength] =
                Crc8((INT8U*)&sIoProtocolSendFrame_g, sIoProtocolSendFrame_g.uHeader.sHeaderTyp1.bitLength + IOPROTOCOL_HEADER_LENGTH);

            UART_sendBuffer(MODGATE_RS485_UART, (INT8U*)&sIoProtocolSendFrame_g,
                sIoProtocolSendFrame_g.uHeader.sHeaderTyp1.bitLength + IOPROTOCOL_HEADER_LENGTH + 1, &eSendState_g);
        }
        if (eIoProtocolState_g == eIoProtocolTimeout)
        {
            PiSlaveAppHandleTimeout();
        }
#endif
        break;
    }
}

TBOOL ModGateRs485IsRunning(void)
{
    TBOOL ret = bTRUE;

    switch (eRs485Protocol_g)
    {
    case eGateProtocol:
    case eIoConfig:
        // we have no cyclic communication here, therefore we cannot detect a communication timeout
        break;
    case eIoProtocol:
        if (eIoProtocolState_g == eIoProtocolTimeout)
            ret = bFALSE;
        break;
    }

    return ret;
}

