/*
MIT License

Copyright 2022-2024 Ebrahim Aleem

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
*/

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
