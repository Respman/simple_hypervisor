OUTPUT_FORMAT("binary")
OUTPUT_ARCH(i386:x86-64)
ENTRY(main);
SECTIONS
{
	.text : { *(.text) *(.rodata) }
	.data : { *(.data) }
}
