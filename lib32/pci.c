/* $Id: pci.c,v 1.3 2000/07/23 16:21:56 sandervl Exp $ */

//******************************************************************************
// OS/2 IDC services (callback to 16 bits MMPM2 driver)
//
// Copyright 2000 Sander van Leeuwen (sandervl@xs4all.nl)
//
// Parts based on Linux kernel code (pci_read/write_*)
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
#include <asm/io.h>

#define LINUX
#include <ossidc.h>
#include <stacktoflat.h>

#define MAX_PCI_DEVICES 	16
#define MAX_PCI_BUSSES	        2

static struct pci_dev pci_devices[MAX_PCI_DEVICES] = {0};
static struct pci_bus pci_busses[MAX_PCI_BUSSES] = {0};

//******************************************************************************
//******************************************************************************
int pcidev_prepare(struct pci_dev *dev)
{
	return 1; //todo: correct return value??
}
//******************************************************************************
//******************************************************************************
int pcidev_activate(struct pci_dev *dev)
{
	return 1; //todo: correct return value??
}
//******************************************************************************
//******************************************************************************
int pcidev_deactivate(struct pci_dev *dev)
{
	return 1; //todo: correct return value??
}
//******************************************************************************
//TODO: Doesn't completely fill in the pci_dev structure
//******************************************************************************
int FindPCIDevice(unsigned int vendor, unsigned int device, struct pci_dev near *pcidev)
{
 IDC_RESOURCE idcres;
 ULONG devid, pResource;
 int i, residx = 0;

	pcidev->prepare    = pcidev_prepare;
	pcidev->activate   = pcidev_activate;
	pcidev->deactivate = pcidev_deactivate;
	pcidev->active     = 1;
	pcidev->ro         = 0;
	pcidev->sibling    = NULL;
	pcidev->next       = NULL;
	pcidev->vendor     = vendor;
	pcidev->device     = device;
	
	devid     = (device << 16) | vendor;

#ifdef KEE
	pResource = __FlatToStack(&idcres);
#else
	pResource = __Compress48Pointer((char FAR48 *)&idcres);
#endif
	if(CallOSS16(IDC16_FIND_PCIDEVICE, devid, pResource) == FALSE) {
		return FALSE;
	}

	for(i=0;i<MAX_RES_IO;i++) {
		if(idcres.io[i] != 0xffff) {
			pcidev->resource[residx].name  = 0;
			pcidev->resource[residx].child = 0;
			pcidev->resource[residx].sibling = 0;
			pcidev->resource[residx].parent = 0;
			pcidev->resource[residx].start = idcres.io[i];
			pcidev->resource[residx].end   = idcres.io[i] + idcres.iolength[i]; //inclusive??
			pcidev->resource[residx].flags = IORESOURCE_IO | PCI_BASE_ADDRESS_SPACE_IO;

			residx++;
		}
	}
	for(i=0;i<MAX_RES_MEM;i++) {
		if(idcres.mem[i] != 0xffffffff) {
			pcidev->resource[residx].name  = 0;
			pcidev->resource[residx].child = 0;
			pcidev->resource[residx].sibling = 0;
			pcidev->resource[residx].parent = 0;
			pcidev->resource[residx].start = idcres.mem[i];
			pcidev->resource[residx].end   = idcres.mem[i] + idcres.memlength[i]; //inclusive??
			pcidev->resource[residx].flags = IORESOURCE_MEM | IORESOURCE_MEM_WRITEABLE;

			residx++;
		}
	}
	for(i=0;i<MAX_RES_DMA;i++) {
		if(idcres.dma[i] != 0xffff) {
			pcidev->dma_resource[i].name  = 0;
			pcidev->dma_resource[i].child = 0;
			pcidev->dma_resource[i].sibling = 0;
			pcidev->dma_resource[i].parent = 0;
			pcidev->dma_resource[i].start = idcres.dma[i];
			pcidev->dma_resource[i].end   = idcres.dma[i];
			//todo: 8/16 bits
			pcidev->dma_resource[i].flags = IORESOURCE_DMA;
		}
	}
	for(i=0;i<MAX_RES_IRQ;i++) {
		if(idcres.irq[i] != 0xffff) {
			pcidev->irq_resource[i].name  = 0;
			pcidev->irq_resource[i].child = 0;
			pcidev->irq_resource[i].sibling = 0;
			pcidev->irq_resource[i].parent = 0;
			pcidev->irq_resource[i].start = idcres.irq[i];
			pcidev->irq_resource[i].end   = idcres.irq[i];
			//todo: irq flags
			pcidev->irq_resource[9].flags = IORESOURCE_IRQ;
		}
	}
	pcidev->irq = pcidev->irq_resource[0].start;
	pcidev->bus = &pci_busses[0];
 	return TRUE;
}
//******************************************************************************
//******************************************************************************
struct pci_dev *pci_find_device (unsigned int vendor, unsigned int device, struct pci_dev *from)
{
 int i;

