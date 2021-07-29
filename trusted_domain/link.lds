OUTPUT_ARCH( "riscv" )
ENTRY( _start )

__stack_size = 0x4000;

MEMORY
{
	/* Fake ROM area */
	rom (rxa) : ORIGIN = 0xBF800000, LENGTH = 1M
	ram (wxa) : ORIGIN = 0xBF900000, LENGTH = 6M
}

SECTIONS
{
    __stack_size = DEFINED(__stack_size) ? __stack_size : 2K;

    .init :
    {
        KEEP (*(SORT_NONE(.init)))
    } >rom AT>rom 

    .ilalign :
    {
        . = ALIGN(8);
        PROVIDE( _ilm_lma = . );
    } >rom AT>rom 

    .ialign :
    {
        PROVIDE( _ilm = . );
    } >rom AT>rom 

    .text :
    {
        *(.rodata .rodata.*)  
        *(.text.unlikely .text.unlikely.*)
        *(.text.startup .text.startup.*)
        *(.text .text.*)
        *(.gnu.linkonce.t.*)
    } >rom AT>rom 

    .fini :
    {
        KEEP (*(SORT_NONE(.fini)))
    } >rom AT>rom 

    . = ALIGN(8);

    PROVIDE (__etext = .);
    PROVIDE (_etext = .);
    PROVIDE (etext = .);
    PROVIDE( _eilm = . );

    .preinit_array :
    {
        PROVIDE_HIDDEN (__preinit_array_start = .);
        KEEP (*(.preinit_array))
        PROVIDE_HIDDEN (__preinit_array_end = .);
    } >rom AT>rom 

    .init_array :
    {
        PROVIDE_HIDDEN (__init_array_start = .);
        KEEP (*(SORT_BY_INIT_PRIORITY(.init_array.*) SORT_BY_INIT_PRIORITY(.ctors.*)))
        KEEP (*(.init_array EXCLUDE_FILE (*crtbegin.o *crtbegin?.o *crtend.o *crtend?.o ) .ctors))
        PROVIDE_HIDDEN (__init_array_end = .);
    } >rom AT>rom 

    .fini_array :
    {
        PROVIDE_HIDDEN (__fini_array_start = .);
        KEEP (*(SORT_BY_INIT_PRIORITY(.fini_array.*) SORT_BY_INIT_PRIORITY(.dtors.*)))
        KEEP (*(.fini_array EXCLUDE_FILE (*crtbegin.o *crtbegin?.o *crtend.o *crtend?.o ) .dtors))
        PROVIDE_HIDDEN (__fini_array_end = .);
    } >rom AT>rom 

    .ctors :
    {
        /* gcc uses crtbegin.o to find the start of
        the constructors, so we make sure it is
        first.  Because this is a wildcard, it
        doesn't matter if the user does not
        actually link against crtbegin.o; the
        linker won't look for a file to match a
        wildcard.  The wildcard also means that it
        doesn't matter which directory crtbegin.o
        is in.  */
        KEEP (*crtbegin.o(.ctors))
        KEEP (*crtbegin?.o(.ctors))
        /* We don't want to include the .ctor section from
        the crtend.o file until after the sorted ctors.
        The .ctor section from the crtend file contains the
        end of ctors marker and it must be last */
        KEEP (*(EXCLUDE_FILE (*crtend.o *crtend?.o ) .ctors))
        KEEP (*(SORT(.ctors.*)))
        KEEP (*(.ctors))
    } >rom AT>rom 

    .dtors :
    {
        KEEP (*crtbegin.o(.dtors))
        KEEP (*crtbegin?.o(.dtors))
        KEEP (*(EXCLUDE_FILE (*crtend.o *crtend?.o ) .dtors))
        KEEP (*(SORT(.dtors.*)))
        KEEP (*(.dtors))
    } >rom AT>rom 

    . = ALIGN(8);
    PROVIDE( _eilm = . );

    .lalign :
    {
        . = ALIGN(8);
        PROVIDE( _data_lma = . );
    } >rom AT>rom 

    .dalign :
    {
        . = ALIGN(8);
        PROVIDE( _data = . );
    } >ram AT>rom 
    
    .data :
    {
        *(.rdata) 
    
        *(.gnu.linkonce.r.*)
        *(.data .data.*)
        *(.gnu.linkonce.d.*)
        . = ALIGN(8);
        PROVIDE( __global_pointer$ = . + 0x800); 
        *(.sdata .sdata.*)
        *(.gnu.linkonce.s.*)
        . = ALIGN(8);
        *(.srodata.cst16)
        *(.srodata.cst8)
        *(.srodata.cst4)
        *(.srodata.cst2)
        *(.srodata .srodata.*)
        . = ALIGN(8);
    } >ram AT>rom 

    . = ALIGN(8);
    PROVIDE( _edata = . );
    PROVIDE( edata = . );

    PROVIDE( _fbss = . );
    PROVIDE( __bss_start = . );

    .bss :
    {
        *(.sbss*)
        *(.gnu.linkonce.sb.*)
        *(.bss .bss.*)
        *(.gnu.linkonce.b.*)
        *(COMMON)
        . = ALIGN(8);
        PROVIDE( __bss_end = . );
    } >ram AT>ram 

    . = ALIGN(8);
    PROVIDE( _end = . );
    PROVIDE( end = . );
    PROVIDE( heap_start = . );

    .stack ORIGIN(ram) + LENGTH(ram) - __stack_size :
    {
        PROVIDE( heap_end = . ); 
        . = ALIGN(16);
        . = __stack_size;  
        . = ALIGN(16);
        PROVIDE( _sp = . ); 
    } >ram AT>ram 
}