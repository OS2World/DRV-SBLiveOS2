/* $Id: ioctl.cpp,v 1.10 2001/09/28 12:09:42 sandervl Exp $ */

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
 * the functions that implement all the IOCTLs (command 10x) received by
 * the device driver via the strategy entry.
 * @version %I%
 * @context Unless otherwise noted, all interfaces are Ring-0, 16-bit,
 *  <stack context>.
 * @history
 *
 */
#define INCL_NOPMAPI
#include <os2.h>
#include <os2me.h>
#include <audio.h>

#include "strategy.h"
#include "audiohw.hpp"
#include "mpu401.hpp"
#include "wavehw.hpp"
#include "dwavestrm.hpp"
#include "midistrm.hpp"

#include <include.h>
#include <sbvsd.h>
#include <dbgos2.h>
#include <devhelp.h>
#include <ossidc.h>
#include <ioctl90.h>
#include <parse.h>
#include "ioctl.h"

#include <daudio.h>

//Map of possible 256 supported ioctls (IOCTl 90 mixer api)
char      SBLiveIOCTLMap[256] = {0};
MIXSTRUCT MixerSettings[16]   = {0};

//override flags for rec src & gain mixer commands
BOOL      fRecSrcIOCTL90      = FALSE;
BOOL      fRecGainIOCTL90     = FALSE;

int       numFreeStreams      = SBLIVECAPS_MAXSTREAMS;

/**@internal
 * @param    PREQPACKET pointer to the strategy request packet
 * @return   None  But the Status in the request packet is updated on error
 * @notes
 * The Audio Init Ioctl only 2 things happen in here :
 * 1. We call CheckForStream to make sure there is not already a stream
 * registered with the same sysfilenumber. If we get a good rc from that
 * 2: we look at sMode in the MCI_AUDIO_INIT to determine the type of stream
 * to init and call the approprate stream constructor.
 *
 */
void IoctlAudioInit(PREQPACKET prp, USHORT LDev)
{
   LPMCI_AUDIO_INIT p = (LPMCI_AUDIO_INIT)prp->s.ioctl.pvData;
   PAUDIOHW pHWobj;
   PSTREAM pstream;
   ULONG HardwareType;

      // if this is an IDLE or De-Init request
      // fetch the stream object based on the sysfilenum and turn on the
      // stream idle bit in the stream state then write the sysfilenum
      // into the request packet, set rc = 0 and return
   if (p->sMode == IDLE) {
      pstream = FindStream_fromFile((ULONG) prp->s.ioctl.usSysFileNum);
      if (pstream)
         pstream->ulStreamState |= STREAM_IDLE;
      p->pvReserved = (VOID FAR *) (ULONG)prp->s.ioctl.usSysFileNum;
      p->sReturnCode = 0;
      return;
   }
       // call FindStream_fromFile to see if there is already a stream
       // for this sysfilenum. if there is then see if the idle bit is on,
       // if the idle bit is on, reset it, then write the sysfilenum
      // into the request packet, set rc = 0 and return
      // MMPM sends idle down to a stream that is "losing" the hardware but
      // should not go away. It usually is associated with an app losing
      // focus. If the idle bit is not the stream is either not registered or
      // is being "re-initted" in another mode or with different file
      // attributes. "Re-initting" a stream is a total hack on the part
      // of MMPM they should de-register the stream and then init a new
      // stream but they don't . If anyone ever writes a VSD that behaves
      // "correctly" then this code can be deleted.
      // Either way delete the stream and build a new one with
      // this request packet.
   pstream = FindStream_fromFile((ULONG) prp->s.ioctl.usSysFileNum);
   if (pstream) {
      if (pstream->ulStreamState & STREAM_IDLE) {
         pstream->ulStreamState &= STREAM_NOT_IDLE;
         p->pvReserved = (VOID FAR *) (ULONG)prp->s.ioctl.usSysFileNum;
         p->sReturnCode = 0;
         return;
      }
      else {
#if 1
	// Rudi: Workaround for MMPM device sharing bug
	if (pstream->ulStreamState & STREAM_STREAMING) {
	  CONTROL_PARM dummy;
	  pstream->PauseStream(&dummy);
	  pstream->ResumeStream();
	  p->pvReserved = (VOID FAR *) (ULONG)prp->s.ioctl.usSysFileNum;
	  p->sReturnCode = 0;
	  return;
	}
#endif
	 delete pstream;
      }
   }

      // get the hardware type
      // return with bad status if the harware type is invalid/unsupported
   HardwareType = GetHardwareType(p->sMode, (USHORT)p->ulOperation, LDev);
   if (HardwareType == AUDIOHW_INVALID_DEVICE) {
      p->sReturnCode = INVALID_REQUEST;
      prp->usStatus |= RPERR;
      return;
   }
      // make sure we have a Hardware object that can handle this
      // data type and operation..
   pHWobj = GetHardwareDevice(HardwareType);
   if (pHWobj == NULL) {
      p->sReturnCode = INVALID_REQUEST;
      prp->usStatus |= RPERR;
      return;
   }

   p->ulFlags = 0;  /* Zero the Flags  */
   switch (HardwareType) {
      case AUDIOHW_WAVE_PLAY:
      case AUDIOHW_WAVE_CAPTURE:
         pstream = new WAVESTREAM(HardwareType,p,prp->s.ioctl.usSysFileNum);
         break;
#if 0
      case AUDIOHW_MPU401_PLAY:
         pstream = new MIDISTREAM(HardwareType, prp->s.ioctl.usSysFileNum);
         break;
#endif
      default:
         p->sReturnCode = INVALID_REQUEST;
         prp->usStatus |= RPERR;
         return;
   } /* endswitch */

   p->ulFlags |= VOLUME;           /* volume control is supported   */
   p->ulFlags |= INPUT;            /* Input select is supported     */
   p->ulFlags |= OUTPUT;           /* Output select is supported    */
   p->ulFlags |= MONITOR;          /* Record Monitor is supported   */
   p->sDeviceID = SB_LIVE;      /* Reported in VSD dll           */
   pstream->ulSysFileNum = prp->s.ioctl.usSysFileNum;
   p->pvReserved = (VOID FAR *) (ULONG) prp->s.ioctl.usSysFileNum;
   p->sReturnCode = 0;
   dprintf(("IoctlAudioInit: file nr: %lx", pstream->ulSysFileNum));
}


