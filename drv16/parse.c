/* $Id: parse.c,v 1.3 2001/09/28 12:09:43 sandervl Exp $ */

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
 *  Parses DEVICE= command line parameters, stuffs values into global vbls.
 * @version %I%
 * @context
 *  Unless otherwise noted, all interfaces are Ring-3, 16-bit,
 *  Init-time kernel stack.
 * @notes
 * @history
 *  13-Nov-96  Timur Tabi
 */


#pragma code_seg ("_inittext");

#ifdef __cplusplus
extern "C" {
#endif
#define INCL_NOPMAPI
#define INCL_DOSMISC
#include <os2.h>
#ifdef __cplusplus
}
#endif

#include "parse.h"         // NUM_DEVICES
#include "runtime.h"       // runtime prototypes
#include <include.h>


// The base I/O ports specified on the command line.
USHORT ausCL_BaseIO[NUM_DEVICES] =
{ 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff };

// The IRQ levels specified on the command line
USHORT ausCL_IRQ[NUM_DEVICES] =
{ 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff };

// The DMA Channels specified on the command line
USHORT ausCL_DMA[NUM_DEVICES] =
{ 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff };

// The device header name specified on the command line
char szCL_DevName[8] = {' ',' ',' ',' ',' ',' ',' ',' '};

// The DSP Ucode file name specified on the command line
char szCL_DSPUcodeName[SIZE_CONFIG_LINE] = {' '};

// The size of the heap, in bytes
USHORT usCL_HeapSize = 0;

// TRUE if /P and /I parameters were specified on the command line
int fParamsSpecified = FALSE;

// True if the /V parameter was specified
int fVerbose = FALSE;    //###

// True if the hardware initialization errors should be ignore (/O:QUIETINIT)
int fQuietInit = FALSE;

// The value of the /R parameter
int iCL_Resolution=2;

// (### No:) declared in strategy.c
// True if /O:LONGNAMES specified
int fLongNames;                     //###

// switch set at init time based on config sys parm that tells us which
// midi to use FM Synth or MPU401
int fFMforMIDI = FALSE;

// ### True if we should to an INT3() on 1st entry to driver.
int fInt3BeforeInit;

int fMicMute  = TRUE;
int fLineMute = TRUE;
int fCDMute   = TRUE;
int fAuxMute   = TRUE;

CHAR *memchr(CHAR *strP, CHAR c, USHORT size)
/* ###!! BLOCK COPY from AD1848, need figure out how memchr() resolves. */
//    This function searches a string for a particular character and returns
// a pointer to the character in the string.
//    The parameter 'strP' is a pointer to a string of characters. The
// parameter 'c' is the character to evaluate. The parameter 'size' is the
// size of the string.
//    The function returns a pointer to the character in the string if found.
// Otherwise, the value null is returned.
{
    USHORT i;

        // search for the character - return position if found
    i = 0;
    while (i <= size - 1) {
        if (*strP == c)
            return (strP);
        strP++;
        i++;
    }

        // character not found - return null
    return ((CHAR *) 0);
}


USHORT sz2us(char __far *sz, int base)
{
   static char digits[] = "0123456789ABCDEF";

   USHORT us=0;
   char *pc;

// skip leading spaces
   while (*sz == ' ') sz++;

// skip leading zeros
   while (*sz == '0') sz++;

// accumulate digits - return error if unexpected character encountered
   for (;;sz++) {
      pc = (char *) memchr(digits, toupper(*sz), base);
      if (!pc)
         return us;
      us = (us * base) + (pc - digits);
   }
}

int IsWhitespace(char ch)
{
   if ( ch > '9' && ch < 'A')
      return TRUE;
   if ( ch < '0' || ch > 'Z')
      return TRUE;

   return FALSE;
}

char __far *SkipWhite(char __far *psz)
{
   while (*psz) {
      if (!IsWhitespace((char) toupper(*psz))) return psz;
      psz++;
   }
   return NULL;
}

