/* $Id: wavehw.hpp,v 1.1 2000/04/23 14:55:22 ktk Exp $ */

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
#ifndef WAVEHW_INCLUDED
#define WAVEHW_INCLUDED

#ifndef OS2_INCLUDED
#define INCL_NOPMAPI
#include <os2.h>
#include <audio.h>
#endif

#include "waudio.hpp"


class WAVEPLAY : public WAVEAUDIO {
public:
   virtual int Start(STREAM *stream);        // Start the operation
   virtual int Stop(STREAM *stream);         // Stop the operation

   virtual void ConfigDev(STREAM *stream, PWAVECONFIGINFO pConfigInfo);

   WAVEPLAY();
};
typedef WAVEPLAY *PWAVEPLAY;

class WAVEREC : public WAVEAUDIO {
public:
   virtual int Start(STREAM *stream);        // Start the operation
   virtual int Stop(STREAM *stream);         // Stop the operation

   virtual void ConfigDev(STREAM *stream, PWAVECONFIGINFO pConfigInfo);

   WAVEREC();
};
typedef WAVEREC *PWAVEREC;
#endif
