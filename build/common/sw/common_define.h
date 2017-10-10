/*=============================================================================================
*
* common_define.h
* This file defines global data types and constants as well as other global definitions.
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
#ifndef _COMMON_DEFINE_H_
#define _COMMON_DEFINE_H_

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

#define _32BIT_U_CONTROLLER_
#undef  _MSC_VER

#define KB_DEFAULT_CFG          __attribute__ ((section(".defaultCfg"),aligned(4)))
#define KB_DEFAULT_CFG_CRC      __attribute__ ((section(".defaultCfgCrc")))             
#define KB_STORE_PARA           __attribute__ ((section(".storePara"),aligned(4)))
#define KB_STORE_PARA_CRC       __attribute__ ((section(".storeParaCrc")))
#define KB_SCRIPT_FLASH         __attribute__ ((section(".scriptFlash"),aligned(4)))
#define KB_SCRIPT_FLASH_LEN     __attribute__ ((section(".scriptFlashLen"),aligned(4)))
#define KB_CCM_RAM              __attribute__ ((section(".ccmRam"),aligned(4)))    //  closely coupled RAM on STM32F40x Processors

#define msleep(m)               // not used on mGate
#define pr_err(fmt, ...)        // not used on mGate
#define pr_info(fmt, ...)       // not used on mGate
#define pr_info_spi(fmt, ...)   // not used on mGate
#define pr_info_spi2(fmt, ...)  // not used on mGate
#define pr_info_modgate(fmt, ...)  // not used on mGate
#define dump_stack()            // not used on mGate


/*
#define COPY_8_P(dest, src) ((INT32U *)(void *)(dest))[0] = ((INT32U *)(void *)(src))[0], \
((INT32U *)(void *)(dest))[1] = ((INT32U *)(void *)(src))[1]
*/

#define COPY_8_P(dest, src) ((INT8U *)(dest))[0] = ((INT8U *)(src))[0], \
  ((INT8U *)(dest))[1] = ((INT8U *)(src))[1], \
  ((INT8U *)(dest))[2] = ((INT8U *)(src))[2], \
  ((INT8U *)(dest))[3] = ((INT8U *)(src))[3], \
  ((INT8U *)(dest))[4] = ((INT8U *)(src))[4], \
  ((INT8U *)(dest))[5] = ((INT8U *)(src))[5], \
  ((INT8U *)(dest))[6] = ((INT8U *)(src))[6], \
  ((INT8U *)(dest))[7] = ((INT8U *)(src))[7]


//+=============================================================================================
//|    Typen / types
//+=============================================================================================

//__GNUC__
#if defined (_MSC_VER)
typedef unsigned char      INT8U;    ///< 8 Bit unsigned integer
typedef signed char        INT8S;    ///< 8 Bit signed integer
typedef unsigned __int16   INT16U;   ///< 16 Bit unsigned integer
typedef signed __int16     INT16S;   ///< 16 Bit signed integer
typedef unsigned __int32   INT32U;   ///< 32 Bit unsigned integer
typedef signed __int32     INT32S;   ///< 32 Bit signed integer
typedef unsigned __int64   INT64U;   ///< 64 Bit unsigned integer
typedef signed __int64     INT64S;   ///< 64 Bit signed integer
typedef float              FLOAT32;  //< 32 Bit signed floating point
typedef double             FLOAT64;  //< 64 Bit signed floating point
#ifdef __cplusplus
typedef bool               TBOOL;
#else
typedef unsigned __int8    TBOOL;    ///< Boolean value (bTRUE/bFALSE)
#endif
typedef unsigned char      CHAR8U;   ///< 8 Bit unsigned character
typedef signed char        CHAR8S;   ///< 8 Bit signed character
typedef char               CHAR8;    ///< 8 Bit character

#define  __attribute__(x)

#elif defined (_8BIT_U_CONTROLLER_)
#define  _8BIT_U_CONTROLLER_TYPES_
#error "no types defined yet!"

#elif defined (_16BIT_U_CONTROLLER_)
#define  _16BIT_U_CONTROLLER_TYPES_
#error "no types defined yet!"

#elif defined (_32BIT_U_CONTROLLER_)
#define _32BIT_U_CONTROLLER_TYPES_
// 32 bit u-controller is used

// Static Types
typedef unsigned char       INT8U;       ///< 8 Bit unsigned integer
typedef signed char         INT8S;       ///< 8 Bit signed integer
typedef unsigned short      INT16U;      ///< 16 Bit unsigned integer
typedef signed short        INT16S;      ///< 16 Bit signed integer
typedef unsigned int        INT32U;      ///< 32 Bit unsigned integer
typedef signed int          INT32S;      ///< 32 Bit signed integer
typedef unsigned long long  INT64U;      ///< 64 Bit unsigned integer
typedef signed long long    INT64S;      ///< 64 Bit signed integer
typedef float               FLOAT32;     ///< 32 Bit signed floating point
typedef double              FLOAT64;     ///< 64 Bit signed floating point
typedef unsigned char       TBOOL;       ///< Boolean value (bTRUE/bFALSE)
typedef unsigned char       CHAR8U;      ///< 8 Bit unsigned character
typedef signed char         CHAR8S;      ///< 8 Bit signed character
typedef char                CHAR8;       ///< 8 Bit character

