/*=============================================================================================
*
* systick.c
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

#include <bsp/systick/systick.h>
#include <bsp/led/Led.h>


//+=============================================================================================
//|		Konstanten / constants
//+=============================================================================================

//+=============================================================================================
//|		Variablen / variables
//+=============================================================================================

static INT32U i32uCounter_s = 0;

#if defined (LED_POLLHANDLER)
extern void LED_pollHandler(void);
#endif

//+=============================================================================================
//|     Function:   SysTickHandler
//+---------------------------------------------------------------------------------------------
//!     Handles System Timer Interrupts.
//+---------------------------------------------------------------------------------------------
//|     Conditions:
//!     \pre    (pre-condition)
//!     \post   (post-condition)
//+---------------------------------------------------------------------------------------------
//|     Annotations:
//+=============================================================================================
void SysTickHandler (
                     void
                     )
{
    // Increment tick count
    i32uCounter_s++;

#if defined (STM_WITH_LED)
    LED_systick();
#endif

#if defined (LED_POLLHANDLER)
    if(0 == i32uCounter_s % 10)
    {
        LED_pollHandler();
    }
#endif
}

//*************************************************************************************************
//| Function: GetTickCount
//|
//! fetches actual System Ticks
//!
//! The System Tick is incremented every millisecond. This function fetches the actual count.
//!
//! ATTENTION: Timer is Re-Triggered in IRQ; DO NOT USE BUFFERED VALUES OF TIMER !
//! ingroup.  bspApi
//-------------------------------------------------------------------------------------------------
INT32U kbGetTickCount ( void )
{
  return (i32uCounter_s);    
}    



//*************************************************************************************************
