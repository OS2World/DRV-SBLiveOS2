/* $Id: idc_vdd.h,v 1.1 2000/04/23 14:55:16 ktk Exp $ */

#ifndef VDDIDC_INCLUDED
#define VDDIDC_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif


#pragma pack (1)        // Force byte structure alignment

#ifndef __IBMC__        // Following is only for the PDD and VDD
ULONG FAR pascal IDCEntry_VDD (ULONG ulFunc, ULONG ul1, ULONG ul2); // ASM
#endif

typedef enum
        {
        IDCOPENPDD,
        IDCQUERYPROTOCOL,
        IDCASSIGNHANDLE,
        IDCOPENDEVICE,
        IDCCLOSEDEVICE,
        IDCGETIOPORTINFO,
        IDCTRUSTEDOPEN
        } IDCFUNCS;

#pragma pack ()

//
// Define ulFuncs for communicaiton between
// PDD and VDD IDC entrypoints.
// Also define version constant to indicate the highest
// functions we will send and can receive.
//

#define PDDFUNC_OPENPDD         0       // Open request sent to PDD by OS
#define PDDFUNC_QUERYPROTOCOL   1       // Insure PDD & VDD are of same level
#define PDDFUNC_ASSIGNHANDLE    2       // Inform PDD of its IDC handle
#define PDDFUNC_OPENDEVICE      3       // Request adapter owndership
#define PDDFUNC_CLOSEDEVICE     4       // Relinquish HW back to PDD control
#define PDDFUNC_GETIOPORTINFO   5       // Provide address of port I/O info
#define PDDFUNC_TRUSTEDOPEN     6       // Trusted Open to PDD

// Inter-Device Communication (IDC) function
// codes for calls from PDD
#define VDDFUNC_INTERRUPT       0       // (not used)
#define VDDFUNC_CLOSE           1       // Audio PDD is done with hardware
#define VDDFUNC_SHUTDOWN        2       // PDD informs VDD of shutdown

#define IDC_PROTOCOL_LEVEL      0xDEADBEEF
#define NUMIORANGES 4         // 4 io ranges on the card WSS,OPL3,SB and SYNTH

// PDD returns a pointer to ADAPTERINFO data structure
// to describe its hardware.
typedef struct
   {
   ULONG ulPort;
   ULONG ulRange;
   } IORANGE;

typedef struct
   {
   USHORT  usIRQLevel;
   USHORT  usDMALevel;
   ULONG   ulNumPorts;                  // number of port ranges
   IORANGE Range[NUMIORANGES];         // Unbounded array holding IORANGEs
} ADAPTERINFO, *PADAPTERINFO;

extern ULONG pLincodec;
extern ADAPTERINFO __far * pfcodec_info;
extern ADAPTERINFO codec_info;
extern USHORT usInUseCount;
extern USHORT usVDDHasHardware;

#ifdef __cplusplus
}
#endif

#endif
