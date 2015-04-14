/* $Id: param.h,v 1.1 2000/04/23 14:55:29 ktk Exp $ */

#ifndef _ASMi386_PARAM_H
#define _ASMi386_PARAM_H

#ifndef HZ
#define HZ 100
#endif

#define EXEC_PAGESIZE	4096

#ifndef NGROUPS
#define NGROUPS		32
#endif

#ifndef NOGROUP
#define NOGROUP		(-1)
#endif

#define MAXHOSTNAMELEN	64	/* max length of hostname */

#endif
