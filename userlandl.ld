ENTRY(_start)

OUTPUT_FORMAT(elf32-i386)

INPUT
(
	build/stdc.elf
)

SECTIONS {
	.text : {
		*(.text)
		*(.rodata)
		*(.rodata1)
	}
	.data : {
		*(.data)
		*(.data1)
	}
	.bss : {
		*(.bss)
	}
}
