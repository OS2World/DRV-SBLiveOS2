/* $Id: module.h,v 1.1 2000/04/23 14:55:32 ktk Exp $ */

/*
 * Dynamic loading of modules into the kernel.
 *
 * Rewritten by Richard Henderson <rth@tamu.edu> Dec 1996
 */

#ifndef _LINUX_MODULE_H
#define _LINUX_MODULE_H

/* Poke the use count of a module.  */

#define MOD_INC_USE_COUNT
#define MOD_DEC_USE_COUNT
#define MOD_IN_USE

#define EXPORT_NO_SYMBOLS

#define __MODULE_STRING_1(x)	#x
#define __MODULE_STRING(x)	__MODULE_STRING_1(x)

/* For documentation purposes only.  */

#define MODULE_AUTHOR(name)						   \
const char __module_author[] = 	   \
"author=" name

#define MODULE_DESCRIPTION(desc)					   \
const char __module_description[] =   \
"description=" desc

/* Could potentially be used by kmod...  */

#define MODULE_SUPPORTED_DEVICE(dev)					   \
const char __module_device[] = 	   \
"device=" dev

/* Used to verify parameters given to the module.  The TYPE arg should
   be a string in the following format:
   	[min[-max]]{b,h,i,l,s}
   The MIN and MAX specifiers delimit the length of the array.  If MAX
   is omitted, it defaults to MIN; if both are omitted, the default is 1.
   The final character is a type specifier:
	b	byte
	h	short
	i	int
	l	long
	s	string
*/

#define MODULE_PARM(var,type)			\
const char __module_parm_##var[]=		\
"parm_" __MODULE_STRING(var) "=" type

#define MODULE_PARM_DESC(var,desc)		\
const char __module_parm_desc_##var[]=		\
"parm_desc_" __MODULE_STRING(var) "=" desc

#endif /* _LINUX_MODULE_H */
