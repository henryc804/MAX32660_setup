platform="$(uname -s)"
echo "$platform"

if [[ "$platform" == 'MINGW32_NT-6.2' ]]
then
	if [ "$1" = "" ]; then
		file="./BUILD/MAX32630FTHR/GCC_ARM/mbed-os.elf"
	else
		file=$1
	fi
openocd -f ./scripts/jtag/max3263x_hdk.cfg -c "program $file verify reset exit"
else
	DIR=$(dirname $(readlink -f $0))

	if [ "$1" = "" ]; then
		fw_file="$DIR/../BUILD/MAX32630FTHR/GCC_ARM/mbed-os.elf"
	else
		fw_file=$1
	fi
	CFG_FILE=$DIR/jtag/max3263x_hdk.cfg
	openocd -f $CFG_FILE -c "program $fw_file verify reset exit"
fi
