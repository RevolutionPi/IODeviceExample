/*=============================================================================================
*
* uart_intern.h
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
#ifndef _UART_H_
#define	_UART_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <common_define.h>

//+=============================================================================================
//|		Include-Dateien / include files
//+=============================================================================================

//+=============================================================================================
//|		Konstanten / constants
//+=============================================================================================

//#define 

//+=============================================================================================
//|		Typen / types
//+=============================================================================================
typedef void (*HW_UART_CB)(void);		    ///< Callback definition

//! Configuration structure for the UART driver
typedef struct s_UART_CONFIGURATION
{
	INT32U				bautrate;		///< desired bitrate
	HW_UART_CB			receive_cb;		///< Receive callback
	HW_UART_CB			transmit_cb;	///< Transmit callback
	HW_UART_CB			error_cb;		///< Error callback
	HW_UART_CB			finish_cb;		///< Transfer callback
} HW_UART_CONFIGURATION;		///< SPI configuration struct

//+=============================================================================================
//|		Makros / macros
//+=============================================================================================


//+=============================================================================================
//|		Prototypen / prototypes
//+=============================================================================================

///	\defgroup	kb_uart_handlers    kb_uart_handlers
/// @{
extern	void	uart_put (INT8U ch);
extern	void	uart_put_string (INT8U *string);
extern  void	uart_put_itoa (INT32U   value);
extern  INT8U	uart_init ( HW_UART_CONFIGURATION *conf );
extern	void	UART_intUart (void);
/// @}
// Internal Functions
void UART_uart1IntHandler (void);
void UART_uart2IntHandler (void);
void UART_uart3IntHandler (void);
#if defined (STM32F2XX)
void UART_uart4IntHandler (void);
void UART_uart5IntHandler (void);
void UART_uart6IntHandler (void);
#endif

#ifdef __cplusplus
}
#endif

//+=============================================================================================
//|		Aenderungsjournal
//+=============================================================================================
#ifdef __DOCGEN
/*!
@page revisions Revisions

*/
#endif


#endif//_UART_H_
