/* $Id: timer.cpp,v 1.3 2001/04/30 21:07:59 sandervl Exp $ */

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
 *  TIMER object implementation.
 * @version %I%
 * @context
 *  Unless otherwise noted, all interfaces are Ring-0, 16-bit, kernel stack.
 *  Many of these functions run in interrupt context, this is noted in each
 *  function header.
 * @notes Suggestions for future work
 *  1.)  Should have a diagnostic in the constructor to ensure the Timer works.
 *  2.)  Should respect the Clock Select on the CS 4232.
 * @history
 */

#include <devhelp.h>
#include <include.h>
#include <string.h>                    // memset()
#include "parse.h"                     // fNoHWTimer
#include "irq.hpp"                     // Object definition.
#include "timer.hpp"                   // Object definition.
#include "midistrm.hpp"                // Object definition.


// Enumerate the technologies from which we can generate a timer tick.
enum {  TIMER_TechNotIdentified = 0,   // Haven't yet identified the technology.
        TIMER_AdapterTimerInt,         // Adapter onboard HW timer interrupt.
        TIMER_SysTimer };              // OS/2 system timer (31 msec).


// Force use of System timer.
int fNoHWTimer;                        // Set in parse.c when "/O:NoHWTimer"
                                       // is included on DEVICE= config.sys line.

// Provide a global reference so interrupt handler can find all Timers.
PQUEUEHEAD pTimerList;


// Definitions for static data members of the Timer class.  Ref timer.hpp
// for comments on each vbl.  All these are initialized by TIMER::TIMER.
IRQ*   TIMER::_pIRQ = 0;
int    TIMER::_eTechnology = 0;
USHORT TIMER::_usTimerCount;
USHORT TIMER::_uInterval_mSec;
USHORT TIMER::_uIntervalErr_uSec;


// Countdown timer resolution of the CS4232, in nano-seconds.
//### The countdown timer resolution is 9.969 uSec when C2SL = 0,
//### and 9.92 uSec when C2SL = 1.  The C2SL can change on the fly,
//### depending on what the Wave objects are doing.
//### We pick a value that is about 1/2 way between the two possible
//### actual values; introduces about a 0.25% error in actual time rates,
//### or 1 second off after 400 seconds of play.
//### It might be better to periodically read and respect the current
//### value of the C2SL select register on the fly (more overhead, but
//### more accurate timing)
const ULONG ulCS4232_Timer_nSec = 9945;


/**@internal TIMER::_vTimerHook
 *  Handle timer tick.  Runs in Interrupt context.
 * @param None.
 * @return BOOL - TRUE if interrupt handled, FALSE otherwise.
 * @notes First checks if this interrupt was intended for us.  If using
 *  interrupt from adapter, we share this Int with Wave.  Then, if our
 *  interrupt, walks the AudioHW list looking for all Timers, and gives
 *  each instantiated timer object an opportunity to perform tasks.
 */
static BOOL __far __loadds __saveregs TIMER::_vTimerHook(void)
{
   TIMER* pTimer;                      // Used to find Timers in AudioHW list.

   // Check if this is our interrupt.

   // Walk through all running timers, perform the per-tick services.
   pTimer = (TIMER*) pTimerList->Head();
   while (pTimer != NULL) {
      if ( pTimer->eState() == TIMER_Running )
         pTimer->_vPerTickTasks();
      pTimer = (TIMER*) pTimer->pNext;
   }

   return TRUE;
}


/**@internal TIMER__iCtxHook
 *  Context hook for timer object.  Used to call the Process() method
 *  on the currently active MIDI stream.
 * @param (register EAX) - Data provided by the timer interrupt handler
 *  when the context hook is armed.  The interrupt handler should have
 *  supplied a valid stream type (ref. stream.hpp).
 * @return int 0 - per DevHelp_* definitions on Context Hook operation.
 * @notes The Context hook function is called by the OS/2 kernel out of a
 *  ring0, task context, _not_ in interrupt context.  The function is called
 *  as result of the context hook being armed.  The timer interrupt handler
 *  arms the context hook in interrupt context; this routine handles it (in
 *  task context).
 * @notes We apply the Process() method only to the 1st stream that we find;
 *  that is, we don't search for more than one active MPU or FMSYNTH stream.
 */
