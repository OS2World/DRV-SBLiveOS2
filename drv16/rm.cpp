/* $Id: rm.cpp,v 1.5 2001/04/30 21:07:58 sandervl Exp $ */

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
 *  RM (Resource manager) object implementation
 * @version %I%
 * @context
 *  Unless otherwise noted, all interfaces are Ring-3, 16-bit, sysinit time
 *  execution context on the kernel stack.
 * @notes
 *  Provides information to the audio driver on the detected devices and the
 *  resources required for device operation.  Obtain required resources from
 *  Resource Manager as directed by the client of this object.  The adapters
 *  and controllers will be controlled by this driver.  The detected resources
 *  that will be operated should be reallocated as driver resources.  This code
 *  is dumped after driver initialization is complete.
 *
 *  Remarks :
 *     *.  PnP Device ID's are always in Compressed Ascii format.
 *     *.  ### Need remove malloc by adding memory mgmt support for [].
 *
 * @history
 */

////#pragma code_seg ("_inittext");
////#pragma data_seg ("_initdata","endds");

#include "rm.hpp"                      // Will include os2.h, etc.
#include <devhelp.h>
#include <string.h>                    // _fmemset()
#include "malloc.h"                    // malloc()
#include <dbgos2.h>

#define EMU10K1_JOYSTICK_EXTENT 0x8	/* 8 byte I/O space */
#define EMU10K1_EXTENT	0x20	/* 32 byte I/O space */

char DeviceName[64] = "Creative Labs SoundBlaster Live!";
char DeviceNameJoy[64] = "Creative Labs SoundBlaster Live! Joystick";
/**@external LDev_Resources::isEmpty
 *  Returns TRUE iff the LDev_Resources structure has no information.
 * @param None.
 * @return BOOL
 */
BOOL LDev_Resources::isEmpty()
{
   BOOL bIsEmpty = TRUE;

   for ( int i=0; i<MAX_ISA_Dev_IO; ++i) {
      if (uIOBase[i] != NoIOValue)
         bIsEmpty = FALSE;
      if ((i < MAX_ISA_Dev_IRQ) && (uIRQLevel[i] != NoIOValue))
         bIsEmpty = FALSE;
      if ((i < MAX_ISA_Dev_DMA) && (uDMAChannel[i] != NoIOValue))
         bIsEmpty = FALSE;
      if ((i < MAX_ISA_Dev_MEM) && (uMemBase[i] != 0xffffffff))
         bIsEmpty = FALSE;
   }
   return bIsEmpty;
}


/**@external LDev_Resources::vClear
 *  Set an LDev_Resources structure to Empty.
 * @param None.
 * @return VOID
 */
void LDev_Resources::vClear()
{
   _fmemset( (PVOID) this, NoIOValue, sizeof(LDev_Resources) );
}


/*
 * --- Linkages required by system Resource Manager (rm.lib).
 */

extern "C" PFN    RM_Help  = 0L;
extern "C" PFN    RM_Help0 = 0L;
extern "C" PFN    RM_Help3 = 0L;
extern "C" ULONG  RMFlags  = 0L;

/*
 * --- Public member functions.
 */

/**@external ResourceManager
 *  Constructor for RM object.
 * @notes Creates a "driver" node for this device driver, but does not
 *  allocate resources.
 */
