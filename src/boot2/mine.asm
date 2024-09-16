[global _start]
_start:

pusha

[extern min]
call min

popa
jmp 0xD600
