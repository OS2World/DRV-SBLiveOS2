/* $Id: rm.hpp,v 1.3 2001/04/30 21:07:58 sandervl Exp $ */

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
 *  ResourceManager and LDev_Resources class definitions.
 * @version %I%
 * @context
 *  Unless otherwise noted, all interfaces are Ring-3, 16-bit, sysinit time
 *  execution context on the kernel stack.
 * @notes
 *     The client of this object should query for their adapters and logical
 *  devices, and associated resources.  If the detected logical devices have
 *  sufficient resources, then allocate those resources.
 *
 *  Provides information to the audio driver on the detected devices and the
 *  resources required for device operation.  Obtain required resources from
 *  Resource Manager as directed by the client of this object.  The adapters
 *  and controllers will be owned by this driver.  The detected resources
 *  that will be operated are normally reallocated as driver resources.  This
 *  code is dumped after driver initialization is complete.
 *
 * @notes
 *     *.  Component design documentation at bottom of file.
 *     *.  PnP Device ID's are always in Compressed Ascii format.
 * @history
 */


#ifndef RM_HPP                  // Skip this file if already included.
#define RM_HPP

#ifndef OS2_INCLUDED            // Doesn't like being included twice.
extern "C" {                    // 16-bit header files are not C++ aware
   #define INCL_NOPMAPI
   #define INCL_DOSINFOSEG      // Need Global info seg in rm.cpp algorithms
   #include <os2.h>
}
#endif                          // end OS2_INCLUDED

extern "C" {
   #include <rmbase.h>          // Resource manager definitions.
   #include <rmcalls.h>
   #include <rmioctl.h>
}

#include <include.h>            // Defn's for WatCom based drivers.
#include "sizedefs.h"           // NumLogicalDevices

#define MAX_DevID 1             // Maximum number of devices with a particular
                                // PnP Device ID that we're prepared to deal with.
                                // Intention is that this defines the number of
                                // adapters we're prepared to deal with.
#define MAX_DescTextLen MAX_TEXT_DATA
                                // MAX_TEXT_DATA in rmioctl.h.  Max length
                                // of the descriptive text on any of the free
                                // ASCIIZ fields in the RM structures.



/*
 * --- LDev_Resources class.
 */

/* These "maximums" for number of I/O, IRQ, etc. per logical device, are defined
   by PnP ISA spec Ver 1.0a 5/5/94, sect 4.6, p. 20. */

#define MAX_ISA_Dev_IO   8
#define MAX_ISA_Dev_IRQ  2
#define MAX_ISA_Dev_DMA  2
#define MAX_ISA_Dev_MEM  4
#define MAX_ResourceCount ( MAX_ISA_Dev_IO + MAX_ISA_Dev_IRQ \
                          + MAX_ISA_Dev_DMA + MAX_ISA_Dev_MEM )


typedef struct
{
        USHORT VendorID;
        USHORT DeviceID;
        USHORT Command;
        USHORT Status;
        UCHAR  RevisionID;
        UCHAR  filler1[7];
        ULONG  Bar0;
        ULONG  Bar1;
        ULONG  filler2[5];
        USHORT SubsystemVendorID;
        USHORT SubsystemID;
        ULONG  filler3[3];
        UCHAR  InterruptLine;
        UCHAR  InterruptPin;
        UCHAR  Max_Gnt;
        UCHAR  Max_Lat;
        UCHAR  TRDY_Timeout;
        UCHAR  Retry_Timeout;
        UCHAR  filler4[0x9a];
        UCHAR  CapabilityID;
        UCHAR  NextItemPtr;
        USHORT PowerMgtCapability;
        USHORT PowerMgtCSR;
} PCIConfigData;

// This value indicates an empty entry in an LDev_Resource data item.
const USHORT NoIOValue = 0xffff;

class LDev_Resources {          // A class to hold the collection of resources
                                // needed by a single logical device.
 public:

