/* $Id: dwavestrm.hpp,v 1.3 2001/04/30 21:07:57 sandervl Exp $ */

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
 * Defines, class definations and prototypes for
 * @version %I%
 * @context Unless otherwise noted, all interfaces are Ring-0, 16-bit,
 *  <stack context>.
 * @history
 *
 */
#ifndef DWAVESTREAM_INCLUDED
#define DWAVESTREAM_INCLUDED

#ifndef OS2_INCLUDED
#define INCL_NOPMAPI
#include <os2.h>
#include <os2medef.h>
#include <audio.h>
#endif

#include "stream.hpp"
#include "wavestrm.hpp"
#include "waudio.hpp"
#include "strmbuff.hpp"

extern "C" {
void HookHandlerAsm();
void cdecl HookHandler(ULONG ulSysFileNum);
}

class DWAVESTREAM : public WAVESTREAM {

public:
   virtual ULONG Write(PSTREAMBUF, ULONG, BOOL fLooping = 0);
   virtual ULONG Register(PDDCMDREGISTER);
   virtual void  DeRegister(void);
   virtual ULONG StartStream();

   DWAVESTREAM(ULONG streamtype, LPMCI_AUDIO_INIT pinit, USHORT filesysnum);
   virtual ~DWAVESTREAM();

   BOOL     IsEverythingOk() { return (fError == FALSE); };

   virtual BOOL  SetProperty(int type, ULONG value, ULONG reserved = 0);
   virtual ULONG GetProperty(int type);

private:
   virtual void ReturnBuffer(void);   // returns one buffer

   virtual void AddBuffers(BOOL fFirst);    	    // Initialize the audio buffer object

   QUEUEHEAD qhReturn;		//queue with buffers to be returned
   ULONG     hCtxHook;
   ULONG     hSem;
   BOOL      fError;

   friend void cdecl HookHandler(ULONG ulSysFileNum);
};
typedef DWAVESTREAM *PDWAVESTREAM;

#endif
