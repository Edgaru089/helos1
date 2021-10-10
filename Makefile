
.SILENT:
include Makefile.flags


objects      = $(patsubst %.c,%.o,$(shell find . -name "*.c")) $(patsubst %.cpp,%.o,$(shell find . -name "*.cpp"))
objects_fasm = $(patsubst %.S,%.o,$(shell find . -name "*.S"))

objects_test = $(patsubst %.c,%.o,$(shell find . -name "test_*.c")) $(patsubst %.cpp,%.o,$(shell find . -name "test_*.cpp"))
objects     := $(filter-out $(objects_test),$(objects))


all: Main.efi


Main.efi: $(objects) $(objects_fasm)
	$(LD) $(LDFLAGS) $(objects) $(objects_fasm) $(LDLIBS)


clean:
	echo " -RM    Main.efi $(objects)"
	-$(RM) -f Main.efi $(objects) $(objects_fasm)

install: Main.efi
	echo "  CP    Main.efi ../FAT/EFI/Boot/bootx64.efi"
	$(CP) Main.efi ../FAT/EFI/Boot/bootx64.efi

upload: Main.efi
	echo "  SCP   Main.efi router.edgaru089.ml:/opt/tftp/"
	scp -P 29657 Main.efi root@router.edgaru089.ml:/opt/tftp/

.PHONY: all clean install upload


