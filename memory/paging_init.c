
#include "../main.h"
#include "memory.h"
#include "../runtime/panic_assert.h"
#include "../runtime/stdio.h"
#include "../graphics/graphics.h"
#include "../kernel/kmain.h"
#include "../interrupt/interrupt.h"
#include "../execformat/pe/reloc.h"
void execformat_pe_ReadSystemHeader(execformat_pe_PortableExecutable *pe);

#include <stdint.h>
#include <string.h>

#include "paging_internal.h"


EFI_MEMORY_DESCRIPTOR *efiMemoryMap;
UINTN                  efiMemoryMapSize;
UINTN                  efiMemoryMapKey;
UINTN                  efiDescriptorSize;
UINT32                 efiDescriptorVertion;

uint64_t paging_TotalBytes, paging_UsableBytes;
bool     paging_SupportExecuteDisable;
uint64_t paging_EndPhysicalAddress;                       // past-the-end marker (and length) for physical memory
int      paging_EndPhysicalPage;                          // past-the-end for physical pages (EndPhysicalAddress/SYSTEM_PAGE_SIZE)
uint64_t paging_PML4Table[512] ALIGNED(4096);             // Kernel-mode virtual memory paging directory pointer table (Level 4 paging)
uint64_t paging_LoaderCodeAddress, paging_LoaderCodeSize; // physical address for loader code section
int      paging_LoaderCodePageCount;                      // page count for loader code section