ResourceManager::ResourceManager(ULONG pciId)
{
   APIRET rc;
   DRIVERSTRUCT DriverStruct;
   PSEL p;
   PGINFOSEG pGIS = 0;

   /* Warp version level, bus type, machine ID, and much other information is
    * readily available.  Reference the RM ADD sample in the DDK for code.
    *
    * Create a "driver" struct for this driver.  This must ALWAYS be the
    * first true RM call executed, as it attaches the Resource Manager
    * library and performs setup for the other RM calls.
    */
   _fmemset( (PVOID) &DriverStruct, 0, sizeof(DriverStruct) );
   DriverStruct.DrvrName     = (PSZ) "SBLIVE16.SYS";                  /* ### IHV */
   DriverStruct.DrvrDescript = (PSZ) "SoundBlaster Live!";    /* ### IHV */
   DriverStruct.VendorName   = (PSZ) "Creative Labs";             /* ### IHV */
   DriverStruct.MajorVer     = CMVERSION_MAJOR;          //rmbase.h /* ### IHV */
   DriverStruct.MinorVer     = CMVERSION_MINOR;          //rmbase.h /* ### IHV */
   DriverStruct.Date.Year    = 2000;                                /* ### IHV */
   DriverStruct.Date.Month   = 2;                                   /* ### IHV */
   DriverStruct.Date.Day     = 1;                                   /* ### IHV */
   DriverStruct.DrvrType     = DRT_AUDIO;
   DriverStruct.DrvrSubType  = 0;
   DriverStruct.DrvrCallback = NULL;
   rc = RMCreateDriver( &DriverStruct, &_hDriver );
   if( rc == RMRC_SUCCESS )
      _state = rmDriverCreated;
   else {
      _state = rmDriverFailed;
      _hDriver = 0;
   }

   // Build a pointer to the Global Information Segment.
   rc = DevHelp_GetDOSVar( DHGETDOSV_SYSINFOSEG, 0, (PPVOID)&p );
   if (rc) {
      _rmDetection = FALSE;
   }
   else {
      SELECTOROF(pGIS) = *p;
      _rmDetection =
         ( (pGIS->uchMajorVersion > 20) ||
           ((pGIS->uchMajorVersion == 20) && (pGIS->uchMinorVersion > 30)) );
   }
}

#pragma off (unreferenced)
bool ResourceManager::bIsDevDetected( DEVID DevID , ULONG ulSearchFlags, bool fPciDevice)
#pragma on (unreferenced)
/*
;  PURPOSE: Search the Resource Manager's "current detected" tree for
;           the matching PnP ID.
;
;  IN:    - DevID - PnP Device ID being sought (Compressed Ascii).
;         - ulSearchFlags - Search flags, ref rmbase.h SEARCH_ID_*;  also
;           documented as SearchFlags parm in PDD RM API RMDevIDToHandleList().
;           Defines whether DevID is a Device ID, Logical device ID, Compatible
;           ID, or Vendor ID.
;
;  OUT:     Boolean indicator, TRUE when number of matching detected devices > 0.
;
*/
{
#if 1
   if(getPCIConfiguration(DevID) == FALSE) {
      return FALSE;
   }

   //Manual detection in ResourceManager class constructor; 
   return (_state == rmDriverCreated || _state == rmAdapterCreated);
#else
   BOOL bReturn = FALSE;
   NPHANDLELIST pHandleList = 0;

   if ( ! _rmDetection )
      bReturn = TRUE;
   else {
      pHandleList = _DevIDToHandleList( DevID, ulSearchFlags, fPciDevice);
                                       // Array of RM handles for the detected
                                       // devices that match the PnP device ID(s).
      bReturn = (pHandleList->cHandles != 0);
                                       // If the size of the array != 0, we found the device.
      delete pHandleList;              // Free the structure.
   }

   return bReturn ;
#endif
}



/**@internal GetRMDetectedResources
 *  Return the set of IO ports, IRQ levels, DMA channels, & memory ranges
 *  required by the specified device, as detected by the OS/2 resource
 *  manager.
 * @param Refer to _bIsDevDetected() for parameters.
 * @notes It's expectded that the spec'd DevID & flags will select a single
 *  device in the system.  If multiples are found, the first matching device
 *  is referenced.
 * @return LDev_Resources* object, filled in with required resources.  Object
 *  is allocated from heap, is responsibility of caller to free object.
 *  Ordering on the resources is preserved.   Unused fields are set to NoIOValue.
 * @return NULL on error situations.
 */