int __far __pascal TIMER__iCtxHook( void )
{
   ULONG ulStreamType;
   MIDISTREAM* pStream;

   // The OS/2 kernel supplies us with the Ctx Hook data in the EAX
   // register.  We take advantage of this, by stuffing the stream
   // type into EAX when we arm the contex hook.  Here, we make the
   // dangerous assumption here that the compiler has not yet clobbered
   // EAX.  The _EAX() function is a pragma that we define in include.h.

   ulStreamType = _EAX();
   pStream = (MIDISTREAM*) FindActiveStream( ulStreamType );

   if (pStream)   // Should always be a valid fn adr when timer is running.
      pStream->Process();

   return 0;
}

/**@external TIMER::TIMER
 *  Constructor for TIMER object.
 * @param IRQ* pIRQ - pointer to IRQ object, NULL if none.
 * @param USHORT uTargetMSec - target resolution in milliseconds.
 * @param ULONG ulStreamType - type of MIDISTREAM that this Timer is
 *  associated with.  This param controls the stream lookup when the
 *  timer goes off.
 * @notes Constructs timer based on interrupt if possible, otherwise
 *  uses 31.25 mSec kernel timer.  The timer interrupt handler is a
 *  static function:  no matter how many Timer objects are defined, they
 *  all use the same timer interrupt handler.  The interrupt handler
 *  makes a callout to process the unique instance information of all
 *  Timer objects in existance.
 * @notes New Timer instance is added to the global AUDIOHW object list
 *  as part of its construction.
 * @notes Timer is left in TIMER_Stopped state on good creation,
 *  TIMER_Disabled state on problem.
 */
#pragma off (unreferenced)
TIMER::TIMER( IRQ* pIRQ, USHORT uTargetMSec, ULONG ulStreamType ) :
   _ulStreamType ( ulStreamType )
