
#include "../main.h"
#include "kmain.h"

#include "../runtime/stdio.h"
#include "../runtime/panic_assert.h"
#include "../memory/memory.h"
#include "../memory/paging_internal.h"
#include "../interrupt/interrupt.h"
#include "../interrupt/handlers.h"
#include "../interrupt/syscall.h"
#include "../driver/irq/pic/pic.h"
#include "../driver/irq/pic/ps2/ps2.h"

#include "../execformat/pe/reloc.h"
void execformat_pe_ReadSystemHeader(execformat_pe_PortableExecutable *pe);

#include "../graphics/graphics.h"
#include "../graphics/xcursor/xcursor.h"
#include "../embed/files.h"

static void tellRIP() {
	uint64_t a, b;
	asm volatile("leaq (%%rip), %0\n\tleaq runtime_InitPaging(%%rip), %1"
				 : "=r"(a), "=r"(b));
	io_Printf("tellRIP(): Stack position: %llx, RIP=%llx, kMain_StackPosition:%llx(%llx), interrupt_Int128: %llx\n", &a, a, (uint64_t)&kMain_StackPosition, b, (uint64_t)interrupt_Int128);
}

static inline int minmax(int val, int min, int max) {
	if (val > max)
		return max;
	if (val < min)
		return min;
	return val;
}


SYSV_ABI void kMain() {
	io_WriteConsoleASCII("Yes! kMain survived!\n");

	uint64_t a;
	asm volatile("leaq (%%rip), %0"
				 : "=r"(a));
	io_Printf("Stack position: %llx, RIP=%llx, runtime_InitPaging:%llx, interrupt_Int128: %llx\n", &a, a, (uint64_t)runtime_InitPaging, (uint64_t)interrupt_Int128);

	interrupt_Init();
	io_WriteConsoleASCII("Interrupts initialized\n");

	Syscall(4, 1, 2, 3, 4, 5, 6);
	io_WriteConsoleASCII("Returning from Syscall()\n");

	tellRIP();

	irq_pic_Init();
	io_WriteConsoleASCII("PIC IRQ OK\n");
	irq_pic_ps2_Init();
	io_WriteConsoleASCII("PIC PS/2 OK\n");

	xcursor_Xcursor cursor;
	xcursor_LoadMemory(&cursor, (void *)embed_Xcursor_Default_Data, embed_Xcursor_Default_Data_End - embed_Xcursor_Default_Data);
	if (cursor.header) {
		for (int i = 0; i < cursor.n; i++) {
			if (cursor.toc[i].type == XCURSOR_CHUNKTYPE_IMAGE /*&& ((xcursor_ChunkHeader_Image *)(embed_Xcursor_Default_Data + cursor.toc[i].offset))->subtype == 24*/) {
				xcursor_ChunkHeader_Image *image = (xcursor_ChunkHeader_Image *)(embed_Xcursor_Default_Data + cursor.toc[i].offset);
				io_Printf("xcursor_Default: Loading size=%dx%d\n", image->width, image->height);
				graphics_Cursor = image;
				break;
			}
		}
	} else
		io_WriteConsoleASCII("xcursor_Default: failed to load\n");

	for (;;) {
		asm volatile("hlt");

		while (queue_Size(&irq_pic_ps2_QueueMouse) >= (irq_pic_ps2_Mouse4Bytes ? 4 : 3)) {
			unsigned int moveX, moveY, state;

			do {
				state = queue_PopByte(&irq_pic_ps2_QueueMouse);
			} while (!(state & (1u << 3)));

			unsigned int d = queue_PopByte(&irq_pic_ps2_QueueMouse);
			moveX          = d - ((state << 4) & 0x100);
			d              = queue_PopByte(&irq_pic_ps2_QueueMouse);
			moveY          = d - ((state << 3) & 0x100);

			graphics_MouseCursorX = minmax(graphics_MouseCursorX + *((int *)&moveX), 0, graphics_SystemVideoMode.Width - 1);
			graphics_MouseCursorY = minmax(graphics_MouseCursorY - *((int *)&moveY), 0, graphics_SystemVideoMode.Height - 1);

			if (irq_pic_ps2_Mouse4Bytes)
				queue_PopByte(&irq_pic_ps2_QueueMouse);
		}

		//io_WriteConsoleASCII("kMain: Interrupt hit\n");
		graphics_SwapBuffer();
	}
}
