            SoundBlaster Live! OS/2 Audio driver version 0.81 (beta)
            ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Contents
========
1    Description
2    Features
3    History
4    Requirements
5    Installation/uninstall
6    Config.sys options
7    Known problems
8    File listing
9    Source code
10   Contacting the author
10.1 SoundBlaster Live OS/2 mailinglist
11   Warranty


1 Description
=============
This is the port of the open source Creative Labs SoundBlaster Live Linux audio 
driver. 
Although the driver is almost feature complete, it should still be considered
beta software. (see the warranty section)

This work would never have been possible without the support by Creative Labs
and the current maintainers of the Linux driver. 


2 Features
==========
- Up to 32 simultaneous wave playback streams
- One record stream
- Full duplex
- Support for the IOCTL90 mixer interface (designed by Joe Nord of
  Crystal Semiconductor and supported by the Crystal OS/2 audio drivers)
- RTMIDI playback & recording
  --> Note: this is not the same as midi playback via MMPM/2 (midi applet)
      Playing midi files requires an external MIDI device


3 History
=========
0.81
- Master volume can now control analog inputs

0.80
- DAC mixer control added (for LBMix)
Updates from Ruediger Ihle:
- Aux volume control fixed
- Aux config.sys flag added
- Warp 3 DART now works

0.75
- Put back joystick code
- Fix for playing small wave files

0.70
- Fail to load if resource manager returned error while allocating resources.
  (prevents SB Live driver from allocating irq when the (e.g.) the USB driver 
   has already claimed exclusive access)
- Added new DirectAudio interface used by Odin for wave playback

0.60
- Fixed detection of SB Live on some systems
- Fixed video mixer control
- Changed wave volume level
- Fixed clicks when playing wave files with large audio buffers
- Print joystick port during init + register joystick device + it's resources

0.50
- Added RTMIDI playback & recording (MPU401)
- Manually detect SB Live! hardware
- Bugfix for TRAP D when starting playback/recording

0.25
- First public beta release


4 Requirements
==============
- OS/2 Warp 4 or Warp Server for e-Business
  (Warp 3 not tested)
- Creative Labs SoundBlaster Live! or
  Creative Labs SoundBlaster PCI 512 soundcard


5 Installation/uninstall
========================
To install the SB Live driver:
- Make sure your BIOS configures the SB Live! card (assigns IRQ and IO addresses)
  Look for a 'PnP OS' (or similar) setting in your BIOS options and set it to
  'No' or 'Off'.
  The SB Live! driver does not work if the BIOS doesn't assign hardware 
  resources to the card.
- Unzip driver archive
- Run install.cmd
- Select 'SoundBlaster Live! Wave Audio' and continue installation
- Reboot

To remove the SB Live driver you should proceed with the installation 
as described above, but select zero SB Live cards when asked.

To use the SB Live's joystick port you need to install one of the
OS/2 joystick drivers and tell it to you the IO port that the
sb live driver prints during boot.
I.e.: Sblive16 prints this during boot:
      SB Live! joystick at : 0xE400
Change the joystick port address to E400 for the gamedd2 driver:
      DEVICE=C:\GAMEDD.SYS /P:E400 /W:400 /C:1E

Note that this is not an ideal solution as the SB Live's joystick
port address can be changed by the BIOS when hardware is added or
removed.
However, it's safer than mapping it to the standard joystick port address
(i.e. 0x201) due to the risk of a conflict with other (audio) hardware.


6 Config.sys options
====================
DEVICE=J:\MMOS2\SBLIVE16.SYS /V /C /M /L
- /V: verbose (prints copyrights during driver init)
- /C: enable CD audio input         (default muted; can be changed with mixer app)
- /M: enable microphone audio input (default muted; can be changed with mixer app)
- /L: enable linein audio input     (default muted; can be changed with mixer app)
- /A: enable aux audio input        (default muted; can be changed with mixer app)

DEVICE=J:\MMOS2\SBLIVE32.SYS /D
- /D: print start and end address of code
      (useful to locate the code that causes a trap (CS:EIP in trapscreen))

The installation adds the verbose and CD settings to the config.sys line.


