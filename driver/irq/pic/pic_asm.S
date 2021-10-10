format elf64

include 'pic_constants.incS'

public irq_pic_Disable

irq_pic_Disable:
	mov al, 0xff
	out PIC1_DATA, al
	out PIC2_DATA, al


