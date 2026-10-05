_estack = 0x20100000;

MEMORY
{
	FLASH (rx)  : ORIGIN = 0x01081000, LENGTH = 0xF00000
	SRAM (rwx)  : ORIGIN = 0x20000000, LENGTH = 1024K
}

__FLASH_segment_start__ = 0x01000000;
__FLASH_segment_end__ = 0x02000000;
__SRAM_segment_start__ = ORIGIN(SRAM);
__SRAM_segment_end__ = ORIGIN(SRAM) + LENGTH(SRAM);

__privileged_functions_start__ = 0x01080000;
__privileged_functions_end__ = 0x010A0000;
__privileged_data_start__ = ORIGIN(SRAM);
__privileged_data_end__ = ORIGIN(SRAM) + 0x00002000;

SECTIONS
{
	.interrupts 0x01081000 :
	{
		. = ALIGN(4);
		KEEP(*(.isr_vector))
	} > FLASH

	.privileged_functions 0x01082000 :
	{
		KEEP(*(.fixSection))
		*(privileged_functions)
		. = __privileged_functions_end__ - 0x01082000;
	} > FLASH

	.freertos_system_calls __privileged_functions_end__ :
	{
		__syscalls_flash_start__ = .;
		KEEP(*(freertos_system_calls))
		__syscalls_flash_used_end__ = .;
		. = __syscalls_flash_start__ + 0x800;
		__syscalls_flash_end__ = .;
	} > FLASH
	ASSERT(__syscalls_flash_used_end__ <= __syscalls_flash_end__, "FreeRTOS system-call veneers exceed their MPU region")

	.sandbox_text 0x010A4000 :
	{
		__sandbox_text_start = .;
		KEEP(*(.sandbox_parser_entry))
		*sandbox_parser*.rcgu.o(.text .text.* .rodata .rodata.*)
		*mpu_sandbox_runtime.c.*(.text .text.* .rodata .rodata.*)
		*eapdu_framing.c.*(.text .text.* .rodata .rodata.*)
		__sandbox_text_used_end = .;
		. = __sandbox_text_start + 0x4000;
		__sandbox_text_end = .;
	} > FLASH
	ASSERT(__sandbox_text_used_end <= __sandbox_text_end, "Sandbox code exceeds its MPU region")

	.text 0x010A8000 :
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
	.privileged_data : AT(_sidata)
	{
		_sdata = .;
		*(privileged_data)
		. = __privileged_data_end__ - __privileged_data_start__;
	} > SRAM
	ASSERT(ADDR(.privileged_data) == __privileged_data_start__, "FreeRTOS privileged data must start at the MPU region base")
	ASSERT((ADDR(.privileged_data) + SIZEOF(.privileged_data)) == __privileged_data_end__, "FreeRTOS privileged data must fill the MPU region")

	.sandbox_writable_guard (NOLOAD) :
	{
		__sandbox_writable_start = .;
		*sandbox_parser*.rcgu.o(.data .data.* .bss .bss.* COMMON)
		*mpu_sandbox_runtime.c.*(.data .data.* .bss .bss.* COMMON)
		*eapdu_framing.c.*(.data .data.* .bss .bss.* COMMON)
		__sandbox_writable_end = .;
	} > SRAM
	ASSERT(__sandbox_writable_start == __sandbox_writable_end, "Sandbox objects must not define writable globals")

	.data : AT(_sidata + SIZEOF(.privileged_data))
	{
		*(.data)
		*(.data.*)
		*(.gnu.linkonce.d.*)
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

	.freertos_heap (NOLOAD) :
	{
		. = ALIGN(8);
		*(.freertos_heap)
		. = ALIGN(8);
	} > SRAM

	.mpu_sandbox_rw (NOLOAD) :
	{
		. = ALIGN(16384);
		__sandbox_rw_start = .;
		*(.mpu_sandbox_rw)
		__sandbox_rw_used_end = .;
		. = __sandbox_rw_start + 0x4000;
		__sandbox_rw_end = .;
	} > SRAM
	ASSERT(__sandbox_rw_used_end <= __sandbox_rw_end, "Sandbox writable state exceeds its MPU region")

	.mpu_sandbox_stack (NOLOAD) :
	{
		. = ALIGN(4096);
		*(.mpu_sandbox_stack)
	} > SRAM

	.mpu_sandbox_input (NOLOAD) :
	{
		. = ALIGN(256);
		*(.mpu_sandbox_input)
	} > SRAM

	.bss :
	{
		_sbss = .;
		*(.bss)
		*(.bss.*)
		*(.gnu.linkonce.b.*)
		*(COMMON)
		_ebss = .;
	} > SRAM


	/DISCARD/ :
	{
		libc.a (*)
		libm.a (*)
		libgcc.a (*)
	}
}

end = .;
