/* $Id: timer.hpp,v 1.1 2000/04/23 14:55:21 ktk Exp $ */

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
 *  Timer object definition.
 * @version %I%
 * @context
 *  Unless otherwise noted, all interfaces are Ring-0, 16-bit, kernel stack.
 * @notes
 * @history
 */

#ifndef TIMER_INCLUDED
#define TIMER_INCLUDED

#ifndef OS2_INCLUDED
#define INCL_NOPMAPI
#include <os2.h>
#endif

#include "irq.hpp"                     // Object definition.
#include "queue.hpp"                   // Object definition.

extern PQUEUEHEAD pTimerList;          // timer.cpp:  List of all instantiated timers.


// Define some states for a timer object.
enum {  TIMER_Disabled = 0,            // Object &/or hardware not initialized.
        TIMER_Stopped,                 // Object initialized & ready to run, but
                                       // interrupt & timer clock _not_ running.
        TIMER_Running };               // Interrupt & timer clock running.

class TIMER : public QUEUEELEMENT {
public:

   // Constructor.
   TIMER ( IRQ* pIRQ, USHORT uTargetMSec, ULONG ulStreamType );

   // No destructor -- object never destroyed.

   // Query state of timer.  TIMER_Disabled inidicates internal failure.
   const int eState ( void );

   // Start, Stop, Pause, Resume functions.  Define these to be consistent
   // with the AUDIOHW class (although we aren't subclassed from AUDIOHW).
   // Each function implements the appropraite verb on a Timer object.
   virtual int Start ( void );         // Start the operation
   virtual int Stop ( void );          // Stop the operation
   virtual int Pause ( void );         // Pause the operation
   virtual int Resume ( void );        // Resume the operation

   // Get current TIMER time (in milliseconds).
   ULONG ulGetTime ( void );

   // Set current TIMER time (in milliseconds).
   void vSetTime ( ULONG ulTime );

   // Schedule the next Context hook invocation, 0 for next tick.
   // If specified time has already passed when vSchedule() called,
   // Ctx hook will be set on next tick (will not return immed. into
   // routine).
   VOID vSchedule ( ULONG ulTime );

private:
   // Private member functions

   // Internal workers for Start + Resume, Stop + Pause
   int _iStart ( void );
   int _iStop ( void );

   // Returns TRUE if any Timer is in the TIMER_Running state.
   static BOOL _isAnyRunning ( void );

   // Start the generation of HW timer ticks on the chip.
   static VOID _vStartHWTicks ( void );

   // Start the generation of HW timer ticks on the chip.
   static VOID _vStopHWTicks ( void );

   // Handle tick.  Runs in Interrupt context.  Calls _vPerTickTasks().
   static BOOL __far __loadds __saveregs _vTimerHook ( void );

   // Perform the per tick, per timer object tasks.
   VOID TIMER::_vPerTickTasks( void );

private:
   // Static data.  All instantiated timers will use same interrupt mechanism,
   // and these members are shared (hense, static) across all Timer objects.
   static IRQ* _pIRQ;                  // IRQ object for the timer.
   static int _eTechnology;            // Timer technololgy being used, see enumeration above.
   static USHORT _usTimerCount;        // Value to write into the chip's countdown timer.
   static USHORT _uInterval_mSec;      // Expected interval between ticks, milliseconds.
   static USHORT _uIntervalErr_uSec;   // uSec error between requested uTargetMSec interval
                                       // and acutal _uInterval_mSec.

   // Data - unique per timer instance, but doesn't change after construction.
   ULONG _ulStreamType;                // Type of stream associated with this timer.
                                       // Ref def'ns in stream.hpp.
   ULONG _ctxHookHandle;               // Handle for this context hook.

   // Data - dynamic.
   ULONG  _ulTime;                     // Current time in milliSec (reflects pauses, etc).
   ULONG  _ulSchedTime;                // Time to arm next context hook.
                                       // "0" if we should arm hook on next tick.
   USHORT _uCumulativeError;           // Accumulated error, in uSec, of the
                                       // elapsed time that we're reporting.
                                       // Always in range of 0..1000 (1 mSec).
   int _eState;                        // Internal Timer state, see enum above.
};

#endif  // TIMER_INCLUDED