void runtime_InitPaging() {
	// TODO Obtain Execute Disable support status instead of assumpting its existence
	paging_SupportExecuteDisable = true;

	// obtain the UEFI memory mapping
	EFI_STATUS status;

	efiMemoryMapSize = 0;
	efiMemoryMap     = NULL;
	status           = efiBootServices->GetMemoryMap(
        &efiMemoryMapSize,
        efiMemoryMap,
        &efiMemoryMapKey,
        &efiDescriptorSize,
        &efiDescriptorVertion);
	assert(status == EFI_BUFFER_TOO_SMALL && "What? An empty buffer is not too small?");

	efiMemoryMapSize += 2 * sizeof(EFI_MEMORY_DESCRIPTOR);
	efiMemoryMap = (EFI_MEMORY_DESCRIPTOR *)efiMalloc(efiMemoryMapSize);
	assert(efiMemoryMap && "efiMemoryMap allocate failed");
	status = efiBootServices->GetMemoryMap(
		&efiMemoryMapSize,
		efiMemoryMap,
		&efiMemoryMapKey,
		&efiDescriptorSize,
		&efiDescriptorVertion);
	assert(!EFI_ERROR(status) && "GetMemoryMap() with buffer allocated failed");

	io_Printf("  .text: [%08x-%08x] len=%d (%d pages)\n", link_TextStart, link_TextEnd, link_TextEnd - link_TextStart, roundUpToPageCount(link_TextEnd - link_TextStart));
	io_Printf("  .data: [%08x-%08x] len=%d (%d pages)\n", link_DataStart, link_DataEnd, link_DataEnd - link_DataStart, roundUpToPageCount(link_DataEnd - link_DataStart));
	io_Printf(".rodata: [%08x-%08x] len=%d (%d pages)\n", link_RodataStart, link_RodataEnd, link_RodataEnd - link_RodataStart, roundUpToPageCount(link_RodataEnd - link_RodataStart));
	io_Printf("   .bss: [%08x-%08x] len=%d (%d pages)\n\n", link_BssStart, link_BssEnd, link_BssEnd - link_BssStart, roundUpToPageCount(link_BssEnd - link_BssStart));

	// iterate the listing, accumlate counters and print info
	paging_LoaderCodeAddress = paging_TotalBytes = paging_UsableBytes = 0;
	memset(paging_physical_Bitmap, 0xff, sizeof(paging_physical_Bitmap));
	io_WriteConsoleASCII("EFI Memory mapping:\n");
	for (EFI_MEMORY_DESCRIPTOR *entry = efiMemoryMap;
		 (char *)entry < (char *)efiMemoryMap + efiMemoryMapSize;
		 entry = NEXT_MEMORY_DESCRITOR(entry, efiDescriptorSize) {
		io_Printf(
			"    [%08llx-%08llx] -> [%08llx] %s (%d)\n",
			entry->PhysicalStart,
			entry->PhysicalStart + SYSTEM_PAGE_SIZE * entry->NumberOfPages,
			entry->VirtualStart,
			memoryTypeName(entry->Type),
			entry->Type);
		paging_TotalBytes += SYSTEM_PAGE_SIZE * entry->NumberOfPages;
		if (entry->Type == EfiConventionalMemory) {
			// TODO include EfiBootServicesCode/Data as usable
			paging_physical_BitmapWriteZero(
				entry->PhysicalStart / SYSTEM_PAGE_SIZE,
				entry->PhysicalStart / SYSTEM_PAGE_SIZE + entry->NumberOfPages);
			paging_UsableBytes += SYSTEM_PAGE_SIZE * entry->NumberOfPages;
		} else // page unusable
			   /*paging_physical_BitmapWriteOne(
				entry->PhysicalStart / SYSTEM_PAGE_SIZE,
				entry->PhysicalStart / SYSTEM_PAGE_SIZE + entry->NumberOfPages);*/
			;
		if (entry->Type == EfiLoaderCode) {
			assert(!paging_LoaderCodeAddress && "Two EfiLoaderCode mappings at the same time");
			paging_LoaderCodeAddress   = entry->PhysicalStart;
			paging_LoaderCodeSize      = entry->NumberOfPages * SYSTEM_PAGE_SIZE;
			paging_LoaderCodePageCount = entry->NumberOfPages;
		}
		if (paging_EndPhysicalAddress < entry->PhysicalStart + entry->NumberOfPages * SYSTEM_PAGE_SIZE)
			paging_EndPhysicalAddress = entry->PhysicalStart + entry->NumberOfPages * SYSTEM_PAGE_SIZE;
	}
	paging_EndPhysicalPage = paging_EndPhysicalAddress / SYSTEM_PAGE_SIZE;

	io_Printf(
		" Total memory: %llu (%.2lf MB, %.2lf GB), EndPhyAddr %08llx\n",
		paging_TotalBytes,
		paging_TotalBytes / 1024.0 / 1024.0,
		paging_TotalBytes / 1024.0 / 1024.0 / 1024.0,
		paging_EndPhysicalAddress);
	io_Printf(
		"Usable memory: %llu (%.2lf MB, %.2lf GB)\n",
		paging_UsableBytes,
		paging_UsableBytes / 1024.0 / 1024.0,
		paging_UsableBytes / 1024.0 / 1024.0 / 1024.0);

	io_PauseForKeystroke();

	assert(paging_LoaderCodeAddress && "EfiLoaderCode mapping not found");

	io_WriteConsoleASCII("Mapping kernel memory:\n");
	// map kernel code
	io_Printf("    .Text...       %d 4K pages\n",roundUpToPageCount(link_TextEnd-link_TextStart));
	paging_map_Page( // map .text
		(uint64_t)link_TextStart,
		KERNEL_CODE_VIRTUAL + ((uint64_t)link_TextStart - paging_LoaderCodeAddress),
		roundUpToPageCount(link_TextEnd - link_TextStart),
		MAP_PROT_READ | MAP_PROT_EXEC);
	io_Printf("    .Data...       %d 4K pages\n",roundUpToPageCount(link_DataEnd-link_DataStart));
	paging_map_Page( // map .data
		(uint64_t)link_DataStart,
		KERNEL_CODE_VIRTUAL + ((uint64_t)link_DataStart - paging_LoaderCodeAddress),
		roundUpToPageCount(link_DataEnd - link_DataStart),
		MAP_PROT_READ | MAP_PROT_WRITE);
	io_Printf("    .Rodata...     %d 4K pages\n",roundUpToPageCount(link_RodataEnd-link_RodataStart));
	paging_map_Page( // map .rodata
		(uint64_t)link_RodataStart,
		KERNEL_CODE_VIRTUAL + ((uint64_t)link_RodataStart - paging_LoaderCodeAddress),
		roundUpToPageCount(link_RodataEnd - link_RodataStart),
		MAP_PROT_READ);
	io_Printf("    .Bss...        %d 4K pages\n",roundUpToPageCount(link_BssEnd-link_BssStart));
	paging_map_Page( // map .bss
		(uint64_t)link_BssStart,
		KERNEL_CODE_VIRTUAL + ((uint64_t)link_BssStart - paging_LoaderCodeAddress),
		roundUpToPageCount(link_BssEnd - link_BssStart),
		MAP_PROT_READ | MAP_PROT_WRITE);
	//paging_map_Page(paging_LoaderCodeAddress,KERNEL_CODE_VIRTUAL,paging_LoaderCodePageCount,MAP_PROT_READ|MAP_PROT_WRITE|MAP_PROT_EXEC);

	// map other VM data
	io_Printf("    Framebuffer... %d 2M pages\n",roundUpToPageCount2M(graphics_FramebufferSize));
	paging_map_Page2M( // map the framebuffer output
		(uint64_t)graphics_DeviceFramebuffer,
		KERNEL_FRAMEBUFFER_MAPPING,
		roundUpToPageCount2M(graphics_FramebufferSize),
		MAP_PROT_READ | MAP_PROT_WRITE);
	io_Printf("    Physical...    %d 2M pages\n",roundUpToPageCount2M(paging_EndPhysicalAddress));
	paging_map_Page2M( // map the physical memory
		0,
		0,
		roundUpToPageCount2M(paging_EndPhysicalAddress),
		MAP_PROT_READ | MAP_PROT_WRITE | MAP_PROT_EXEC);
	io_Printf("    Stack\n");
	paging_map_Page(paging_physical_AllocateOneFrame(),KERNEL_STACK_END_VIRTUAL-SYSTEM_PAGE_SIZE,1,MAP_PROT_READ|MAP_PROT_WRITE);
	/*paging_map_Page2M( // stack, allocate a fresh new 2M
		paging_physical_AllocateOneFrame2M(),
		KERNEL_STACK_END_VIRTUAL - KERNEL_STACK_INITIAL_SIZE,
		KERNEL_STACK_INITIAL_SIZE / SYSTEM_PAGE_2M_SIZE, 
		MAP_PROT_READ | MAP_PROT_WRITE);*/
	kMain_StackPosition = KERNEL_STACK_END_VIRTUAL;
	/*paging_map_PageAllocated(
		KERNEL_MISC_MAPPING,
		roundUpToPageCount(KERNEL_MISC_SIZE),
		MAP_PROT_READ|MAP_PROT_WRITE);*/
	io_Printf("    Misc...        %d 2M pages\n",roundUpToPageCount2M(KERNEL_MISC_SIZE));
	paging_map_PageAllocated2M( // misc data
		KERNEL_MISC_MAPPING,
		roundUpToPageCount2M(KERNEL_MISC_SIZE),
		MAP_PROT_READ|MAP_PROT_WRITE);
	//paging_map_PageAllocated(KERNEL_HEAP_VIRTUAL,512,MAP_PROT_READ|MAP_PROT_WRITE);

	io_WriteConsoleASCII("Mapping completed\n");



	// woohoo byebye!
	efiBootServices->ExitBootServices(efiImageHandle, efiMemoryMapKey);
	io_WriteConsoleASCII("Goodbye BootServices!\n");

	// so now we're in unmanaged mode, we need to set up heap and stack, and jump to the new entry point kMain_Init.
	// disable interrupts asap
	asm volatile("cli":::"memory");
	interrupt_Enabled=false;

	// set the new virtual memory mapping
	if (paging_SupportExecuteDisable)
		paging_modeswitch_4LevelPagingNX(paging_PML4Table, 0);
	else
		paging_modeswitch_4LevelPaging(paging_PML4Table, 0);
	graphics_DeviceFramebuffer = (void *)KERNEL_FRAMEBUFFER_MAPPING;
	io_WriteConsoleASCII("Virtual Memory mapping switched\n");

	// relocate the hardcoded symbols
	execformat_pe_PortableExecutable pe;
	execformat_pe_ReadSystemHeader(&pe);
	execformat_pe_BaseRelocate(&pe, (void *)link_RelocStart, (void *)link_RelocEnd, paging_LoaderCodeAddress, KERNEL_CODE_VIRTUAL);

	io_WriteConsoleASCII("Relocation OK\n");

	// find the symbol kMain_Init
	//uint64_t target_kmain = KERNEL_CODE_VIRTUAL + ((uint64_t)kMain_Init - paging_LoaderCodeAddress);
	// call it, once and for all
	//((kMainType)target_kmain)();
	kMain_Init();
	__builtin_unreachable(); // execution cannot reach here
}
