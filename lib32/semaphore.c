/* $Id: semaphore.c,v 1.1 2000/04/23 14:55:38 ktk Exp $ */

//******************************************************************************
// OS/2 implementation of Linux semaphore kernel services (stubs)
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

void init_MUTEX (struct semaphore *sem)
{
}

void init_MUTEX_LOCKED (struct semaphore *sem)
{
}

void down(struct semaphore * sem)
{

}

void up(struct semaphore * sem)
{

}
