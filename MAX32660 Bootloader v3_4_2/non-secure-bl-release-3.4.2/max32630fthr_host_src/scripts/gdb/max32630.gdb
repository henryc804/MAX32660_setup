define connect
	set trace-commands on
	set remotetimeout 1000000000
	set logging on
	target remote localhost:3333
end

define reset
	monitor reset halt
	c
end

define settings
	# set unlimited string size while print
	set print elements 0
	set print pretty on
end

define program
	file ./BUILD/MAX32630FTHR/GCC_ARM/mbed-os.elf
	monitor reset halt
	load ./BUILD/MAX32630FTHR/GCC_ARM/mbed-os.elf
	monitor reset halt
end

define run_openocd
	target remote | openocd scripts/jtag/max3263x_hdk.cfg -c "gdb_port pipe; log_output ./openocd.log"
end

define program_and_run
	connect
	settings
	program
	c
end

define configure
	connect
	settings
	c
end

#program_and_run

