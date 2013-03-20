/* $Id: irq.cpp,v 1.3 2001/03/13 20:11:06 sandervl Exp $ */

/* SCCSID = %W% %E% */
/****************************************************************************
 *                                                                          *
 * Copyright (c) IBM Corporation 1994 - 1997.                               *
 *                                                                          *
 * The following IBM OS/2 source code is provided to you solely for the     *
 * the purpose of assisting you in your development of OS/2 device drivers. *
 * You may use this code in accordance with the IBM License Agreement       *
 * provided in the IBM Device Driver Source Kit for OS/2.                   *
 *                                                                          *
 ****************************************************************************/
/**@internal %W%
 *  IRQ object implementation.
 * @version %I%
 * @context
 *  Unless otherwise noted, all interfaces are Ring-0, 16-bit, kernel stack.
 * @notes
 *    Refer to .HPP file for interface rules.
 * @history
 */

#include <devhelp.h>
#include <include.h>
#include <string.h>                    // memset()
#include "irq.hpp"                     // Object definition

const int NumIrqLevels = 16;


// We need a global array to keep track of the IRQ objects that have been created.
// The interrupt-time ISR uses this array to find the object associated with
// interrupt level "i".  IRQ "i" is at position "i" in array.

static IRQ* pIrqObject[ NumIrqLevels ] =  { 0 };

// These are the ISR's that take the interrupt from the kernel.  We've
// defined handlers only for the IRQ's supported by this sound card.
void far ISR03( void )   { pIrqObject[ 3]->CallHandlers(); }
void far ISR04( void )   { pIrqObject[ 4]->CallHandlers(); }
void far ISR05( void )   { pIrqObject[ 5]->CallHandlers(); }
void far ISR07( void )   { pIrqObject[ 7]->CallHandlers(); }
void far ISR09( void )   { pIrqObject[ 9]->CallHandlers(); }
void far ISR10( void )   { pIrqObject[10]->CallHandlers(); }
void far ISR11( void )   { pIrqObject[11]->CallHandlers(); }
void far ISR12( void )   { pIrqObject[12]->CallHandlers(); }
void far ISR13( void )   { pIrqObject[13]->CallHandlers(); }
void far ISR14( void )   { pIrqObject[14]->CallHandlers(); }
void far ISR15( void )   { pIrqObject[15]->CallHandlers(); }

// List of handlers here.
typedef void (NEAR *pfnISR)(void);
static pfnISR pISR[NumIrqLevels] = {
   NULL,
   NULL,
   NULL,
   (pfnISR) ISR03,
   (pfnISR) ISR04,
   (pfnISR) ISR05,
   NULL,
   (pfnISR) ISR07,
   NULL,
   (pfnISR) ISR09,
   (pfnISR) ISR10,
   (pfnISR) ISR11,
   (pfnISR) ISR12,
   (pfnISR) ISR13,
   (pfnISR) ISR14,
   (pfnISR) ISR15
};


/**@external pMakeIRQ
 *  Returns a pointer to the IRQ object which manages the IRQ for a given
 *  level.
 * @param unsigned irq_level - the irq level of interest.
 * @return IRQ* pIRQ - pointer to the IRQ object for named level.
 * @notes This routine makes sure that we make exactly one IRQ for an
 *  interrupt level.
 */
IRQ* pMakeIRQ( unsigned irq_level )
{
   if ( ! pIrqObject[irq_level] )
      new IRQ( irq_level );
   return pIrqObject[ irq_level ];
}


IRQ* getIRQ( unsigned irq_level )
{
   if ( ! pIrqObject[irq_level] )
      return 0;
   return pIrqObject[ irq_level ];
}

/**@internal IRQ::IRQ
 *  Constructor for an IRQ object.
 * @notes This method should never be called by a client, the pMakeIRQ()
 *  function should be used instead.  pMakeIRQ ensures that we have no
 *  more than one IRQ object per level.
 */
IRQ::IRQ( unsigned irq_level )
{
   memset( (PVOID) this, 0, sizeof(IRQ) );     // Set all data to 0.
   _usIRQLevel = irq_level;
   pIrqObject[ _usIRQLevel ] = this;
}


IRQ::~IRQ(void)
{
   ISR_Entry* pEntry;       // Pointer into array of int handlers for this IRQ.

   // Remove object from global array.
   pIrqObject[ _usIRQLevel ] = NULL;

   // Remove all registered handlers.  Quit when find NULL
   // entry.  Array is guaranteed to be terminated with a NULL entry.
   for ( pEntry=_handlerList; pEntry->pfnHandler; ++pEntry )
      bRemoveHandler( pEntry->pfnHandler );
}


BOOL IRQ::bAddHandler( PFN_InterruptHandler pfn )
{
   ISR_Entry* pEntry;       // Pointer into array of int handlers for this IRQ.

   // If the handler is already registered on this level, done.
   if ( _pFindHandler( pfn ) )
      return TRUE;

   pEntry = _pFindHandler( NULL );     // Find an empty slot.
   if (! pEntry) return FALSE;         // None available.

   // Set up new slot.  No serialization needed, entry is disabled.
   pEntry->bEnabled = FALSE;           // Sanity check - ensure entry is disabled.
   pEntry->pfnHandler = pfn;           // Save pointer to handler.
   return TRUE;                        // Done.
}


