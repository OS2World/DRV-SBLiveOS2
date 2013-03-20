/* $Id: commdbg.c,v 1.2 2000/07/17 18:32:33 sandervl Exp $ */

/******************************************************************************
*                       Jazz16 Physical Device Driver
*                     Production code and toolkit sample
*
* (c) Copyright IBM Corporation 1993.  All rights reserved
* (c) Copyright Media Vision Corporation 1993, 1994.  All rights reserved
*
* DISCLAIMER OF WARRANTIES.  The following [enclosed] code is
* sample code created by IBM Corporation and Media Vision Corporation.
* It is provided to you solely for the purpose of assisting you in the
* development of your applications.
* The code is provided "AS IS", without warranty of any kind.
* IBM and Media Vision shall not be liable for any damages arising out of
* your use of the sample code, even if they have been advised of the
* possibility of such damages.
*
*******************************************************************************
*
* commdbg.c = debug routines to send data to COM: port
*
* This file contains routines to send debug information to COM: ports
* during execution of device driver.
* The routines provide 'C' printf like functions that send their output
* to the serial port rather than stdout.
* The calls to these routines are only called for debug versions of the
* driver.  See file debug.h for flags to turn on/off different debug options.
*
* MODIFICATION HISTORY:
* DATE      CHANGE DESCRIPTION
* unknown   Creation (Media Vision)
* 07/30/93  Add headers for toolkit
* 09/21/93  Header modifications
******************************************************************************/

#define INCL_NOPMAPI
#define INCL_DOSERRORS           // for ERROR_INVALID_FUNCTION
#include <os2.h>
#include "dbgos2.h"
#include "commdbg.h"

BOOL fLineTerminate=TRUE;

#define CR 0x0d
#define LF 0x0a


#define LEADING_ZEROES          0x8000
#define SIGNIFICANT_FIELD       0x0007

int dbglevel = 2;

char hextab[]="0123456789ABCDEF";

                                        //-------------------- DecWordToASCII -
char far * cdecl DecWordToASCII(char far *StrPtr, USHORT wDecVal, USHORT Option)
{
  BOOL fNonZero=FALSE;
  USHORT Digit;
  USHORT Power=10000;

  while (Power)
     {
     Digit=0;
     while (wDecVal >=Power)                   //Digit=wDecVal/Power;
        {
        Digit++;
        wDecVal-=Power;
        }

     if (Digit)
        fNonZero=TRUE;

     if (Digit ||
         fNonZero ||
         (Option & LEADING_ZEROES) ||
         ((Power==1) && (fNonZero==FALSE)))
         {
         *StrPtr=(char)('0'+Digit);
         StrPtr++;
         }

     if (Power==10000)
        Power=1000;
     else if (Power==1000)
        Power=100;
     else if (Power==100)
        Power=10;
     else if (Power==10)
        Power=1;
     else
        Power=0;
     } // end while

  return (StrPtr);
}

                                        //-------------------- DecLongToASCII -
char far * cdecl DecLongToASCII(char far *StrPtr, ULONG lDecVal,USHORT Option)
{
   BOOL  fNonZero=FALSE;
   ULONG Digit;
   ULONG Power=1000000000;                      // 1 billion

   while (Power)
      {
      Digit=0;                                                                        // Digit=lDecVal/Power
      while (lDecVal >=Power)                   // replaced with while loop
         {
         Digit++;
         lDecVal-=Power;
         }

      if (Digit)
         fNonZero=TRUE;

      if (Digit ||
          fNonZero ||
          (Option & LEADING_ZEROES) ||
          ((Power==1) && (fNonZero==FALSE)))
         {
         *StrPtr=(char)('0'+Digit);
         StrPtr++;
         }

      if (Power==1000000000)                    // 1 billion
         Power=100000000;
      else if (Power==100000000)
         Power=10000000;
      else if (Power==10000000)
         Power=1000000;
      else if (Power==1000000)
         Power=100000;
      else if (Power==100000)
         Power=10000;
      else if (Power==10000)
         Power=1000;
      else if (Power==1000)
         Power=100;
      else if (Power==100)
         Power=10;
      else if (Power==10)
         Power=1;
      else
         Power=0;
      }
   return (StrPtr);
}
                                        //-------------------- HexWordToASCII -
char far * cdecl HexWordToASCII(char far *StrPtr, USHORT wHexVal, USHORT Option)
{
   BOOL fNonZero=FALSE;
   USHORT Digit;
   USHORT Power=0xF000;
   USHORT ShiftVal=12;

   while (Power)
      {
      Digit=(wHexVal & Power)>>ShiftVal;
      if (Digit)
         fNonZero=TRUE;

      if (Digit ||
          fNonZero ||
          (Option & LEADING_ZEROES) ||
          ((Power==0x0F) && (fNonZero==FALSE)))
         //*StrPtr++=(char)('0'+Digit);
         *StrPtr++=hextab[Digit];

      Power>>=4;
      ShiftVal-=4;
      } // end while

   return (StrPtr);
}

                                        //-------------------- HexLongToASCII -
