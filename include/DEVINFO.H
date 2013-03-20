/* $Id: DEVINFO.H,v 1.1 2000/04/23 14:55:25 ktk Exp $ */

/************************************************************************\
**                                                                      **
**               OS/2(r) Physical Device Driver Libraries               **
**                         for Watcom C/C++ 10                          **
**                                                                      **
**  COPYRIGHT:                                                          **
**                                                                      **
**    (C) Copyright Advanced Gravis Computer Technology Ltd 1994.       **
**        All Rights Reserved.                                          **
**                                                                      **
**  DISCLAIMER OF WARRANTIES:                                           **
**                                                                      **
**    The following [enclosed] code is provided to you "AS IS",         **
**    without warranty of any kind.  You have a royalty-free right to   **
**    use, modify, reproduce and distribute the following code (and/or  **
**    any modified version) provided that you agree that Advanced       **
**    Gravis has no warranty obligations and shall not be liable for    **
**    any damages arising out of your use of this code, even if they    **
**    have been advised of the possibility of such damages.  This       **
**    Copyright statement and Disclaimer of Warranties may not be       **
**    removed.                                                          **
**                                                                      **
\************************************************************************/


// DevInfo.h
//
// Type definitions for OS/2 2.x global and local info segments.
//
// History:
//
// Sep 30, 94  David Bollo    Initial version


// Prevent multiple inclusion
#if !defined(DevInfo_h)
#define DevInfo_h 1

#if !defined(__cplusplus)
#error C++ must be used for strict type checking
#endif


// Include the fundamental type definitions
#include "DevType.h"


// Global (per-system) information segment
struct        GINFO
  {
  WORD32      Time;           // Time in seconds since 1970.01.01
  WORD32      MilliSeconds;   // Time in milliseconds
  BYTE        Hour;           // Current hour
  BYTE        Minute;         // Current minute
  BYTE        Second;         // Current second
  BYTE        Hundredth;      // Current hundredth of a second
  WORD16      Timezone;       // Minutes from UTC (Greenwich mean-time)
  WORD16      Interval;       // Timer interval (tenths of milliseconds)
  BYTE        Day;            // Current day of month
  BYTE        Month;          // Current month
  WORD16      Year;           // Current year
  BYTE        Weekday;        // Current day of week (0 = Sunday, 6 = Saturday)
  BYTE        MajorVersion;   // Major version number of OS/2
  BYTE        MinorVersion;   // Minor version number of OS/2
  BYTE        Revision;       // Revision letter of OS/2
  BYTE        CurrentSession; // Current foreground full-screen session ID
  BYTE        MaxSessions;    // Maximum number of full-screen sessions
  BYTE        HugeShift;      // Shift count for huge segments
  BYTE        ProtectFlag;    // 1 = protected mode only, 0 = mixed mode
  WORD16      ForegroundPID;  // Process ID of the current foreground process
  BYTE        DynamicFlag;    // 1 = dynamic variation enabled, 0 = disabled
  BYTE        MaxWaitTime;    // Maximum wait to get control (seconds)
  WORD16      MinTimeSlice;   // Minimum time slice length (milliseconds)
  WORD16      MaxTimeSlice;   // Maximum time slice length (milliseconds)
  WORD16      BootDrive;      // ID of system startup drive (1 = A:, 26 = Z:)
  BYTE        Trace[32];      // System tracing flags (RAS)
  BYTE        MaxVIOSessions; // Maximum number of VIO (windowed) sessions
  BYTE        MaxPMSessions;  // Maximum number of PM sessions
  WORD16      ErrorLog;       // Error logging status
  WORD16      RASSelector;    // RAS memory mapped IO selector
  WORD32      RASAddress;     // RAS memory mapped IO address
  BYTE        MaxVDMSessions; // Maximum number of virtual DOS machines
  };

// Local (per-process) information segment
struct        LINFO
  {
  WORD16      ProcessID;      // Current process ID
  WORD16      ParentID;       // Current process' parent's PID
  WORD16      Priority;       // Current thread's priority
  WORD16      ThreadID;       // Current thread ID
  WORD16      SessionID;      // Current session ID
  BYTE        Status;         // Process status
  BYTE        Unused1;        // Unused
  WORD16      ForegroundFlag; // Current process has keyboard focus
  BYTE        Type;           // Type of process (0 = FS, 1 = real, 2 = VIO,
  BYTE        Unused2;        //                  3 = PM, 4 = detached)
  WORD16      EnvSelector;    // Selector for environment
  WORD16      CommandLineOff; // Offset of command line in EnvironmentSel
  WORD16      DSLength;       // Length of data segment
  WORD16      StackSize;      // Size of stack (bytes)
  WORD16      HeapSize;       // Size of heap (bytes)
  WORD16      Module;         // Module handle
  WORD16      DSSelector;     // Selector for data segment
  };


#endif // DevInfo_h
