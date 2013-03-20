  #define INCL_DOSDEVIOCTL
  #define INCL_DOSDEVICES
#define INCL_DOSPROCESS
#define INCL_VIO
#define INCL_DOSMEMMGR
#define INCL_DOSSESMGR
#define INCL_DOSMISC
#define INCL_DOSDEVICES
#define INCL_DOS
#define INCL_DOSMODULEMGR

#include <os2.h>
#include <audio.h>

// C RunTime Header Files
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <memory.h>

#include <daudio.h>

HFILE hSBLive = 0;

int main(int argc, char *argv[])
{
   APIRET rc;
   ULONG  action;

   rc = DosOpen( "SBLIVE1$", &hSBLive, &action, 0,
                 FILE_NORMAL, FILE_OPEN, OPEN_ACCESS_READWRITE | 
	         OPEN_SHARE_DENYNONE | OPEN_FLAGS_WRITE_THROUGH,
	         NULL );
   if(rc) {
       printf("DosOpen failed with error %d\n", rc);
       return 1;
   }

   ULONG ParmLength = 0, DataLength;

   MCI_AUDIO_INIT init = {0};
   DataLength = sizeof(init);

   init.lSRate = 44100;
   init.lBitsPerSRate = 16;
   init.sChannels = 2;
   init.sMode = PCM;
   rc = DosDevIOCtl(hSBLive, DAUDIO_IOCTL_CAT, DAUDIO_QUERYFORMAT, NULL, 0, 
 	            &ParmLength, &init, DataLength, &DataLength);

   if(rc) {
       printf("DosDevIOCtl failed with error %d\n", rc);
       goto fail;
   }
   if(init.sReturnCode != 0) {
       printf("init.sReturnCode = %d\n", init.sReturnCode);
       goto fail;
   }
   printf("Query format successfull for %d hz, %d bps & %d channels\n", init.lSRate, init.lBitsPerSRate, init.sChannels);

   printf("Creating stream..\n");
   rc = DosDevIOCtl(hSBLive, DAUDIO_IOCTL_CAT, DAUDIO_OPEN, NULL, 0, 
 	            &ParmLength, &init, DataLength, &DataLength);
   if(rc) {
       printf("DosDevIOCtl failed with error %d\n", rc);
       goto fail;
   }

   DAUDIO_CMD cmd;
   DataLength = sizeof(cmd);


   rc = DosDevIOCtl(hSBLive, DAUDIO_IOCTL_CAT, DAUDIO_GETPOS, NULL, 0, 
 	            &ParmLength, &cmd, DataLength, &DataLength);
   if(rc) {
       printf("DosDevIOCtl failed with error %d\n", rc);
       goto fail;
   }
   printf("Current stream position = %d\n", cmd.Pos.ulCurrentPos);

   rc = DosDevIOCtl(hSBLive, DAUDIO_IOCTL_CAT, DAUDIO_CLOSE, NULL, 0, 
 	            &ParmLength, &cmd, DataLength, &DataLength);

   DosClose(hSBLive);
   return 0;

fail:
   DosClose(hSBLive);
   return 1;
}

