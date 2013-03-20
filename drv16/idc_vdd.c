/* $Id: idc_vdd.c,v 1.1 2000/04/23 14:55:16 ktk Exp $ */

#define  INCL_16
#define  INCL_DOSINFOSEG
#include <os2.h>

#include "idc_vdd.h"
#include <include.h>
#include <devhelp.h>

ULONG pLincodec = 0;

ADAPTERINFO __far * pfcodec_info;
ADAPTERINFO codec_info;
USHORT usInUseCount = 0;
USHORT usVDDHasHardware = FALSE;

#pragma off (unreferenced);


//                                  -------------------------- IDCVddOPENPDD -
// OS/2 MVDM kernel calls us to provide the IDC address
// of the Virtual Device driver.
// The VDD can also call (with address of NULL) to terminate
// future IDC between the drivers.
//
ULONG IDCVddOPENPDD (ULONG ul1, ULONG ul2)
{
   // Nothing to do, we don't call the VDD
   // ###should save the vdd entry point so we can call him later...
   // we should inform him of shutdowns, apm events and when the hardware
   // becomes free......
   return (TRUE);
}
//                                  -------------------- IDCVddQUERYPROTOCOL -
ULONG IDCVddQUERYPROTOCOL (ULONG ul1, ULONG ul2)
{
   // if the VDD sends us Dead Beef
   // We Return Dead Beef just like Hudson Foods !!
   if (ul1 == IDC_PROTOCOL_LEVEL) {
      return (IDC_PROTOCOL_LEVEL);
   }
   // Else Return junk
   else {
      return (-1);
   } /* endif */
}

//                                 ---------------------- IDCVddASSIGNHANDLE -
ULONG IDCVddASSIGNHANDLE (ULONG ul1, ULONG ul2)
{
   return (FALSE);
}
//                                   ---------------------- IDCVddOPENDEVICE -
ULONG IDCVddOPENDEVICE (ULONG ul1, ULONG ul2)
{
   if (usInUseCount == 0) {
      usVDDHasHardware = TRUE;
      return (TRUE);
   }
   else {
      return (FALSE);
   } /* endif */
}
//                                    -------------------- IDCVddCLOSEDEVICE -
ULONG IDCVddCLOSEDEVICE (ULONG ul1, ULONG ul2)
{
   usVDDHasHardware = FALSE;
   return (TRUE);
}
//                                    ------------------ IDCVddGETIOPORTINFO -
ULONG IDCVddGETIOPORTINFO (ULONG ul1, ULONG ul2)
{
   return (pLincodec);
}
//                                   --------------------- IDCVddTRUSTEDOPEN -
ULONG IDCVddTRUSTEDOPEN (ULONG ul1, ULONG ul2)
{
   return (FALSE);
}


ULONG (*IDCVddFuncs[]) (ULONG ul1, ULONG ul2) =
{
   IDCVddOPENPDD,
   IDCVddQUERYPROTOCOL,
   IDCVddASSIGNHANDLE,
   IDCVddOPENDEVICE,
   IDCVddCLOSEDEVICE,
   IDCVddGETIOPORTINFO,
   IDCVddTRUSTEDOPEN
};
USHORT MaxIDCVddFuncs = sizeof(IDCVddFuncs)/sizeof(USHORT);


//                                      ---------------------- IDCEntry_VDD_c -
// Interface for other VDDs to call this device driver
//
ULONG pascal IDCEntry_VDD_c (ULONG ulFunc, ULONG ul1, ULONG ul2)
{
   if (ulFunc > MaxIDCVddFuncs) {
      return (FALSE);
   }
   else {
      return (IDCVddFuncs [ulFunc](ul1,ul2));
   }
}
