/***************************************************************************
* Copyright (C) 2017 Maxim Integrated Products, Inc., All Rights Reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation
* the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
* OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL MAXIM INTEGRATED BE LIABLE FOR ANY CLAIM, DAMAGES
* OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
* ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
* OTHER DEALINGS IN THE SOFTWARE.
*
* Except as contained in this notice, the name of Maxim Integrated
* Products, Inc. shall not be used except as stated in the Maxim Integrated
* Products, Inc. Branding Policy.
*
* The mere transfer of this software does not imply any licenses
* of trade secrets, proprietary technology, copyrights, patents,
* trademarks, maskwork rights, or any other form of intellectual
* property whatsoever. Maxim Integrated Products, Inc. retains all
* ownership rights.
****************************************************************************
*/

#include <ctype.h>
#include <string.h>
#include "DSInterface.h"
#include "../version.h"
#include "Peripherals.h"
#include "assert.h"
#include "utils.h"

#ifdef ENABLE_BLE
#include "BLE_ICARUS.h"
#endif

static const char *glbl_cmds[] = {
	"stop",
	"get_device_info",
	"silent_mode 0",
	"silent_mode 1",
	"pause 0",
	"pause 1",
	"enable console",
	"disable console",
};

typedef enum {
	stop=0,
	get_device_info,
	silent0_mode,
	silent1_mode,
	pause0_mode,
	pause1_mode,
	enable_console,
	disable_console,
	NUM_CMDS,
} glbl_cmd_state;


DSInterface::DSInterface(USBSerial* USB)
{
	cmd_idx = 0;
	silent_mode = false;
	pause_mode = false;

	memset(sensor_list, 0, DS_MAX_NUM_SENSORCOMMS * sizeof(SensorComm*));
	num_sensors = 0;

	m_USB = USB;

	ds_console_interface_exists_ = false;
}

DSInterface::~DSInterface()
{
}

void DSInterface::add_sensor_comm(SensorComm *s)
{
	assert_msg(num_sensors < DS_MAX_NUM_SENSORCOMMS, "Too many sensors added to DSInterface. Increase DS_MAX_NUM_SENSORCOMMS.");
	sensor_list[num_sensors++] = s;
}

void DSInterface::enable_console_interface()
{
	ds_console_interface_exists_ = true;
}

void DSInterface::ds_set_ble_status(bool en){
	SensorComm *p_sensor;
	ds_ble_interface_exists_ = en;

	for(int sensor_num = 0; sensor_num < num_sensors; ++sensor_num) {
		p_sensor = sensor_list[sensor_num];
		p_sensor->SensorComm_Set_Ble_Status(ds_ble_interface_exists_);
	}
}

void DSInterface::build_command(char ch)
{
	if (!this->silent_mode) /* BUG: POTENTIAL BUG, what uart port to echo, not only console */
		m_USB->printf("%c", ch);

	if (ch == 0x00) {
		pr_err("Ignored char 0x00");
		return;
	}

	if ((ch == '\n') || (ch == '\r')) {
		if (cmd_idx < (int)CONSOLE_STR_BUF_SZ)
		cmd_str[cmd_idx++] = '\0';
		m_USB->printf("\r\n");
		parse_command();

		//Clear cmd_str
		while (cmd_idx > 0) /* BUG: POTENTIAL BUG for multiple port access */
			cmd_str[--cmd_idx] = '\0';

	} else if ((ch == 0x08 || ch == 0x7F) && cmd_idx > 0) {
		//Backspace character
		if (cmd_idx > 0)
			cmd_str[--cmd_idx] = '\0';
	} else {
		/* BUG: POTENTIAL BUG for multiple port access */
		if (cmd_idx < (int)CONSOLE_STR_BUF_SZ)
			cmd_str[cmd_idx++] = ch;
	}

}

