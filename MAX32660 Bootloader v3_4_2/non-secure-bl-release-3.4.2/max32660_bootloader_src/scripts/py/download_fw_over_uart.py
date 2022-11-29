#!/usr/bin/python

################################################################################
# Copyright (C) 2018 Maxim Integrated Products, Inc., All Rights Reserved.
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
# IN NO EVENT SHALL MAXIM INTEGRATED BE LIABLE FOR ANY CLAIM, DAMAGES
# OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
# ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.
#
# Except as contained in this notice, the name of Maxim Integrated
# Products, Inc. shall not be used except as stated in the Maxim Integrated
# Products, Inc. Branding Policy.
#
# The mere transfer of this software does not imply any licenses
# of trade secrets, proprietary technology, copyrights, patents,
# trademarks, maskwork rights, or any other form of intellectual
# property whatsoever. Maxim Integrated Products, Inc. retains all
# ownership rights.
#
###############################################################################

# Please note that this script is only compatible with python 2.7

import os
import sys
import serial
import threading
import logging
import signal
import time
import argparse
import atexit
import re
import struct
import ctypes
import zlib
from copy import deepcopy
from ctypes import *
from threading import Timer, Thread, Event
from datetime import datetime

logging.basicConfig(level=logging.DEBUG,
					format='(%(threadName)-10s) %(message)s',
					)

platform_types = { 1: 'MAX32660' }

class MsblHeader(Structure):
	_fields_ = [('magic', 4 * c_char),
				('formatVersion', c_uint),
				('target', 16 * c_char),
				('enc_type', 16 * c_char),
				('nonce', 11 * c_ubyte),
				('resv0', c_ubyte),
				('auth', 16 * c_ubyte),
				('numPages', c_ushort),
				('pageSize', c_ushort),
				('crcSize', c_ubyte),
				('resv1', 3 * c_ubyte)]

class AppHeader(Structure):
	_fields_ = [('crc32', c_uint),
				('length', c_uint),
				('validMark', c_uint),
				('boot_mode', c_uint)]

class Page(Structure):
	_fields_ = [('data', (8192 + 16) * c_ubyte)]

class CRC32(Structure):
	_fields_ = [('val', c_uint)]

class Object(object):
    pass

bl_exit_mode = { 0 : 'Jump immediately',
				1 : 'Wait for programmable delay',
				2 : 'remain in bootloader until receive exit command'}

bl_gpio_polarities = {  0 : 'active low',
						1 : 'active high'}

bl_entry_check = { 0 : 'always enter',
					1 : 'check EBL pin'}


class MaximBootloader(object):
	def __init__(self, msbl_file, port):
		self.ser = serial.Serial();
		self.ser.port = port #'/dev/ttyACM0';
