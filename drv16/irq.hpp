/* $Id: irq.hpp,v 1.1 2000/04/23 14:55:16 ktk Exp $ */

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
 *  IRQ object definition.
 * @version %I%
 * @context
 *  Unless otherwise noted, all interfaces are Ring-0, 16-bit, kernel stack.
 *
 * @notes The IRQ object permits multiple interrupt handlers, all located
 *  within this driver, to share an interrupt level.  The IRQ object also
 *  handles the bookkeeping and handshaking associated with the kernel and
 *  with the PIC.  EOI is not issued to the PIC until a handler is found
 *  which services the interrupt.  If no handler indicates that the hardware
 *  interrupt has been serviced, then no EOI is issued and the return code
 *  back to the kernel indicates that the interrupt has not been serviced.
 *
 * @notes No public constructor is provided for this object.  Use the
 *  pMakeIRQ( irq_level ) friend function to obtain a pointer to the IRQ object
 *  of interest.
 *
 * @notes A client of this object should register its interrupt service
 *  routine by the AddHandler() method, and then enable the handler via
 *  EnableHandler.  The service routine must conform to the definition of a
 *  PFN_InterruptHandler, returning True if the interrupt was handled and
 *  False otherwise.  Reference PFN_InterruptHandler definition below.
 *
 * @history
 */

#ifndef IRQ_INCLUDED
#define IRQ_INCLUDED

#ifndef OS2_INCLUDED
#define INCL_NOPMAPI
#include <os2.h>
#endif

// clude <stddef.h>             //### NULL?  to be removed

// ###IHV:  IRQ sharing flag.  Set this to "0" for ISA bus, "1" for PCI
// PCI and Microchannel environments (where hardware can share an IRQ level.
const IRQ_HW_Sharing = 0;

// Max num of int handlers per IRQ level.
const MAX_HandlersPerIRQLevel = 3;

// PFN_InterruptHandler:  Pointer to an interrupt handler.  This is a near
// function which returns TRUE if it handled the interrupt, FALSE otherwise.
typedef BOOL __far __loadds __saveregs (*PFN_InterruptHandler) ( int irqnr );

class ISR_Entry {
public:
   BOOL bEnabled;                         // true <=> this handler is enabled.
   PFN_InterruptHandler pfnHandler;       // Handler within this driver.
};


class IRQ {

   // Use this globally scoped function to create an IRQ object
   // rather than calling a constructor.  This function ensures that
   // no more than one IRQ object is constructed per level.
   friend IRQ* pMakeIRQ( unsigned irq_level );
   friend IRQ* getIRQ( unsigned irq_level );

public:

   // Add an interrupt handler to the list.
   BOOL bAddHandler ( PFN_InterruptHandler pfn );

   // Enable the named handler.
   BOOL bEnableHandler ( PFN_InterruptHandler pfn );

   // Disables the named handler.
   BOOL bDisableHandler ( PFN_InterruptHandler pfn );

   // Removes the interrupt handler from the list.
   BOOL bRemoveHandler ( PFN_InterruptHandler pfn );

   // Receives interrupt from kernel, then calls out to registered handlers.
   // Runs in interrupt context.
   void CallHandlers (void);

private:
   // Data:

   ISR_Entry _handlerList[ MAX_HandlersPerIRQLevel + 1 ];
                            // List of handlers associated with this interrupt.
                            // Used slots are always contiguous starting from
                            // slot 0.  Null PFN entry inidicates free slot and end
                            // of list.  The last slot in the array is an extra slot,
                            // guaranteed to be NULL.
   USHORT _nEnabled;        // Number of handlers currently enabled.
   USHORT _usIRQLevel;      // The IRQ level itself

   // Methods:

   // Constructor.  Note that this is private; a client should use the
   // pMakeIRQ() friend function to obtain a pointer to an IRQ object.
   IRQ ( unsigned irq_level );

   // Destructor.
   ~IRQ ( void );

   // Find named handler in _handlerList[].
   ISR_Entry* _pFindHandler( PFN_InterruptHandler pfn );


};

/* Note: do not call AddHandler, RemoveHandler, in an interrupt thread.
         do not create or delete an IRQ object (or any object) in an interrupt thread
*/

#endif
