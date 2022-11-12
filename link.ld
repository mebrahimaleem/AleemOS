ENTRY(_start)

OUTPUT_FORMAT(binary)

STARTUP(build/kentry.elf)

INPUT
(
	build/memory.elf
	build/portio.elf
	build/basicio.elf
	build/utils.elf
	build/kernel.elf
)

OUTPUT(build/kernel.bin)

SECTIONS {
	. = 0xE000;
	.text : {
		*(.text)
	}
	.data : {
		*(.data)
		*(.rodata)
	}
	.bss : {
		*(.bss)
	}
}


