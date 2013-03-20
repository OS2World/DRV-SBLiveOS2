/* $Id: ossidc.cpp,v 1.3 2000/05/28 16:50:44 sandervl Exp $ */

//******************************************************************************
// OS/2 IDC services (callback to 16 bits MMPM2 driver)
//
// Copyright 2000 Sander van Leeuwen (sandervl@xs4all.nl)
//
//     This program is free software; you can redistribute it and/or
//     modify it under the terms of the GNU General Public License as
//     published by the Free Software Foundation; either version 2 of
//     the License, or (at your option) any later version.
//
//     This program is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU General Public License for more details.
//
//     You should have received a copy of the GNU General Public
//     License along with this program; if not, write to the Free
//     Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139,
//     USA.
//
//******************************************************************************
#define INCL_NOPMAPI
#define INCL_DOSERRORS           // for ERROR_INVALID_FUNCTION
#include <os2.h>
#include <ossdefos2.h>
#include <ossidc.h>
#include <devhelp.h>
#ifdef KEE
#include <kee.h>
#endif

//******************************************************************************
//******************************************************************************
BOOL CallOSS16(ULONG cmd, ULONG param1, ULONG param2)
{
  IDC16_PACKET idcpacket;
  ULONG        pPacket;
  BOOL         rc;

  if(idc16_PddHandler == 0) {
	return FALSE;
  }
  idcpacket.param1 = param1;
  idcpacket.param2 = param2;
#ifdef KEE
  pPacket          = (ULONG)__FlatToStack(&idcpacket);
#else
  pPacket          = (ULONG)__Compress48Pointer((char FAR48 *)&idcpacket);
#endif

#ifdef KEE
  //NOTE: This isn't safe if the compiler uses ebp (or esp) to hold 
  //      idc16_PddHandler, cmd or pPacket!! (works now though)
  KernThunkStackTo16();
////  KernSerialize16BitDD();
#endif
  rc = idc16_PddHandler(cmd, pPacket);
#ifdef KEE
////  KernUnserialize16BitDD();
  KernThunkStackTo32();
#endif

  return rc;
}
//******************************************************************************
//******************************************************************************
BOOL OSS32_SetIrq(int irq, ULONG handler)
{
  return CallOSS16(IDC16_SETIRQ, irq, handler);
}
//******************************************************************************
//******************************************************************************
BOOL OSS32_FreeIrq(int irq)
{
  return CallOSS16(IDC16_FREEIRQ, irq, 0);
}
//******************************************************************************
extern "C" int  init_module();
extern "C" void cleanup_module();
//******************************************************************************
BOOL OSS32_InitDriver()
{
  return init_module() == 0;
}
//******************************************************************************
//Called during OS/2 shutdown
//******************************************************************************
void OSS32_RemoveDriver()
{
  cleanup_module();
}
//******************************************************************************
//******************************************************************************
extern "C" int OSS32_ProcessIRQ(int fWaveOut, unsigned long streamid)
{
  return CallOSS16(IDC16_PROCESS, (fWaveOut) ? IDC16_WAVEOUT_IRQ : IDC16_WAVEIN_IRQ, streamid);
}
//******************************************************************************
//******************************************************************************
extern "C" int OSS32_ProcessMidiIRQ(unsigned long streamid)
{
  return CallOSS16(IDC16_PROCESS, IDC16_MIDI_IRQ, streamid);
}
//******************************************************************************
//******************************************************************************
