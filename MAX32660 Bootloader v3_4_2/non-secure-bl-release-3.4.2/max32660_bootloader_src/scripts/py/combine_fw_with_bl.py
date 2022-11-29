#!/usr/bin/env python

import os
import sys
import zlib
import struct
import argparse
import subprocess

from colorama import Fore, Back, Style, init

VERSION = "0.01"

def uint32_to_bytearray(target_number):
    buffer = []
    buffer[0:4] = struct.pack("<I", target_number)
    return bytearray(buffer)

VALID_MARK = uint32_to_bytearray(0x4D41524B)

def exec_piped_cmd_result(first_cmd_arg_list, sec_cmd_arg_list):
    fst_process = subprocess.Popen(first_cmd_arg_list, stdout=subprocess.PIPE)
    sec_process = subprocess.Popen(sec_cmd_arg_list, stdin=fst_process.stdout, stdout=subprocess.PIPE)
    fst_process.stdout.close()
    return sec_process.communicate()[0].rstrip()

def get_addr(address, linker_file, linker_flags):
    linker_cmd = ['arm-none-eabi-ld']
    if linker_flags:
        linker_cmd.extend(linker_flags.split())
    linker_cmd.extend(['-M', linker_file])
    return exec_piped_cmd_result(linker_cmd, ['awk', '$2 == "' + address + '" { print $1 }'])

def combine_firmware(bootloader, application, linker_file, linker_flags, output):
    if not (os.path.exists(bootloader) and os.path.exists(application)
        and os.path.exists(linker_file)):
        sys.stderr.write('Could not find all/some of the input files')
        sys.exit(1)
    bl_start = get_addr('_bl_start', linker_file, linker_flags)
    bl_len = get_addr('_bl_len', linker_file, linker_flags)
    bl_end = hex(int(bl_start, 16) + int(bl_len, 16))
    app_start = get_addr('_app_start', linker_file, linker_flags)
    app_len = get_addr('_app_len', linker_file, linker_flags)
    app_end = get_addr('_app_end', linker_file, linker_flags)
    boot_mem_start = get_addr('_boot_mem_start', linker_file, linker_flags)
    boot_mem_len = get_addr('_boot_mem_len', linker_file, linker_flags)
    boot_mem_end = hex(int(boot_mem_start, 16) + int(boot_mem_len, 16))
    print('>>> Addresses <<<')
    print('Bootloader start: ' + bl_start)
    print('Bootloader end: ' + bl_end)
    print('Bootloader length: ' + bl_len)
    print('Application start:' + app_start)
    print('Application end: ' + app_end)
    print('Application len: ' + app_len)
    print('Boot memory start: ' + boot_mem_start)
    print('Boot memory length: ' + boot_mem_len)
    print('Boot memory end: ' + boot_mem_end)
    with open(output, 'wb') as unified_bin:
        empty_byte = bytearray([0xFF])
        for file_index in range(0, int(bl_start, 16)):
            unified_bin.write(empty_byte)
        with open(bootloader, 'rb') as bl_file:
            unified_bin.write(bl_file.read())
        bl_end_index = unified_bin.tell()
        for file_index in range(bl_end_index, int(app_start, 16)):
            unified_bin.write(empty_byte)
        with open(application, 'rb') as app_file:
            unified_bin.write(app_file.read())
        app_end_index = unified_bin.tell()
        for file_index in range(app_end_index, int(boot_mem_start, 16)):
            unified_bin.write(empty_byte)
        crc_of_app = zlib.crc32(open(application,"rb").read()) & 0xFFFFFFFF
        print crc_of_app
        unified_bin.write(uint32_to_bytearray(crc_of_app))
        app_length = os.path.getsize(application)
        print app_length
        unified_bin.write(uint32_to_bytearray(app_length))
        unified_bin.write(VALID_MARK)
        unified_bin.write(uint32_to_bytearray(0))
        for i in range(0, 12):
            unified_bin.write(empty_byte)

def main():
	parser = argparse.ArgumentParser()
	parser.add_argument("-b", "--bootloader", required=True, type=str,
                    help="Bootloader binary file path")
	parser.add_argument("-a", "--application", required=True, type=str,
                    help="Application binary file path")
	parser.add_argument("-l", "--linker-file", required=True, type=str,
                    help="Linker file path")
	parser.add_argument("-o", "--output", default='bl_with_app.bin', type=str,
                    help="Output binary file path")

	parser.add_argument('--version', action='version', version='%(prog)s ' + VERSION)
	parser.add_argument("-f", "--linker-flags", type=str, help="Linker flags")
	args = parser.parse_args()

	linker_flags = args.linker_flags

	print(Fore.CYAN + '\n\nMAXIM FIRMWARE COMBINE SCRIPT ' + VERSION + '\n\n')
	print(">>> Parameters <<<")
	print("Bootloader Path: ", args.bootloader)
	print("Application Path: ", args.application)
	print("Linker Script Path:" , args.linker_file)
	print("Output Path: ", args.output)
	combine_firmware(args.bootloader, args.application, args.linker_file, args.linker_flags, args.output)

if __name__ == '__main__':
	main()
