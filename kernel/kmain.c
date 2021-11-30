
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
#include "../driver/input/input.h"
#include "../smp/kthread.h"
#include "../smp/condiction.h"
#include "../driver/irq/pic/serial/serial.h"
#include "../driver/filesystem/filesystem_init.h"

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

static SYSV_ABI void kThreader(int a, int b, int c, int d, int e, int f) {
	io_Printf("kThreader: args[%d,%d,%d,%d,%d,%d]\n", a, b, c, d, e, f);
	for (int i = 0; i < 10; i++) {
		io_Printf("kThreader: Not sleeping!\n");
		smp_thread_Sleep(768);
	}
	io_Printf("kThreader: byebye!\n");
}

static SYSV_ABI void kThreader_SerialReader() {
	for (;;) {
		smp_Condition_Wait(pic_serial_COM1.cond);
		while (!queue_Empty(&pic_serial_COM1.buffer)) {
			uint8_t b = queue_PopByte(&pic_serial_COM1.buffer);
			io_Printf("Serial char: (%d)%c\n", b, b);
		}
	}
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
	pic_ps2_Init();
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

	filesystem_Init();
	io_WriteConsoleASCII("kMain: Filesystem OK\n");
	filesystem_Ls("/dev");
	filesystem_Ls("/");

	io_WriteConsoleASCII("kMain: Initializing threading\n");
	smp_thread_ID tid = smp_thread_Init();
	io_Printf("kMain: Threading OK, id=%d\n", tid);

	smp_thread_Arguments args = {1, 2, 3, 4, 5, 6};

	tid = smp_thread_Start(kThreader, &args, SMP_NICENESS_DEFAULT);
	io_Printf("New thread, id=%d\n", tid);

	input_Condition  = smp_Condition_Create();
	input_EventQueue = kMalloc(sizeof(queue_Queue));
	queue_InitBuffered(input_EventQueue, kMalloc(sizeof(input_Event) * 32), sizeof(input_Event) * 32);

	if (pic_serial_InitInput(&pic_serial_COM1)) {
		io_WriteConsoleASCII("kMain: Serial Input OK\n");
		smp_thread_Start(kThreader_SerialReader, NULL, SMP_NICENESS_DEFAULT);
	}

	io_ErrorASCII("kMain: Initialization done\n");

	for (;;) {
		smp_Condition_Wait(input_Condition);
		graphics_SwapBuffer();

		input_Event e;
		INTERRUPT_DISABLE;
		while (queue_Size(input_EventQueue) > 0) {
			queue_Pop(input_EventQueue, &e, sizeof(e));
			INTERRUPT_RESTORE;

			if (e.Type == input_EventType_KeyPressed) {
				io_Printf("kMain: KeyPressed: %s\n", input_Key_GetName(e.Key.Key));
			} else if (e.Type == input_EventType_KeyReleased) {
				io_Printf("kMain: KeyReleased: %s\n", input_Key_GetName(e.Key.Key));
			} else if (e.Type == input_EventType_MouseButtonPressed) {
				io_Printf("kMain: MouseButtonPressed: %d\n", (int)e.MouseButton.Button);
			} else if (e.Type == input_EventType_MouseButtonReleased) {
				io_Printf("kMain: MouseButtonReleased: %d\n", (int)e.MouseButton.Button);
			}
			INTERRUPT_DISABLE;
		}
		INTERRUPT_RESTORE;
	}
}
