ENTRY(_start)

OUTPUT_FORMAT(elf32-i386)

STARTUP(build/mine.elf)

INPUT
(
	build/min.elf
)

OUTPUT(build/mind.elf)

SECTIONS {
	. = 0xD000;
	.text : {
		*(.text)
		*(.rodata)
	}
	.data : {
		*(.data)
	}
	.bss : {
		*(.bss)
	}
}
