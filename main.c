
#include "main.h"
#include "efimain.h"

#include <string.h>
#include "runtime/panic_assert.h"
#include "runtime/stdio.h"
#include "graphics/graphics.h"
#include "graphics/unifont.h"
#include "memory/memory.h"
#include "driver/irq/pic/serial/serial.h"


FASTCALL_ABI EFI_STATUS efiMain(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
	SystemTable->ConOut->EnableCursor(SystemTable->ConOut, TRUE);

	efiImageHandle  = ImageHandle;
	efiSystemTable  = SystemTable;
	efiBootServices = SystemTable->BootServices;
	efiStdin        = SystemTable->ConIn;
	efiStdout       = SystemTable->ConOut;
	efiStderr       = SystemTable->StdErr;

	// disable the watchdog timer so that the application does not reset after 5mins
	efiBootServices->SetWatchdogTimer(0, 0, 0, NULL);

	pic_serial_Init(&pic_serial_COM1, 115200, 0);

	io_WriteConsole(" Welcome to " PROJECT_NAME " !\r\n\r\n");

	graphics_Init();

	graphics_ClearBuffer(&HelosGraphics_Color_Black);
	graphics_FillPixel(0, 0, 20, 20, &HelosGraphics_Color_Black);
	graphics_FillPixel(20, 0, 40, 20, &HelosGraphics_Color_White);
	graphics_FillPixel(40, 0, 60, 20, &HelosGraphics_Color_Red);
	graphics_FillPixel(60, 0, 80, 20, &HelosGraphics_Color_Green);
	graphics_FillPixel(80, 0, 100, 20, &HelosGraphics_Color_Blue);
	graphics_FillPixel(100, 0, 120, 20, &HelosGraphics_Color_Cyan);
	graphics_FillPixel(120, 0, 140, 20, &HelosGraphics_Color_Magenta);
	graphics_FillPixel(140, 0, 160, 20, &HelosGraphics_Color_Yellow);

	uint32_t helloString[] = {0x89C6, 0x7A97, 0x7CFB, 0x7EDF, 0x4E09, ' ', 'H', 'e', 'l', 'l', 'o', ',', ' ', 'a', 'n', 'd', ',', ' ', 'n', 'i', 'c', 'e', ' ', 't', 'o', ' ', 'm', 'e', 'e', 't', ' ', 'y', 'o', 'u', '!'};
	unifont_DrawString(0, 20, &HelosGraphics_Color_White, helloString, sizeof(helloString) / sizeof(uint32_t));
	graphics_CursorX = 0;
	graphics_CursorY = 52;
	graphics_Invalidate(0, 0, graphics_SystemVideoMode.Width, 52);
	graphics_SwapBuffer();

	runtime_InitPaging();


	return EFI_SUCCESS;
}

EFI_HANDLE         efiImageHandle;
EFI_SYSTEM_TABLE  *efiSystemTable;
EFI_BOOT_SERVICES *efiBootServices;

SIMPLE_TEXT_OUTPUT_INTERFACE *efiStdout, *efiStderr;
SIMPLE_INPUT_INTERFACE       *efiStdin;

char Buffer[HELOS_BUFFER_SIZE];