void DSInterface::parse_command()
{
	int i;
	glbl_cmd_state cmd;
	char charbuf[512];
	int data_len = 0;
	int ret;
	bool parsed_cmd = true;

	//If top level command, then handle it
	for (i = 0; i < NUM_CMDS; i++) {
		if (starts_with(&cmd_str[0], glbl_cmds[i])) {
			cmd = (glbl_cmd_state)i;

			switch (cmd) {
				case (stop): {
					for (int i = 0; i < num_sensors; i++) {
						sensor_list[i]->stop();
					}
					data_len += snprintf(charbuf, sizeof(charbuf) - 1,
							"\r\n%s err=0\r\n", cmd_str);
				} break;
				case (get_device_info): {
					data_len = snprintf(charbuf, sizeof(charbuf),
						"\r\n%s platform=%s firmware_ver=%s sensors=", cmd_str, platform_name, FIRMWARE_VERSION);

					//Add list of sensors
					for (int i = 0; i < num_sensors; i++) {
						if (sensor_list[i]->is_visible()) {
							data_len += snprintf(charbuf + data_len, sizeof(charbuf) - data_len,
											"%s", sensor_list[i]->get_type());
							if (i < (num_sensors - 1))
								data_len += snprintf(charbuf + data_len, sizeof(charbuf) - data_len, ",");
						}
					}


					for (int i = 0; i < num_sensors; i++) {
						SensorComm *s = sensor_list[i];
						if (!s->is_visible())
							continue;

						//Add algo_ver
						data_len += snprintf(charbuf + data_len, sizeof(charbuf) - data_len,
											" algo_ver_%s=%s", s->get_type(), s->get_algo_ver());

						//Add part name
						data_len += snprintf(charbuf + data_len, sizeof(charbuf) - data_len,
											" part_name_%s=%s", s->get_type(), s->get_part_name());

						uint8_t part_id, part_rev;
						ret = s->get_part_info(&part_id, &part_rev);

						if (ret == 0) {
							//Add part id
							data_len += snprintf(charbuf + data_len, sizeof(charbuf) - data_len,
											" part_id_%s=%d", s->get_type(), part_id);

							//Add rev id
							data_len += snprintf(charbuf + data_len, sizeof(charbuf) - data_len,
											" part_rev_%s=%d", s->get_type(), part_rev);
						}
					}

					if(firmware_version){
						data_len += snprintf(charbuf + data_len, sizeof(charbuf) - data_len, " hub_firm_ver=%s", firmware_version);
					}

					data_len += snprintf(charbuf + data_len, sizeof(charbuf) - data_len, " err=0\r\n");

				} break;
				case (silent0_mode): {
					silent_mode = false;
					data_len += snprintf(charbuf, sizeof(charbuf) - 1,
							"\r\n%s err=0\r\n", cmd_str);
				} break;
				case (silent1_mode): {
					silent_mode = true;
					data_len += snprintf(charbuf, sizeof(charbuf) - 1,
							"\r\n%s err=0\r\n", cmd_str);
				} break;
				case (pause0_mode): {
					pause_mode = false;
					data_len += snprintf(charbuf, sizeof(charbuf) - 1,
							"\r\n%s err=0\r\n", cmd_str);
				} break;
				case (pause1_mode): {
					pause_mode = true;
					data_len += snprintf(charbuf, sizeof(charbuf) - 1,
							"\r\n%s err=0\r\n", cmd_str);
				} break;
				case (enable_console): {
					ds_console_interface_exists_ = true;
					data_len += snprintf(charbuf, sizeof(charbuf) - 1,
							"\r\n%s err=0\r\n", cmd_str);
				} break;
				case (disable_console): {
					ds_console_interface_exists_ = false;
					data_len += snprintf(charbuf, sizeof(charbuf) - 1,
							"\r\n%s err=0\r\n", cmd_str);
				} break;
				default:
					parsed_cmd = false;
					break;
			}

			if (parsed_cmd) {
				m_USB->printf(charbuf);
#if ENABLE_BLE
				if (BLE_Icarus_Interface_Exists()) {
					BLE_Icarus_AddtoQueue((uint8_t *)charbuf, (int32_t)sizeof(charbuf), data_len);
				}
#endif
			}

			return;
		}
	}

	//Loop through each sensor in sensorList
	//If sensor.get_type() is contained in cmd_str, pass cmd_str to that sensor's parser
	for (int i = 0; i < num_sensors; i++) {
		if (strstr(&cmd_str[0], sensor_list[i]->get_type())) {
			if (sensor_list[i]->parse_command(cmd_str))
				return;
			break;
		}
	}

	//If we still haven't found a way to parse the command,
	//send it to every sensor until one handles it
	for (int i = 0; i < num_sensors; i++) {
		if (sensor_list[i]->parse_command(cmd_str))
			return;
	}
	

	//No one could handle the command, print cmd not recognized string
	data_len += snprintf(charbuf, sizeof(charbuf) - 1,
			"\r\n%s err=-255\r\n", cmd_str);
	m_USB->printf(charbuf);
#if ENABLE_BLE
	if (BLE_Icarus_Interface_Exists()) {
		BLE_Icarus_AddtoQueue((uint8_t *)charbuf, (int32_t)sizeof(charbuf), data_len);
	}
#endif
}

void DSInterface::data_report_execute()
{
	char buffer[256];
	int data_len = 0;

	buffer[0] = '\0';

	for (int i = 0; i < num_sensors; i++) {
		//if ((*it)->is_enabled()) {
		data_len = sensor_list[i]->data_report_execute(buffer, sizeof(buffer));
		if (!this->pause_mode && (data_len > 0)) {
#ifdef ENABLE_BLE
			if (BLE_Icarus_Interface_Exists()) {
				ds_console_interface_exists_ = false;
				BLE_Icarus_AddtoQueue((uint8_t *)buffer, (int32_t)sizeof(buffer), data_len);
			}
#endif
			if (ds_console_interface_exists_){
#ifdef ASCII_COMM
				m_USB->printf(buffer);
#else
				m_USB->writeBlock((uint8_t*)buffer, data_len);
#endif
			}
			data_len = 0;
		}
		//We only support streaming from one device at a time, so break here
		//	break;
		//}
	}
}

void DSInterface::set_fw_version(const char* version)
{
    if (version && *version)
        firmware_version = version;
}

void DSInterface::set_fw_platform(const char* platform)
{
    if (platform && *platform)
        platform_name = platform;
}

const char* DSInterface::get_fw_version()
{
    return firmware_version;
}
