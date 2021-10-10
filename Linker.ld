/* Default linker script, for normal executables */
/* Copyright (C) 2014-2020 Free Software Foundation, Inc.
   Copying and distribution of this script, with or without modification,
   are permitted in any medium without royalty provided the copyright
   notice and this notice are preserved.  */
OUTPUT_FORMAT(pei-x86-64)
SEARCH_DIR("/usr/x86_64-w64-mingw32/lib");

__section_alignment__ = 4096;

SECTIONS
{
  /* Make the virtual address and file offset synced if the alignment is
     lower than the target page size. */
  . = SIZEOF_HEADERS;
  . = ALIGN(__section_alignment__);
  .text  __image_base__ + ( __section_alignment__ < 0x1000 ? . : __section_alignment__ ) :
  {
	PROVIDE(link_TextStart = .);
    KEEP (*(SORT_NONE(.init)))
    *(.text)
    *(SORT(.text$*))
     *(.text.*)
     *(.gnu.linkonce.t.*)
    *(.glue_7t)
    *(.glue_7)
    . = ALIGN(8);
       /* Note: we always define __CTOR_LIST__ and ___CTOR_LIST__ here,
          we do not PROVIDE them.  This is because the ctors.o startup
	  code in libgcc defines them as common symbols, with the
          expectation that they will be overridden by the definitions
	  here.  If we PROVIDE the symbols then they will not be
	  overridden and global constructors will not be run.
	  See PR 22762 for more details.

	  This does mean that it is not possible for a user to define
	  their own __CTOR_LIST__ and __DTOR_LIST__ symbols; if they do,
	  the content from those variables are included but the symbols
	  defined here silently take precedence.  If they truly need to
	  be redefined, a custom linker script will have to be used.
	  (The custom script can just be a copy of this script with the
	  PROVIDE() qualifiers added).
	  In particular this means that ld -Ur does not work, because
	  the proper __CTOR_LIST__ set by ld -Ur is overridden by a
	  bogus __CTOR_LIST__ set by the final link.  See PR 46.  */
       ___CTOR_LIST__ = .;
       __CTOR_LIST__ = .;
       LONG (-1); LONG (-1);
       KEEP (*(.ctors));
       KEEP (*(.ctor));
       KEEP (*(SORT_BY_NAME(.ctors.*)));
       LONG (0); LONG (0);
       /* See comment about __CTOR_LIST__ above.  The same reasoning
    	  applies here too.  */
       ___DTOR_LIST__ = .;
       __DTOR_LIST__ = .;
       LONG (-1); LONG (-1);
       KEEP (*(.dtors));
       KEEP (*(.dtor));
       KEEP (*(SORT_BY_NAME(.dtors.*)));
       LONG (0); LONG (0);
    KEEP (*(SORT_NONE(.fini)))
    /* ??? Why is .gcc_exc here?  */
     *(.gcc_exc)
    PROVIDE (etext = .);
	PROVIDE(link_TextEnd = .);
     KEEP (*(.gcc_except_table))
  }
  /* The Cygwin32 library uses a section to avoid copying certain data
     on fork.  This used to be named ".data".  The linker used
     to include this between __data_start__ and __data_end__, but that
     breaks building the cygwin32 dll.  Instead, we name the section
     ".data_cygwin_nocopy" and explicitly include it after __data_end__. */
  .data BLOCK(__section_alignment__) :
  {
	PROVIDE(link_DataStart = .);
    __data_start__ = . ;
    *(.data)
    *(.data2)
    *(SORT(.data$*))
    KEEP(*(.jcr))
    __data_end__ = . ;
    *(.data_cygwin_nocopy)
  }
  .idata BLOCK(__section_alignment__) :
  {
    /* This cannot currently be handled with grouped sections.
	See pep.em:sort_sections.  */
    KEEP (SORT(*)(.idata$2))
    KEEP (SORT(*)(.idata$3))
    /* These zeroes mark the end of the import list.  */
    LONG (0); LONG (0); LONG (0); LONG (0); LONG (0);
    KEEP (SORT(*)(.idata$4))
    __IAT_start__ = .;
    SORT(*)(.idata$5)
    __IAT_end__ = .;
    KEEP (SORT(*)(.idata$6))
    KEEP (SORT(*)(.idata$7))
	PROVIDE(link_DataEnd = .);
  }
  .rdata BLOCK(__section_alignment__) :
  {
	PROVIDE(link_RodataStart = .);
    *(.rdata)
	     *(SORT(.rdata$*))
	*(.rodata)
    . = ALIGN(4);
    __rt_psrelocs_start = .;
    KEEP(*(.rdata_runtime_pseudo_reloc))
    __rt_psrelocs_end = .;
	PROVIDE(link_RodataEnd = .);
  }
  __rt_psrelocs_size = __rt_psrelocs_end - __rt_psrelocs_start;
  ___RUNTIME_PSEUDO_RELOC_LIST_END__ = .;
  __RUNTIME_PSEUDO_RELOC_LIST_END__ = .;
  ___RUNTIME_PSEUDO_RELOC_LIST__ = . - __rt_psrelocs_size;
  __RUNTIME_PSEUDO_RELOC_LIST__ = . - __rt_psrelocs_size;
  .eh_frame BLOCK(__section_alignment__) :
  {
    KEEP (*(.eh_frame*))
  }
  .pdata BLOCK(__section_alignment__) :
  {
    KEEP(*(.pdata*))
  }
  .xdata BLOCK(__section_alignment__) :
  {
    KEEP(*(.xdata*))
  }
  .edata BLOCK(__section_alignment__) :
  {
    *(.edata)
  }
  .bss BLOCK(__section_alignment__) :
  {
	PROVIDE(link_BssStart = .);
    __bss_start__ = . ;
    *(.bss)
    *(COMMON)
    __bss_end__ = . ;
	PROVIDE(link_BssEnd = .);
  }
  /DISCARD/ :
  {
    *(.debug$S)
    *(.debug$T)
    *(.debug$F)
    *(.drectve)
     *(.note.GNU-stack)
     *(.gnu.lto_*)
  }
  .CRT BLOCK(__section_alignment__) :
  {
    ___crt_xc_start__ = . ;
    KEEP (*(SORT(.CRT$XC*)))  /* C initialization */
    ___crt_xc_end__ = . ;
    ___crt_xi_start__ = . ;
    KEEP (*(SORT(.CRT$XI*)))  /* C++ initialization */
    ___crt_xi_end__ = . ;
    ___crt_xl_start__ = . ;
    KEEP (*(SORT(.CRT$XL*)))  /* TLS callbacks */
    /* ___crt_xl_end__ is defined in the TLS Directory support code */
    ___crt_xp_start__ = . ;
    KEEP (*(SORT(.CRT$XP*)))  /* Pre-termination */
    ___crt_xp_end__ = . ;
    ___crt_xt_start__ = . ;
    KEEP (*(SORT(.CRT$XT*)))  /* Termination */
    ___crt_xt_end__ = . ;
  }
  /* Windows TLS expects .tls$AAA to be at the start and .tls$ZZZ to be
     at the end of the .tls section.  This is important because _tls_start MUST
     be at the beginning of the section to enable SECREL32 relocations with TLS
     data.  */
  .tls BLOCK(__section_alignment__) :
  {
    ___tls_start__ = . ;
    KEEP (*(.tls$AAA))
    KEEP (*(.tls))
    KEEP (*(.tls$))
    KEEP (*(SORT(.tls$*)))
    KEEP (*(.tls$ZZZ))
    ___tls_end__ = . ;
  }
  .endjunk BLOCK(__section_alignment__) :
  {
    /* end is deprecated, don't use it */
    PROVIDE (end = .);
    PROVIDE ( _end = .);
     __end__ = .;
  }
  .reloc BLOCK(__section_alignment__) :
  {
	PROVIDE(link_RelocStart = .);
    *(.reloc)
	PROVIDE(link_RelocEnd = .);
  }
}