/**@internal
 * @param    PREQPACKET pointer to the strategy request packet
 * @return   None  But the Status in the request packet is updated on error
 * @notes
 */
void IoctlAudioCapability(PREQPACKET prp, USHORT LDev)
{
   PAUDIOHW pHWobj;
   PAUDIO_CAPS p = (PAUDIO_CAPS)prp->s.ioctl.pvData;
   ULONG ulDevicetype;

   // get the hardware device type based on the datatype and operation
   ulDevicetype = GetHardwareType((USHORT)p->ulDataType,(USHORT)p->ulOperation,LDev);

   // Tell the caller we support this IOCTL
   p->ulCapability = SUPPORT_CAP;

   // get the pointer to the hardware object
   // call DevCaps
   // bailout if no hardware object is returned..
   pHWobj = GetHardwareDevice(ulDevicetype);
   if (pHWobj) {
      pHWobj->DevCaps(p);
      if (p->ulSupport != SUPPORT_SUCCESS) {
         prp->usStatus |= RPERR;
      }
   }
   else {
      p->ulSupport = UNSUPPORTED_DATATYPE;
      prp->usStatus |= RPERR;
   }
}

/**@internal IoctlAudioControl
 * @param    PREQPACKET pointer to the strategy request packet
 * @return   None  But the Status in the request packet is updated on error
 * @notes
 * if it's AUDIO_CHANGE, just report success, otherwise report failure
 * this is because we don't support volume, balance, multiple in/out devices,
 * etc.  Also, START, STOP, RESUME, and PAUSE are redundant, so we don't
 * support those either.
 */
