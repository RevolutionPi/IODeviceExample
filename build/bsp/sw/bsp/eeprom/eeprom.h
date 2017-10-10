//+=============================================================================================
//|
//!    \file eeprom.h
//|    EEprom emulation specific definitions and declarations.
//|
//+---------------------------------------------------------------------------------------------
//|
//|    File-ID:    $Id: eeprom.h 8194 2015-03-13 09:19:21Z fbrandauer $
//|    Location:   $URL: http://srv-kunbus03.de.pilz.local/feldbus/software/trunk/platform/bsp/sw/bsp/eeprom/eeprom.h $
//|    Copyright:  KUNBUS GmbH
//|
//+=============================================================================================

#ifndef EEPROM_H_INC
#define EEPROM_H_INC

//+=============================================================================================
//|		Typen / types
//+=============================================================================================

// structure for the default values for the EEPROM which are derived from a reference
typedef struct BSP_EEPROM_StrDefaultRef
{
    INT16U i16uEepromAddr;
    const void *vpSrc;
    INT16U i16uLen;
} BSP_EEPROM_TDefaultRef;

//+=============================================================================================
//|		Makros / macros
//+=============================================================================================

#define BSP_EEPROM_ADDR(var) ((INT16U)(INT32U)&(((BSP_EEPROM_TLayout *)0)->var))
#define BSP_EEPROM_SIZE(var) (sizeof (((BSP_EEPROM_TLayout *)0)->var))

//+=============================================================================================
//|		Include-Dateien / include files
//+=============================================================================================

#include <eepromLayout.h>    // project specific EEPROM Layout definition



#ifdef __cplusplus
extern "C" {
#endif

//+=============================================================================================
//|		Globale Variablen / global variables
//+=============================================================================================

#ifdef _WIN32   
// in Win32 BSP_EEPROM_TLayout cannot be const, instead we need a init-function
extern BSP_EEPROM_TLayout		BSP_EEPROM_ctDefaultValue;  
void BSP_EEPROM_ctDefaultValue_Init(void);
#else
extern const	BSP_EEPROM_TLayout		BSP_EEPROM_ctDefaultValue;
#endif
extern const	BSP_EEPROM_TDefaultRef	BSP_EEPROM_atDefaultRef[];

//+=============================================================================================
//|		Prototypen / prototypes
//+=============================================================================================
extern void		BSP_EEPROM_init(void);

extern INT8U	BSP_EEPROM_getDefaultData	(INT16U i16uVirtAddress_p);
extern void		BSP_EEPROM_factoryReset		(void);

extern INT8U	BSP_EEPROM_readByte			(INT16U i16uVirtAddress_p);
extern void		BSP_EEPROM_writeByte		(INT16U i16uVirtAddress_p, INT8U i8uData_p);

extern INT16U	BSP_EEPROM_readWord			(INT16U i16uVirtAddress_p);
extern void		BSP_EEPROM_writeWord		(INT16U i16uVirtAddress_p, INT16U i16uData_p);

extern INT32U	BSP_EEPROM_readDWord		(INT16U i16uVirtAddress_p);
extern void		BSP_EEPROM_writeDWord		(INT16U i16uVirtAddress_p, INT32U i32uData_p);


#ifdef __cplusplus
}
#endif


#endif /* EEPROM_H_INC */