#		self.ser.port = 'COM11' #ser.port = '/dev/ttyUSB0'
		self.ser.baudrate = 115200;
		self.ser.timeout = 300;
		self.ser.open();	 # open the serial port

		if self.ser.isOpen():
			 print(self.ser.name + ' is open...')
		else:
			print('Unable to open serial port')
			exit(-1)

		self.thread = threading.Thread(name='serial', target=self.run, args=());
		self.thread.daemon=True;
		self.thread.start();
		logging.debug('\n\nInitializing bl downloader');

		self.msbl = Object()
		self.msbl.file_name = msbl_file
		print('msbl file name: ', self.msbl.file_name)

	def key_press_to_continue(self):
		try:
			input("Press enter to continue")
		except SyntaxError:
			pass
		except KeyboardInterrupt:
			logging.debug('Interrupted by Ctrl + C...');

	def run(self):
		sys.stdout.flush();
		logging.debug('\nStarting bl downloader thread');
		time.sleep(0.1);
		self.bootloader();
		#self.test();

	def read_msbl_file(self):
		total_size = 0
		print('msbl file name: ', self.msbl.file_name)
		with open(self.msbl.file_name, 'rb') as self.f:
			header = MsblHeader()
			if self.f.readinto(header) == sizeof(header):
				print('magic: ', header.magic,
						'  formatVersion: ', header.formatVersion,
						'  target: ', header.target,
						'  enc_type: ', header.enc_type,
						'  resv0: ', header.resv0,
						'  numPages: ', header.numPages,
						'  pageSize: ', header.pageSize,
						'  crcSize: ', header.crcSize,
						'  size of header: ', sizeof(header))

				self.print_as_hex('nonce', header.nonce)
				self.print_as_hex('auth', header.auth)
				self.print_as_hex('resv1', header.resv1)
			else:
				return False

			self.msbl.header = header

			i = 0
			self.msbl.page = {}
			tmp_page = Page()
			last_pos = self.f.tell()
			total_size = total_size + sizeof(header)
			print 'last_pos: ', last_pos
			while self.f.readinto(tmp_page) == sizeof(tmp_page):
				self.msbl.page[i] = deepcopy(tmp_page.data)
				total_size = total_size + sizeof(tmp_page)
				print ('read page ', i);
				i = i + 1
				last_pos = self.f.tell()
				print 'last_pos: ', last_pos
				#self.print_as_hex('page', self.msbl.page[0])

			self.msbl.crc32 = CRC32()
			self.f.seek(-4, 2)
			#last_pos = self.f.tell()
			#print 'last_pos: ', last_pos

			self.f.readinto(self.msbl.crc32)
			boot_mem_page = i - 1
			total_size = total_size + sizeof(self.msbl.crc32)
			print('Total file size: ', total_size, 'CRC32: ', hex(self.msbl.crc32.val))
			print('Reading msbl file succeed.')
		self.f.close()
		return True

	def set_iv(self):
		print('\nSet IV')
		tx = [0x80, 0x00]
		tx.extend(self.msbl.header.nonce)
		ret = self.send_cmd(tx, 1)
		return ret[0]

	def set_auth(self):
		print('\nSet Auth')
		tx = [0x80, 0x01]
		tx.extend(self.msbl.header.auth)
		ret = self.send_cmd(tx, 1)
		return ret[0]

	def set_page_size(self, num_pages):
		print('\nSet page size')
		tx = [0x80, 0x02]
		#num_pages = self.msbl.header.numPages
		tx.extend([(num_pages >> 8) & 0xFF, num_pages & 0xFF])
		ret = self.send_cmd(tx, 1)
		return ret[0]

	def erase_app(self):
		print('\nErase App')
		ret = self.send_cmd([0x80, 0x03], 1);
		# Make sure delay is enough target to finish app erase
		time.sleep(0.6);
		return ret[0]

	def download_page(self, page_num):
		tx = [0x80, 0x04]
		tx.extend(self.msbl.page[page_num])
		#self.print_as_hex('',tx)
		ret = self.send_cmd(tx, 1)
		return ret[0]

	def erase_page(self, page_num):
		print('\nErase page: ', page_num)
		tx = [0x80, 0x05]
		tx.extend([(page_num >> 8) & 0xFF, page_num & 0xFF])
		ret = self.send_cmd(tx, 1)
		return ret[0]

	def get_bl_fw_version(self):
		print('\nGet bootloader version')
		ret = self.send_cmd([0x81, 0x00], 4);
		if ret[0] == True:
			self.version = 'v'+ str(ret[1][1]) + '.' + str(ret[1][2]) + '.' + str(ret[1][3])
			print('Bootloader version: ', self.version)
		return ret[0]

	def get_flash_page_size(self):
		print('\nGet page size')
		ret = self.send_cmd([0x81, 0x01], 3);
		if ret[0] == True:
			self.page_size = ret[1][1] * 256 + ret[1][2]
			print('Target page size: ', self.page_size)
			if self.page_size != 8192:
				print ('WARNING: Page size is not 8192. page_size: ', self.page_size)
		return ret[0]

	######### Configure Bootloader Entry #########
	def bootloader_entry_settings(self):
		print('\nBootloader entry settings')
		port = 0
		pin = 1
		self.config_bootloader_entry_set_pin(port, pin)
		self.config_bootloader_entry_set_polarity(0) # Active low
		self.config_bootloader_entry_check(0) # Always enter
		return ret[0]

	def config_bootloader_entry_set_pin(self, port, pin):
		print('\nBootloader entry, port and pin configuration')
		pin_cfg = (port & 0xC0) | (pin & 0x3F)
		ret = self.send_cmd([0x82, 0x01, 0x01, pin_cfg], 1);
		if ret[0] == True:
			print('Configured P', port, '.', pin, ' pin to stay in bootlaoder')
		return ret[0]

	def config_bootloader_entry_set_polarity(self, polarity):
		# if polarity is 0, sets to active low.
		# MCU will stay in bootloader mode if EBL pin is held low during startup
		#
		# if polarity is 1, sets active high
		#
		print('\nBootloader entry, set polarity')
		pin_cfg = (port & 0xC0) | (pin & 0x3F)
		ret = self.send_cmd([0x82, 0x01, 0x01, pin_cfg], 1);
		if ret[0] == True:
			print('Configured polarity to ', bl_gpio_polarities[polarity])
		return ret[0]

	def config_bootloader_entry_check(self, state):
		print('\nBootloader entry: ', state)
		ret = self.send_cmd([0x82, 0x01, 0x00, state], 1);
		if ret[0] == True:
			print('Configured bootloader entry check: ', bl_entry_check[state])
		return ret[0]

	######### Save Bootloader Config #########
	def save_bootloader_settings(self):
		print('\nBootloader entry: ', state)
		ret = self.send_cmd([0x82, 0x00], 1);
		if ret[0] == True:
			bl_check = {0 : 'always enter',
						1 : 'check EBL pin'}
			print('Configured bootloader entry check: ', bl_check[state])
		return ret[0]

	######### Configure Bootloader Exit #########
	def bootloader_exit_settings(self):
		print('\nBootloader exit settings')
		self.config_bootloader_exit_mode(1) # wait programmable delay
		self.config_bootloader_delay(10) # 100msec

	def config_bootloader_exit_mode(self, mode):
		print('\nBootloader mode: ', mode)
		ret = self.send_cmd([0x82, 0x02, 0x00, mode], 1);
		if ret[0] == True:
			print('Configured bootloader exit mode: ', bl_exit_mode[mode])
		return ret[0]

	def config_bootloader_delay(self, timeout):
		print('\nBootloader timeout: ', timeout * 10, ' ms')
		ret = self.send_cmd([0x82, 0x02, 0x01, timeout], 1);
		if ret[0] == True:
			print('Configured programmable timeout to ', timeout * 10, ' ms')
		return ret[0]

	######### Read bootloader configurations #########
	def bootloader_exit_settings(self):
		self.print_bl_exit_delay()
		self.print_bl_exit_mode_config()
		self.print_bl_entry_ebl_polarity_config()
		self.print_bl_entry_ebl_pin_config()
		self.print_bl_entry_method_config()
		return True

	def print_bl_exit_delay(self):
		print('\nRead programmable delay')
		ret = self.send_cmd([0x83, 0x02, 0x01], 2);
		if ret[0] == True:
			exit_delay = ret[1][1]
			print('Exit delay: ', exit_delay, '(',exit_delay * 10, 'ms)')
		return ret[0]

	def print_bl_exit_mode_config(self):
		print('\nRead exit mode config')
		ret = self.send_cmd([0x83, 0x02, 0x00], 2);
		if ret[0] == True:
			mode = ret[1][1]
			print('Bootloader exit mode: ', bl_exit_mode[mode])
		return ret[0]

	def print_bl_entry_ebl_polarity_config(self):
		print('\nRead ebl polarity config')
		ret = self.send_cmd([0x83, 0x01, 0x02], 2);
		if ret[0] == True:
			polarity = ret[1][1]
			print('BL polarity: ', bl_gpio_polarities[polarity])
		return ret[0]

	def print_bl_entry_ebl_pin_config(self):
		print('\nRead ebl ping config')
		ret = self.send_cmd([0x83, 0x01, 0x01], 2);
		if ret[0] == True:
			pin_cfg = ret[1][1]
			port = pin_cfg >> 6;
			pin = pin_cfg & 0x3F;
			print('BL GPIO: P', port, '.', pin)
		return ret[0]


	def print_bl_entry_method_config(self):
		print('\nRead entry method config')
		ret = self.send_cmd([0x83, 0x01, 0x00], 2);
		if ret[0] == True:
			state = ret[1][1]
			print('Configured bootloader entry check: ', bl_entry_check[state])
		return ret[0]

	######### Bootloader #########
	def bootloader(self):
		logging.debug('\nDownloading msbl file')

		if self.read_msbl_file() != True:
			print('reading msbl file failed')
			return

		if self.enter_bootloader_mode() != True:
			print('Entering bootloader mode failed')
			return

		if self.get_device_mode() != True:
			print('Reading device mode failed')
			return

		if self.get_platform_type() != True:
			print('Reading platform type failed')
			return

		if self.get_bl_fw_version() != True:
			print('Reading bootloader version failed')
			return

		if self.get_flash_page_size() != True:
			print('Reading flash page size failed')
			return

		num_pages = self.msbl.header.numPages
		if self.set_page_size(num_pages) != True:
			print('Setting page size (',num_pages,') failed. ')
			return

		#self.set_iv()
		#self.set_auth()

		if self.erase_app() != True:
			print('Erasing app memory failed')
			return

		for i in range(0, num_pages):
			print '\nFlashing ', i+1, '/',num_pages,' page...'
			if self.download_page(i) != True:
				print 'Flashing ', i, '. page failed'
				return

		print('Flashing MSBL file succeed...')
		if self.restart_device() != True:
			print('Restarting device failed...')
			return

		print('Restarting device...')
		print('SUCCEED...')
		self.close()
		sys.exit(0)

	######### Test #########
	def test(self):
		logging.debug('\nRunning serial port')
		if self.read_msbl_file() != True:
			print('reading msbl file failed')

		if self.get_platform_type() != True:
			print('Reading platform type failed')


		if self.bootloader_entry_settings() != True:
			print('Reading msbl file failed')

		#self.bootloader_exit_settings()
		##self.get_platform_type()
		#self.enter_bootloader_mode()
		#self.exit_from_bootloader_mode()
		#self.restart_device()
		#self.get_device_mode()
		#self.get_flash_page_size()
		#self.get_bl_fw_version()
		#self.set_page_size()
		#self.set_iv()
		#self.set_auth()
		#self.erase_app()
		#self.erase_page(0)
		#self.download_page(0)

	def enter_bootloader_mode(self):
		print('\nEnter to bootloader')
		ret = self.send_cmd([0x01, 0x00, 0x08], 1);
		return ret[0]

	def exit_from_bootloader_mode(self):
		print('\nExit from bootloader')
		ret = self.send_cmd([0x01, 0x00, 0x00], 1);
		return ret[0]

	def restart_device(self):
		print('\nRestart device')
		ret = self.send_cmd([0x01, 0x00, 0x02], 1);
		return ret[0]

	def get_device_mode(self):
		print('\nGet device mode')
		ret = self.send_cmd([0x02, 0x00], 2);
		return ret[0]

	def print_as_hex(self, label, arr):
		print label, ' :', ' '.join(format(i, '02x') for i in arr)

	def get_platform_type(self):
		print('\nGet Platform Type')
		ret = self.send_cmd([0xFF, 0x00], 2);
		if ret[0] == True:
			platform_id = ret[1][1]
			print('Platform Type: ', platform_types[platform_id], ' id: ', platform_id)
		return ret[0]

	def send_cmd(self, cmd, nb_to_read):
		ret = [True]
		#self.print_as_hex('TX', cmd);
		self.ser.write(serial.to_bytes(cmd));

		resp = bytearray(nb_to_read)
		while True:
			if self.ser.readinto(resp) != len(resp):
				print('Reading failed.')
				ret = [False]

			if resp[0] == 0xFE:
				time.sleep(0.1);
				continue;

			if not (resp[0] == 0xAA):
				print('Failed. err: ', resp[0])
				ret = [False]
			break

		#self.print_as_hex('RX', resp);
		ret.append(resp)
		#self.key_press_to_continue()
		return ret;

	def close(self):
		try:
			self.ser.close()
			if not (self.thread is None):
				self.thread.join()
				self.thread = None
		except:
			pass

def main():
	parser = argparse.ArgumentParser()
	parser.add_argument("msblfile", type=str,
                    help="msbl file as input")
	parser.add_argument("port", type=str,
                    help=("Serial port name in Windows and device file path in Linux."
							"For example:"
							"	/dev/ttyACM0 in linux"
							"	COM1 in Windows"))
	args = parser.parse_args()
	print('Arguments', args)
	logging.debug('Maxim Firmware Downloader');
	bl = MaximBootloader(args.msblfile, args.port)
	print(bl.__dict__);
	logging.debug('### Press Ctrl + C to stop\t');
	try:
		try:
			while True:
				signal.pause()
		except AttributeError:
			# signal.pause() is missing for Windows; wait 1ms and loop instead
			while True:
				time.sleep(100);

	except KeyboardInterrupt:
		logging.debug('Interrupted by Ctrl + C...');
		bl.close();
		sys.exit(0);

if __name__ == '__main__':
	main()
