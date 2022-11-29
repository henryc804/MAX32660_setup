# Prerequisites

### Install

	- cmake
	- openssl

# HOW TO BUILD MSBLGEN TOOL

## 1. INSTALLING CMAKE
	Linux:
		On the terminal Using "sudo apt install cmake" is sufficent.
	Windows:
		Choose appropriate installer(64 or 32 bit) on page https://cmake.org/download/
		Install Cmake using this installer. Select "Add CMake to the system PATH" when you are asked.


	To verify that you have installed it correctly
		On terminal (Linux), Mingw or Cmd(on Windows) use command "cmake --version". If you observe such an output: "cmake version X.X.X" it means that installation is successful.

## 2. INSTALLING OPENSSL PACKAGES
	Linux:
		Using command "sudo apt install openssl" then "sudo apt install libssl1.0.0" "sudo apt install libssl-dev". (Although latter is libssl-dev is enough it is recomended to install these packages).
	Windows:
		Download [openssl](https://wiki.openssl.org/index.php/Binaries) and install it. Please make sure it is installed to "C:\OpenSSL-Win32" to avoid further problems.


## 3. COMPILING MSBLGEN

	Toplevel `makefile` generates Makefile for a platform and builds msblGen, and then it makes .msbl files.

	 `$	make`

	If you want to manually build msblGen, please follow the following instructions.

	Linux:
		Using terminal go to your msblGen folder. Then type following

		** Creating a build folder **
		`mkdir build`

		** Changing current dir to build **
		`cd build`

		** Creating make file with cmake **
		`cmake .. -G "Unix Makefiles"`

		** Compiling the tool **
		`make`

		This will create the tool binary in your msblGen folder NOT in "build" folder under it.
	Windows:
		You should follow the same instructions above (Linux steps). However you need to use "MINGW32" or similar GNU environment. It is included in the EV KIT SOFTWARE(Low Power ARM Micro Toolchain) at
		https://www.maximintegrated.com/en/design/software-description.html/swpart=SFW0001500A

## 4. USING MSBLGEN
	MsblGen needs 4 arguments
	On Linux and Windows
		./msblGen <file to be encrypted> <target name> <page size> [<keyname>]

	Please note that <keyname> is important.
		If it is not specified. Tool will not encrypted it but will create .msbl file accordingly.
		If it is specified:
			Tool will look for keyname_key.txt under the folder that msblGen tool is present.
			ex.
				./msblGen myProgram.bin MAX32660 8192 KEYX

				will try to get the keys from "KEYX_key.txt"

	If there is no problems up to now 2 files "myProgram.msbl" and "myProgram_bootmem.bin" will be created.

## Links

  - https://wiki.openssl.org/index.php/Binaries
  - https://cmake.org/download/