#pragma off (unreferenced)
LDev_Resources* ResourceManager::GetRMDetectedResources ( DEVID DevID , ULONG ulSearchFlags, bool fPciDevice, bool fJoystick)
#pragma on (unreferenced)
{
#if 1
   LDev_Resources* pResources = 0;     // Used to return result.

   pResources = new LDev_Resources();
   if (!pResources) return NULL;

   pResources->vClear();

   //Fill in resources read from PCI Configuration space
   if(!fJoystick) {
	pResources->uIRQLevel[0]  = pciConfigData->InterruptLine;
	if(pResources->uIRQLevel[0] == 0 || pResources->uIRQLevel[0] > 15)  {
		dprintf(("Invalid PCI irq %x", (int)pResources->uIRQLevel[0]));
		DebugInt3();
        	return NULL;
	}
   }
   pResources->uIOBase[0]   = (USHORT)(pciConfigData->Bar0 & 0xFFFFFFF0);
   if(fJoystick) {
	pResources->uIOLength[0] = EMU10K1_JOYSTICK_EXTENT;
   }
   else pResources->uIOLength[0] = EMU10K1_EXTENT;

   return pResources;
#else
   LDev_Resources* pResources = 0;     // Used to return result.
   NPRM_GETNODE_DATA pNode = 0;        // Node resource data for spec'd DEVID's.
   NPRESOURCELIST pResourceList = 0;   // Resource list contained within Node data.
   int indexIO = 0;                    // Number of IO, IRQ, etc. requested.
   int indexIRQ = 0;
   int indexDMA = 0;
   int indexMEM = 0;
   int i;

   pResources = new LDev_Resources();
   if (!pResources) goto error_cleanup;
   pResources->vClear();

   // Get resources list from system RM.  Returned pNode should have
   // pNode->NodeType equal to RMTYPE_DETECTED (=5).
   pNode = _DevIDToNodeData( DevID, ulSearchFlags, fPciDevice );
   if ( !pNode ) goto error_cleanup;

   pResourceList = (NPRESOURCELIST) pNode->RMNode.pResourceList;
   if (! pResourceList) {
      goto error_cleanup;
   }
   if (pResourceList->Count > MAX_ResourceCount) {
      goto error_cleanup;
   }

   //--- Format resources into an LDev_Resource format.
   for (i=0; i < pResourceList->Count; ++i) {
      RESOURCESTRUCT* pRes = &pResourceList->Resource[i];
                                       // Pointer to next resource in list.
      switch( pRes->ResourceType )  {
      case RS_TYPE_IO:
         pResources->uIOBase[ indexIO ] = pRes->IOResource.BaseIOPort;
         pResources->uIOLength[ indexIO ] = pRes->IOResource.NumIOPorts;
         ++indexIO;
         break;

      case RS_TYPE_IRQ:
         pResources->uIRQLevel[ indexIRQ ] = pRes->IRQResource.IRQLevel;
         ++indexIRQ;
         break;

      case RS_TYPE_DMA:
         pResources->uDMAChannel[ indexDMA ] = pRes->DMAResource.DMAChannel;
         ++indexDMA;
         break;

      case RS_TYPE_MEM:
         pResources->uMemBase[ indexMEM ] = pRes->MEMResource.MemBase;
         pResources->uMemLength[ indexMEM ] = pRes->MEMResource.MemSize;
         ++indexMEM;
         break;
      }
   }  /* end for loop through resource list. */

   delete pNode;
   return pResources;

error_cleanup:
   delete pNode;
   delete pResources;
   return NULL;
#endif
}


// declarations for functions defined in rmWarp3.cpp
LDev_Resources* GetPnpLDevConfig( int iLDev );
void SetPnpLDevConfig( int iLDev, LDev_Resources* pResources );

/**@external pGetDevResources
 *
 *  Allocate set of IO ports, IRQ levels, DMA channels requested by the
 *  specified device.
 *
 * @param Refer to bIsDevDetected()
 *
 * @return LDev_Resources object, filled in with required resources.  Object
 *  is returned on stack. Ordering on the resources is preserved.   Unused
 *  fields are set to 0xFF.
 *
 * @notes The allocation from OS/2's RM is required; if not performed, the
 *  resources could be allocated by a driver which loads after this one.
 *  Also perform other bookeepping by registering driver, adapter, and
 *  device with the system RM.
 *
 * @notes Additional comments from DevCon 'ADD' sample for RM: "Create all
 *  Resource Manager nodes required by this driver.  The Resource Manager
 *  structures created in this module may be thought of as being allocated
 *  in a seperate tree called the 'driver' tree.  The snooper resource nodes
 *  were created in the 'current detected' tree.  Therefore, these
 *  allocations will not compete with previous snooper resource allocations.
 *
 * @notes
 *
 *  - Fetch defaults for named device
 *     - Warp3:  GetSpecDefaults( pnpID )
 *     - (not implemented:)  snoop on Warp3 -> GetSnoopedResources()
 *     - Warp4:  GetLDev_Resources( pnpID ) (rename to GetRMDetectedResources())
 *     - @return LDev_Resources structure
 *  - Fill in any user overrides
 *     - object Override( pnpID ) subclasses an LDev_Resources
 *     - has an added "exists" flag, set true if any overrides exist
 *     - on creation, interacts with parsed information for overrides
 *     - bool Override.exist( pnpID )
 *     - LDev_Resources Override.apply( pnpID, LDev_Resources )
 *  - Format LDev_Resources into RESOURCELIST
 *     - pResourceList = MakeResourceList( LDev_Resources )
 *  - Allocate adapter if this is the 1st time through
 *     - RMCreateAdapter()
 *  - Allocate resources to device
 *     - Call GetDescriptiveName( pnpID ) if Warp3
 *     - Call RMCreateDevice() to allocate the resources
 *  - Add as last step:
 *     - if (Warp3 or any command line overrides) and
 *     - if resources are successfully allocated
 *     - then SLAM the chip with the allocated resources
 */

