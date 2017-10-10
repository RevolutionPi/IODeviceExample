/*=============================================================================================
*
*   PiDio.h
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

#ifndef PIDIO_H_INC
#define PIDIO_H_INC

#include <IoProtocol.h>

typedef enum
{
    PIDIO_ERROR_NO_ERROR,
    PIDIO_ERROR_INPUT,
    PIDIO_ERROR_OUTPUT,
    PIDIO_ERROR_CONFIG,
    PIDIO_ERROR_CRC,
} PIDIO_ERROR;

typedef enum
{
    PIDIO_MODULTYPE_PIIDO14IO = 0, // 14 Inputs und Outputs
    PIDIO_MODULTYPE_PIIDO16O  = 1, // 16 Outputs
    PIDIO_MODULTYPE_PIIDO16I  = 2, // 16 Inputs
} PIDIO_MODULETYPE;

#ifdef __cplusplus
extern "C" {
#endif

#ifdef  __cplusplus 
} 
#endif 

PIDIO_ERROR PiDioSetConfig(SDioConfig *pDioConfig_p);

PIDIO_ERROR PiDioReadInput(INT16U *pi16uInputValue_p);
PIDIO_ERROR PiDioWriteOutput(INT16U i16uOutputValue_p);

PIDIO_ERROR PiDioGetOutputStatus(INT16U *pi16uOutputStatus_p);
PIDIO_ERROR PiDioGetModuleStatus(SDioModuleStatus *psDioModuleStatus_p);

void PiDioSetStateLed();
void PiDioInitSpi();
void PiDioInitGpios();
#endif //PIDIO_H_INC
