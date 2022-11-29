#include "mbed.h"
#include "USBSerial.h"
#include "version.h"
#include "DSInterface.h"
#include "Peripherals.h"
#include "max32630fthr.h"
#include "SSInterface.h"
#include "SSBootloaderComm.h"

#ifndef MAXIM_PLATFORM_NAME
#define MAXIM_PLATFORM_NAME	"Pegasus"
#endif

#if ENABLE_LED_STATUS
//Set yellow light during boot
DigitalOut rLED(LED1, LED_ON);
DigitalOut gLED(LED2, LED_ON);
DigitalOut bLED(LED3, LED_OFF);
#endif

MAX32630FTHR pegasus(MAX32630FTHR::VIO_3V3);

// Virtual serial port over USB
USBSerial microUSB(0x1f00, 0x2012, 0x0001, false);
// Hardware serial port over DAPLink
//Serial daplink(USBRX, USBTX, 115200);
Serial daplink(P2_1, P2_0);

//GUI/Terminal Interface
DSInterface dsInterface(&microUSB);

//Bootloader Interface
DigitalIn sdaDummy(P3_4, PullUp);
DigitalIn sclDummy(P3_5, PullUp);

PinName ss_mfio(P5_4);
PinName ss_reset(P5_6);

//Creates all interfaces at the beginning
SPI ssSPI(P5_1, P5_2, P5_0, P5_3);
SSInterface ssInterfaceSpi(ssSPI, ss_mfio, ss_reset);

I2C ssI2C(P3_4, P3_5);
SSInterface ssInterfaceI2c(ssI2C, ss_mfio, ss_reset);

Serial ssUart(P3_0, P3_1, 115200);
SSInterface ssInterfaceUart(ssUart, ss_mfio, ss_reset);

SSBootloaderComm ssBoot(&microUSB, &ssInterfaceSpi, &ssInterfaceI2c, &ssInterfaceUart, &dsInterface);


#ifdef ENABLE_LED_STATUS
static int led_on_ms = 100;
static int led_off_ms = 1900;
static Timer blink_timer;
static int current_state = 0;
void update_led_state()
{
	if (current_state == LED_ON) {
		if (blink_timer.read_ms() > led_on_ms) {
			gLED = LED_OFF;
			current_state = LED_OFF;
			blink_timer.reset();
		}
	}
	else
	{
		if (blink_timer.read_ms() > led_off_ms) {
			gLED = LED_ON;
			current_state = LED_ON;
			blink_timer.reset();
		}
	}
}
#endif

void print_build_version()
{
	printf("\n\nPegasus mBED EVKit\r\n");
	printf("Fw version: %s, mbed version: %d\r\n", FIRMWARE_VERSION, MBED_VERSION);
	printf("Build source: (%s)  %s\r\n", BUILD_SOURCE_BRANCH, BUILD_SOURCE_HASH);
	printf("Build time: %s  %s\r\n\n", __TIME__, __DATE__);
}

int main()
{
	daplink.printf("Init NVIC Priorities...\r\n");
	fflush(stdout);
	NVIC_SetPriority(GPIO_P0_IRQn, 5);
	NVIC_SetPriority(GPIO_P1_IRQn, 5);
	NVIC_SetPriority(GPIO_P2_IRQn, 5);
	NVIC_SetPriority(GPIO_P3_IRQn, 5);
	NVIC_SetPriority(GPIO_P4_IRQn, 5);
	NVIC_SetPriority(GPIO_P5_IRQn, 5);
	NVIC_SetPriority(GPIO_P6_IRQn, 5);
	print_build_version();
	daplink.printf("daplink serial port\r\n");
	microUSB.printf("micro USB serial port\r\n");
	dsInterface.set_fw_version(FIRMWARE_VERSION);
	dsInterface.set_fw_platform(MAXIM_PLATFORM_NAME);
	Peripherals::setUSBSerial(&microUSB);
	Peripherals::setDaplinkSerial(&daplink);

	ssI2C.frequency(400000);
	ssSPI.format(8, 0);
	ssSPI.frequency(1000000);

#if 0
	dsInterface.set_fw_platform(ssInterface.get_ss_platform_name());
	dsInterface.set_fw_version(ssInterface.get_ss_fw_version());
#endif
	dsInterface.add_sensor_comm(&ssBoot);

#ifdef ENABLE_LED_STATUS
	//Indicate we're done with setup
	rLED = LED_OFF;
	gLED = LED_OFF;
	bLED = LED_OFF;

	blink_timer.start();
#endif

	while(1) {
		USBSerial *serial = &microUSB;
		uint8_t ch;
		while (serial->readable()) {
			ch = serial->_getc();
			dsInterface.enable_console_interface();
		    dsInterface.build_command(ch);
		}

		dsInterface.data_report_execute();
#ifdef ENABLE_LED_STATUS
		update_led_state();
#endif
	}
}