LDev_Resources* ResourceManager::pGetDevResources ( DEVID DevID , ULONG ulSearchFlags, bool fPciDevice, bool fJoystick)
{
   APIRET rc;

   // These pointers must be initialized to 0 so that they're valid for
   // delete operation on an exception cleanup.

   NPRM_GETNODE_DATA pNode = 0;        // Resources list from RM.  Be sure to
                                       // return this memory to heap when done.
   NPRESOURCELIST pResourceList = 0;   // Pointer to the resource list; may or
                                       // may not be contained within the Node data.
   NPAHRESOURCE pahResources = 0;      // Pointer to the resource handle
                                       // structure.
   LDev_Resources* pResources = 0;     // Used to return result.

   // Initialize resource object.  Use detected information if available,
   // otherwise use hardcoded defaults.
   pResources = GetRMDetectedResources( DevID, ulSearchFlags, fPciDevice, fJoystick);
   if (!pResources) goto exit;

   // Convert the data format into an OS/2 RM RESOURCELIST & allocate resources.
   pResourceList = _makeResourceList( *pResources );
   if (! pResourceList) goto exit;
   pahResources = _pahRMAllocResources( pResourceList );
   if (! pahResources) {
       _state = rmAllocFailed;
       goto exit;
   }

   //--- Here, we got all the resources we wanted.  Register adapter if not yet done.
   //### Problem here with mult adpaters, would need to cross ref PnP ID's to adapters created.
   if (_state != rmAdapterCreated) {
      rc = _rmCreateAdapter();
   }

   // Register the device with OS/2 RM.
   if(fJoystick) {
  	_rmCreateDevice((unsigned char __far *)DeviceNameJoy, pahResources );
   }
   else _rmCreateDevice((unsigned char __far *)DeviceName, pahResources );

exit:
   delete pahResources;
   delete pResourceList;

   return pResources;
}

/*
 * --- Private member functions.
 */


/**@internal ResourceManager::_makeResourceList
 *  Converts an LDevResource structure into an RESOURCELIST.
 * @param LDev_Resources resources - structure to translate
 * @return PRESOURCELIST pResourceList - pointer to resource list.
 * @return NULL on storage allocation exception.
 * @notes Storage for the RESOURCELIST is allocated from the heap, and
 *  it's the responsibility of the caller to free this storage after use.
 */
