/*=============================================================================================
*
*   application.c
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
#include <bsp/led/Led.h>

TBOOL bLedBlocked_g = bFALSE;

//+=============================================================================================
//|		Some BSP Functions
//+=============================================================================================


//*************************************************************************************************

void APPL_set_allLED( INT8U i8uSwitchLed_p)
{

    //to do... implement.

    //i8uWitchLed_p = 1 ------> set all GREEN!
    //i8uWitchLed_p = 2 ------> set all RED!
    bLedBlocked_g = bTRUE;
    LED_setLed(0, LED_ST_GREEN_OFF);
    LED_setLed(1, LED_ST_GREEN_OFF);
    LED_setLed(2, LED_ST_GREEN_OFF);
    switch(i8uSwitchLed_p)
    {
    case 1://set all green
        {
            LED_setLed(0, LED_ST_GREEN_ON);
            LED_setLed(1, LED_ST_GREEN_ON);
            LED_setLed(2, LED_ST_GREEN_ON);
        }
        break;
    case 2://set all red
        {
            LED_setLed(0, LED_ST_RED_ON);
            LED_setLed(1, LED_ST_RED_ON);
            LED_setLed(2, LED_ST_RED_ON);
        }
        break;
    default:
        {
            //do nothing
            break;
        }
    }//end switch
}
