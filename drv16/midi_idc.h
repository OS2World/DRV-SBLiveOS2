/* $Id: midi_idc.h,v 1.1 2000/04/23 14:55:17 ktk Exp $ */

/* MIDI_IDC.H

   MODIFICATION HISTORY
   DATE       PROGRAMMER   COMMENT
   01-Jul-95  Timur Tabi   Creation
*/

#ifndef MIDI_IDC_INCLUDED
#define MIDI_IDC_INCLUDED

#ifndef OS2_INCLUDED
#define INCL_NOPMAPI
#include <os2.h>
#endif

#define MIDI_NAME_LENGTH   32

//
// DEFINITIONS AND PROTOTYPES FOR TYPE A REGISTRATION
//

// The capabilities flags for Type A drivers
#define MIDICAPSA_INPUT             0x00000001     // driver supports receiving data
#define MIDICAPSA_OUTPUT            0x00000002     // driver can send data
#define MIDICAPSA_ALSO_DEVICE       0x00000004     // driver represents a device
#define MIDICAPSA_USES_BRIDGE       0x00000008     // candidate node for MMPM/2 bridge
#define MIDICAPSA_NOT_DEFAULT       0x00000010     // don't let this node be a default HW node
#define MIDICAPSA_RUNNING_STATUS    0x00000020     // the driver/device can accpt running status

// definitions used for the Open and Close commands
#define MIDIMODE_OPEN_RECEIVE       0        // open for receiving data from MIDI.SYS
#define MIDIMODE_OPEN_SEND          1        // open for sending data to MIDI.SYS

// error codes that the PDD returns to the MIDI driver
#define MIDIERRA_GEN_FAILURE        1        // if it's not one of the other errors
#define MIDIERRA_NO_HARDWARE        2        // hardware just isn't there
#define MIDIERRA_HW_FAILED          3        // hw is there, but can't be accessed
#define MIDIERRA_CANT_GET_IRQ       4        // IRQ is unavailable
#define MIDIERRA_CANT_GET_DMA       5        // DMA is unavailable
#define MIDIERRA_IN_USE             6        // device is in use by something else

// error codes that the MIDI driver returns to the PDD after a failed registration
#define MIDIERRA_BAD_PARAMETER      1        // something was wrong with the MIDIREG_TYPEA structure passed
#define MIDIERRA_NAME_EXISTS        2        // the name for this device is used by another device
#define MIDIERRA_OUT_OF_RESOURCES   3        // The MIDI driver doesn't have enough resources
#define MIDIERRA_INTERNAL_SYS       4        // something's really wrong with the MIDI driver

typedef struct {
   BYTE bCategory;                     // category code 
   BYTE bCode;                         // function code 
   void __far *pvParm;                 // address of parameter buffer 
   void __far *pvData;                 // address of data buffer 
   USHORT usSysFileNum;                // system file number 
   USHORT usPLength;                   // length of parameter buffer
   USHORT usDLength;                   // length of data buffer
} IOCTL_RP, __far *PIOCTL_RP;

typedef void (__far __loadds __cdecl *PFNMIDI_SENDBYTE) (ULONG ulHandle, BYTE bData);
// called by the PDD to send 1 byte of MIDI data to the MIDI driver

typedef void (__far __loadds __cdecl *PFNMIDI_SENDSTRING) (ULONG ulHandle, BYTE __far *pbData, USHORT usLength);
// called by the PDD to send a sequence of MIDI data to the MIDI driver

typedef void (__far __loadds __cdecl *PFNMIDI_DEREGISTER) (ULONG ulHandle);
// called by the PDD to tell the MIDI driver that it's been uninstalled/removed
// don't call this during interrupt time!


typedef USHORT (__far __loadds __cdecl *PFNMIDI_RECVSTRING) (USHORT usDevId, BYTE __far *pbData, USHORT usLength);
// called by the MIDI driver to send a string of data to the PDD
// returns error code

typedef USHORT (__far __loadds __cdecl *PFNMIDI_RECVBYTE) (USHORT usDevId, BYTE bData);
// called by the MIDI driver to send a single byte to the PDD
// returns error code

typedef USHORT (__far __loadds __cdecl *PFNMIDI_OPENCLOSE) (USHORT usDevId, USHORT usMode);
// called by the MIDI driver to open or close the PDD
// usMode = 0 for input (sending data to MIDI driver), = 1 for output
// returns error code

