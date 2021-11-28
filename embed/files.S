format elf64

public embed_Xcursor_Default_Data
public embed_Xcursor_Default_Data_End
public embed_Initrd
public embed_Initrd_End

section '.rodata'

embed_Xcursor_Default_Data:
	file "xcursor_Default"
embed_Xcursor_Default_Data_End:

embed_Initrd:
	file "initrd.tar"
embed_Initrd_End:

