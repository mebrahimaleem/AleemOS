ENTRY(_start)

OUTPUT_FORMAT(binary)

STARTUP(build/kentry.elf)

INPUT
(
	build/memory.elf
	build/portio.elf
	build/basicio.elf
	build/utils.elf
	build/signals.elf
	build/pci.elf
	build/kbd.elf
	build/ELFparse.elf
	build/process.elf
	build/kernel.elf
)

OUTPUT(build/kernel.bin)

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
