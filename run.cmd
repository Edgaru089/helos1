
qemu-system-x86_64 -no-reboot -no-shutdown -d cpu_reset -bios ../OVMF.fd -drive file=fat:rw:../FAT,format=raw,media=disk -m 2G -serial stdio