NPRESOURCELIST ResourceManager::_makeResourceList( const LDev_Resources& resources )
{
   int i, j;                           // Index vbls
   int nResources;                     // Number of resources.
   PRESOURCELIST pRL;                  // Return value, pointer to resource list
                                       // constructured in the heap.
   // Count up number of resources.
   for ( i=0, nResources=0; i<MAX_ISA_Dev_IO; ++i) {
      if (resources.uIOBase[i] != NoIOValue)
         ++ nResources;
      if ((i < MAX_ISA_Dev_IRQ) && (resources.uIRQLevel[i] != NoIOValue))
         ++ nResources;
      if ((i < MAX_ISA_Dev_DMA) && (resources.uDMAChannel[i] != NoIOValue))
         ++ nResources;
      if ((i < MAX_ISA_Dev_MEM) && (resources.uMemBase[i] != 0xffffffff))
         ++ nResources;
   }

   // Allocate storage for the resource list.
   USHORT nBytes = sizeof(RESOURCELIST) +
                   (nResources * sizeof(RESOURCESTRUCT));
   pRL = (PRESOURCELIST) malloc( nBytes );
   if (!pRL) return NULL;

   // Fill in resource list values.
   _fmemset( (PVOID) pRL, 0, nBytes );
   pRL->Count = nResources;

   // 'i' is the resource list index and points to next empty slot.
   // 'j' indexes the LDev_Resources struct.
   i = 0;
   for ( j=0; j<MAX_ISA_Dev_IO; ++j) {
      if (resources.uIOBase[j] != NoIOValue) {
         pRL->Resource[i].ResourceType = RS_TYPE_IO;
         pRL->Resource[i].IOResource.BaseIOPort = resources.uIOBase[j];
         pRL->Resource[i].IOResource.NumIOPorts = resources.uIOLength[j];
         pRL->Resource[i].IOResource.IOFlags    = RS_IO_EXCLUSIVE;
         if (resources.uIOBase[j] > 0x3ff)                        // ### IHV
            pRL->Resource[i].IOResource.IOAddressLines = 16;      // ### IHV
         else                                                     // ### IHV
            pRL->Resource[i].IOResource.IOAddressLines = 10;      // ### IHV
         ++i;
      }
   }
   for ( j=0; j<MAX_ISA_Dev_IRQ; ++j) {
      if (resources.uIRQLevel[j] != NoIOValue) {
         pRL->Resource[i].ResourceType = RS_TYPE_IRQ;
         pRL->Resource[i].IRQResource.IRQLevel = resources.uIRQLevel[j];
         pRL->Resource[i].IRQResource.IRQFlags = RS_IRQ_SHARED;
         pRL->Resource[i].IRQResource.PCIIrqPin = RS_PCI_INT_NONE;
         ++i;
      }
   }
   for ( j=0; j<MAX_ISA_Dev_DMA; ++j) {
      if (resources.uDMAChannel[j] != NoIOValue) {
         pRL->Resource[i].ResourceType = RS_TYPE_DMA;
         pRL->Resource[i].DMAResource.DMAChannel = resources.uDMAChannel[j];
         pRL->Resource[i].DMAResource.DMAFlags = RS_DMA_EXCLUSIVE;
         ++i;
      }
   }

   for ( j=0; j<MAX_ISA_Dev_MEM; ++j) {
      if (resources.uMemBase[j] != 0xffffffff) {
         pRL->Resource[i].ResourceType = RS_TYPE_MEM;
         pRL->Resource[i].MEMResource.MemBase  = resources.uMemBase[j];
         pRL->Resource[i].MEMResource.MemSize  = resources.uMemLength[j];
         pRL->Resource[i].MEMResource.MemFlags = RS_MEM_EXCLUSIVE;
         ++i;
      }
   }

   return pRL;
}



NPHANDLELIST ResourceManager::_DevIDToHandleList ( DEVID DevID , ULONG ulSearchFlags, bool fPciDevice)
/*
;  PURPOSE: Search the Resource Manager's "current detected" tree for
;           the specified PnP ID, and return all matching RM handles.
;
;  IN:      Refer to bIsDevDetected()
;
;  OUT:     List of RM handles matching the search, in HandleList format (rmbase.h)
;           Info returned in heap memory, caller must ensure this is later freed.
*/
{
   APIRET rc;
   DEVID  DeviceID, FunctionID, CompatID, VendorID;

   //--- Stuff the search value into the appropriate vbl.  Need not initialize
   //    or zero out the other vbls, they won't be referenced during the search.
   switch (ulSearchFlags) {
   case SEARCH_ID_DEVICEID:
      DeviceID = DevID;
      break;
   case SEARCH_ID_FUNCTIONID:
      FunctionID = DevID;
      break;
   case SEARCH_ID_COMPATIBLEID:
      CompatID = DevID;
      break;
   case SEARCH_ID_VENDOR:
      VendorID = DevID;
      break;
   default:
      return NULL;
   }

   NPHANDLELIST pDevHandleList = (NPHANDLELIST)
      malloc ( sizeof(HANDLELIST) + (sizeof(RMHANDLE) * MAX_DevID) );   //###
      //### new char[ sizeof(HANDLELIST) + (sizeof(RMHANDLE) * MAX_DevID) ];

                  // List of RM handles associated w/ Device ID.  Will normally
                  // be a single handle for the single adapter or function found.

   pDevHandleList->cHandles = 0;                  // clear handle count
   pDevHandleList->cMaxHandles = MAX_DevID;       // set size dimension

   // Use the PnP ID to get a list of detected devices which used this ID,
   // by searching for all snooped devices in the CURRENT detected tree.
   rc = RMDevIDToHandleList(
                        (fPciDevice) ? RM_IDTYPE_PCI : RM_IDTYPE_EISA,           // input device IDs' format
                        DeviceID,                 // device (adapter) ID
                        FunctionID,               // logical device (function) ID
                        CompatID,                 // compatible ID
                        VendorID,                 // vendor ID
                        0,                        // serial number
                        ulSearchFlags,
                        HANDLE_CURRENT_DETECTED,
                        pDevHandleList );         // place output here
   if (rc != RMRC_SUCCESS) {
      return NULL;
   }

   return pDevHandleList;
}


