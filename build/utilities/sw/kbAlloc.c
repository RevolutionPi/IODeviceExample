/*=============================================================================================
*
* kbAlloc.c
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

#include <kbAlloc.h>
#ifdef __KUNBUSPI_KERNEL__
#include <linux/string.h>
#else
#include <string.h>
#endif
#include <bsp/globalIntCtrl/bspGlobalIntCtrl.h>
#include <stm32f2xx.h>

//*************************************************************************************************
#define kbUT_ARGM_ST_INVALID							1
#define kbUT_ARGM_ST_FREE                               2
#define kbUT_ARGM_ST_OCCUPIED                           3


#define kbUT_ARGM_OWN_SYSTEM                            0   //!< Default owner of a memory block

//*************************************************************************************************

typedef struct kbUT_StrHeap
{
    INT8U i8uState;                         //!< State of block; free or occupied
    INT8U i8uOwner;                         //!< Debug Marker. Who has allocated the block ?
    INT32U i32uLen;                         //!< Length of block
    struct kbUT_StrHeap *ptPrev;            //!< Previous descriptor structure
    struct kbUT_StrHeap *ptNext;            //!< Previous descriptor structure
} kbUT_THeap;

//*************************************************************************************************

static volatile INT32U i32uSyncCnt_s = 0;


//*************************************************************************************************
//| Function: kbUT_initHeap
//|
//! brief. Initialize the linked list of blocks in the heap.
//!
//!
//! ingroup.
//-------------------------------------------------------------------------------------------------
void *kbUT_initHeap (
    INT8U *pHeap,						//!< [in] pointer to heap memory
    INT32U i32uLen_p					//!< [in] length of the heap memory area
    )
                                        //! \return handle for memory partition; = NULL: error
{
    kbUT_THeap *ptStart_l;
    kbUT_THeap *ptEnd_l;

    ptStart_l = (kbUT_THeap *)pHeap;
    ptStart_l = (kbUT_THeap *)(((INT32U)ptStart_l+3) & 0xfffffffc); // Do 4 Byte Allignment

    ptEnd_l = (kbUT_THeap *)(pHeap +  i32uLen_p - sizeof (kbUT_THeap));
    ptEnd_l = (kbUT_THeap *)((INT32U)ptEnd_l & 0xfffffffc); // Do 4 Byte Allignment
    
    ptStart_l->ptPrev = (kbUT_THeap *)0;
    ptStart_l->ptNext = ptEnd_l;
    ptStart_l->i8uOwner = kbUT_ARGM_OWN_SYSTEM;
    ptStart_l->i32uLen = (INT32U)(((INT8U *)ptStart_l->ptNext - (INT8U *)ptStart_l) - sizeof (kbUT_THeap));
    ptStart_l->i8uState = kbUT_ARGM_ST_FREE;

    ptEnd_l->ptPrev = ptStart_l;
    ptEnd_l->ptNext = (kbUT_THeap *)0;
    ptEnd_l->i8uOwner = kbUT_ARGM_OWN_SYSTEM;
    ptEnd_l->i32uLen = i32uLen_p;
    ptEnd_l->i8uState = kbUT_ARGM_ST_OCCUPIED;

    return (void *)ptStart_l;
}    

//*************************************************************************************************
//| Function: kbUT_malloc
//|
//! brief. Allocate memory from the heap.
//!
//! the function is thread safe, that means it can be use in an interrupt routine. 
//! The function works in a preemptive multitasking system only correct, if at least the last thread 
//! that enters the function is not interrupted. Otherwise it could come to a dead lock.
//!
//! Due to the lack of a heap handle in kbUT_free, there is only a global synchronisation of all heaps
//! (via the static variable i32uSyncCnt_s). This could be improved by assigning each heap an own 
//! synchronization variable. 
//!
//!
//! ingroup.
//-------------------------------------------------------------------------------------------------

//#################################
INT32U DBG_i32uMaxMallocBlocks_g = 0;
//#################################


void *kbUT_malloc (
    void *pHandle_p,					//!< [in] handle of memory partition
    INT8U i8uOwner_p,                   //!< [in] for Debug purposes. Identifies the requestor
    INT32U i32uLen_p)					//!< [in] requested length of the memory area
                                        //! \return Pointer to memory; = NULL: out of mem
{
    #define kbUT_MIN_FREE_BLOCK_LEN   (10 + sizeof (kbUT_THeap))       

    kbUT_THeap *ptStart_l;
    kbUT_THeap *ptAct_l;
    kbUT_THeap *ptMarker_l;
    INT32U i32uLen_l;
    void *pvRv_l;
    INT32U i32uSyncCnt_l;
    
    //############################
    INT32U i32uCntMallocBlocks_l = 0;
    //############################

    i32uLen_l = (i32uLen_p + 3) & -4;   // round off to 4 Byte allignment
    pvRv_l = (void *)0;
    ptStart_l = (kbUT_THeap *)pHandle_p;
    

    do
    {   // try it as long as there are access overlaps in time 
    
        GLOBAL_INTERRUPT_DISABLE ();        // lock section

        i32uSyncCnt_l = ++i32uSyncCnt_s;
        
        GLOBAL_INTERRUPT_ENABLE ();         // unlock section               


                                             // check if chain is still integer
        for (ptAct_l = ptStart_l; ptAct_l && (i32uSyncCnt_l == i32uSyncCnt_s); ptAct_l = ptAct_l->ptNext)
        {
            //#############################
            i32uCntMallocBlocks_l++;
            //#############################

            if (   (ptAct_l->i8uState == kbUT_ARGM_ST_FREE)
                && (ptAct_l->i32uLen >= i32uLen_l)
               )
            {   // suitable block found
            
                GLOBAL_INTERRUPT_DISABLE ();        // lock section
                
                // check if this block is meanwhile not fetched by an interrupt routine
                if (i32uSyncCnt_l == i32uSyncCnt_s)
                {   // suitable block found
                    if ((ptAct_l->i32uLen - i32uLen_l) >= kbUT_MIN_FREE_BLOCK_LEN)
                    {   // it is worth to split the block
                        ptMarker_l = (kbUT_THeap *)((INT8U *)ptAct_l + i32uLen_l + sizeof (kbUT_THeap));
                        
                        ptMarker_l->ptNext = ptAct_l->ptNext;
                        ptAct_l->ptNext->ptPrev = ptMarker_l;
                        ptMarker_l->i32uLen = (INT32U)(((INT8U *)ptMarker_l->ptNext - (INT8U *)ptMarker_l) - sizeof (kbUT_THeap));
                        ptMarker_l->i8uOwner = kbUT_ARGM_OWN_SYSTEM;
                        ptMarker_l->i8uState = kbUT_ARGM_ST_FREE;
                       
                        ptAct_l->ptNext = ptMarker_l;

                        ptMarker_l->ptPrev = ptAct_l;
                        ptAct_l->i32uLen = (INT32U)(((INT8U *)ptAct_l->ptNext - (INT8U *)ptAct_l) - sizeof (kbUT_THeap));
                        // Falls ptMarker der vorletzte Block ist, schreibe die Größe in den letzten Block
                        // dann steht im letzten Block die kleinste Größe des noch freien Speichers, bzw. 0
                        if (ptMarker_l->ptNext && ptMarker_l->ptNext->ptNext == 0 && ptMarker_l->i32uLen < ptMarker_l->ptNext->i32uLen)
                            ptMarker_l->ptNext->i32uLen = ptMarker_l->i32uLen;
                    }
                    
                    ptAct_l->i8uOwner = i8uOwner_p;
                    ptAct_l->i8uState = kbUT_ARGM_ST_OCCUPIED;
                    pvRv_l = ptAct_l + 1;
                    if (ptAct_l->ptNext->ptNext == 0)	// last block was allocated, no memory left
                        ptAct_l->ptNext->i32uLen = 0;
                }
                
                GLOBAL_INTERRUPT_ENABLE (); // unlock section               
                break;
            }    
        }
    }   while (pvRv_l == NULL && i32uSyncCnt_l != i32uSyncCnt_s);  // do until we either found a block, or we are one cycle not was disturbed


    //#############################
    if (i32uCntMallocBlocks_l > DBG_i32uMaxMallocBlocks_g)
    {
        DBG_i32uMaxMallocBlocks_g = i32uCntMallocBlocks_l;
    }
    //#############################

    return (pvRv_l);

}
    
//*************************************************************************************************
//| Function: kbUT_calloc
//|
//! brief. allocates memory from the heap and sets it to zero.
//!
//! the function is thread safe, that means it can be used in an interrupt routine
//!
//!
//!
//! ingroup.
//-------------------------------------------------------------------------------------------------
void *kbUT_calloc (
    void *pHandle_p,					//!< [in] handle of memory partition
    INT8U i8uOwner_p,                   //!< [in] for Debug purposes. Identifies the requestor
    INT32U i32uLen_p)                   //!< [in] requested length of the memeory area
                                        //! \return Pointer to memory; = NULL: out of mem
{
    void *pvRv_l = kbUT_malloc (pHandle_p, i8uOwner_p, i32uLen_p);
    
    if (pvRv_l)
    {
        memset (pvRv_l, 0, i32uLen_p);
    }
    
    return (pvRv_l);
}

//*************************************************************************************************
//| Function: kbUT_free
//|
//! brief. frees a previously allocated memory block.
//!
//! the function is thread safe, that means it can be use in an interrupt routine
//!
//!
//!
//! ingroup.
//-------------------------------------------------------------------------------------------------
void kbUT_free (
    void *pvMem_p						//!< [in] pointer to memory block
    )
{
    kbUT_THeap *ptAct_l;
    kbUT_THeap *ptPrev_l;
    kbUT_THeap *ptNext_l;
    
    ptAct_l = ((kbUT_THeap *)pvMem_p) - 1;

    GLOBAL_INTERRUPT_DISABLE ();        // lock section

    i32uSyncCnt_s++;        // indicate a manipulation of the system

    // look if there is a previous free block
    ptPrev_l = ptAct_l->ptPrev; 
    if (   (ptPrev_l == NULL)
        || (ptPrev_l->i8uState != kbUT_ARGM_ST_FREE)
       )
    {
        ptPrev_l = ptAct_l;    
    }    
    
    // look if next block is free    
    ptNext_l = ptAct_l->ptNext;
    if (ptNext_l->i8uState == kbUT_ARGM_ST_FREE)
    {
        ptNext_l = ptNext_l->ptNext;
    }
    
    if (ptPrev_l->ptNext != ptNext_l)
    {   // combine all free blocks to one
    
        ptPrev_l->ptNext = ptNext_l;
        ptNext_l->ptPrev = ptPrev_l;
        
        ptPrev_l->i32uLen = (INT32U)(((INT8U *)ptPrev_l->ptNext - (INT8U *)ptPrev_l) - sizeof (kbUT_THeap));
    }
    
    ptPrev_l->i8uOwner = kbUT_ARGM_OWN_SYSTEM;
    ptPrev_l->i8uState = kbUT_ARGM_ST_FREE;
    GLOBAL_INTERRUPT_ENABLE (); // unlock section               
    
}



//*************************************************************************************************
//| Function: kbUT_minFree
//|
//! brief. return the minimal number of bytes that was left in the heap in one block.
//!
//!
//! ingroup.
//-------------------------------------------------------------------------------------------------
INT32U kbUT_minFree(void *pHandle_p)
{
    INT32U len = 0;
    kbUT_THeap *pt_l = (kbUT_THeap *)pHandle_p;
    while (pt_l->ptNext)
    {
        pt_l = pt_l->ptNext;
        len++;
    }
    return 1000000*len + pt_l->i32uLen;
}
