/* $Id: cpp.cpp,v 1.1 2000/04/23 14:55:39 ktk Exp $ */

/* CPP.CPP - Replacement runtime routines for C++
*/

#include <malloc.h>
#include <include.h>

extern "C" void __wcpp_2_undefed_cdtor_(void)
{
   int3();
}

extern "C" void __wcpp_2_undefined_member_function_(void)
{
   int3();
}

extern "C" void __wcpp_2_pure_error_(void)
{
   int3();
}

extern "C" void __wcpp_2_undef_vfun_(void)
{
   int3();
}

extern "C" void __wcpp_4_undefed_cdtor_(void)
{
   int3();
}

extern "C" void __wcpp_4_undefined_member_function_(void)
{
   int3();
}

extern "C" void __wcpp_4_pure_error_(void)
{
   int3();
}

extern "C" void __wcpp_4_undef_vfun_(void)
{
   int3();
}

void* operator new(unsigned u)
{
   return malloc(u);
}

void operator delete(void *p)
{
   free(p);
}

void *operator new(unsigned u, void near *p)
{
   return u ? p : NULL;
}

