ENTRY(_start)

OUTPUT_FORMAT(elf32-i386)

INPUT
(
	build/stdc/crt0.o
	build/stdc/stdlib.o
	build/stdc/stdio.o
	build/stdc/string.o
	build/stdc/AleemOS.o
)

OUTPUT(build/stdc.elf)

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