void IoctlAudioControl(PREQPACKET prp)
{
   LPMCI_AUDIO_CONTROL p = (LPMCI_AUDIO_CONTROL) prp->s.ioctl.pvData;
   LPMCI_AUDIO_CHANGE  pAudChange;
   LPMCI_TRACK_INFO    pMasterVol;
   PSTREAM             pStream;
   ULONG               volume;
   ULONG               addr;

   if (p->usIOCtlRequest != AUDIO_CHANGE) {
       p->sReturnCode = INVALID_REQUEST;
       prp->usStatus |= RPERR | RPBADCMD;
       return;
   }
   p->sReturnCode=0;

   pAudChange = (LPMCI_AUDIO_CHANGE)p->pbRequestInfo;

   //Test for MMPM/2 bug (structure crosses selector boundary)
   addr  = OFFSETOF(pAudChange);
   addr += sizeof(MCI_AUDIO_CHANGE);
   if(addr >= 0x10000UL) {
       dprintf(("Invalid MCI_AUDIO_CHANGE pointer %lx!!", (ULONG)pAudChange));
       p->sReturnCode = INVALID_REQUEST;
       prp->usStatus |= RPERR | RPBADCMD;
       return;
   }
   pMasterVol = (LPMCI_TRACK_INFO)pAudChange->pvDevInfo;

   //Test for MMPM/2 bug (structure crosses selector boundary)
   addr  = OFFSETOF(pMasterVol);
   addr += sizeof(MCI_TRACK_INFO);
   if(addr >= 0x10000UL) {
       dprintf(("Invalid MCI_TRACK_INFO pointer %lx!!", (ULONG)pMasterVol));
       p->sReturnCode = INVALID_REQUEST;
       prp->usStatus |= RPERR | RPBADCMD;
       return;
   }

   pStream = FindStream_fromFile((ULONG) prp->s.ioctl.usSysFileNum);
   if(pStream == NULL) {
       dprintf(("IoctlAudioControl stream %lx not found!", (ULONG) prp->s.ioctl.usSysFileNum));
       DebugInt3();
       return;
   }
   if(pAudChange->lBalance != AUDIO_IGNORE)
       pStream->SetProperty(PROPERTY_BALANCE, pAudChange->lBalance);

   if(pAudChange->lVolume != AUDIO_IGNORE) {
       // stream volume ranges from 0 to 0x7FFFFFFF (linear)
       volume = pAudChange->lVolume >> 16UL;
       volume = (volume*100UL)/0x7FFFUL;
       dprintf(("Set stream volume of %x to %d", prp->s.ioctl.usSysFileNum, volume));
       pStream->SetProperty(PROPERTY_VOLUME, MAKE_VOLUME_LR(volume, volume));
   }

   if(pMasterVol && pMasterVol->usMasterVolume != AUDIO_IGNORE) {
       // master volume ranges from 0 to 0x7FFF (linear)
       volume = pMasterVol->usMasterVolume;
       volume = (volume*100UL)/0x7FFFUL;
       if(volume > 100) {
           volume = 100;
       }
       dprintf(("Set mastervolume to %d", volume));
       pStream->SetProperty(PROPERTY_MASTERVOL, MAKE_VOLUME_LR(volume, volume));
   }
   if(!fRecSrcIOCTL90) {
    for(int i=0;i<8;i++)
    {
        switch(pAudChange->rInputList[0].ulDevType) {
        case NULL_INPUT:
            break; //continue;
        case STEREO_LINE_INPUT:
        case LEFT_LINE_INPUT:
        case RIGHT_LINE_INPUT:
            pStream->SetProperty(PROPERTY_INPUTSRC, MIX_RECSRC_LINE);
            break;

        case MIC_INPUT:
        case BOOSTED_MIC_INPUT:
            pStream->SetProperty(PROPERTY_INPUTSRC, MIX_RECSRC_MIC);
            break;

        case PHONE_LINE_INPUT:
        case HANDSET_INPUT:
        case SYNTH_INPUT:
        case DIGITAL_PHONE_LINE_INPUT:
        case DIGITAL_HANDSET_INPUT:
        case MIDI_IN_PORT:
//      case LOOPBACK:
            pStream->SetProperty(PROPERTY_INPUTSRC, MIX_RECSRC_MIXER);
            break;
        }
    }
   }
   if(!fRecGainIOCTL90 && pAudChange->lGain != AUDIO_IGNORE) {
       // input ranges from 0 to 0x7FFFFFFF (linear)
       volume = pAudChange->lGain >> 16UL;
       volume = (volume*100UL)/0x7FFFUL;
       if(volume > 100) {
           volume = 100;
       }
       dprintf(("Set input gain of %x to %d", prp->s.ioctl.usSysFileNum, volume));
       pStream->SetProperty(PROPERTY_INPUTGAIN, MAKE_VOLUME_LR(volume, volume));
   }
}
//******************************************************************************
//******************************************************************************
void IoctlDirectAudio(PREQPACKET prp)
{
    if(prp->s.ioctl.bCode == DAUDIO_OPEN)
    {
        LPMCI_AUDIO_INIT pInit = (LPMCI_AUDIO_INIT) prp->s.ioctl.pvData;
        PAUDIOHW pHWobj;
        MCI_AUDIO_CAPS audioCaps;
        PDWAVESTREAM pStream;

        if(DevHelp_VerifyAccess(SELECTOROF(pInit), sizeof(MCI_AUDIO_INIT), OFFSETOF(pInit), VERIFY_READWRITE))
        {
            dprintf(("Invalid MCI_AUDIO_INIT pointer %lx!!", (ULONG)pInit));
            prp->usStatus |= RPERR | RPBADCMD;
            return;
        }

        audioCaps.ulLength        = sizeof(MCI_AUDIO_CAPS);
        audioCaps.ulSamplingRate  = pInit->lSRate;
        audioCaps.ulChannels      = pInit->sChannels;
        audioCaps.ulBitsPerSample = pInit->lBitsPerSRate;
        audioCaps.ulDataType      = pInit->sMode;
        audioCaps.ulOperation     = OPERATION_PLAY;

        // get the pointer to the hardware object
        // call DevCaps
        // bailout if no hardware object is returned..
        pHWobj = GetHardwareDevice(AUDIOHW_WAVE_PLAY);
        if (pHWobj)
        {
            pHWobj->DevCaps(&audioCaps);
            if (audioCaps.ulSupport != SUPPORT_SUCCESS) {
                dprintf(("IoctlDirectAudio: DevCaps failed"));
                pInit->sReturnCode = INVALID_REQUEST;
                prp->usStatus |= RPERR;
                return;
            }
        }
        else {
            pInit->sReturnCode = INVALID_REQUEST;
            prp->usStatus |= RPERR;
            return;
        }

        pStream = new DWAVESTREAM(AUDIOHW_WAVE_PLAY, pInit,  prp->s.ioctl.usSysFileNum);
        if(pStream == NULL) {
            DebugInt3();
            pInit->sReturnCode = INVALID_REQUEST;
            prp->usStatus |= RPERR;
            return;
        }

        if(!pStream->IsEverythingOk()) {
            delete pStream;
            DebugInt3();
            pInit->sReturnCode = INVALID_REQUEST;
            prp->usStatus |= RPERR;
            return;
        }
        pInit->ulFlags |= VOLUME;           /* volume control is supported   */
        pInit->ulFlags |= INPUT;            /* Input select is supported     */
        pInit->ulFlags |= OUTPUT;           /* Output select is supported    */
        pInit->ulFlags |= MONITOR;          /* Record Monitor is supported   */
        pInit->sDeviceID = SB_LIVE;      /* Reported in VSD dll           */
        pStream->ulSysFileNum = prp->s.ioctl.usSysFileNum;
        pInit->pvReserved = (VOID FAR *) (ULONG) prp->s.ioctl.usSysFileNum;
        pInit->sReturnCode = 0;

        return;
    }
    else
    if(prp->s.ioctl.bCode == DAUDIO_QUERYFORMAT)
    {
        LPMCI_AUDIO_INIT pInit = (LPMCI_AUDIO_INIT) prp->s.ioctl.pvData;
        PAUDIOHW pHWobj;
        MCI_AUDIO_CAPS audioCaps;

        if(DevHelp_VerifyAccess(SELECTOROF(pInit), sizeof(MCI_AUDIO_INIT), OFFSETOF(pInit), VERIFY_READWRITE))
        {
            dprintf(("Invalid MCI_AUDIO_INIT pointer %lx!!", (ULONG)pInit));
            prp->usStatus |= RPERR | RPBADCMD;
            return;
        }

        audioCaps.ulLength        = sizeof(MCI_AUDIO_CAPS);
        audioCaps.ulSamplingRate  = pInit->lSRate;
        audioCaps.ulChannels      = pInit->sChannels;
        audioCaps.ulBitsPerSample = pInit->lBitsPerSRate;
        audioCaps.ulDataType      = pInit->sMode;
        audioCaps.ulOperation     = OPERATION_PLAY;

        // get the pointer to the hardware object
        // call DevCaps
        // bailout if no hardware object is returned..
        pHWobj = GetHardwareDevice(AUDIOHW_WAVE_PLAY);
        if (pHWobj)
        {
            pHWobj->DevCaps(&audioCaps);
            if (audioCaps.ulSupport != SUPPORT_SUCCESS) {
                dprintf(("IoctlDirectAudio: DevCaps failed"));
                prp->usStatus |= RPERR;
                pInit->sReturnCode = INVALID_REQUEST;
                return;
            }
            pInit->sReturnCode = 0;
            return;
        }
        else {
            pInit->sReturnCode = INVALID_REQUEST;
            prp->usStatus |= RPERR;
            return;
        }
    }
    else 
    if(prp->s.ioctl.bCode == DAUDIO_QUERYCAPS) 
    {
        LPDAUDIO_CAPS lpCaps = (LPDAUDIO_CAPS) prp->s.ioctl.pvData;
        PWAVEAUDIO pHWobj;

        if(DevHelp_VerifyAccess(SELECTOROF(lpCaps), sizeof(DAUDIO_CAPS), OFFSETOF(lpCaps), VERIFY_READWRITE) ||
           lpCaps->dwSize != sizeof(DAUDIO_CAPS))
        {
            dprintf(("Invalid DAUDIO_CAPS pointer %lx!!", (ULONG)lpCaps));
            prp->usStatus |= RPERR | RPBADCMD;
            return;
        }
        // get the pointer to the hardware object
        // call DevCaps
        // bailout if no hardware object is returned..
        pHWobj = (PWAVEAUDIO)GetHardwareDevice(AUDIOHW_WAVE_PLAY);
        if (pHWobj)
        {
            pHWobj->DevCaps(lpCaps);
            lpCaps->dwFreeHwMixingAllBuffers       = numFreeStreams;
            lpCaps->dwFreeHwMixingStaticBuffers    = numFreeStreams;
            lpCaps->dwFreeHwMixingStreamingBuffers = numFreeStreams;
            return;
        }
        else {
            prp->usStatus |= RPERR;
            return;
        }
    }
    PSTREAM pStream;

    pStream = FindStream_fromFile((ULONG) prp->s.ioctl.usSysFileNum);
    if(pStream == NULL) {
        dprintf(("IoctlDirectAudio stream %lx not found!", (ULONG) prp->s.ioctl.usSysFileNum));
        DebugInt3();
        prp->usStatus |= RPERR | RPBADCMD;
        return;
    }

    LPDAUDIO_CMD pDAudioCmd = (LPDAUDIO_CMD) prp->s.ioctl.pvData;
    ULONG        rc = 0;

    if(DevHelp_VerifyAccess(SELECTOROF(pDAudioCmd), sizeof(DAUDIO_CMD), OFFSETOF(pDAudioCmd), VERIFY_READWRITE))
    {
        dprintf(("Invalid DAUDIO_CMD pointer %lx!!", (ULONG)pDAudioCmd));
        prp->usStatus |= RPERR | RPBADCMD;
        return;
    }

    switch(prp->s.ioctl.bCode)
    {
    case DAUDIO_CLOSE:
        delete pStream;
        break;

    case DAUDIO_SETVOLUME:
    {
        pStream->SetProperty(PROPERTY_VOLUME, MAKE_VOLUME_LR(pDAudioCmd->Vol.VolumeL, pDAudioCmd->Vol.VolumeR));
        break;
    }

    case DAUDIO_GETVOLUME:
        pDAudioCmd->Vol.VolumeL = GET_VOLUME_L(pStream->GetProperty(PROPERTY_VOLUME));
        pDAudioCmd->Vol.VolumeR = GET_VOLUME_R(pStream->GetProperty(PROPERTY_VOLUME));
        break;

    case DAUDIO_START:
        if(numFreeStreams > 0) {
              rc = pStream->StartStream();
              if(!rc) numFreeStreams--;
        }
        else  rc = 1; //fail

        break;

    case DAUDIO_STOP:
    {
        CONTROL_PARM cParm;
        int          fActive = pStream->isActive();

        rc = pStream->StopStream(&cParm);
        if(!rc && fActive) numFreeStreams++;
        break;
    }

    case DAUDIO_PAUSE:
    {
        CONTROL_PARM cParm;
        rc = pStream->PauseStream(&cParm);
        break;
    }

    case DAUDIO_RESUME:
        rc = pStream->ResumeStream();
        break;

    case DAUDIO_GETPOS:
        pDAudioCmd->Pos.ulCurrentPos = pStream->GetCurrentPos();
        pDAudioCmd->Pos.ulWritePos   = pStream->GetCurrentWritePos();
        break;

    case DAUDIO_ADDBUFFER:
    {
        rc = pStream->Write((PSTREAMBUF)pDAudioCmd->Buffer.lpBuffer, pDAudioCmd->Buffer.ulBufferLength);
        break;
    }

    case DAUDIO_SETPROPERTY:
    {
        rc = pStream->SetProperty(pDAudioCmd->SetProperty.type, pDAudioCmd->SetProperty.value);
        break;
    }

    case DAUDIO_REGISTER_THREAD:
    {
        DDCMDREGISTER reg;

        reg.ulFunction     = DDCMD_REG_STREAM;
        reg.hStream        = pDAudioCmd->Thread.hSemaphore;
        reg.ulSysFileNum   = prp->s.ioctl.usSysFileNum;
        reg.pSHDEntryPoint = NULL;
        rc = pStream->Register(&reg);
        break;
    }

    case DAUDIO_DEREGISTER_THREAD:
    {
        pStream->DeRegister();
        break;
    }

    case DAUDIO_QUERYVERSION:
        pDAudioCmd->Version.ulVersion = DAUDIO_VERSION;
        break;
    }

    if(rc) {
        prp->usStatus |= RPERR | RPBADCMD;
        return;
    }
    return;
}
//******************************************************************************
//******************************************************************************
void IoctlMixer(PREQPACKET prp)
{
 MIXSTRUCT FAR *pMixStruct = (MIXSTRUCT FAR *)prp->s.ioctl.pvData;
 ULONG     FAR *pIoctlData = (ULONG FAR *)prp->s.ioctl.pvData;
 CHAR      FAR *pIoctlMap  = (CHAR FAR *)prp->s.ioctl.pvData; //getapimap
 USHORT    VolumeL, VolumeR;

   if((prp->s.ioctl.bCode & 0xF0) == 0x40)
   {
    if(DevHelp_VerifyAccess(SELECTOROF(pMixStruct), sizeof(MIXSTRUCT), OFFSETOF(pMixStruct), VERIFY_READWRITE))
    {
        dprintf(("Invalid IOCTL90 pointer %lx!!", (ULONG)pMixStruct));
        prp->usStatus |= RPERR | RPBADCMD;
        return;
    }
    MixerSettings[prp->s.ioctl.bCode & 0xF].VolumeL = pMixStruct->VolumeL;
    MixerSettings[prp->s.ioctl.bCode & 0xF].VolumeR = pMixStruct->VolumeR;
    MixerSettings[prp->s.ioctl.bCode & 0xF].Mute = pMixStruct->Mute;
    VolumeL = (USHORT)pMixStruct->VolumeL;
    VolumeR = (USHORT)pMixStruct->VolumeR;
    if(prp->s.ioctl.bCode != BASSTREBLESET) {
        if(pMixStruct->Mute == 1)
            VolumeL = VolumeR = 0;
    }
   }
   else
   if((prp->s.ioctl.bCode & 0xF0) == 0x60)
   {
    if(DevHelp_VerifyAccess(SELECTOROF(pMixStruct), sizeof(MIXSTRUCT), OFFSETOF(pMixStruct), VERIFY_READONLY))
    {
        dprintf(("Invalid IOCTL90 pointer %lx!!", (ULONG)pMixStruct));
        prp->usStatus |= RPERR | RPBADCMD;
        return;
    }
   }

    switch(prp->s.ioctl.bCode) {
    case MICSET:
        OSS16_SetGlobalVol(prp->s.ioctl.usSysFileNum, MIX_SETMICVOL, MAKE_VOLUME_LR(VolumeL, VolumeR));
        break;
    case LINESET:
        OSS16_SetGlobalVol(prp->s.ioctl.usSysFileNum, MIX_SETLINEINVOL, MAKE_VOLUME_LR(VolumeL, VolumeR));
        break;
    case CDSET:
        OSS16_SetGlobalVol(prp->s.ioctl.usSysFileNum, MIX_SETCDVOL, MAKE_VOLUME_LR(VolumeL, VolumeR));
        break;
    case VIDEOSET:
        OSS16_SetGlobalVol(prp->s.ioctl.usSysFileNum, MIX_SETVIDEOVOL, MAKE_VOLUME_LR(VolumeL, VolumeR));
        break;
    case AUXSET:
        OSS16_SetGlobalVol(prp->s.ioctl.usSysFileNum, MIX_SETAUXVOL, MAKE_VOLUME_LR(VolumeL, VolumeR));
        break;

    case BASSTREBLESET:
        OSS16_SetGlobalVol(prp->s.ioctl.usSysFileNum, MIX_SETBASS, MAKE_VOLUME_LR(VolumeL, VolumeL));
        OSS16_SetGlobalVol(prp->s.ioctl.usSysFileNum, MIX_SETTREBLE, MAKE_VOLUME_LR(VolumeR, VolumeR));
        break;

    case STREAMVOLSET:
        OSS16_SetGlobalVol(prp->s.ioctl.usSysFileNum, MIX_SETPCMVOL, MAKE_VOLUME_LR(VolumeL, VolumeR));
        break;

    case RECORDSRCSET:
    {
        int recsrc = MIX_RECSRC_LINE;

        //release recording source override?
        if(pMixStruct->Mute == 2) {
            fRecSrcIOCTL90 = FALSE;
            break;
        }
        fRecSrcIOCTL90 = TRUE;
        switch(pMixStruct->VolumeL) {
        case I90SRC_MIC:
            recsrc = MIX_RECSRC_MIC;
            break;
        case I90SRC_CD:
            recsrc = MIX_RECSRC_CD;
            break;
        case I90SRC_VIDEO:
            recsrc = MIX_RECSRC_VIDEO;
            break;
        case I90SRC_LINE:
            recsrc = MIX_RECSRC_LINE;
            break;
        case I90SRC_AUX:
            recsrc = MIX_RECSRC_AUX;
            break;
    //  case I90SRC_RES5:
    //  case I90SRC_RES6:
    //  case I90SRC_PHONE:
        default:
            break;
        }
        OSS16_SetGlobalVol(prp->s.ioctl.usSysFileNum, MIX_SETINPUTSRC, recsrc);
        break;
    }
    case RECORDGAINSET:
        //release recording gain override?
        if(pMixStruct->Mute == 2) {
            fRecGainIOCTL90 = FALSE;
            break;
        }
        fRecGainIOCTL90 = TRUE;
        OSS16_SetGlobalVol(prp->s.ioctl.usSysFileNum, MIX_SETINPUTGAIN, MAKE_VOLUME_LR(VolumeL, VolumeL));
        break;

    case MICQUERY:
    case LINEQUERY:
    case AUXQUERY:
    case CDQUERY:
    case VIDEOQUERY:
    case RECORDSRCQUERY:
    case RECORDGAINQUERY:
    case STREAMVOLQUERY:
        pMixStruct->VolumeL = MixerSettings[prp->s.ioctl.bCode & 0xF].VolumeL;
        pMixStruct->VolumeR = MixerSettings[prp->s.ioctl.bCode & 0xF].VolumeR;
        pMixStruct->Mute    = MixerSettings[prp->s.ioctl.bCode & 0xF].Mute;
        break;

    case BASSTREBLEQUERY:
        pMixStruct->VolumeL = MixerSettings[prp->s.ioctl.bCode & 0xF].VolumeL;
        pMixStruct->VolumeR = MixerSettings[prp->s.ioctl.bCode & 0xF].VolumeR;
        pMixStruct->Mute    = 3; //bass & treble
        break;

    case APILEVELQUERY:
        if(DevHelp_VerifyAccess(SELECTOROF(pIoctlData), sizeof(ULONG), OFFSETOF(pIoctlData), VERIFY_READWRITE))
        {
            dprintf(("Invalid IOCTL90 pointer %lx!!", (ULONG)pIoctlData));
            prp->usStatus |= RPERR | RPBADCMD;
            return;
        }
        *pIoctlData = 2;
        break;

    case GETAPIMAP:
        if(DevHelp_VerifyAccess(SELECTOROF(pIoctlMap), sizeof(SBLiveIOCTLMap), OFFSETOF(pIoctlMap), VERIFY_READWRITE))
        {
            dprintf(("Invalid IOCTL90 pointer %lx!!", (ULONG)pMixStruct));
            prp->usStatus |= RPERR | RPBADCMD;
            return;
        }
        _fmemcpy(pIoctlMap, SBLiveIOCTLMap, sizeof(SBLiveIOCTLMap));
        break;

//   case MONOINSET:
//   case PHONESET:
//   case THREEDSET:
//   case MONOINQUERY:
//   case PHONEQUERY:
//   case THREEDQUERY:
//   case CALLBACKREG:
//   case MSGBUF:
    default:
        prp->usStatus |= RPERR | RPBADCMD;
        return;
    }
}
//******************************************************************************
//******************************************************************************
void MixerInit()
{
 REQPACKET rp;
 MIXSTRUCT mixinfo;

    SBLiveIOCTLMap[MICSET]           = 1;
    SBLiveIOCTLMap[MICQUERY]         = 1;
    SBLiveIOCTLMap[LINESET]          = 1;
    SBLiveIOCTLMap[LINEQUERY]        = 1;
    SBLiveIOCTLMap[CDSET]            = 1;
    SBLiveIOCTLMap[CDQUERY]          = 1;
    SBLiveIOCTLMap[VIDEOSET]         = 1;
    SBLiveIOCTLMap[VIDEOQUERY]       = 1;
    SBLiveIOCTLMap[AUXSET]           = 1;
    SBLiveIOCTLMap[AUXQUERY]         = 1;
    SBLiveIOCTLMap[BASSTREBLESET]    = 1;
    SBLiveIOCTLMap[BASSTREBLEQUERY]  = 1;
    SBLiveIOCTLMap[STREAMVOLSET]     = 1;
    SBLiveIOCTLMap[STREAMVOLQUERY]   = 1;
    SBLiveIOCTLMap[RECORDSRCSET]     = 1;
    SBLiveIOCTLMap[RECORDSRCQUERY]   = 1;
    SBLiveIOCTLMap[RECORDGAINSET]    = 1;
    SBLiveIOCTLMap[RECORDGAINQUERY]  = 1;
    SBLiveIOCTLMap[APILEVELQUERY]    = 1;
    SBLiveIOCTLMap[GETAPIMAP]        = 1;
//  SBLiveIOCTLMap[CALLBACKREG]      = 1;

    //Set mic
    rp.s.ioctl.bCode  = MICSET;
    rp.s.ioctl.pvData = (void FAR *)&mixinfo;
    mixinfo.Mute    = fMicMute;
    mixinfo.VolumeR = mixinfo.VolumeL = 60;
    IoctlMixer(&rp);

    //Set line
    rp.s.ioctl.bCode  = LINESET;
    rp.s.ioctl.pvData = (void FAR *)&mixinfo;
    mixinfo.Mute    = fLineMute;
    mixinfo.VolumeR = mixinfo.VolumeL = 80;
    IoctlMixer(&rp);

    //Set CD
    rp.s.ioctl.bCode  = CDSET;
    rp.s.ioctl.pvData = (void FAR *)&mixinfo;
    mixinfo.Mute    = fCDMute;
    mixinfo.VolumeR = mixinfo.VolumeL = 80;
    IoctlMixer(&rp);

   //Set aux
    rp.s.ioctl.bCode  = AUXSET;
    rp.s.ioctl.pvData = (void FAR *)&mixinfo;
    mixinfo.Mute    = fAuxMute;
    mixinfo.VolumeR = mixinfo.VolumeL = 80;
    IoctlMixer(&rp);

   //Set bass/treble
    rp.s.ioctl.bCode  = BASSTREBLESET;
    rp.s.ioctl.pvData = (void FAR *)&mixinfo;
    mixinfo.Mute    = 3;
    mixinfo.VolumeR = mixinfo.VolumeL = 50;
    IoctlMixer(&rp);

    //Set recording source to line in
    rp.s.ioctl.bCode  = RECORDSRCSET;
    rp.s.ioctl.pvData = (void FAR *)&mixinfo;
    mixinfo.Mute    = 0;
    mixinfo.VolumeL = I90SRC_LINE;
    IoctlMixer(&rp);

    //Release recording source override
    rp.s.ioctl.bCode  = RECORDSRCSET;
    rp.s.ioctl.pvData = (void FAR *)&mixinfo;
    mixinfo.Mute    = 2;
    mixinfo.VolumeL = I90SRC_LINE;
    IoctlMixer(&rp);

    //Set PCM volume
    rp.s.ioctl.bCode  = STREAMVOLSET;
    rp.s.ioctl.pvData = (void FAR *)&mixinfo;
    mixinfo.Mute    = 0;
    mixinfo.VolumeR = mixinfo.VolumeL = 90;
    IoctlMixer(&rp);
}
//******************************************************************************
//******************************************************************************
/**@internal
 * @param    PREQPACKET pointer to the strategy request packet
 * @return   None  But the Status in the request packet is updated on error
 * @notes
 * StrategyIoctl is called from the strategy entry point to process IOCTL
 * requests. only catagory 80x requests will be processed.
 */
/* extern "C" void StrategyIoctl(PREQPACKET prp, USHORT LDev)
*
*
*
*/
extern "C" void StrategyIoctl(PREQPACKET prp, USHORT LDev)
{
    if(prp->s.ioctl.bCategory == DAUDIO_IOCTL_CAT) {
        IoctlDirectAudio(prp);
        return;
    }

    if(prp->s.ioctl.bCategory == 0x90) {
        IoctlMixer(prp);
        return;
    }
    if(prp->s.ioctl.bCategory != AUDIO_IOCTL_CAT) {
        prp->usStatus |= RPERR | RPBADCMD;
        return;
    }

    switch (prp->s.ioctl.bCode)
    {
    case AUDIO_INIT:
        IoctlAudioInit(prp, LDev);
        break;
    case AUDIO_CONTROL:
        IoctlAudioControl(prp);
        break;
    case AUDIO_CAPABILITY:
        IoctlAudioCapability(prp, LDev);
        break;
    default:
        prp->usStatus |= RPERR | RPBADCMD;
        break;
    }
    return;
}
