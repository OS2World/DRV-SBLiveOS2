/* $Id: audiohw.cpp,v 1.1 2000/04/23 14:55:15 ktk Exp $ */

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
 * @notes
 * Parent audio hardware class all audio hardware objects will be decendents
 * of this class
 * @version %I%
 * @context Unless otherwise noted, all interfaces are Ring-0, 16-bit,
 *  <stack context>.
 * @history
 *
 */
#define INCL_NOPMAPI
#include <os2.h>
#include <os2medef.h>
#include <audio.h>
#include <include.h>
#include "audiohw.hpp"

PQUEUEHEAD pAudioHWList;        // Queuehead for all audio HW objects created
                                // during driver initialization.
hardware_index HardwareArray[NUM_HARDWARE_DEVICE_TYPES] = {
   {
     AUDIOHW_WAVE_PLAY,
      0,
      DATATYPE_WAVEFORM,
      OPERATION_PLAY
   },
   {
      AUDIOHW_WAVE_PLAY,
      0,
      PCM,
      OPERATION_PLAY
   },
   {
      AUDIOHW_WAVE_PLAY,
      0,
      DATATYPE_ALAW,
      OPERATION_PLAY
   },
   {
      AUDIOHW_WAVE_PLAY,
      0,
      DATATYPE_RIFF_ALAW,
      OPERATION_PLAY
   },
   {
      AUDIOHW_WAVE_PLAY,
      0,
      A_LAW,
      OPERATION_PLAY
   },
   {
      AUDIOHW_WAVE_PLAY,
      0,
      DATATYPE_MULAW,
      OPERATION_PLAY
   },
   {
      AUDIOHW_WAVE_PLAY,
      0,
      DATATYPE_RIFF_MULAW,
      OPERATION_PLAY
   },
   {
      AUDIOHW_WAVE_PLAY,
      0,
      MU_LAW,
      OPERATION_PLAY
   },
   {
      AUDIOHW_WAVE_CAPTURE,
      0,
      DATATYPE_WAVEFORM,
      OPERATION_RECORD
   },
   {
      AUDIOHW_WAVE_CAPTURE,
      0,
      PCM,
      OPERATION_RECORD
   },
   {
      AUDIOHW_WAVE_CAPTURE,
      0,
      DATATYPE_ALAW,
      OPERATION_RECORD
   },
   {
      AUDIOHW_WAVE_CAPTURE,
      0,
      DATATYPE_RIFF_ALAW,
      OPERATION_RECORD
   },
   {
      AUDIOHW_WAVE_CAPTURE,
      0,
      A_LAW,
      OPERATION_RECORD
   },
   {
      AUDIOHW_WAVE_CAPTURE,
      0,
      DATATYPE_MULAW,
      OPERATION_RECORD
   },
   {
      AUDIOHW_WAVE_CAPTURE,
      0,
      DATATYPE_RIFF_MULAW,
      OPERATION_RECORD
   },
   {
      AUDIOHW_WAVE_CAPTURE,
      0,
      MU_LAW,
      OPERATION_RECORD
   },
   {  // Last device is marked as AUDIOHW_INVALID_DEVICE
      AUDIOHW_INVALID_DEVICE,
      -1,
      -1,
      -1
   },
   {  // Last device is marked as AUDIOHW_INVALID_DEVICE
      AUDIOHW_INVALID_DEVICE,
      -1,
      -1,
      -1
   },
   {  // Last device is marked as AUDIOHW_INVALID_DEVICE
      AUDIOHW_INVALID_DEVICE,
      -1,
      -1,
      -1
   },
   {  // Last device is marked as AUDIOHW_INVALID_DEVICE
      AUDIOHW_INVALID_DEVICE,
      -1,
      -1,
      -1
   },
   {  // Last device is marked as AUDIOHW_INVALID_DEVICE
      AUDIOHW_INVALID_DEVICE,
      -1,
      -1,
      -1
   },
   {  // Last device is marked as AUDIOHW_INVALID_DEVICE
      AUDIOHW_INVALID_DEVICE,
      -1,
      -1,
      -1
   },
   {  // Last device is marked as AUDIOHW_INVALID_DEVICE
      AUDIOHW_INVALID_DEVICE,
      -1,
      -1,
      -1
   },
   {  // Last device is marked as AUDIOHW_INVALID_DEVICE
      AUDIOHW_INVALID_DEVICE,
      -1,
      -1,
      -1
   },
   {  // Last device is marked as AUDIOHW_INVALID_DEVICE
      AUDIOHW_INVALID_DEVICE,
      -1,
      -1,
      -1
   },
   {  // Last device is marked as AUDIOHW_INVALID_DEVICE
      AUDIOHW_INVALID_DEVICE,
      -1,
      -1,
      -1
   },
   {  // Last device is marked as AUDIOHW_INVALID_DEVICE
      AUDIOHW_INVALID_DEVICE,
      -1,
      -1,
      -1
   },
   {  // Last device is marked as AUDIOHW_INVALID_DEVICE
      AUDIOHW_INVALID_DEVICE,
      -1,
      -1,
      -1
   },
   {  // Last device is marked as AUDIOHW_INVALID_DEVICE
      AUDIOHW_INVALID_DEVICE,
      -1,
      -1,
      -1
   },
   {  // Last device is marked as AUDIOHW_INVALID_DEVICE
      AUDIOHW_INVALID_DEVICE,
      -1,
      -1,
      -1
   }

};