NPRM_GETNODE_DATA ResourceManager::_RMHandleToNodeData ( RMHANDLE rmHandle )
/*
;  PURPOSE: Return the list of resources requested by the device
;           represented by the resource manager handle.
;
;  IN:    - rmHandle - rmHandle representing the Device or Logical device of
;           interest.
;
;  OUT:     List of resources (GETNODE_DATA  format, rmbase.h, rmioctl.h), saved
;           in heap memory.  Caller must ensure this memory is later freed.
;
;  ALGORITHM:
;           1.  Call RMHandleToResourceHandleList to get a count of # of resources.
;           2.  Allocate heap memory for array of Resources.
;           3.  Construct resource list by one of the following methods
;               a. Call RMGetNodeInfo on each resource hangle (n calls to RM)
;       used->  b. Call RMGetNodeInfo with device handle (1 call to RM)
*/
{
   APIRET rc;

   //--- Fetch list of resource handles for this device handle.  We use the
   //    handle list only to get a count on the number of resources.
   char work[ sizeof(HANDLELIST) + (sizeof(RMHANDLE) * MAX_ResourceCount) ];
   PHANDLELIST pResourceHandleList = (PHANDLELIST) work;
                     // List of handles for IO, IRQ, DMA, etc. resources.

   pResourceHandleList->cHandles = 0;
   pResourceHandleList->cMaxHandles = MAX_ResourceCount;
   rc = RMHandleToResourceHandleList( rmHandle, pResourceHandleList );
   if (rc != RMRC_SUCCESS) {
      return NULL;
   }

   //--- Allocate heap memory to hold complete list of resources for device.
   USHORT uNodeSize = sizeof(RM_GETNODE_DATA)
                      + sizeof(ADAPTERSTRUCT) + MAX_DescTextLen
                      + sizeof(DRIVERSTRUCT) + MAX_DescTextLen
                      + (sizeof(RESOURCESTRUCT) * pResourceHandleList->cHandles);
   //### NPRM_GETNODE_DATA pNodeData = (NPRM_GETNODE_DATA) new char[ uNodeSize ];
   NPRM_GETNODE_DATA pNodeData = (NPRM_GETNODE_DATA) malloc( uNodeSize );

   //--- Get resource info, use single call to GetNodeInfo on device handle.
   rc = RMGetNodeInfo( rmHandle, pNodeData, uNodeSize );
   if (rc != RMRC_SUCCESS) {
      delete pNodeData;
      return NULL;
   }

   return pNodeData;             // Return getnode data.
}


NPRM_GETNODE_DATA ResourceManager::_DevIDToNodeData ( DEVID DevID , ULONG ulSearchFlags, bool fPciDevice )
/*
;  PURPOSE: Compose the functions
;                _DevIDToHandleList
;                _RMHandleToNodeData (applied to 1st RM handle in handle list)
;
;  IN:      Refer to bIsDevDetected()
;  OUT:     Refer to _RMHandleToNodeData.
;
;  REMARKS: Returns pointer to heap memory allocated.  Caller must ensure heap memory
;           freed after use.
*/
{
   NPHANDLELIST pDevHandleList;        // RM handles for spec'd DEVID's
   NPRM_GETNODE_DATA pNode;            // Node resource data for spec'd DEVID's.

   pDevHandleList = _DevIDToHandleList( DevID, ulSearchFlags, fPciDevice );
                                       // Convert PnP ID -> RM handle.
   if ( pDevHandleList ) {             // If we got a valid handle list
      if ( pDevHandleList->cHandles )  // ... and if we got >0 handles
         pNode = _RMHandleToNodeData( pDevHandleList->Handles[0] );

      delete pDevHandleList;           // We've made the transform, now free memory.
   }

   return pNode;
}


