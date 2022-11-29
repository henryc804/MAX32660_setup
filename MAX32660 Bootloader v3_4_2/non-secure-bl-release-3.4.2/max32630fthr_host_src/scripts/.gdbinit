set trace-commands on
set logging on
set logging file gdb.txt
target remote localhost:3333
file ./build/max3263x.elf
monitor reset halt
load
monitor reset halt
b main
info breakpoints
c