7 Known problems
================
- Users have reported that sometimes applications can no longer play audio
  (unable to load mdm.dll). This apparently happens from time to time (requiring
  a reboot), but I have not been able to reproduce this myself.
  If you find a way to reliably reproduce this, please contact the author.
  (see section 9)
- Crash in PMMERGE when installing the driver on a system that uses the Scitech
  display drivers. 
  Workaround: move the first minstall window (which asks you to select the sb
  driver) almost completely outside of the screen and press enter.
- No sound when there are still inactive audio drivers in MMOS2\mmpm2.ini.
  To correct the problem uninstall the SB Live and your old audio driver.
  You can also manually remove references to the old driver:
        - uninstall the SB Live driver
        - edit MMOS2\mmpm2.ini 
               - search for 'Waveaudio=' section and remove any names that
                 are listed on that line
               - search for 'Ampmix=' section and remove any names that
                 are listed on that line
                 (result: Waveaudio=
                          Ampmix= 
                 )
        - reinstall the SB Live driver
          MMOS2\mmpm2.ini should now contain:
                 Waveaudio=SBLIVEWAVE01
                 Ampmix=SBLIVEAMPMIX01
        - reboot

- IRQ conflicts can occurs with drivers that are unable to share their interrupt.
  (the USB drivers have this problem, but only on some systems)
  The SBLive driver can detect some of these conflicts and will print the 
  following messages:
   Another device driver was granted exclusive access to IRQ xx
   Unable to allocate hardware resources! Aborting...

  When you see this message, you must force the BIOS to allocate another 
  interrupt for the sblive driver.
  

8 File listing
==============
Installation files:
	audfiles.scr
	audplay.ico
	AUDHELP.HLP
	CARDINFO.dll
	control.scr
	midiplay.ico
	vidplay.ico

16 bits MMPM/2 audio driver:
	sblive16.sys

32 bits SB Live Core audio driver:
	sblive32w4.sys

32 bits SB Live Core audio driver: 
(uses the new KEE api found in Warp 4 + Fixpack 13 or Warp Server for 
 e-Business) 
	sblive32kee.sys


9 Source code
=============
As this driver is based on the open source SoundBlaster Live Linux driver,
the source code is available under the GNU LIBRARY GENERAL PUBLIC LICENSE.
The CVS repository is located at the Netlabs server:
	set CVSROOT=:pserver:guest@www.netlabs.org:d:/netlabs.src/sbliveos2
	set USER=guest

Login as 'guest' with password 'readonly'.


10 Contacting the author
=======================
When you find a bug in the driver, you can contact the author by
email at sandervl@xs4all.nl.
Bug reports should include the following information:
- SoundBlaster Live model
- OS/2 version + fixpack level
- Description of the procedure to reproduce the bug
- Trap description (register contents) (if you're reporting a crash)
  Add the /D option to the sblive32.sys config.sys line and write down
  the start & end address printed during the driver initialization.

Please note that I do *not* want people to mail me about problems that
aren't clearly bugs in the driver. There is a mailinglist at egroups.com
that can be used for general questions about the driver.

Also, do NOT mail the Linux authors about problems with the OS/2 driver.
If a problem in the driver turns out to be a bug in their code, then I will
contact them.


10.1 SoundBlaster Live OS/2 mailinglist
======================================
A mailinglist to discuss the OS/2 sblive driver has been created at yahoogroups.com.
Go to www.yahoogroups.com to subscribe to sbliveos2@yahoogroups.com.


11 Warranty
===========
EXCEPT AS OTHERWISE RESTRICTED BY LAW, THIS WORK IS PROVIDED
WITHOUT ANY EXPRESSED OR IMPLIED WARRANTIES OF ANY KIND, INCLUDING
BUT NOT LIMITED TO, ANY IMPLIED WARRANTIES OF FITNESS FOR A
PARTICULAR PURPOSE, MERCHANTABILITY OR TITLE.  EXCEPT AS
OTHERWISE PROVIDED BY LAW, NO AUTHOR, COPYRIGHT HOLDER OR
LICENSOR SHALL BE LIABLE TO YOU FOR DAMAGES OF ANY KIND, EVEN IF 
THEY HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