	if(from) {
		//requesting 2nd device of the same type; don't support this for now
		return 0;
        }
	for(i=0;i<MAX_PCI_DEVICES;i++) {
		if(pci_devices[i].devfn == 0) {
			if(FindPCIDevice(vendor, device, (struct pci_dev near *)&pci_devices[i]) == TRUE) {
				return &pci_devices[i];
			}
			break;
		}
	}
	return 0;
}
//******************************************************************************
#define CONFIG_CMD(dev, where)   (0x80000000 | (dev->bus->number << 16) | (dev->devfn << 8) | (where & ~3))
//******************************************************************************
int pci_read_config_byte(struct pci_dev *dev, int where, u8 *value)
{
	outl(CONFIG_CMD(dev,where), 0xCF8);
	*value = inb(0xCFC + (where&3));
	return PCIBIOS_SUCCESSFUL;
}
//******************************************************************************
//******************************************************************************
int pci_read_config_word(struct pci_dev *dev, int where, u16 *value)
{
	outl(CONFIG_CMD(dev,where), 0xCF8);    
	*value = inw(0xCFC + (where&2));
	return PCIBIOS_SUCCESSFUL;    
}
//******************************************************************************
//******************************************************************************
int pci_read_config_dword(struct pci_dev *dev, int where, u32 *value)
{
	outl(CONFIG_CMD(dev,where), 0xCF8);
	*value = inl(0xCFC);
	return PCIBIOS_SUCCESSFUL;    
}
//******************************************************************************
//******************************************************************************
int pci_write_config_byte(struct pci_dev *dev, int where, u8 value)
{
	outl(CONFIG_CMD(dev,where), 0xCF8);    
	outb(value, 0xCFC + (where&3));
	return PCIBIOS_SUCCESSFUL;
}
//******************************************************************************
//******************************************************************************
int pci_write_config_word(struct pci_dev *dev, int where, u16 value)
{
	outl(CONFIG_CMD(dev,where), 0xCF8);
	outw(value, 0xCFC + (where&2));
	return PCIBIOS_SUCCESSFUL;
}
//******************************************************************************
//******************************************************************************
int pci_write_config_dword(struct pci_dev *dev, int where, u32 value)
{
	outl(CONFIG_CMD(dev,where), 0xCF8);
	outl(value, 0xCFC);
	return PCIBIOS_SUCCESSFUL;
}
//******************************************************************************
//******************************************************************************
int pcibios_present(void)
{
	return 1;
}
//******************************************************************************
//******************************************************************************
struct pci_dev *pci_find_slot (unsigned int bus, unsigned int devfn)
{
	return NULL;
}
//******************************************************************************
//******************************************************************************
int pci_dma_supported(struct pci_dev *dev, unsigned long mask)
{
 	return 1;
}
//******************************************************************************
//******************************************************************************
int pci_enable_device(struct pci_dev *dev)
{
	return 0;
}
//******************************************************************************
struct pci_dev *sblivepcidev = NULL;
//******************************************************************************
int pci_register_driver(struct pci_driver *driver)
{
 struct pci_dev *pcidev;

	pcidev = pci_find_device(driver->id_table->vendor, driver->id_table->device, NULL);
	if(pcidev) {
		if(driver->probe(pcidev, driver->id_table) == 0) {
			sblivepcidev = pcidev;
			return 0;
		}
	}
	return 1;
}
//******************************************************************************
//******************************************************************************
int pci_unregister_driver(struct pci_driver *driver)
{
	if(driver && sblivepcidev) {
		driver->remove(sblivepcidev);
	}
	return 0;
}
//******************************************************************************
//******************************************************************************
void pci_set_master(struct pci_dev *dev)
{

}
//******************************************************************************
//******************************************************************************
int __compat_get_order(unsigned long size)
{
        int order;

        size = (size-1) >> (PAGE_SHIFT-1);
        order = -1;
        do {
                size >>= 1;
                order++;
        } while (size);
        return order;
}
//******************************************************************************
//******************************************************************************
void *pci_alloc_consistent(struct pci_dev *hwdev,
                           size_t size, dma_addr_t *dma_handle) {
        void *ret;
        int gfp = GFP_ATOMIC;

        if (hwdev == NULL)
                gfp |= GFP_DMA;
        ret = (void *)__get_free_pages(gfp, __compat_get_order(size));

        if (ret != NULL) {
                memset(ret, 0, size);
                *dma_handle = virt_to_bus(ret);
        }
        return ret;
}
//******************************************************************************
//******************************************************************************
void pci_free_consistent(struct pci_dev *hwdev, size_t size,
                         void *vaddr, dma_addr_t dma_handle)
{
        free_pages((unsigned long)vaddr, __compat_get_order(size));
}
//******************************************************************************
//******************************************************************************
