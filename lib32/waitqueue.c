/* $Id: waitqueue.c,v 1.2 2001/09/09 15:30:52 sandervl Exp $ */

//******************************************************************************
// OS/2 implementation of Linux wait queue kernel services (stubs)
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
#include "hwaccess.h"
#include <linux/init.h>
#include <linux/poll.h>
#include <asm/uaccess.h>
#include <asm/hardirq.h>


void init_waitqueue_head(wait_queue_head_t *q)
{

}

void add_wait_queue(wait_queue_head_t *q, wait_queue_t * wait)
{
}

void add_wait_queue_exclusive(wait_queue_head_t *q)
{
}

void remove_wait_queue(wait_queue_head_t *q, wait_queue_t * wait)
{
}

long interruptible_sleep_on_timeout(wait_queue_head_t *q, signed long timeout)
{
  return 1;
}

void interruptible_sleep_on(wait_queue_head_t *q)
{

}

void __wake_up(wait_queue_head_t *q, unsigned int mode)
{

}

