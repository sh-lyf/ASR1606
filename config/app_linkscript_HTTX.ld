MEMORY
{
    ram(RW) : ORIGIN = 0x7e070000, LENGTH = 0x40000
}


SECTIONS
{
	. = 0x7e070000;

	.userSpaceRegTable :
	{
		;PROVIDE_HIDDEN (__userSpaceRegTable_start__ = .);
		KEEP(*(.userSpaceRegTable))
		;PROVIDE_HIDDEN (__userSpaceRegTable_end__ = .);
	} >ram
	.text :
	{
	    *(.text)
	} >ram
	.rodata :
	{
	    *(.rodata)
	} >ram
	.data :
	{
		PROVIDE_HIDDEN (__data_load = LOADADDR(.data));
		PROVIDE_HIDDEN (__data_start = .);
		*(.data)
		PROVIDE_HIDDEN (_edata = .);
	} >ram
	.ARM.exidx :
	{
		PROVIDE_HIDDEN (__exidx_start = .);
		*(.ARM.exidx);
		PROVIDE_HIDDEN (__exidx_end = .);
	} >ram
	.bss :
	{
		PROVIDE_HIDDEN (__bss_start__ = .);
		*(.bss)
		PROVIDE_HIDDEN (__bss_end__ = .);
	} >ram

	end = 0x7E0B0000;
}