int CopyDevicename(char __far *psz)
{
   int i,j;
   char ch;

// first, check if the filename is valid
   for (i=0; i<9; i++) {
      ch=(char) toupper(psz[i]);
      if (!ch || ch == ' ')
         break;
      if (i==8)                           // too long?
         return FALSE;
      if ( ch > '9' && ch < 'A')
         return FALSE;
      if ( (ch != '$' && ch < '0') || ch > 'Z')
         return FALSE;
   }
   if (!i) return FALSE;                        // zero-length name?

   for (j=0; j<i; j++)
      szCL_DevName[j]=(char) toupper(psz[j]);

   for (;j<8;j++)
      szCL_DevName[j]=' ';

   return TRUE;
}

int DoParm(char cParm, int iPort, char __far *pszOption)
{
   switch (cParm) {
      case '3':
	 fInt3BeforeInit = TRUE;
	 break;
      case 'J':            // which MIDI.  MPU by default, if /J then FMSYNTH
         fFMforMIDI = TRUE;
         break;
      case 'M':                     // Enable mic
	 fMicMute = FALSE;
         break;
      case 'L':                     // Enable line
	 fLineMute = FALSE;
         break;
      case 'C':                     // Enable cd
	 fCDMute = FALSE;
	 break;
      case 'A':                     // Enable cd
	 fAuxMute = FALSE;
	 break;
      case 'N':                     // device header name
	 if (iPort)
	    return FALSE;
	 if (!pszOption)
	    return FALSE;
         if (!CopyDevicename(pszOption))
            return FALSE;
         break;
      case 'V':                     // Verbose option
         if (iPort)
            return FALSE;
         if (pszOption)
            return FALSE;
         fVerbose=TRUE;
         break;
      break;
      default:
         return FALSE;              // unknown parameter
    }

   return TRUE;
}

/* Function: ParseParm
   Input: pointer to the letter of the parameter (e.g. the 'P' in 'P1:330').
          length of this parameter, which must be at least 1
   Output: TRUE if the parameter was value
   Purpose: parses the string into three parts: the letter parameter, the port
            number, and the option string.  Calls DoParm with these values.
   Notes:
      the following describes the format of valid parameters.
         1. Parameters consist of a letter, an optional number, and an
            optional 'option'.  The format is x[n][:option], where 'x' is the
            letter, 'n' is the number, and 'option' is the option.
         2. Blanks are delimeters between parameters, therefore there are no
            blanks within a parameter.
         3. The option is preceded by a colon
      This gives us only four possibilities:
         P (length == 1)
         P1 (length == 2)
         P:option (length >= 3)
         P1:option (length >= 4)
*/
int ParseParm(char __far *pszParm, int iLength)
{
   char ch,ch1=(char) toupper(*pszParm);       // get letter

   if (iLength == 1)                // only a letter?
      return DoParm(ch1,0,NULL);

   ch=pszParm[1];                   // should be either 1-9 or :
   if (ch < '1' || (ch > '9' && ch != ':'))
      return FALSE;

   if (iLength == 2) {
      if (ch == ':')
         return FALSE;
      return DoParm(ch1,ch - '0',NULL);
   }

   if (iLength == 3) {
      if (ch != ':')
         return FALSE;
      return DoParm(ch1,0,pszParm+2);
   }

   if (ch == ':')
      return DoParm(ch1,0,pszParm+2);

   return DoParm(ch1,ch - '0',pszParm+3);
}

int GetParms(char __far *pszCmdLine)
{
   int iLength;

   while (*pszCmdLine != ' ') {              // skip over filename
      if (!*pszCmdLine) return TRUE;         // no params?  then just exit
      pszCmdLine++;
   }

   while (TRUE) {
      pszCmdLine=SkipWhite(pszCmdLine);      // move to next param
      if (!pszCmdLine) return TRUE;          // exit if no more

      for (iLength=0; pszCmdLine[iLength]; iLength++)    // calculate length
         if (pszCmdLine[iLength] == ' ') break;          //  of parameter

      if (!ParseParm(pszCmdLine,iLength))    // found parameter, so parse it
         return FALSE;

      while (*pszCmdLine != ' ') {              // skip over parameter
         if (!*pszCmdLine) return TRUE;         // no params?  then just exit
         pszCmdLine++;
      }
   }
}