/**@internal GetHardwareDevice
 * @param    ULONG    the device type being queried (see audiohw.hpp)
 * @return   PAUDIOHW the address of the correct hardware object
 * @return   NULL     ERROR hardware object not found
 * @notes
 * Globally scoped function to get the address of a hardware object based on
 * the type.
 */
PAUDIOHW GetHardwareDevice(ULONG Devicetype)
{
    PAUDIOHW pEle = (PAUDIOHW)pAudioHWList->Head();
    while (pEle != NULL) {
       if (pEle->ulDeviceType == Devicetype)
          return(pEle);
       pEle = (PAUDIOHW)pEle->pNext;
    } /* endwhile */
    return(NULL);
}
/**@internal SetHardwareType
 * @param    ULONG
 * @param    USHORT
 * @param    USHORT
 * @return   No return value
 * @notes
 * Globally scoped function
 *
 */
void SetHardwareType(ULONG HardwareType, USHORT DataType, USHORT Operation, USHORT LDev)
{
   int i;

   for (i = 0;
       ((i < NUM_HARDWARE_DEVICE_TYPES) &&
       (HardwareArray[i].DeviceType != AUDIOHW_INVALID_DEVICE)); ++ i) {
      if ((HardwareArray[i].DeviceDataType == DataType) &&
         (HardwareArray[i].DeviceOperation == Operation) &&
         (HardwareArray[i].LogicalDevice == LDev))
         break;
   } /* endfor */
     // if i is not pointing to the last entry in the table then either we
     // are either pointing to an empty table entry or one that we want
     // to change. like the midi entries......
     //
   if (i < (NUM_HARDWARE_DEVICE_TYPES - 1)) { // not at last entry
      HardwareArray[i].DeviceType = HardwareType;
      HardwareArray[i].LogicalDevice = LDev;
      HardwareArray[i].DeviceDataType = DataType;
      HardwareArray[i].DeviceOperation = Operation;
      HardwareArray[i + 1].DeviceType = AUDIOHW_INVALID_DEVICE;
   }
}
/**@internal GetHardwareType
 * @param    USHORT
 * @param    USHORT
 * @return   ULONG
 * @return   AUDIOHW_INVALID_DEVICE if not found
 * @notes
 * Globally scoped function
 *
 */
ULONG GetHardwareType(USHORT DataType, USHORT Operation, USHORT LDev)
{
   int i;

   for (i = 0;
       ((i < NUM_HARDWARE_DEVICE_TYPES) &&
       (HardwareArray[i].DeviceType != AUDIOHW_INVALID_DEVICE)); ++ i) {
      if ((HardwareArray[i].DeviceDataType == DataType) &&
         (HardwareArray[i].DeviceOperation == Operation) &&
         (HardwareArray[i].LogicalDevice == LDev))
         break;
   } /* endfor */
   return (HardwareArray[i].DeviceType);

}