   /* Public data.  Any unused data member is set to all 0xF.  Arrays are
    * always initialized to all 0xF's, then filled in in order, from 0..N.
    */
      USHORT uIOBase[ MAX_ISA_Dev_IO ];
      USHORT uIOLength[ MAX_ISA_Dev_IO ];
      USHORT uIRQLevel[ MAX_ISA_Dev_IRQ ];
      USHORT uDMAChannel[ MAX_ISA_Dev_DMA ];
      ULONG  uMemBase[ MAX_ISA_Dev_MEM ];
      ULONG  uMemLength[ MAX_ISA_Dev_MEM ];

   /* Information available from RM interface but not used:
    *   For all:  Flags (Exclusive, Multiplexed, Shared, Grant-Yield)
    *   IO:       IOAddressLines (10 or 16)
    *   IRQ:      PCI Irq Pin (10 or 16)
    *   DMA:      (Flags only)
    *   Memory:   Base, Length
    */

   // Are there any values in this structure?
   BOOL isEmpty ( void );

   // Clear the structure (set all fields to "NoIOValue".
   void vClear ( void );
};


/*
 * --- ResourceManager class.
 */

enum RMObjectState { rmDriverCreated, rmDriverFailed, rmAdapterCreated, rmAdapterFailed, rmAllocFailed };

class ResourceManager {         // A class to encapsulate system resource
                                // allocation issues.  Ref. documentation
 public:                        // at bottom of file.

   ResourceManager ( ULONG pciId );
        // Register the device driver (this activates the RM interface).
        // Intention is that only one ResourceManager object is created
        // in driver, which will handle all audio.

   bool bIsDevDetected ( DEVID DevID , ULONG ulSearchFlags, bool fPciDevice);
        // Return an indicator of whether or not named device is present.
        // in:   - PnP Device ID, compressed ASCII.
        //       - Search flags, ref rmbase.h SEARCH_ID_*;  also documented
        //         as SearchFlags parm in PDD RM API RMDevIDToHandleList().
        //         Only one of the search flags can be selected.
        // out:    True iff at least one device is detected
        // rem:    It's expected that the 1st parm, the PnP DevID, will
        //         be a DEVICE ID and the

   //###
   LDev_Resources* GetRMDetectedResources ( DEVID DevID , ULONG ulSearchFlags, bool fPciDevice, bool fJoystick );

   LDev_Resources* pGetDevResources ( DEVID DevID , ULONG ulSearchFlags, bool fPciDevice, bool fJoystick = FALSE);

   inline RMObjectState getState() { return _state; };
        // Return state of RM object.

 private:

   //--- Private data

   RMObjectState _state;   // Current state of object, as enumerated above.

   BOOL  _rmDetection;     // TRUE if OS/2 RM detection services available.

   HDRIVER    _hDriver;    // Handle for our driver - assigned to us by RM.

   HADAPTER   _hAdapter;   // Handle for our adapter - output from RM.
                           // ### This will need to be an array & associated
                           // ### w/PnP ID's to handle mult adapters.

   //--- Private methods

   // Converts an LDevResource structure into an RESOURCELIST.
   NPRESOURCELIST _makeResourceList( const LDev_Resources& resources );

   NPHANDLELIST _DevIDToHandleList ( DEVID DevID , ULONG ulSearchFlags, bool fPciDevice);
       // Search the Resource Manager's "current detected" tree for
        // the specified PnP ID, and return all matching RM handles.

   NPRM_GETNODE_DATA _RMHandleToNodeData ( RMHANDLE rmHandle );
        // Return the list of resources requested by the device
        // or logical device represented by the resource manager handle.

   NPRM_GETNODE_DATA _DevIDToNodeData( DEVID DevID , ULONG ulSearchFlags, bool fPciDevice );
        // Composes functions _DevIDToHandleList + _RMHandleToNodeData

   NPAHRESOURCE _pahRMAllocResources( NPRESOURCELIST pResourceList );

   APIRET _rmCreateAdapter();

   APIRET _rmCreateDevice( PSZ pszName, NPAHRESOURCE pahResource );

   BOOL   getPCIConfiguration(ULONG pciId);

   ULONG          PCIConfig[64];
   PCIConfigData *pciConfigData;

};

#endif  /* RM_HPP */