char far * cdecl HexLongToASCII(char far *StrPtr, ULONG wHexVal, USHORT Option)
{
   BOOL  fNonZero=FALSE;
   ULONG Digit;
   ULONG Power=0xF0000000;
   ULONG ShiftVal=28;

   while (Power)
      {
      Digit=(wHexVal & Power)>>ShiftVal;
      if (Digit)
         fNonZero=TRUE;

      if (Digit ||
          fNonZero ||
          (Option & LEADING_ZEROES) ||
          ((Power==0x0F) && (fNonZero==FALSE)))
          *StrPtr++=hextab[Digit];

      if (Power==0xF0000000)                  // 1 billion
         Power=0xF000000;     
      else if (Power==0xF000000)
         Power=0xF00000;
      else if (Power==0xF00000)
         Power=0xF0000;
      else if (Power==0xF0000)
         Power=0xF000;
      else if (Power==0xF000)
         Power=0xF00;
      else if (Power==0xF00)
         Power=0xF0;
      else if (Power==0xF0)
         Power=0xF;
      else Power=0;

      ShiftVal-=4;
      } // end while

   return (StrPtr);
}

#ifdef  DEBUG
char BuildString[1024];

                                        //------------------------- PrintfOut -
void cdecl PrintfOut(char far *DbgStr , ...)
{
   char far *BuildPtr=BuildString;
   char far *pStr=(char far *) DbgStr;
   char far *SubStr;
   union {
         void    far *VoidPtr;
         USHORT    far *WordPtr;
         ULONG   far *LongPtr;
         ULONG far *StringPtr;
         } Parm;
   USHORT wBuildOption;

   Parm.VoidPtr=(void far *) &DbgStr;
   Parm.StringPtr++;                            // skip size of string pointer

   while (*pStr)
      {
      // don't overflow target
      if (BuildPtr >= (char far *) &BuildString[1024-2])
         break;

      switch (*pStr)
         {
         case '%':
            wBuildOption=0;
            pStr++;
            if (*pStr=='0')
               {
               wBuildOption|=LEADING_ZEROES;
               pStr++;
               }
            if (*pStr=='u')                                                         // always unsigned
               pStr++;

            switch(*pStr)
               {
               case 'x':
	       case 'X':
                  BuildPtr=HexWordToASCII(BuildPtr, *Parm.WordPtr++,wBuildOption);
                  pStr++;
                  continue;

               case 'd':
                  BuildPtr=DecWordToASCII(BuildPtr, *Parm.WordPtr++,wBuildOption);
                  pStr++;
                  continue;

               case 's':
                  SubStr=(char far *)*Parm.StringPtr;
                  while (*BuildPtr++ = *SubStr++);
                  Parm.StringPtr++;
                  BuildPtr--;                      // remove the \0
                  pStr++;
                  continue;

               case 'l':
                  pStr++;
                  switch (*pStr)
                  {
                  case 'x':
                  case 'X':
                  BuildPtr=HexLongToASCII(BuildPtr, *Parm.LongPtr++,wBuildOption);
                  pStr++;
                  continue;

                  case 'd':
                     BuildPtr=DecLongToASCII(BuildPtr, *Parm.LongPtr++,wBuildOption);
                     pStr++;
                     continue;
                  } // end switch
                  continue;                        // dunno what he wants

               case 0:
                  continue;
               } // end switch
            break;

      case '\\':
         pStr++;
         switch (*pStr)
            {
            case 'n':
            *BuildPtr++=LF;
            pStr++;
            continue;

            case 'r':
            *BuildPtr++=CR;
            pStr++;
            continue;

            case 0:
            continue;
            break;
            } // end switch

         break;
         } // end switch

      *BuildPtr++=*pStr++;
      } // end while

   *BuildPtr=0;                                 // cauterize the string
   StringOut((char far *) BuildString);         // print to comm port
}
#endif                            //DEBUG


#ifdef DEBUG                            //------------------------- StringOut -
void StringOut(char far *DbgStr)
{
   while (*DbgStr)
      CharOut(*DbgStr++);

   if (fLineTerminate)
   {
      CharOut(CR);                              // append carriage return,
      CharOut(LF);                              // linefeed
   }
}
#endif

#ifdef DEBUG
//#define       MAGIC_COMM_PORT 0x3f8           // pulled from word ptr 40:0
#define         MAGIC_COMM_PORT 0x2f8           // pulled from word ptr 40:0


#define UART_DATA               0x00            // UART Data port
#define UART_INT_ENAB           0x01            // UART Interrupt enable
#define UART_INT_ID             0x02            // interrupt ID
#define UART_LINE_CTRL          0x03            // line control registers
#define UART_MODEM_CTRL         0x04            // modem control register
#define UART_LINE_STAT          0x05            // line status register
#define UART_MODEM_STAT         0x06            // modem status regiser
#define UART_DIVISOR_LO         0x00            // divisor latch least sig
#define UART_DIVISOR_HI         0x01h           // divisor latch most sig

#define DELAY   nop
#endif

#ifdef DEBUG                            //--------------------------- CharOut -
void CharOut(char c)
{
        _asm    {

        mov     dx, MAGIC_COMM_PORT     // address of PS/2's first COM port
        add     dx, UART_LINE_STAT

ReadyCheck:
        in      al, dx                                                          // wait for comm port ready signal

        DELAY
        DELAY
        DELAY

        test    al, 020h
        jz      ReadyCheck

        // Send the character

        add     dx, UART_DATA - UART_LINE_STAT
        mov     al,c
        out     dx, al

        DELAY
        DELAY
        DELAY
        }
}
#endif