/**@internal _pahRMAllocResources
 *  Allocate a set of resources from OS/2 by interfacing with the
 *  OS/2 resource manager.
 * @param PRESOURCELIST pResourceList - list of resources to allocate.
 * @return PAHRESOURCE - normal return, pointer to an AHRESOURCE structure
 *  as defined by OS/2 RM.  This structure is allocated from the heap and
 *  must be freed by the caller.
 * @return NULL - on failure
 * @notes Logs appropriate errors in global error log if any allocation
 *  problem.
 * @notes Either all resources are allocated, or none.  If there is a
 *  failure within this function when some (but not all) resources are
 *  allocated, then any allocated resources are freed.
 */
NPAHRESOURCE ResourceManager::_pahRMAllocResources( NPRESOURCELIST pResourceList )
{
   APIRET rc;
   USHORT nBytes =
            sizeof(AHRESOURCE) + (sizeof(HRESOURCE) * pResourceList->Count);
   NPAHRESOURCE pahResource = (NPAHRESOURCE) malloc( nBytes );
                                       // Handles for allocated resources are saved here.
   if (!pahResource) return NULL;

   pahResource->NumResource = 0;       // Init # of resources successfully allocated.

   //--- Allocate each resource requested.
   for (int i=0; i < pResourceList->Count; ++i) {
      rc = RMAllocResource( _hDriver,                       // Handle to driver.
                            &pahResource->hResource[i],     // OUT:  "allocated" resource node handle
                            &pResourceList->Resource[i] );  // Resource to allocate.
      if (rc == RMRC_SUCCESS)
         ++pahResource->NumResource;
      else {
         PRESOURCESTRUCT pRes = &pResourceList->Resource[i];
                                       // Pointer to the resource that we can't allocate.

         // Put a good error message out on the resource that couldn't be allocated.
         switch( pRes->ResourceType ) {
         case RS_TYPE_IO:
            break;
         case RS_TYPE_IRQ:
            break;
         case RS_TYPE_DMA:
            break;
         case RS_TYPE_MEM:
            break;
         default:
            break;
         }

         // Deallocate any allocated resources, then fail the function call.
         for (int j=0; i < pahResource->NumResource; ++j)
            RMDeallocResource( _hDriver, pahResource->hResource[j] );
                                       // Deallocate any resources we've reserved.

         delete pahResource;
         return NULL;
      }
   }

   return pahResource;
}

/**@internal _rmCreateAdapter
 *  Create the "adapter" node.  The "adapter" node belongs to this driver's
 *  "driver" node.  Also as part of this operation, the "resource" nodes
 *  associated with this driver will be moved to the "adapter" node.
 * @param None.
 * @notes Changes state of the RM object to 'rmAdapterCreated'.
 * @return APIRET rc - 0 iff good creation.  Returns non-zero and logs a soft
 *  error on failure.
 */
APIRET ResourceManager::_rmCreateAdapter()
{
   APIRET rc;
   ADAPTERSTRUCT AdapterStruct;

   if (_state != rmAdapterCreated) {
      _fmemset( (PVOID) &AdapterStruct, 0, sizeof(AdapterStruct) );
      AdapterStruct.AdaptDescriptName = (PSZ) "SoundBlaster Live!" ;      /* ### IHV */
      AdapterStruct.AdaptFlags        = AS_16MB_ADDRESS_LIMIT;    // AdaptFlags         /* ### IHV */
      AdapterStruct.BaseType          = AS_BASE_MMEDIA;           // BaseType
      AdapterStruct.SubType           = AS_SUB_MM_AUDIO;          // SubType
      AdapterStruct.InterfaceType     = AS_INTF_GENERIC;          // InterfaceType
      AdapterStruct.HostBusType       = AS_HOSTBUS_PCI;           // HostBusType        /* ### IHV */
      AdapterStruct.HostBusWidth      = AS_BUSWIDTH_32BIT;        // HostBusWidth       /* ### IHV */
      AdapterStruct.pAdjunctList      = NULL;                     // pAdjunctList       /* ### IHV */

      //--- Register adapter.  We'll record any error code, but won't fail
      // the driver initialization and won't return resources.
      rc = RMCreateAdapter( _hDriver,          // Handle to driver
                            &_hAdapter,        // (OUT) Handle to adapter
                            &AdapterStruct,    // Adapter structure
                            NULL,              // Parent device (defaults OK)
                            NULL );            // Allocated resources.  We assign ownership
                                               // of the IO, IRQ, etc. resources to the
                                               // device (via RMCreateDevice()), not to the
                                               // adapter (as done in disk DD sample).
      if (rc == RMRC_SUCCESS)
         _state = rmAdapterCreated;
   }
   return rc;
}