BOOL IRQ::bEnableHandler ( PFN_InterruptHandler pfn )
{
   ISR_Entry* pEntry;       // Pointer into array of int handlers for this IRQ.

   pEntry = _pFindHandler( pfn );
   if (! pEntry) return FALSE;          // Can't find it.

   if ( pEntry->bEnabled )              // Already running.
      return TRUE;

   pEntry->bEnabled = TRUE;             // Enable handler.
   ++ _nEnabled;                        // Bump count of active handlers.

   // If we just changed from none enabled to 1 enabled, turn on Interrupt.
   // SvL: TODO: DOES NOT WORK FOR ISA CARDS!!!!!!!!!! (shared irq)
   if ( _nEnabled == 1 ) {
      USHORT uResult =
         DevHelp_SetIRQ( (NPFN) pISR[ _usIRQLevel ],
                         _usIRQLevel,
                         1 );   // first try shared shared
      if (uResult != 0) {                    // If error ...
         uResult = DevHelp_SetIRQ( (NPFN) pISR[ _usIRQLevel ],
                                  _usIRQLevel,
                                  0 );   // failed, so try exclusive instead

      	 if (uResult != 0) {                    // If error ...
         	-- _nEnabled;
         	pEntry->bEnabled = FALSE;
         	return FALSE;
	 }
      }
   }

   return TRUE;
}

BOOL IRQ::bDisableHandler ( PFN_InterruptHandler pfn )
{
   ISR_Entry* pEntry;       // Pointer into array of int handlers for this IRQ.

   pEntry = _pFindHandler( pfn );
   if (! pEntry) return FALSE;         // Can't find it.

   if (! pEntry->bEnabled )            // Already disabled.
      return TRUE;

   pEntry->bEnabled = FALSE;           // Disable handler.
   -- _nEnabled;                       // Bump down count of active handlers.

   // If we just changed from 1 enabled to 0 enabled, turn off Interrupt.
   if ( _nEnabled == 0 )
      DevHelp_UnSetIRQ( _usIRQLevel );

   return TRUE;
}



BOOL IRQ::bRemoveHandler( PFN_InterruptHandler pfn )
{
   ISR_Entry* pEntry;       // Pointer into array of int handlers for this IRQ.
   ISR_Entry* pLastEntry;   // Points to last handler entry in array.

   pEntry = _pFindHandler( pfn );
   if (! pEntry) return FALSE;
   if (! bDisableHandler( pfn )) return FALSE;

   /* If this is the last entry in the array, just zero the slot.  If this
   ;  entry is in the middle of the array, then move the last entry in the
   ;  list into this slot that is being freed.  We guarantee that the ISR's
   ;  are contiguous in the array and cannot put a NULL entry in the middle.
   */

   // Find last entry in the array.
   for ( pLastEntry=pEntry; pLastEntry->pfnHandler; ++pLastEntry )
      ;
   --pLastEntry;

   // If 'pfn' wasn't at end of the list, move end of list into this slot.
   if ( pLastEntry != pEntry) {
      // If we don't cli/sti here, we could lose an interrupt.
      cli();
      *pEntry = *pLastEntry;           // Copy down entry.
      pLastEntry->bEnabled = 0;        // Disable last slot.
      pLastEntry->pfnHandler = NULL;   // Zero out last slot.
      sti();
   } else {                            // Else the entry selected for deletion is the
                                       // last entry in list;  no need to move anything.
      pEntry->bEnabled = FALSE;        // Disable this slot (redundant to DisableHandler).
      pEntry->pfnHandler = NULL;       // Zero out the slot.
   }
   return TRUE;
}


ISR_Entry* IRQ::_pFindHandler( PFN_InterruptHandler pfn )
/*
;  Finds named handler in __handlerList[].  Returns pointer to ISR_Entry
;  if found, NULL otherwise.
*/
{
   ISR_Entry* pEntry;       // Pointer into array of int handlers for this IRQ.

   // Walk array, looking for named handler function.
   for ( pEntry=_handlerList;
         pEntry != &_handlerList[MAX_HandlersPerIRQLevel]; ++pEntry )
      if ( pfn == pEntry->pfnHandler )
         return pEntry;

   // If we got here, we didn't find the entry.  Return NULL.
   return NULL;
}


void IRQ::CallHandlers ( void )
/*
;  Receives interrupt from ISR, then calls out to registered handlers.
;  Runs in interrupt context.
*/
{
   USHORT i;                // Indexes through ISR's in _handlerList.
   USHORT retry;            // Retry count.

   for (retry=0; retry < _nEnabled; ++retry) {
      // Call all enabled handlers.  Stop when find NULL entry (end of list).
      for ( i=0; _handlerList[i].pfnHandler; ++i ) {
         if ( _handlerList[i].bEnabled ) {      // If handler enabled...
            if (_handlerList[i].pfnHandler(_usIRQLevel)) {  // Call handler.
               cli();
               // We've cleared all service requests.  Send EOI and clear
               // the carry flag (tells OS/2 kernel that Int was handled).
               DevHelp_EOI( _usIRQLevel );
               clc();
               sti();
               return;
            }
         }
      }
   }
   // Indicate Interrupt not serviced by setting carry flag before
   // returning to OS/2 kernel.  OS/2 will then shut down the interrupt!
   // NOTE: Make sure interrupts are not turned on again when this irq isn't ours!
   stc();
}


