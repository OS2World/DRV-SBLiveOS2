/* $Id: devown.h,v 1.1 2000/04/23 14:55:23 ktk Exp $ */

extern WORD32 deviceOwner;	//located in iwisr.c 
extern WORD32 numOS2Opens;

#define DEV_NO_OWNER		0
#define DEV_PDD_OWNER		1       //owner is still unknown
#define DEV_PCM_OWNER		2	//PCM&MIDI, PCM&ULTIMOD
#define DEV_MIDI_OWNER		4	//MIDI&PCM
#define DEV_VDD_OWNER		8	//exclusive
#define DEV_ULTIMOD_OWNER      16       //ULTIMOD&PCM
#define DEV_EXCLUSIVE_OWNER    32	//3rd party app using toolkit
