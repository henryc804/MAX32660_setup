mkdir release
make clean;make PROJECT=EvKit_V1_BL
cp build/EvKit_V1_BL.bin release/EvKit_V1_BL.bin
cp build/EvKit_V1_BL.elf release/EvKit_V1_BL.elf

make clean;make PROJECT=EvKit_V1_I2C_BL
cp build/EvKit_V1_I2C_BL.bin release/EvKit_V1_I2C_BL.bin
cp build/EvKit_V1_I2C_BL.elf release/EvKit_V1_I2C_BL.elf

make clean;make PROJECT=EvKit_V1_SPI_BL
cp build/EvKit_V1_SPI_BL.bin release/EvKit_V1_SPI_BL.bin
cp build/EvKit_V1_SPI_BL.elf release/EvKit_V1_SPI_BL.elf


make clean;make PROJECT=EvKit_V1_Uart_BL
cp build/EvKit_V1_Uart_BL.bin release/EvKit_V1_Uart_BL.bin
cp build/EvKit_V1_Uart_BL.elf release/EvKit_V1_Uart_BL.elf


make clean;make PROJECT=max32660_i2c_bootloader
cp build/max32660_i2c_bootloader.bin release/max32660_i2c_bootloader.bin
cp build/max32660_i2c_bootloader.elf release/max32660_i2c_bootloader.elf

md5sum release/** > release/md5sum.txt