#pragma on (unreferenced)
{
   USHORT rc;
   static ULONG hookHandle;

   // Set initial state - "not functional".
   _eState = TIMER_Disabled;

   // Setup the new context hook.
   rc = DevHelp_AllocateCtxHook( (NPFN) TIMER__iCtxHook, &hookHandle );
   if (!rc)
      _ctxHookHandle = hookHandle;
   else {
      return;
   }

   // If this is the first Timer we've created, do the "first time only"
   // work, such as selecting the interrupt technology & programming the
   // timer interrupt.

   if (TIMER::_eTechnology != 0)       // Will be 0 if 1st time through.
      _eState = TIMER_Stopped;         // Not the first Timer.
#if 0
   else {                              // First time through, set up static vbls.
      // Try using timer feature on the audio adapter.  The fNoHWTimer
      // flag is set from the DEVICE= config.sys linne, and can be used to
      // force system timer.
      if (! fNoHWTimer) {

         _pIRQ = pIRQ;
         if ( _pIRQ )
            bGoodReturn = _pIRQ->bAddHandler( TIMER::_vTimerHook );
         else
            bGoodReturn = FALSE;

         // We got an interrupt slot, now figure out the values for HW timer setup.
         if ( bGoodReturn )
         {
            TIMER::_usTimerCount =
               (USHORT) (((ULONG) uTargetMSec * 1000000L) / ulCS4232_Timer_nSec);

            // Figure out what this timer count equates to in mSec and uSec.
            ULONG ulInterval_nSec = (((ULONG) _usTimerCount) * ulCS4232_Timer_nSec);
            _uInterval_mSec = ulInterval_nSec / 1000000L;
            _uIntervalErr_uSec =
               (ulInterval_nSec - (_uInterval_mSec * 1000000L)) / 1000;
                                 // Should always be positive, in range of 0..1000.

            // All set to enable timer interrupt on the chip.  Log status.
            //### Would be very good to check that it works, so we know to go
            //### to alternate strategy if necessary.
            _eTechnology = TIMER_AdapterTimerInt;
            _eState = TIMER_Stopped;
         }
      }
#endif

      // Timer IRQ hook didn't work for some reason, use system timer.
      if ( _eState == TIMER_Disabled ) {
         _eTechnology = TIMER_SysTimer;
         _uInterval_mSec = 31;
         _uIntervalErr_uSec = 250;
         _eState = TIMER_Stopped;
      }
//   }  // End of setup for interrupt operation, executed for 1st Timer only.

   // If good creation, add Timer to global timer list & reset all time vbls.
   if ( _eState != TIMER_Disabled ) {
      _ulTime = 0;
      _ulSchedTime = 0;
      _uCumulativeError = 0;
      pTimerList->PushOnTail( this );
   }

   // TIMER is in TIMER_Stopped state upon normal return, TIMER_Disabled
   // state on error.
}


/**@external TIMER::Start
 *  Start the timer.
 * @param None
 * @return int
 * @notes Maps operation to _iStart (Start and Resume are identical).
 */
int TIMER::Start( void )
{
   return _iStart();
}


/**@external TIMER::Stop
 *  Stop the operation of the timer.
 * @param None.
 * @return int 0.
 * @notes Maps operation to _iStop (Stop and Pause are identical).
 */
virtual int TIMER::Stop( void )
{
   return _iStop();
}


/**@external TIMER::Pause
 *  Pause the operation of the timer.
 * @param None.
 * @return int 0.
 * @notes Maps operation to _iStop (Stop and Pause are identical).
 */
virtual int TIMER::Pause( void )
{
   return _iStop();
}


/**@external TIMER::Resume
 *  Resume the operation of the timer interrupt.
 * @param None.
 * @return int 0.
 * @notes Maps operation to _iStart (Start and Resume are identical).
 */
virtual int TIMER::Resume( void )
{
   return _iStart();
}


/**@external TIMER::vSchedule
 *  Schedule the next Context hook invocation, 0 for next tick.
 * @param ULONG ulTime - Absolute time (mSec) at which to schedule next Ctx hook.
 * @return VOID
 * @notes
 */
VOID TIMER::vSchedule ( ULONG ulTime )
{
   cli();
   _ulSchedTime = ulTime;
   sti();
}


// Get current stream time (milliseconds).  Runs in task context.
ULONG TIMER::ulGetTime( void )
{
   ULONG ulResult;                     // Return value.

   cli();
   ulResult = _ulTime;
   sti();
   return ulResult;
}

// Set current stream time (milliseconds).  Runs in task context.
VOID TIMER::vSetTime( ULONG ulTime )
{
   cli();
   _ulTime = ulTime;
   sti();
}


/**@external TIMER::eState
 *  Query state of timer.  TIMER_Disabled inidicates internal failure.
 * @param None.
 * @return int Timer state, as enumerated in .hpp file.
 */
const int TIMER::eState ( void ) { return _eState; };


/**@internal TIMER::_isAnyRunning()
 *  Predicate that determines whether any Timer is in Running state.
 * @param None.
 * @return BOOL TRUE iff at least one Timer is in Running state.
 * @notes Normally called in Task context by Start and Stop routines.
 */
static BOOL TIMER::_isAnyRunning( void )
{
   TIMER* pTimer;                      // Used to find Timers in AudioHW list.

   // Walk through all Timers, seeking one that is running.
   pTimer = (TIMER*) pTimerList->Head();
   while (pTimer != NULL) {
      if ( pTimer->eState() == TIMER_Running )
         return TRUE;
      pTimer = (TIMER*) pTimer->pNext;
   }
   return FALSE;
}


/**@internal TIMER::_iStart
 *  Internal worker to start the timer operation.  If we have a HW timer,
 *  will start the interrupt generation.  If kernel timer, start it.
 * @param None
 * @return int 1 (Boolean TRUE) if timer starts properly
 * @return int 0 (Boolean FALSE) if problem starting timer
 * @notes Will force Ctx hook to be scheduled on next tick.
 */
int TIMER::_iStart( void )
{
   USHORT rc;

   // Disable any any pre-existing timer and reset state variables.
   if ( _eState != TIMER_Stopped )
      Stop();                          // Stop() method on this Timer.

   // _eState now equals TIMER_Stopped.

   // Reset Timer vbls & start Timer interrupt if it's not already running.
   // MMPM/2 will reset the stream time when the user rewinds, etc.

   _ulSchedTime = 0;                   // Force arming ctx hook on next tick.
   _uCumulativeError = 0;              // Zero out any fractional time.

   if ( TIMER::_isAnyRunning() )       // If timer hardware already running
      _eState = TIMER_Running;         // Then just flip our state.
   else {                              // Otherwise start interrupts.
      switch ( _eTechnology ) {
#if 0
      case TIMER_AdapterTimerInt:
         bGoodReturn = _pIRQ->bEnableHandler( TIMER::_vTimerHook );
           /*** Our ISR can be called at any point after this, so all our state
            * variables must be consistent.  Even though we haven't enabled
            * the Timer on the chip, we could get an Int from Wave operations
            * (which should be ignored).
            */
         // Everything is setup for the interrupt, now enable it on the chip.
         if (bGoodReturn) {
            _vStartHWTicks();
            _eState = TIMER_Running;
         }
         break;
#endif
      case TIMER_SysTimer:
         rc = DevHelp_SetTimer( (NPFN) TIMER::_vTimerHook );
         if (! rc)
            _eState = TIMER_Running;
         else
         break;
      }
   }

   if ( _eState != TIMER_Running ) {   // Set error condition & log problem.
      _eState = TIMER_Disabled;
   }

   return ( _eState == TIMER_Running );
}


/**@internal TIMER::_iStop
 *  Internal worker to shutdown the timer interrupt and the timer clock.
 * @param None.
 * @return int 0.
 * @notes Will not destroy "next scheduled ctx hook" information.
 * @notes Leaves timer in TIMER_Stopped state.  Timer interrupts are stopped
 *  as well if no other Timer is running.
 */
int TIMER::_iStop()
{
   _eState = TIMER_Stopped;            // Stop this Timer.
   if (! TIMER::_isAnyRunning()) {     // If no other Timers are running...
      switch( _eTechnology ) {         // Then shutdown the interrupt.
      case TIMER_AdapterTimerInt:
         // Disable the interrupt on the chip and in the IRQ object.
         _vStopHWTicks();
         break;
      case TIMER_SysTimer:
         DevHelp_ResetTimer( (NPFN) TIMER::_vTimerHook );
         break;
      }
   }
   return 0;
}


// Perform the per tick, per timer object tasks:  maintain time,
// arm a context hook if it's time to run the MIDI parser.  Runs
// in interrupt context.
VOID TIMER::_vPerTickTasks( void )
{
   USHORT uMSec;

   if ( _eState == TIMER_Running ) {
      // Update our clock.
      uMSec = _uInterval_mSec;
      _uCumulativeError += _uIntervalErr_uSec;
      if (_uCumulativeError >= 1000) {
         ++uMSec;
         _uCumulativeError -= 1000;
      }
      _ulTime += uMSec;

      // Set the context hook if it's time.
      if ((_ulSchedTime == 0) || (_ulTime >= _ulSchedTime))
      {
         // Arm ctx hook, pass stream type to the ctx hook handler.
         DevHelp_ArmCtxHook( _ulStreamType, _ctxHookHandle );
         _ulSchedTime = 0xFFFFFFFF;
      }
   }
}


// Start the generation of HW timer ticks on the chip.
static VOID TIMER::_vStartHWTicks( void )
{
}


// Start the generation of HW timer ticks on the chip.
static VOID TIMER::_vStopHWTicks( void )
{
}
