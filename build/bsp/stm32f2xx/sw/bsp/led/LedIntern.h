//+=============================================================================================
//|
//!        \file LedIntern.h
//!
//!        Internal led definitions.
//|
//+---------------------------------------------------------------------------------------------
//|
//|        File-ID:        $Id: LedIntern.h 7394 2014-11-05 08:12:32Z reusch $
//|        Location:       $URL: http://srv-kunbus03.de.pilz.local/feldbus/software/trunk/platform/bsp/stm32f2xx/sw/bsp/led/LedIntern.h $
//|        Company:        $Cpn: KUNBUS GmbH $
//|
//+---------------------------------------------------------------------------------------------
//|
//|        Files required:    (none)
//|
//+=============================================================================================
#ifndef LED_INTERN_H_INC
#define LED_INTERN_H_INC


//+=============================================================================================
//|        Include-Dateien / include files
//+=============================================================================================

//+=============================================================================================
//|        Globale Variablen / global variables
//+=============================================================================================

//+=============================================================================================
//|        Konstanten / constants
//+=============================================================================================

#ifdef STM_LED_GPIO
#if (LED_MAX_DUAL_LED < 1 || LED_MAX_DUAL_LED > 12)
    #error "LED_MAX_DUAL_LED out of range [1..12]"
#endif    
#else
#if defined (LED_MAX_DUAL_LED)
#if LED_MAX_DUAL_LED > 16
#error "maximum 16 LEDs allowed this time ..."
#endif
#else
#define LED_MAX_DUAL_LED    8
#endif
#endif

//+=============================================================================================
//|        Typen / types
//+=============================================================================================

typedef struct LED_SLed
{
    LED_EState enLedState;              ///< state as it was set by LED_setLed()
    INT16U i16uCode;                    ///< Flash mode: Determines (together with current mask) whether LED will be set or reset
    INT16U i16uMask;                    ///< Flash mode: Determines (together with code) whether LED is set or reset; shifts left from 1 upto MaxMask
    INT16U i16uMaxMask;                 ///< Flash mode: Mask is reset to 1, when reaching MaxMask
#if defined (STM_LED_GPIO)
    GPIO_TypeDef *ptGPIO;
  #if defined (STM32F2XX)
    __IO uint16_t *pi16uSetReg;         ///< GPIO Port register for set
    __IO uint16_t *pi16uResetReg;       ///< GPIO Port register for reset
  #else
    __IO uint32_t *pi16uSetReg;         ///< GPIO Port register for set
    __IO uint32_t *pi16uResetReg;       ///< GPIO Port register for reset
  #endif
#endif
    INT16U	i16uLedBit;                  ///< LED Port Bit for set/reset
    INT8U	i8uPreScale;                  ///< Flash-mode: Prescaler count down
    INT8U	i8uMaxPreScale;               ///< Flash-mode: Prescaler for flash-mode:  Update time interval (50ms) is multiplied by this value
    TBOOL	bBlink;                       ///< = bTRUE: LED is blinking, = bFALSE: LED is static on or off
    
}   LED_TLed;

//+=============================================================================================
//|        Makros / macros
//+=============================================================================================

//+=============================================================================================
//|        Prototypen / prototypes
//+=============================================================================================
#ifdef __cplusplus
extern "C" { 
#endif 

extern LED_TLed LED_atLed_g[];


#ifdef __cplusplus
}    
#endif 


#endif //LED_INTERN_H_INC 

