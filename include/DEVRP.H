/* $Id: DEVRP.H,v 1.2 2001/06/20 17:10:08 sandervl Exp $ */

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


// DevRP.h
//
// Type definitions for OS/2 2.x physical device driver request packets.
//
// History:
//
// Sep 30, 94  David Bollo         Initial version
// Jan 22, 97  Sander van Leeuwen  Changed far pointers to WORD32

// Prevent multiple inclusion
#if !defined(DevRP_h)
#define DevRP_h 1

#if !defined(__WATCOMC__) || !defined(__cplusplus)
#error C++ must be used for strict type checking
#endif


// Include the fundamental type definitions
#include "DevType.h"


// Device driver request packet (B=block devices, C=character devices)
struct        RP
  {
  BYTE        Length;         // Length of request packet
  BYTE        Unit;           // Unit code (B)
  BYTE        Command;        // Command code
  WORD16      Status;         // Status code
  BYTE        Flags;          // Flags
  BYTE        Reserved[3];    // Reserved
  WORD32      Link;           // Link to next request packet in queue
  };

// Request packet 00 (BC): Initialization
struct        RPInit          : public RP
  {
  union
    {
    struct
      {
      BYTE    Reserved;       // Unused
      WORD32  DevHlp;         // Address of DevHlp entry point
      WORD32  Args;           // Command-line arguments
      BYTE    Drive;          // Drive number of first unit (B)
      } In;
    struct
      {
      BYTE    Units;          // Number of supported units (B)
      WORD16  FinalCS;        // Offset of last resident code
      WORD16  FinalDS;        // Offset of last resident data
      WORD32  Bpb;            // BIOS parameter block (B)
      } Out;
    };
  };

// Request packet 01 (B ): Media check
struct        RPMediaCheck    : public RP
  {
  BYTE        Media;          // Media descriptor
  BYTE        Result;         // -1 = Changed, 0 = Unsure, 1 = Unchanged
  WORD32      PrevVolID;      // Pointer to previous volume ID
  };

// Request packet 02 (B ): Build BIOS parameter block
struct        RPBuildBPB      : public RP
  {
  BYTE        Media;          // Media descriptor
  WORD32      Transfer;       // Transfer virtual address
  WORD32      Bpb;            // Pointer to BPB table
  BYTE        Drive;          // Drive number
  };

// Request packet 04 (BC): Read
// Request packet 08 (BC): Write
// Request packet 09 (BC): Write verify
struct        RPRWV           : public RP
  {
  BYTE        Media;          // Media descriptor
  PHYSICAL    Transfer;       // Transfer physical address
  WORD16      Count;          // Number of bytes (C) or sectors (B)
  WORD32      Start;          // Starting sector number (B)
  USHORT      FileID;         // System file number
  };

// Request packet 05 ( C): Nondestructive read with no wait
struct        RPReadNoWait    : public RP
  {
  BYTE        Data;           // Return character
  };

// Request packet 06 ( C): Input status
// Request packet 0A ( C): Output status
struct        RPIOStatus      : public RP
  {
  };

// Request packet 07 ( C): Input flush
// Request packet 0B ( C): Output flush
struct        RPIOFlush       : public RP
  {
  };

// Request packet 0D (BC): Open
// Request packet 0E (BC): Close
struct        RPOpenClose     : public RP
  {
  USHORT      FileID;         // System file number
  };

// Request packet 0F (B ): Removable media check
struct        RPRemovable     : public RP
  {
  };

// Request packet 10 (BC): IO Control
struct        RPIOCtl         : public RP
  {
  BYTE        Category;       // Category code
  BYTE        Function;       // Function code
  WORD32      ParmPacket;     // Parameter packet
  WORD32      DataPacket;     // Data packet
  USHORT       FileID;         // System file number
  WORD16      ParmLength;     // Parameter packet length
  WORD16      DataLength;     // Data packet length
  };

// Request packet 11 (B ): Reset media
struct        RPReset         : public RP
  {
  };

// Request packet 12 (B ): Get logical unit
// Request packet 13 (B ): Set logical unit
struct        RPUnit          : public RP
  {
  };

// Request packet 14 ( C): Deinstall character device driver
struct        RPDeinstall     : public RP
  {
  };

// Request packet 16 (B ): Count partitionable fixed disks
struct        RPPartitionable : public RP
  {
  BYTE        Count;          // Number of partitionable fixed disks
  WORD32      Reserved;       // Reserved
  };

// Request packet 17 (B ): Get logical unit mapping of fixed disk
struct        RPUnitMap       : public RP
  {
  WORD32      Units;          // Bitmap of supported logical units
  WORD32      Reserved;       // Reserved
  };

// Request packet 1C (BC): Notify start or end of system shutdown
struct        RPShutdown      : public RP
  {
  BYTE        Function;       // Shutdown code: 0 = start, 1 = end
  WORD32      Reserved;       // Reserved
  };

// Request packet 1D (B ): Get driver capabilities
struct        RPDriverCaps    : public RP
  {
  BYTE        Reserved[3];    // Reserved
  WORD32      DCS;            // Pointer to driver capabilities structure
  WORD32      VCS;            // Pointer to volume characteristics structure
  };

// Request packet 1F (BC): Notify end of initialization
struct        RPInitComplete  : public RP
  {
  };



// RP Result codes

#define RPERR                 0x8000      // Error
#define RPDEV                 0x4000      // Device-specific error code
#define RPBUSY                0x0200      // Device is busy
#define RPDONE                0x0100      // Command complete

#define RPERR_PROTECT         0x8000      // Write-protect error
#define RPERR_UNIT            0x8001      // Unknown unit
#define RPERR_READY           0x8002      // Not ready
#define RPERR_COMMAND         0x8003      // Unknown command
#define RPERR_CRC             0x8004      // CRC error
#define RPERR_LENGTH          0x8005      // Bad request length
#define RPERR_SEEK            0x8006      // Seek error
#define RPERR_FORMAT          0x8007      // Unknown media
#define RPERR_SECTOR          0x8008      // Sector not found
#define RPERR_PAPER           0x8009      // Out of paper
#define RPERR_WRITE           0x800A      // Write fault
#define RPERR_READ            0x800B      // Read fault
#define RPERR_GENERAL         0x800C      // General failure
#define RPERR_DISK            0x800D      // Disk change
#define RPERR_MEDIA           0x8010      // Uncertain media
#define RPERR_INTERRUPTED     0x8011      // Call interrupted (character)
#define RPERR_MONITOR         0x8012      // Monitors unsupported
#define RPERR_PARAMETER       0x8013      // Invalid parameter
#define RPERR_USE             0x8014      // Device in use
#define RPERR_INIT            0x8015      // Non-critical init failure



#endif // DevRP_h