typedef unsigned short      uint16_t;      ///< 16 Bit unsigned integer

#else
#error "Current architecture is not supported by BSP/CommonDefine"
#endif//_32BIT_U_CONTROLLER_TYPES_

#ifdef __cplusplus
#define bTRUE      true
#define bFALSE     false
#else
#define bTRUE      ((TBOOL)1)
#define bFALSE    ((TBOOL)0)
#endif
#define ROMCONST__         const
#define NOP__          _nop_

#ifndef NULL
#ifdef __cplusplus
#define NULL      0
#define NULL_PTR  0
#else
#define NULL    ((void *)0)
#define NULL_PTR  ((void *)0)
#endif
#endif

//+=============================================================================================
//|    Konstanten / constant data
//+=============================================================================================

typedef struct S_KUNBUS_REV_NUMBER
{
  INT8U  IDENT1[9];    ///< identification String 1 "KB_SW_REV"
  INT8U  SW_MAJOR;     ///< major revision number; valid numbers 0-50, other numbers reserved
  INT16U  SW_MINOR;    ///< minor revision number; valid numbers 0-1000, other numbers reserved
  INT32U  REVISION;    ///< SVN revision number (mainly for internal use);
  ///< valid numbers 0-999999, other numbers reserved; typicalli SVN rev.
} T_KUNBUS_REV_NUMBER;  ///< Kunbus internal revision and release number



#define KUNBUS_FW_DESCR_TYP_MG_CAN_OPEN                            71
#define KUNBUS_FW_DESCR_TYP_MG_CCLINK                              72
#define KUNBUS_FW_DESCR_TYP_MG_DEV_NET                             73
#define KUNBUS_FW_DESCR_TYP_MG_ETHERCAT                            74
#define KUNBUS_FW_DESCR_TYP_MG_ETHERNET_IP                         75
#define KUNBUS_FW_DESCR_TYP_MG_POWERLINK                           76
#define KUNBUS_FW_DESCR_TYP_MG_PROFIBUS                            77
#define KUNBUS_FW_DESCR_TYP_MG_PROFINET_RT                         78
#define KUNBUS_FW_DESCR_TYP_MG_PROFINET_IRT                        79
#define KUNBUS_FW_DESCR_TYP_MG_CAN_OPEN_MASTER                     80
#define KUNBUS_FW_DESCR_TYP_MG_SERCOS3                             81
#define KUNBUS_FW_DESCR_TYP_MG_SERIAL                              82
#define KUNBUS_FW_DESCR_TYP_MG_PROFINET_SITARA                     83
#define KUNBUS_FW_DESCR_TYP_MG_PROFINET_IRT_MASTER                 84
#define KUNBUS_FW_DESCR_TYP_MG_ETHERCAT_MASTER                     85
#define KUNBUS_FW_DESCR_TYP_MG_MODBUS_RTU                          92
#define KUNBUS_FW_DESCR_TYP_MG_MODBUS_TCP                          93
#define KUNBUS_FW_DESCR_TYP_PI_CORE                                95
#define KUNBUS_FW_DESCR_TYP_PI_DIO_14                              96
#define KUNBUS_FW_DESCR_TYP_PI_DI_16                               97
#define KUNBUS_FW_DESCR_TYP_PI_DO_16                               98
#define KUNBUS_FW_DESCR_TYP_PI_AIO                                103


#define KUNBUS_FW_DESCR_TYP_INTERN                                  0xffff
#define KUNBUS_FW_DESCR_TYP_UNDEFINED                               0xffff

#define KUNBUS_FW_DESCR_MAC_ADDR_LEN                  6     //!< number of bytes in a MAC Address


// defines for field i32uBootFlags in T_KUNBUS_APPL_DESCR
#define KUNBUS_APPL_DESCR_BOOT_FACTORY_RESET                    0x00000001  // if this bit is set in the default appl descr, a factory reset is performed on the first boot after a firmware update



//-- defines for product variants -----------------------------------------------------------------
#define VARIANT_PILZ        1       // Pilz Variant of a Product
#define VARIANT_KUNBUS      2       // KUNBUS Variant of a Product

#ifdef VARIANT_PRODUCT
    #if !(  (VARIANT_PRODUCT == VARIANT_PILZ)           \
          ||(VARIANT_PRODUCT == VARIANT_KUNBUS)         \
         )
        #error "VARIANT_PRODUCT with unregistered VARIANT used"
    #endif
#endif


