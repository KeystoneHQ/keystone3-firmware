_estack = 0x20100000;

MEMORY
{
	FLASH (rx)  : ORIGIN = 0x01081000, LENGTH = 0xF00000
	SRAM (rwx)  : ORIGIN = 0x20000000, LENGTH = 1024K
}

SECTIONS
{
	.interrupts 0x01081000 :
	{
		. = ALIGN(4);
		KEEP(*(.isr_vector))
	} > FLASH

	.fixSegment 0x01082000 : {KEEP(*(.fixSection))} > FLASH

	.text 0x01083000 :
	{
		. = 0x0;
		_stext = .;
		*(.text*)
		*(.rodata*)
		_etext = .;
	} > FLASH

	.ARM.exidx :
	{
		__exidx_start = .;
		*(.ARM.exidx*)
		*(.gnu.linkonce.armexidx.*)
		__exidx_end = .;
	} > FLASH

	_sidata = .;
	.data : AT(_sidata)
	{
		_sdata = .;
		*(.data*)
		_edata = .;
	} > SRAM

    .stack : 
    {
        . = ALIGN(4);
        _sstack = .;
        . = . + 0x2000;
        . = ALIGN(4);
        _estack = .;
    } > SRAM

	.bss :
	{
		_sbss = .;
		*(.bss*)
		*(COMMON)
		_ebss = .;
	} > SRAM

.data_parser_section 0x200A0000 :
{
    . = ALIGN(4);
    __data_parser_start = .;
    *(.data_parser_section)
    . = ALIGN(4);
    __data_parser_end = .;
} > SRAM
	
	/DISCARD/ :
	{
		libc.a (*)
		libm.a (*)
		libgcc.a (*)
	}
}

end = .;
