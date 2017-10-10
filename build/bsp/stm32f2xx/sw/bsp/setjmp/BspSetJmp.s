/*=============================================================================================
//|
//|    Topic
//|
//+---------------------------------------------------------------------------------------------
//|
//|    File-ID:    $Id: $
//|    Location:   $URL: $
//|    Copyright:  KUNBUS GmbH
//|
//+=============================================================================================
*/


/*************************************************************************************************/
    .syntax unified
    .cpu cortex-m3
    .fpu softvfp
    .eabi_attribute 20, 1
    .eabi_attribute 21, 1
    .eabi_attribute 23, 3
    .eabi_attribute 24, 1
    .eabi_attribute 25, 1
    .eabi_attribute 26, 1
    .eabi_attribute 30, 6
    .eabi_attribute 18, 4
    .thumb
    .text
    .cfi_sections	.debug_frame

/*************************************************************************************************/
    .extern BSP_OS_ptTcbHighestReady_g              
    .extern BSP_OS_ptTcbCurrentActive_g              

    .equ    REGSTR_OFF_MSP       , 0 
    .equ    REGSTR_OFF_PSP       , 4 

    .equ    STKF_OFF_R0          ,  0
    .equ    STKF_OFF_R1          ,  4 
    .equ    STKF_OFF_R2          ,  8 
    .equ    STKF_OFF_R3          , 12 
    .equ    STKF_OFF_R12         , 16 
    .equ    STKF_OFF_LR          , 20 
    .equ    STKF_OFF_PC          , 24 
    .equ    STKF_OFF_PSR         , 28 

/**************************************************************************************************
//| Function: bspSetJmp 
//!
//!  INT32S bspSetJmp (
//!        BSP_TJumpBuf tJmpBuf_p)      //!< [out] Buffer for 12 values 0f 32 bit width to store the context
//!
//! \brief
//!
//! \detailed
//!
//!
//!
//! \ingroup
//------------------------------------------------------------------------------------------------*/
    .text
    .align	2
    .global	bspSetJmp
    .thumb
    .thumb_func
    .type	bspSetJmp, %function

bspSetJmp:
    .cfi_startproc

    mrs     r2, msp
    mrs     r3, psp
    stmia	r0!, {r2, r3, r4, r5, r6, r7, r8, r9, sl, fp, ip, lr}
    
    movw    r0,#0              @ return value 0
    movt    r0,#0

    bx      lr 	
    
    .cfi_endproc

/**************************************************************************************************
//| Function: bspLongJmp 
//|
//! void	bspLongJmp(
//!     BSP_TJumpBuf tJmpBuf_p,   //!< [in] Buffer for 12 values 0f 32 bit width to restore the context
//!     INT32S i32sValue_p)      //!< [in] Return Value for the setjmp function
//!
//! \brief
//!
//! \detailed
//!
//!
//!
//! \ingroup
//------------------------------------------------------------------------------------------------*/
    .text
    .align	2
    .global	bspLongJmp
    .thumb
    .thumb_func
    .type	bspLongJmp, %function

bspLongJmp:
    .cfi_startproc

    ldmia	r0!, {r2, r3, r4, r5, r6, r7, r8, r9, sl, fp, ip, lr}
    msr msp, r2
    msr psp, r3
    
    mov r0, r1    @ return i32sValue_p
    
    bx r14
    
    .cfi_endproc


/*************************************************************************************************/