typedef USHORT (__far __loadds __cdecl *PFNMIDI_IOCTL) (PIOCTL_RP);
// called by the MIDI driver to re-route an MMPM/2 IOCtl to the PDD

typedef struct {
   USHORT usSize;       // set this equal to sizeof(MIDIREG_TYPEA) before calling!
   struct {                               // filled by Type A driver
      ULONG flCapabilities;               // the capabilities (MIDICAPSA_xxxx)
      char __far *pszInstanceName;        // ptr to instance name, must be a valid ASCIIZ
      PFNMIDI_OPENCLOSE pfnOpen;          // can be NULL
      PFNMIDI_OPENCLOSE pfnClose;         // can be NULL
      PFNMIDI_RECVBYTE pfnRecvByte;       // can be NULL if MIDICAPSA_INPUT is not set
      PFNMIDI_RECVSTRING pfnRecvString;   // can be NULL if MIDICAPSA_INPUT is not set
      PFNMIDI_IOCTL pfnIOCtl;             // can be NULL if MIDICAPSA_USESBRIDGE is not set
      USHORT usDevId;                     // the DevID the MIDI driver should use
   } in;
   struct {                               // filled by MIDI.SYS
      ULONG ulHandle;                     // device handle
      PFNMIDI_SENDBYTE pfnSendByte;
      PFNMIDI_SENDSTRING pfnSendString;
      PFNMIDI_DEREGISTER pfnDeregister;
   } out;
} MIDIREG_TYPEA, __far *PMIDIREG_TYPEA;

typedef USHORT (__far __cdecl __loadds *PFN_REGA) (PMIDIREG_TYPEA);
// Used by the PDD to register a Type A driver with the MIDI driver.
// returns error code

//
// DEFINITIONS AND PROTOTYPES FOR TYPE B REGISTRATION
//

// error codes that the PDD returns to the MIDI driver
#define MIDIERRB_BAD_CONFIG_SIZE    1        // the usLength field in pfnConfig is bad
#define MIDIERRB_BAD_CONFIG_DATA    2        // the config data format is bad

// error codes that the MIDI driver returns to the PDD after a failed registration
#define MIDIERRB_BAD_PARAMETER      1        // something was wrong with the MIDIREG_TYPEB structure passed
#define MIDIERRB_CANT_SEND_MSG      2        // pfnSendMessage failed: scheduler queue is full

#ifndef MIDI_MESSAGE_DEFINED

typedef struct {
   ULONG ulSourceNode;
   ULONG ulTime;
   ULONG ulTrack;
   union {
      ULONG ulMessage;
      struct {
         BYTE bStatus;     // The 1st byte of the message
         BYTE abData[3];   // the rest of the message
      } bytes;
      BYTE abData[4];
   } msg;
} MESSAGE, __far *PMESSAGE;

#define MIDI_MESSAGE_DEFINED
#endif

typedef USHORT (__far __loadds __cdecl *PFNMIDIB_PROCESSMESSAGE) (
   void __far *pvInstanceData, 
   MESSAGE __far *pmsg
   );

typedef USHORT (__far __loadds __cdecl *PFNMIDIB_CONFIGURE) (
   void __far *pvInstanceData,         // ptr to instance data
   void __far *pvConfigData,           // ptr to config data buffer
   USHORT usLength                     // length of config data buffer
   );

typedef USHORT (__far __loadds __cdecl *PFNMIDIB_QUERY) (
   void __far *pvInstanceData,         // ptr to instance data
   void __far *pvQueryBuffer,          // ptr to query buffer
   USHORT usLength                     // length of query buffer
   );

typedef USHORT (__far __loadds __cdecl *PFNMIDIB_INSTCREATED) (
   void __far *pvInstanceData,         // ptr to instance data
   ULONG ulInstance                    // the instance number
   );

typedef void (__far __loadds __cdecl *PFNMIDIB_INSTDELETED) (
   void __far *pvInstanceData          // ptr to instance data
   );


typedef USHORT (__far __loadds __cdecl *PFNMIDIB_SENDMESSAGE) (
   ULONG ulInstance,                   // inst # from pfnCreated
   MESSAGE __far *pmsg,                // ptr to message
   USHORT usSlot                       // slot # of targets to send to
   );

