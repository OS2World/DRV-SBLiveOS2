/* $Id: memory.cpp,v 1.3 2000/07/17 18:36:34 sandervl Exp $ */

//******************************************************************************
// OS/2 implementation of Linux memory kernel services
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
extern "C" {
#define INCL_NOPMAPI
#define INCL_DOSERRORS           // for ERROR_INVALID_FUNCTION
#include <os2.h>
}
#include <devhelp.h>
#include <ossidc.h>
#include <string.h>
#ifdef KEE
#include <kee.h>
#endif

#pragma off (unreferenced)

#define PAGE_SIZE 4096

extern "C" {

//******************************************************************************
//NOTE: Assumes memory is continuous!!
//******************************************************************************
unsigned long virt_to_phys(void * address)
{
#ifdef KEE
 KEEVMPageList pagelist;
 ULONG         nrpages;

	if(KernLinToPageList(address, PAGE_SIZE, &pagelist, &nrpages)) {
		DebugInt3();
		return 0;
	}
	return pagelist.addr;
#else
 LINEAR addr = (LINEAR)address;
 PAGELIST pagelist;

	if(DevLinToPageList(addr, PAGE_SIZE, (PAGELIST near *)__StackToFlat((ULONG)&pagelist))) {
		DebugInt3();
		return 0;
	}
	return pagelist.physaddr;
#endif
}
//******************************************************************************
//******************************************************************************
void * phys_to_virt(unsigned long address)
{
	DebugInt3();
	return 0;
}
//******************************************************************************
//******************************************************************************
void *__get_free_pages(int gfp_mask, unsigned long order)
{
 ULONG addr;

	order = (1 << order); //TODO: Is this correct???
#ifdef KEE
 SHORT sel;

    	if(KernVMAlloc(order*PAGE_SIZE, VMDHA_FIXED|VMDHA_CONTIG, 
                       (PVOID*)&addr, (PVOID*)-1, &sel)) {
#else
    	if(DevVMAlloc(VMDHA_FIXED|VMDHA_CONTIG, order*PAGE_SIZE, (LINEAR)-1, __StackToFlat((ULONG)&addr))) {
#endif
		DebugInt3();
		return 0;
	}
////	dprintf(("__get_free_pages %d returned %x", order*PAGE_SIZE, addr));
	return (void *)addr;
}
//******************************************************************************
//******************************************************************************
int free_pages(unsigned long addr, unsigned long order)
{
#ifdef KEE
	KernVMFree((PVOID)addr);
#else
	DevVMFree((LINEAR)addr);
#endif
////	dprintf(("free_pages %x", addr));
	return 0;
}
//******************************************************************************
//******************************************************************************
struct page * alloc_pages(int gfp_mask, unsigned long order)
{
	DebugInt3();
	return 0;
}
//******************************************************************************
//******************************************************************************
int remap_page_range(unsigned long from, unsigned long to, unsigned long size, unsigned long prot)
{
	DebugInt3();
	return 0;
}
//******************************************************************************
//******************************************************************************
int is_access_ok(int type, void *addr, unsigned long size)
{
	return 1;
}
//******************************************************************************
//******************************************************************************
void __copy_user(void *to, const void *from, unsigned long n)
{
	if(to == NULL || from == NULL) {
		DebugInt3();
		return;
	}
        if(n == 0) return;
#ifdef KEE
	memcpy(to, from, n);
#else
	_fmemcpy(to, from, n);
#endif
}
//******************************************************************************
//******************************************************************************
unsigned long copy_to_user(void *to, const void *from, unsigned long n)
{
	if(to == NULL || from == NULL) {
		DebugInt3();
		return 0;
	}
        if(n == 0) return 0;
#ifdef KEE
	memcpy(to, from, n);
#else
	_fmemcpy(to, from, n);
#endif
	return 0;
}
//******************************************************************************
//******************************************************************************
void __copy_user_zeroing(void *to, const void *from, unsigned long n)
{
	if(to == NULL || from == NULL) {
		DebugInt3();
		return;
	}
        if(n == 0) return;
	copy_to_user(to, from, n);
}
//******************************************************************************
//******************************************************************************
unsigned long copy_from_user(void *to, const void *from, unsigned long n)
{
	if(to == NULL || from == NULL) {
		DebugInt3();
		return 0;
	}
        if(n == 0) return 0;
#ifdef KEE
	memcpy(to, from, n);
#else
	_fmemcpy(to, from, n);
#endif
	return 0;
}
//******************************************************************************
//******************************************************************************
int get_user(int size, void *dest, void *src)
{
	if(size == 0)	return 0;

	if(dest == NULL || src == NULL) {
		DebugInt3();
		return 0;
	}
#ifdef KEE
	memcpy(dest, src, size);
#else
	_fmemcpy(dest, src, size);
#endif
	return 0;
}
//******************************************************************************
//******************************************************************************
int put_user(int x, void *ptr)
{
	if(ptr == NULL) {
		DebugInt3();
		return 0;
	}

	*(int *)ptr = x;
	return 0;
}
//******************************************************************************
//******************************************************************************
#if 0
int __put_user(int size, int x, void *ptr)
{
#ifdef KEE
	memcpy(ptr,
#else
	_fmemcpy(ptr, 
#endif
	return 0;
}
#endif
//******************************************************************************
//******************************************************************************
void *kmalloc(int size, int flags)
{
 char near *addr;

  if(size == 0) {
	DebugInt3();
	return NULL;
  }
  if(size > 1024) {
#ifdef KEE
   SHORT sel;

    	if(KernVMAlloc(size+4, VMDHA_FIXED, 
                       (PVOID*)&addr, (PVOID*)-1, &sel)) {
#else
    	if(DevVMAlloc(VMDHA_FIXED, size+4, (LINEAR)-1, __StackToFlat((ULONG)&addr))) {
#endif
		DebugInt3();
		return 0;
	}
	*(ULONG *)addr = 0; //flat address
//// 	dprintf(("kmalloc %d returned %x", size, addr));
	return addr+4;
  }
  addr = (char near *)CallOSS16(IDC16_MALLOC, size, 0);
  if(addr == NULL) {
	DebugInt3();
	return 0;
  }
////  dprintf(("kmalloc %d returned %x", size, addr));
  return addr+4; //first 4 bytes contain original 16:16 address
}
//******************************************************************************
//******************************************************************************
void kfree(const void *ptr)
{
 ULONG addr;

  addr  = (ULONG)ptr;
  if(addr == NULL) {
	DebugInt3();
	return;
  }
  addr -= 4;  //first 4 bytes contain original 16:16 address or 0 if allocated by VMAlloc
////  dprintf(("kfree %x", addr));
  if(*(ULONG near *)addr) {  
  	CallOSS16(IDC16_FREE, *(ULONG near *)addr, 0);  
  }
#ifdef KEE
  else  KernVMFree((PVOID)addr);
#else
  else  DevVMFree((LINEAR)addr);
#endif
}
//******************************************************************************
//******************************************************************************
void kfree_s(const void *ptr, unsigned int size)
{
  kfree(ptr);
}
//******************************************************************************
//******************************************************************************

}