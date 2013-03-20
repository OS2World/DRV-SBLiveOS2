/* $Id: audiohw.hpp,v 1.1 2000/04/23 14:55:15 ktk Exp $ */

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
 * Defines, class definations and prototypes for Audiohw object
 * @version %I%
 * @context Unless otherwise noted, all interfaces are Ring-0, 16-bit,
 *  <stack context>.
 * @history
 *
 */
#ifndef AUDIOHW_INCLUDED
#define AUDIOHW_INCLUDED

#ifndef OS2_INCLUDED
#define INCL_NOPMAPI
#include <os2.h>
#endif
#ifndef AUDIO_CAPABILITY
#include <audio.h>                      // PAUDIO_CAPS
#endif
#include "queue.hpp"

// Audio Hardware Device Types values for ulDeviceType
#define AUDIOHW_INVALID_DEVICE     0xFFFFFFFF
#define AUDIOHW_READ_DEVICE        0x00000000
#define AUDIOHW_WRITE_DEVICE       0x00000001
#define AUDIOHW_WAVE_CAPTURE       0x00000010
#define AUDIOHW_WAVE_PLAY          0x00000011
#define AUDIOHW_FMSYNTH_CAPTURE    0x00000020
#define AUDIOHW_FMSYNTH_PLAY       0x00000021
#define AUDIOHW_MPU401_CAPTURE     0x00000040
#define AUDIOHW_MPU401_PLAY        0x00000041
#define AUDIOHW_TIMER              0x00000080

#define NUM_HARDWARE_DEVICE_TYPES 32

extern PQUEUEHEAD  pAudioHWList;

class AUDIOHW;
class STREAM;

class AUDIOHW : public QUEUEELEMENT {
protected:
   AUDIOHW(ULONG devicetype) :
      ulDeviceType (devicetype)
   {pAudioHWList->PushOnTail(this);};

public:
   const ULONG ulDeviceType;
   virtual int Start(STREAM *stream) = 0;        // Start the operation
   virtual int Stop(STREAM *stream) = 0;         // Stop the operation
   virtual int Pause(STREAM *stream) = 0;        // Pause the operation
   virtual int Resume(STREAM *stream) = 0;        // Resume the operation
   virtual void DevCaps(PAUDIO_CAPS pCaps) = 0;
};
typedef AUDIOHW *PAUDIOHW;

// Globally Scoped function that returns the pointer to a particular
// hardware device.
PAUDIOHW GetHardwareDevice(ULONG Devicetype);

// Globally Scoped function to setup a hardware data type
void SetHardwareType(ULONG HardwareType, USHORT DataType, USHORT Operation, USHORT LDev);

// Globally Scoped function associate hardware types to the type of data files
// they can play or record....
ULONG GetHardwareType(USHORT DataType, USHORT Operation, USHORT LDev);

// Hardware Index Class
// MMPM/2 when sends an Audio Ioctl Init or Audio Ioctl Capability to the
// driver,it includes a Data Type (see AUDIO.H and OS2MEDEF.H). The driver needs
// to associate the data type with the hardware class that process it. The
// the hardware_index class holds information that identifies which hardware
// classes can handle a particular data type and what operations (play or
// record) it can provide for that data type
class hardware_index {
public:
   ULONG  DeviceType;      // The audiohw data type (see above)
   USHORT LogicalDevice;   // The strategy entry this request came from.
                           // MMPM/2 will not send requests for 2 like devices
                           // the same driver, but it can be "fooled" into
                           // thinking there are 2 drivers each capable of
                           // 1 of said devices..
   USHORT DeviceDataType;  // the DataType frim MMPM/2 (see AUDIO.H/OS2MEDEF.H)
   USHORT DeviceOperation; // the operation type (see AUDIO.H
                           // AUDIO_INIT.ulOperation)
};

#endif