/**@internal _rmCreateDevice
 *  Create Device node in the OS/2 RM allocation tree.  Device nodes belong
 *  to the Adapter node, just like the Resource nodes.
 * @param PSZ pszName - Descriptive name of device.
 * @param PAHRESOURCE pahResource - Handles of allocated resources that are
 *  owned by this device.
 * @return APIRET rc - Value returned by RMCreateDevice() call.
 * @notes Same "soft" error strategy as adapter registration: we'll record
 *  any errors but hold onto the resources and continue to operate the
 *  driver.
 * @notes
 */
APIRET ResourceManager::_rmCreateDevice( PSZ pszName, NPAHRESOURCE pahResource )
{
   DEVICESTRUCT DeviceStruct;
   HDEVICE hDevice;
   APIRET rc;

   _fmemset( (PVOID) &DeviceStruct, 0, sizeof(DeviceStruct) );
   DeviceStruct.DevDescriptName = pszName;
   DeviceStruct.DevFlags        = DS_FIXED_LOGICALNAME;
   DeviceStruct.DevType         = DS_TYPE_AUDIO;
   DeviceStruct.pAdjunctList    = NULL;

   rc = RMCreateDevice( _hDriver,      // Handle to driver
                        &hDevice,      // (OUT) Handle to device, unused.
                        &DeviceStruct, // Device structure
                        _hAdapter,     // Parent adapter
                        pahResource ); // Allocated resources
   return rc;
}


#define PCI_CONFIG_ENABLE       0x80000000
#define PCI_CONFIG_ADDRESS      0xCF8
#define PCI_CONFIG_DATA         0xCFC

unsigned long _inpd(unsigned short);
#pragma aux _inpd "_inpd" \
  parm   [dx] \
  value  [dx ax];

//COMPILER BUG: bx cx == cx bx
void _outpd(unsigned short, unsigned long);
#pragma aux _outpd "_outpd" \
  parm   [dx] [cx bx] \
  modify [ax dx];

BOOL ResourceManager::getPCIConfiguration(ULONG pciId)
{
 ULONG devNr, busNr, funcNr, temp, cfgaddrreg, detectedId;
 BOOL  found = FALSE;

	cfgaddrreg = _inpd(PCI_CONFIG_ADDRESS);
      	for(busNr=0;busNr<255;busNr++)     //BusNumber<255
      	{
  		for(devNr=0;devNr<32;devNr++)
  		{
			for(funcNr=0;funcNr<8;funcNr++) 
			{
		                temp = ((ULONG)((ULONG)devNr<<11UL) + ((ULONG)busNr<<16UL) + ((ULONG)funcNr << 8UL));

		                _outpd(PCI_CONFIG_ADDRESS, PCI_CONFIG_ENABLE|temp);
				detectedId = _inpd(PCI_CONFIG_DATA);
		                if(detectedId == pciId)
		                {
		                        found = TRUE;
		                        break;
		                }
			}
			if(found) break;
		}
		if(found) break;
        }

        if(!found) {
                _outpd(PCI_CONFIG_ADDRESS, cfgaddrreg);
                return FALSE;
        }

        for(int i=0;i<64;i++)
        {
                temp = ((ULONG)((ULONG)devNr<<11UL) + ((ULONG)busNr<<16UL) + ((ULONG)funcNr << 8UL) + (i << 2));
                _outpd(PCI_CONFIG_ADDRESS, PCI_CONFIG_ENABLE|temp);

                PCIConfig[i] = _inpd(PCI_CONFIG_DATA);
        }
        _outpd(PCI_CONFIG_ADDRESS, cfgaddrreg);

	pciConfigData = (PCIConfigData *)&PCIConfig[0];

        if(pciConfigData->Bar0 == 0 || pciConfigData->Bar0 == 0xFFFFFFFF)
        {
		DebugInt3();
                return(FALSE);
        }
        return TRUE;
}

