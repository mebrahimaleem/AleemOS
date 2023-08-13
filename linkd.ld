ENTRY(_start)

OUTPUT_FORMAT(elf32-i386)

STARTUP(build/kentry.elf)

INPUT
(
	build/memory.elf
	build/portio.elf
	build/basicio.elf
	build/utils.elf
	build/signals.elf
	build/kbd.elf
	build/ELFparse.elf
	build/process.elf
	build/kernel.elf
)

OUTPUT(build/kerneld.elf)

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
