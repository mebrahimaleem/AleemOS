ENTRY(_start)

OUTPUT_FORMAT(binary)

STARTUP(build/mine.elf)

INPUT
(
	build/min.elf
)

OUTPUT(build/min.bin)

SECTIONS {
	. = 0xE000;
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
