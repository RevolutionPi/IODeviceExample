/*=============================================================================================
*
* PiBridgeSlave.c
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
#include <stdarg.h>

#include <project.h>

#include <common_define.h>
#include <kbUtilities.h>
#include <ModGateRS485.h>
#include <PiSlaveApplication.h>
#include <IoProtocol.h>
#include <PiBridgeSlave.h>
#include <bsp/led/Led.h>
#include <bsp/timer/timer.h>

#define MASTER_PRESENT_TIME 10
#define MASTER_PRESENT_TIME_MIN  7500   // for count up timer with 1탎 resolution
#define MASTER_PRESENT_TIME_MAX  9500   // for count up timer with 1탎 resolution
#define NEXT_CONFIG_TIME 10
#define END_CONFIG_TIME 3000


static INT32U tReset, tResetMin, tResetMax;

void PiBridgeSlaveInit(BSP_TJumpBuf *ptExceptionPoint_p,
    void(*cbErrHandler_p)(INT32U i32uErrorCode_p, TBOOL bFatal_p, INT8U i8uParaCnt_p, va_list argptr_p))
    //!< [in] Pointer to application specific error handler
    //! \return Error Code, defined in "bspError.h"    
{
    PiSlaveAppInit(ptExceptionPoint_p, cbErrHandler_p);
    ModGateRs485Init();
    ModGateRs485SetResponseState(bTRUE);
    tResetMin = 1000000;
    tResetMax = 0;
}

void PiBridgeSlaveRun()
{
    static E_PIBRIDGESLAVE_STATUS eRunStatus_s = PIBRIDGESLAVE_ST_INIT;
    static PISLAVEAPPLICATION_STATE eState_s = PISLAVEAPPLICATION_STATE_INIT;
    static TBOOL bEntering_s = bTRUE;
    static kbUT_Timer tTimeoutTimer_s;
    static kbUT_Timer tNextConfigTimer_s;
    static INT8U i8uSniff1bState_s = 0;
    static ERs485Protocol eRs485ProtocolBeforeReset;
    static INT8U sniff2, lastSniff2;

    ModGateRs485Run();

    switch (eRunStatus_s)
    {
    case PIBRIDGESLAVE_ST_INIT: // Do some initializations and go to next state
        if (bEntering_s)
        {
            bEntering_s = bFALSE;
            // configure PiBridge Sniff lines as input
            PIBS_WriteSniff1A(bTRUE);
            PIBS_WriteSniff1B(bTRUE);
            PIBS_WriteSniff2(bFALSE);
        }
        eRunStatus_s = PIBRIDGESLAVE_ST_APPLICATION_RUN;
        bEntering_s = bTRUE;
        break;
        // *****************************************************************************************

    case PIBRIDGESLAVE_ST_APPLICATION_RUN:
        if (bEntering_s)
        {
            bEntering_s = bFALSE;
            PIBS_WriteSniff1B(bTRUE);
            lastSniff2 = PIBS_ReadSniff2();
        }
        sniff2 = PIBS_ReadSniff2();
        if (lastSniff2 == 0 && sniff2 == 1)
        {
            eRunStatus_s = PIBRIDGESLAVE_ST_CONFIG_INIT;
            bEntering_s = bTRUE;
            eRs485ProtocolBeforeReset = ModGateRs485GetProtocol();
        }
        else
        {
#ifndef PI_MGATE_MODULE
            if (bConfigurationComplete_g)
#endif
            {
                eState_s = PISLAVEAPPLICATION_STATE_RUNNING;
                PiSlaveAppRun();
            }
        }
        lastSniff2 = sniff2;
        break;
        // *****************************************************************************************


    case PIBRIDGESLAVE_ST_CONFIG_INIT:
        if (bEntering_s)
        {
            eState_s = PISLAVEAPPLICATION_STATE_INIT;
            bEntering_s = bFALSE;
            ModGateRs485SetResponseState(bFALSE);

            // use a count up timer instead of tTimeoutTimer_s, because tTimeoutTimer_s is not accurate enough
            BSP_count_up_timer_init(1);     // timer is counting in 1탎 steps
            BSP_count_up_timer_start();     // start timer with 0
        }
        sniff2 = PIBS_ReadSniff2();
        tReset = BSP_count_up_timer_get();  // get 탎 since BSP_count_up_timer_start()
        if (sniff2 == 0)
        {
            if (tReset < MASTER_PRESENT_TIME_MIN)
            {
                // input 2 turned back to 0 before the timer ran out. 
                // This was not a valid signal for the initialization sequence
                // -> go back to RUN state
                eRunStatus_s = PIBRIDGESLAVE_ST_APPLICATION_RUN;
                bEntering_s = bTRUE;
            }
            else if (tReset <= MASTER_PRESENT_TIME_MAX)
            {
                // the reset pulse had the correct len, wait for the config messages
                eRunStatus_s = PIBRIDGESLAVE_ST_WAIT_FOR_CONFIG;
                bEntering_s = bTRUE;
                ModGateRs485SetProtocol(eIoConfig);
            }
            else
            {
                // the reset pulse was too long -> ignore it and go back to run
                eRunStatus_s = PIBRIDGESLAVE_ST_APPLICATION_RUN;
                bEntering_s = bTRUE;
            }
            tResetMin = (tReset < tResetMin) ? tReset : tResetMin;
            tResetMax = (tReset > tResetMax) ? tReset : tResetMax;
        }
        else 
        {
            if (tReset > MASTER_PRESENT_TIME_MAX)
            {
                // the reset pulse was too long -> ignore it and go back to run
                eRunStatus_s = PIBRIDGESLAVE_ST_APPLICATION_RUN;
                bEntering_s = bTRUE;
            }
            else
            {
                // call PiSlaveAppRun(), the normal work must be done until the reset pulse was detected successfully
#ifndef PI_MGATE_MODULE
                if (bConfigurationComplete_g)
#endif
                {
                    eState_s = PISLAVEAPPLICATION_STATE_RUNNING;
                    PiSlaveAppRun();
                }
            }
        }
        break;
        // *****************************************************************************************


    case PIBRIDGESLAVE_ST_WAIT_FOR_CONFIG:
        if (bEntering_s)
        {
            // wait 10 ms before pin 2 is set high
            bEntering_s = bFALSE;
            kbUT_TimerStart(&tTimeoutTimer_s, MASTER_PRESENT_TIME);
            i8uSniff1bState_s = PIBS_ReadSniff1B();
        }
        if (kbUT_TimerExpired(&tTimeoutTimer_s))
        {
            eRunStatus_s = PIBRIDGESLAVE_ST_WAIT_FOR_CONFIG2;
            bEntering_s = bTRUE;
        }
        break;

    case PIBRIDGESLAVE_ST_WAIT_FOR_CONFIG2:
        if (bEntering_s)
        {
            bEntering_s = bFALSE;
            PIBS_WriteSniff2(bTRUE);
            kbUT_TimerStart(&tTimeoutTimer_s, END_CONFIG_TIME);
        }
        if (PIBS_ReadSniff1A() == 0)
        {
            // 1a changed from 1 to 0 --> right module
            eRunStatus_s = PIBRIDGESLAVE_ST_CONFIG_MODULE_RIGHT_N;
            bEntering_s = bTRUE;
            bConfigurationComplete_g = bFALSE;
            i8uOwnAddress_g = 0;
            ModGateRs485SetResponseState(bTRUE);
        }
        else
        {
            if (i8uSniff1bState_s && PIBS_ReadSniff1B() == 0)
            {
                // 1b changed from 0 to 1 to 0 --> left module
                eRunStatus_s = PIBRIDGESLAVE_ST_CONFIG_MODULE_LEFT_N;
                bEntering_s = bTRUE;
                bConfigurationComplete_g = bFALSE;
                i8uOwnAddress_g = 0;
                ModGateRs485SetResponseState(bTRUE);
            }
        }
        if (eRunStatus_s == PIBRIDGESLAVE_ST_WAIT_FOR_CONFIG2 && kbUT_TimerExpired(&tTimeoutTimer_s))
        {
            eRunStatus_s = PIBRIDGESLAVE_ST_APPLICATION_RUN;
            bEntering_s = bTRUE;
            // configuration did not continue -> go back to old protocol
            ModGateRs485SetProtocol(eRs485ProtocolBeforeReset);
            PIBS_WriteSniff2(bFALSE);
        }
        break;
        // *****************************************************************************************

    case PIBRIDGESLAVE_ST_CONFIG_MODULE_RIGHT_N:
        if (bEntering_s)
        {
            bEntering_s = bFALSE;
            kbUT_TimerStart(&tTimeoutTimer_s, END_CONFIG_TIME);
        }
        if (bConfigurationComplete_g)
        {
            // Configuration complete
            ModGateRs485SetResponseState(bFALSE);
            eRunStatus_s = PIBRIDGESLAVE_ST_WAIT_FOR_END_CONFIG_RIGHT;
            bEntering_s = bTRUE;
        }
        if (kbUT_TimerExpired(&tTimeoutTimer_s))
        {
            eRunStatus_s = PIBRIDGESLAVE_ST_FATAL_CONFIG_ERROR;
            bEntering_s = bTRUE;
        }
        break;
        // *****************************************************************************************


    case PIBRIDGESLAVE_ST_CONFIG_MODULE_LEFT_N:
        if (bEntering_s)
        {
            bEntering_s = bFALSE;
            kbUT_TimerStart(&tTimeoutTimer_s, END_CONFIG_TIME);
        }
        if (bConfigurationComplete_g)
        {
            // Configuration complete
            ModGateRs485SetResponseState(bFALSE);
            eRunStatus_s = PIBRIDGESLAVE_ST_WAIT_FOR_END_CONFIG_LEFT;
            bEntering_s = bTRUE;
        }
        if (kbUT_TimerExpired(&tTimeoutTimer_s))
        {
            eRunStatus_s = PIBRIDGESLAVE_ST_FATAL_CONFIG_ERROR;
            bEntering_s = bTRUE;
        }
        break;
        // *****************************************************************************************


    case PIBRIDGESLAVE_ST_WAIT_FOR_END_CONFIG_RIGHT:
        if (bEntering_s)
        {
            bEntering_s = bFALSE;
            kbUT_TimerStart(&tNextConfigTimer_s, NEXT_CONFIG_TIME);
            kbUT_TimerStart(&tTimeoutTimer_s, END_CONFIG_TIME);
        }
        if (kbUT_TimerExpired(&tNextConfigTimer_s))
        {
            PIBS_WriteSniff1B(bFALSE);
            PIBS_WriteSniff2(bFALSE);
        }
        if (kbUT_TimerExpired(&tTimeoutTimer_s))
        {
            eRunStatus_s = PIBRIDGESLAVE_ST_FATAL_CONFIG_ERROR;
            bEntering_s = bTRUE;
        }
        else if (PIBS_ReadSniff2() == 0)
        {
            eRunStatus_s = PIBRIDGESLAVE_ST_APPLICATION_RUN;
            bEntering_s = bTRUE;
        }
        break;
        // *****************************************************************************************


    case PIBRIDGESLAVE_ST_WAIT_FOR_END_CONFIG_LEFT:
        if (bEntering_s)
        {
            bEntering_s = bFALSE;
            kbUT_TimerStart(&tNextConfigTimer_s, NEXT_CONFIG_TIME);
            kbUT_TimerStart(&tTimeoutTimer_s, END_CONFIG_TIME);
        }
        if (kbUT_TimerExpired(&tNextConfigTimer_s))
        {
            PIBS_WriteSniff1A(bFALSE);
            PIBS_WriteSniff2(bFALSE);
        }
        if (kbUT_TimerExpired(&tTimeoutTimer_s))
        {
            eRunStatus_s = PIBRIDGESLAVE_ST_FATAL_CONFIG_ERROR;
            bEntering_s = bTRUE;
        }
        else if (PIBS_ReadSniff2() == 0)
        {
            PIBS_WriteSniff1A(bTRUE);
            eRunStatus_s = PIBRIDGESLAVE_ST_APPLICATION_RUN;
            bEntering_s = bTRUE;
        }
        break;
        // *****************************************************************************************


    case PIBRIDGESLAVE_ST_FATAL_CONFIG_ERROR:
        eState_s = PISLAVEAPPLICATION_STATE_TIMEOUT;
        if (bEntering_s)
        {
            bEntering_s = bFALSE;
            // configure PiBridge Sniff lines as input
            PIBS_WriteSniff1A(bTRUE);
            PIBS_WriteSniff1B(bTRUE);
            PIBS_WriteSniff2(bFALSE);
        }
        if (PIBS_ReadSniff2() == 0)
        {
            eRunStatus_s = PIBRIDGESLAVE_ST_FATAL_CONFIG_ERROR2;
            bEntering_s = bTRUE;
        }
        break;
        // *****************************************************************************************


    case PIBRIDGESLAVE_ST_FATAL_CONFIG_ERROR2:
        if (bEntering_s)
        {
            bEntering_s = bFALSE;
        }
        if (PIBS_ReadSniff2() == 1)
        {
            eRunStatus_s = PIBRIDGESLAVE_ST_CONFIG_INIT;
            bEntering_s = bTRUE;
            ModGateRs485SetProtocol(eIoConfig);
        }
        break;
        // *****************************************************************************************


    default:
        eState_s = PISLAVEAPPLICATION_STATE_UNKNOWN_ERROR;
        eRunStatus_s = PIBRIDGESLAVE_ST_FATAL_CONFIG_ERROR;

    }


    if (ModGateRs485IsRunning() == bFALSE && eState_s == PISLAVEAPPLICATION_STATE_RUNNING)
    {
        eState_s = PISLAVEAPPLICATION_STATE_TIMEOUT;
    }

    PiSlaveAppSetStateLed(eState_s);
}