typedef
#include <COMP_packBegin.h>
struct S_KUNBUS_FW_DESCR
{
  INT32U  i32uLength;                   ///< number of bytes in struct, used to determine which elements are present
  ///  must be always the first member of the struct
  INT32U  i32uSerialNumber;             ///< 32 Bit serial number
  INT16U  i16uDeviceType;               ///< Kunbus internal, unambiguous device type
  INT16U  i16uHwRevision;               ///< Revision of hardware, used for firmware update
  INT8U   ai8uMacAddr[KUNBUS_FW_DESCR_MAC_ADDR_LEN];  ///< Field for manufacturer set MAC Address
  INT32U  i32uFwuEntryAddr;             //!< Entry of Firmwareupdate from application
  INT32U  i32uApplStartAddr;            //!< Startaddress of application specific flash area
  INT32U  i32uApplEndAddr;              //!< Last address of application specific flash area
}                                       ///< Kunbus internal option bytes
#include <COMP_packEnd.h>
T_KUNBUS_FW_DESCR;


#define KUNBUS_APPL_BOOT_FLAG_FACTORY_RESET     0x00000001

typedef
#include <COMP_packBegin.h>
struct S_KUNBUS_APPL_DESCR_V1
{
  INT32U i32uLength;                    //!< number of bytes in struct, used to determine which elements are present
  INT32U i32uVectorAddr;                //!< address of vector table.
  INT32U i32uCrcCheckStartAddr;         //!< first address of application area for crc check
  INT32U i32uCrcCheckEndAddr;           //!< last address of application area for crc check
  INT32U i32uCrcAddr;                   //!< address of CRC Checksum over application area
  INT8U  ai8uIdent1[9];                 ///< identification String 1 "KB_SW_REV"
  INT8U  i8uSwMajor;                    ///< major revision number; valid numbers 0-50, other numbers reserved
  INT16U i16uSwMinor;                   ///< minor revision number; valid numbers 0-1000, other numbers reserved
  INT32U i32uSvnRevision;               ///< SVN revision number (mainly for internal use);
}
#include <COMP_packEnd.h>
T_KUNBUS_APPL_DESCR_V1;

typedef
#include <COMP_packBegin.h>
struct S_KUNBUS_APPL_DESCR
{
    INT32U i32uLength;                    //!< number of bytes in struct, used to determine which elements are present
    INT32U i32uVectorAddr;                //!< address of vector table.
    INT32U i32uCrcCheckStartAddr;         //!< first address of application area for crc check
    INT32U i32uCrcCheckEndAddr;           //!< last address of application area for crc check
    INT32U i32uCrcAddr;                   //!< address of CRC Checksum over application area
    INT8U  ai8uIdent1[9];                 ///< identification String 1 "KB_SW_REV"
    INT8U  i8uSwMajor;                    ///< major revision number; valid numbers 0-50, other numbers reserved
    INT16U i16uSwMinor;                   ///< minor revision number; valid numbers 0-1000, other numbers reserved
    INT32U i32uSvnRevision;               ///< SVN revision number (mainly for internal use);
    INT32U i32uBootFlags;                 ///< Boot action flags
}
#include <COMP_packEnd.h>
T_KUNBUS_APPL_DESCR;

typedef
#include <COMP_packBegin.h>
struct S_KUNBUS_CNFG_DATA_HDR
{
  INT8U  ai8uIdent[4];                  ///< identification String 1 "KBCD"
  INT32U i32uCrc;                       ///< address of CRC Checksum over application area
  INT32U i32uLength;                    ///< number of bytes stored directly after this header
  INT16U i16uDeviceType;                ///< Kunbus internal, unambiguous device type
  INT16U i16uHwRevision;                ///< Revision of hardware
  INT8U  i8uSwMajor;                    ///< major revision number; valid numbers 0-50, other numbers reserved
  INT8U  ai8uDummy[3];                  ///< padding
}
#include <COMP_packEnd.h>
T_KUNBUS_CNFG_DATA_HDR;


#define SER_WR_SUCCESS              (0xF0)  ///< Serial number successful written
#define SER_WR_FAIL_ILLEGAL_NUM     (0xF1)  ///< Illegal / Invalid Serial number
#define SER_WR_FAIL_ALREADY_SET     (0xF2)  ///< Serial number already set; it MUST not be changed !

//+=============================================================================================
//|    Prototypen / prototypes
//+=============================================================================================

#ifdef __cplusplus
extern "C" {
#endif

extern const T_KUNBUS_APPL_DESCR ctKunbusApplDescription_g;
extern const T_KUNBUS_FW_DESCR ctKunbusFirmwareDescription_g;
extern char *sKunbusFWDescription_g;

#ifdef __cplusplus
}
#endif


#else   // _COMMON_DEFINE_H_
//  #pragma message "_COMMON_DEFINE_H_ already defined !"
#endif   // _COMMON_DEFINE_H_
