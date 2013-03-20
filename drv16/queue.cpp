/* $Id: queue.cpp,v 1.1 2000/04/23 14:55:19 ktk Exp $ */

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
 * @version %I%
 * @context Unless otherwise noted, all interfaces are Ring-0, 16-bit,
 *  <stack context>.
 * @history
 *
 */
#define INCL_NOPMAPI
//#define INCL_DOSERRORS            // for ERROR_INVALID_FUNCTION
#include <os2.h>
#include "queue.hpp"


PQUEUEELEMENT QUEUEHEAD::Head(void)
{
   return(pHead);
}

PQUEUEELEMENT QUEUEHEAD::Tail(void)
{
   return(pTail);
}

void QUEUEHEAD::PushOnHead(PQUEUEELEMENT pElement)
{
   if (pHead == NULL) {
      pElement->pNext = NULL;
      pHead = pElement;
      pTail = pElement;
   }
   else {
      pElement->pNext = pHead;
      pHead = pElement;
   } /* endif */

}
void QUEUEHEAD::PushOnTail(PQUEUEELEMENT pElement)
{
   pElement->pNext = NULL;
   if (pHead == NULL) {
      pHead = pElement;
      pTail = pElement;
   }
   else {
      pTail->pNext = pElement;
      pTail = pElement;
   } /* endif */

}
PQUEUEELEMENT QUEUEHEAD::PopHead(void)
{
   PQUEUEELEMENT temp = pHead;

   if (temp) {
      pHead = temp->pNext;
      if (pHead == NULL)
         pTail = NULL;
      temp->pNext = NULL;
   }
   return(temp);
}
PQUEUEELEMENT QUEUEHEAD::PopTail(void)
{
   PQUEUEELEMENT temp = pTail;
   PQUEUEELEMENT temp2 = pHead;

    // only 1 element on queue or it's empty
    // zapp the head and tail pointers
    // zapp the next pointer in the element being returned
    // get out of here
   if (pHead == pTail) {
      pHead = NULL;
      pTail = NULL;
      if (temp)
         temp->pNext = NULL;
      return(temp);
   }
   else {
      if (temp) {
         while (temp2->pNext != temp)
            temp2 = temp2->pNext;
         pTail = temp2;

         if (pTail == NULL)
            pHead = NULL;
         else
            pTail->pNext = NULL;
         temp->pNext = NULL;
      }
   }
   return(temp);
}

USHORT QUEUEHEAD::DestroyElement(PQUEUEELEMENT pElement)
{
   if (PopElement(pElement)) {
      delete(pElement);
      return(1);
   }
   else
      return(0);


}
PQUEUEELEMENT QUEUEHEAD::PopElement(PQUEUEELEMENT pElement)
{
   PQUEUEELEMENT temp;

      // if the queue is empty return NULL
   if (pHead == NULL)
      return (NULL);

      // Popping the head element
   if (pElement == pHead)
      return(PopHead());

      // Popping the tail  element
   if (pElement == pTail)
      return(PopTail());

   temp = pHead;
      // walk the queue till we find the element or we hit the end
   while ((temp->pNext != pElement) && (temp->pNext != NULL))
      temp = temp->pNext;
   if (temp == NULL) // element not found
      return(NULL);
   temp->pNext = pElement->pNext;
   return(pElement);

}
ULONG QUEUEHEAD::IsElements(void)
{
   if (pHead == NULL)
      return(0);
   else
      return(1);
}

QUEUEHEAD::QUEUEHEAD(void)
{
    pHead = NULL;
    pTail = NULL;
}