typedef struct {
   USHORT usSize;       // set this equal to sizeof(MIDIREG_TYPEB) before calling!
   struct {                            // filled by Type B driver
      ULONG flCapabilities;            // the capabilities (MIDICAPSB_xxxx)
      USHORT usNumSlots;               // # of slots for this class
      USHORT usInstanceDataSize;       // size of instance data for this class
      char __far *pszClassName;        // ptr to class name, must be a valid ASCIIZ
      PFNMIDIB_CONFIGURE pfnConfigure;             // called to configure instance
      PFNMIDIB_QUERY pfnQuery;                     // called to obtain config info
      PFNMIDIB_INSTCREATED pfnCreated;             // called when inst is created
      PFNMIDIB_INSTDELETED pfnDeleted;             // called when inst is deleted
      PFNMIDIB_PROCESSMESSAGE pfnProcessMessage;   // called to process a message
   } in;
   struct {                                     // filled by MIDI.SYS
      ULONG ulClassNumber;                      // class number of this new class
      PFNMIDIB_SENDMESSAGE pfnSendMessage;      // call to send msg to all nodes on a slot
   } out;
} MIDIREG_TYPEB, __far *PMIDIREG_TYPEB;

typedef USHORT (__far __cdecl __loadds *PFN_REGB) (PMIDIREG_TYPEB);
// Used by the PDD to register a Type B driver with the MIDI driver.
// returns error code

//
// DEFINITIONS AND PROTOTYPES FOR TYPE C REGISTRATION
//

// error codes that the MIDI driver returns to the PDD after a failed registration
#define MIDIERRC_BAD_PARAMETER      1        // something was wrong with the MIDIREG_TYPEC structure passed

typedef USHORT (__far __loadds __cdecl *PFNMIDIC_OPENCLOSE) (void);
// called by the MIDI driver to open or close the PDD
// returns error code

typedef struct {
   USHORT usSize;       // set this equal to sizeof(MIDIREG_TYPEC) before calling!
   struct {                               // filled by Type C driver
      ULONG flCapabilities;               // the capabilities (MIDICAPSC_xxxx)
      char __far *pszTimesourceName;      // ptr to timesource name, must be a valid ASCIIZ
      PFNMIDIC_OPENCLOSE pfnOpen;
      PFNMIDIC_OPENCLOSE pfnClose;
   } in;
   struct {                               // filled by MIDI.SYS
      volatile ULONG __far *pulMIDIClock; // pointer to 32-bit MIDI timer count
      USHORT __far *pfTimerActive;        // if non-zero, then MIDI timer is running
   } out;
} MIDIREG_TYPEC, __far *PMIDIREG_TYPEC;

typedef USHORT (__far __cdecl __loadds *PFN_REGC) (PMIDIREG_TYPEC);
// Used by the PDD to register a Type C driver with the MIDI driver.
// returns error code

// definitions for all drivers

#define MIDI_VERSION_DEV      0
#define MIDI_VERSION_ALPHA    1
#define MIDI_VERSION_BETA     2
#define MIDI_VERSION_GA       3

#define MIDI_DDVER_MAJOR(x)   ( ((USHORT) (x)) >> 14 )
#define MIDI_DDVER_MINOR(x)   ( (((USHORT) (x)) >> 10) & 15)
#define MIDI_DDVER_BUGFIX(x)  ( (((USHORT) (x)) >> 6) & 15)
#define MIDI_DDVER_PHASE(x)   ( (((USHORT) (x)) >> 4) & 3)
#define MIDI_DDVER_BUILD(x)   ( ((USHORT) (x)) & 3)

typedef struct {
   USHORT usSize;       // set this equal to sizeof(MIDI_REGISTER) before calling!
   USHORT usVersion;
   PFN_REGA pfnRegisterA;
   PFN_REGB pfnRegisterB;
   PFN_REGC pfnRegisterC;
} MIDI_REGISTER, __far *PMIDI_REGISTER;

typedef void (__far __cdecl __loadds *PFN_MIDIIDC) (PMIDI_REGISTER);

typedef struct { 
   USHORT      ausReserved[3];     // 3 reserved words
   PFN_MIDIIDC pfn;                // far pointer to IDC entry
   USHORT      ds;                 // data segment of IDC
} MIDI_ATTACH_DD;

#endif
