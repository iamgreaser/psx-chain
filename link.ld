/*OUTPUT_FORMAT(binary);*/

SECTIONS
{
	. = 0x80010800;
	_LOADABLE_START_WHEAD = .;
	.main : {
		*(.boot)
		. = ALIGN(8);
		*(.rodata.cst8)
		*(.rodata.cst4)
		*(.text*)
		*(.rodata)
		*(.rodata.str1.4)
		*(.data*)
		_gp = ALIGN(16) + 0x7ff0;
		*(.scommon*)
		*(.sdata*)
		. = (. + 0x7FF) & ~ 0x7FF;
	}
	_LOADABLE_END = .;
	_LOADABLE_SIZE = _LOADABLE_END - (_LOADABLE_START_WHEAD + 0x800);
	_BSS_START = .;
	.bss : {
		*(.sbss*)
		*(.bss)
	}
	. = (. + 0x7FF) & ~ 0x7FF;
	_BSS_END = .;
	_end = .; PROVIDE (end = .);
}

