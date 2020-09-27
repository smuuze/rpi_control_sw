
// -------- INCLUDES --------------------------------------------------------------------

#include "shc_project_configuration.h"
#include "shc_common_types.h"
#include "shc_common_string.h"
#include "shc_common_configuration.h"

#include "shc_timer.h"
#include "shc_file_interface.h"
#include "shc_common_string.h"
#include "shc_gpio_interface.h"
#include "shc_command_interface.h"
#include "shc_mqtt_interface.h"
#include "shc_spi_interface.h"
#include "shc_debug_interface.h"
#include "shc_lcd_interface.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <linux/spi/spidev.h>

#include <time.h>

// -------- SCHEDULE INTERVAL DEFAULTS --------------------------------------------------

#define REPORT_SCHEDULE_DEFAULT_INTERVAL_MS		60000
#define EVENT_SCHEDULE_DEFAULT_INTERVAL_MS		1500
#define CONFIGURATION_SCHEDULE_DEFAULT_INTERVAL_MS	60000

// --------------------------------------------------------------------------------------

#define CFG_DEBUG_MSG					noDEBUG_MSG

// --------------------------------------------------------------------------------------

u8 command_line_parser_match_argument(char* p_argument, char* p_key) {

	if (string_length(p_argument) != string_length(p_key)) {
		return 0;
	}

	if (memcmp(p_argument, p_key, string_length(p_key)) != 0) {
		return 0;
	}

	return 1;
}

u8 command_line_parser(int argc, char* argv[], CFG_INTERFACE* p_cfg_interface, COM_INTERFACE* p_com_interface, MQTT_INTERFACE* p_mqtt_interface, COMMAND_INTERFACE* p_cmd_interface, GPIO_INTERFACE* p_gpio_interface, SCHEDULING_INTERFACE* p_scheduling_interface) {

	// --- Initialize Communication-Interface

	if (p_scheduling_interface != NULL) {

		CFG_DEBUG_MSG("command_line_parser() - Set default values for SCHEDULING-Interface\n");

		p_scheduling_interface->report.interval = REPORT_SCHEDULE_DEFAULT_INTERVAL_MS;
		p_scheduling_interface->event.interval = EVENT_SCHEDULE_DEFAULT_INTERVAL_MS;
		p_scheduling_interface->configuration.interval = CONFIGURATION_SCHEDULE_DEFAULT_INTERVAL_MS;
	}

	if (p_com_interface != NULL) {

		CFG_DEBUG_MSG("command_line_parser() - Set default values for SPI-Interface\n");

		// 48000  4800 	9600 	38400 	115200
		p_com_interface->is_enabled = 0;
		p_com_interface->data.spi.speed_hz = 9600;
		p_com_interface->data.spi.bits_per_word = 8;
		p_com_interface->data.spi.delay = 0;
		p_com_interface->data.spi.mode = SPI_MODE_3;
	}

	if (p_mqtt_interface != NULL) {

		CFG_DEBUG_MSG("command_line_parser() - Set default values for MQTT-Interface\n");

		p_mqtt_interface->quality_of_service = MQTT_QOS;
		p_mqtt_interface->msg_delivered = 1;
	}

	if (p_gpio_interface != NULL) {

		CFG_DEBUG_MSG("command_line_parser() - Set default values for GPIO-Interface\n");

		p_gpio_interface->pin_num = GPIO_EVENT_PIN;
		p_gpio_interface->is_input = 1;
		p_gpio_interface->is_high_level = 0;
		p_gpio_interface->match_event_level = 1;
		p_gpio_interface->event_rised = 0;
		p_gpio_interface->event_ref_time = 0;
		p_gpio_interface->event_timeout = 1000;
		p_gpio_interface->is_initialized = 0;
		p_gpio_interface->sample_time_reference = 0;
		p_gpio_interface->sample_timeout = 100;
	}

	if (p_cmd_interface != NULL) {

		CFG_DEBUG_MSG("command_line_parser() - Set default values for COMMAND-Interface\n");

		p_cmd_interface->command_file.handle = 0;
		p_cmd_interface->command_file.act_file_pointer = 0;
		p_cmd_interface->report_file.handle = 0;
		p_cmd_interface->report_file.act_file_pointer = 0;
		p_cmd_interface->event_file.handle = 0;
		p_cmd_interface->event_file.act_file_pointer = 0;
		p_cmd_interface->is_active = 0;
		p_cmd_interface->fail_counter = 0;
	}

	p_cfg_interface->log_file.act_file_pointer = 0;

	memset(p_cfg_interface->cfg_file.path, 0x00, FILE_PATH_MAX_STRING_LENGTH);

	u8 i = 0;
	for ( ; i < argc; i++) {

		//CFG_DEBUG_MSG("command_line_parser() - Parsing cli-argument: %s\n", argv[i]);

		if (memcmp(argv[i], COMMAND_LINE_ARGUMENT_CFG_FILE, string_length(COMMAND_LINE_ARGUMENT_CFG_FILE)) == 0) {

			if (i + 1 >= argc) {
				break;
			}

			memset(p_cfg_interface->cfg_file.path, 0x00, FILE_PATH_MAX_STRING_LENGTH);
			memcpy(p_cfg_interface->cfg_file.path, argv[i + 1], string_length(argv[i + 1]));

			CFG_DEBUG_MSG("Using Config-File: %s\n", p_cfg_interface->cfg_file.path);
			
			// do not process the parameter as a new argument
			i += 1;

		} else if (memcmp(argv[i], COMMAND_LINE_ARGUMENT_LCD, string_length(COMMAND_LINE_ARGUMENT_LCD)) == 0) {
			CFG_DEBUG_MSG("command_line_parser() - Enabling LCD\n");

			LCD_INIT(1);

		} else if (memcmp(argv[i], COMMAND_LINE_ARGUMENT_CONTROLLER, string_length(COMMAND_LINE_ARGUMENT_CONTROLLER)) == 0) {
			CFG_DEBUG_MSG("command_line_parser() - Enabling Control-Board\n");

		} else if (command_line_parser_match_argument(argv[i], COMMAND_LINE_ARGUMENT_DEVICE) && p_com_interface != NULL) {

			if (i + 1 >= argc) {
				break;
			}

			CFG_DEBUG_MSG("command_line_parser() - Enabling SPI-Interface\n");

			memcpy(p_com_interface->data.spi.device, argv[i + 1], COM_DEVICE_NAME_STRING_LENGTH);
			p_com_interface->is_enabled = 1;

		} else if (command_line_parser_match_argument(argv[i], COMMAND_LINE_ARGUMENT_COMMAND) && p_cmd_interface != NULL) {

			if (i + 1 >= argc) {
				break;
			}

			p_cmd_interface->command.length = hex_string_to_byte_array(argv[i + 1], string_length(argv[i + 1]), p_cmd_interface->command.payload, GENERAL_STRING_BUFFER_MAX_LENGTH);	
		}
	}

	u8 zero_buffer[FILE_PATH_MAX_STRING_LENGTH];
	memset(zero_buffer, 0x00, FILE_PATH_MAX_STRING_LENGTH);

	if (memcmp(p_cfg_interface->cfg_file.path, zero_buffer, FILE_PATH_MAX_STRING_LENGTH) == 0) {
		CFG_DEBUG_MSG("command_line_parser() - No Configuration File was given\n");
		return NO_ERR;
	}

	char path[128];
	sprintf(path, "%s", p_cfg_interface->cfg_file.path);

	FILE* config_file_handle = fopen((const char*)p_cfg_interface->cfg_file.path, "r");
	if (config_file_handle == NULL) {
		CFG_DEBUG_MSG("--- Open Configuration-File has FAILED !!! --- (FILE: %s / ERROR: %d)\n", p_cfg_interface->cfg_file.path,  EXIT_FAILURE);
		return ERR_FILE_OPEN;
	}

	struct stat file_attribute;
	stat(p_cfg_interface->cfg_file.path, &file_attribute);
	p_cfg_interface->cfg_file.timestamp_last_modified = file_attribute.st_mtime;

	char line[512];
	char cfg_key[GENERAL_STRING_BUFFER_MAX_LENGTH];
	char cfg_value[GENERAL_STRING_BUFFER_MAX_LENGTH];
	u16 num_bytes = read_line(config_file_handle, line, 512);

	while (num_bytes != 0) {
	 
		if (line[0] == '#') {
			CFG_DEBUG_MSG("command_line_parser() - Ignoring line: %s\n", line);
			goto NEXT_CONFIG_LINE;
		}

		split_string('=', line, num_bytes, cfg_key, GENERAL_STRING_BUFFER_MAX_LENGTH, cfg_value, GENERAL_STRING_BUFFER_MAX_LENGTH);

		u16 length_key = string_length(cfg_key);
		if (length_key == 0) {
			goto NEXT_CONFIG_LINE;
		}

		u16 length_value = string_length(cfg_value);
		if (length_value == 0) {
			goto NEXT_CONFIG_LINE;
		}

		//memset(cfg_key + length_key, 0x00, GENERAL_STRING_BUFFER_MAX_LENGTH - length_key);
		//memset(cfg_value + length_value, 0x00, GENERAL_STRING_BUFFER_MAX_LENGTH - length_value);

		CFG_DEBUG_MSG("command_line_parser() - key:%s : value:%s\n", cfg_key, cfg_value);

		if (memcmp(cfg_key, CFG_NAME_MQTT_HOST_ADDR, length_key) == 0 && p_mqtt_interface != NULL) {
			memcpy(p_mqtt_interface->host_address, cfg_value, MQTT_HOST_ADDRESS_STRING_LENGTH);

		} else

		if (memcmp(cfg_key, CFG_NAME_MQTT_CLINET_ID, length_key) == 0 && p_mqtt_interface != NULL) {
			memcpy(p_mqtt_interface->client_id, cfg_value, MQTT_CLIENT_ID_STRING_LENGTH);

		} else

		if (memcmp(cfg_key, CFG_NAME_MQTT_TOPIC_NAME, length_key) == 0 && p_mqtt_interface != NULL) {
			memcpy(p_mqtt_interface->topic_name, cfg_value, MQTT_TOPIC_NAME_STRING_LENGTH);
		} else

		if (memcmp(cfg_key, CFG_NAME_MQTT_WELCOME_MESSAGE, length_key) == 0 && p_mqtt_interface != NULL) {
			memcpy(p_mqtt_interface->welcome_msg, cfg_value, MQTT_WELCOME_MSG_STRING_LENGTH);
		} else

		if (memcmp(cfg_key, CFG_NAME_MQTT_TIMEOUT, length_key) == 0 && p_mqtt_interface != NULL) {
			p_mqtt_interface->timeout_ms = MQTT_TIMEOUT;

		} else

		if (memcmp(cfg_key, CFG_NAME_COMMUNICATION_TYPE, length_key) == 0 && p_com_interface != NULL) {
			p_com_interface->type = SPI;

		} else

		if (memcmp(cfg_key, CFG_NAME_COM_SPI_BAUDRATE, length_key) == 0 && p_com_interface != NULL) {
			char *ptr;
			p_com_interface->data.spi.speed_hz = (u32)strtol(cfg_value, &ptr, 10);
		} else

		if (memcmp(cfg_key, CFG_NAME_COM_SPI_DEVICE, length_key) == 0 && p_com_interface != NULL) {
			memcpy(p_com_interface->data.spi.device, cfg_value, COM_DEVICE_NAME_STRING_LENGTH);
			p_com_interface->is_enabled = 1;
		} else

		if (memcmp(cfg_key, CFG_NAME_LOG_FILE_PATH, length_key) == 0) {
			memcpy(p_cfg_interface->log_file.path, cfg_value, FILE_PATH_MAX_STRING_LENGTH);
		} else

		if (memcmp(cfg_key, CFG_NAME_COMMAND_FILE_PATH, length_key) == 0 && p_cmd_interface != NULL) {
			memcpy(p_cmd_interface->command_file.path, cfg_value, FILE_PATH_MAX_STRING_LENGTH);
		} else

		if (memcmp(cfg_key, CFG_NAME_REPORT_FILE_PATH, length_key) == 0 && p_cmd_interface != NULL) {
			memcpy(p_cmd_interface->report_file.path, cfg_value, FILE_PATH_MAX_STRING_LENGTH);
		} else

		if (memcmp(cfg_key, CFG_NAME_EVENT_FILE_PATH, length_key) == 0 && p_cmd_interface != NULL) {
			memcpy(p_cmd_interface->event_file.path, cfg_value, FILE_PATH_MAX_STRING_LENGTH);
		} else

		if (memcmp(cfg_key, CFG_NAME_EXECUTION_FILE_PATH, length_key) == 0 && p_cmd_interface != NULL) {
			memcpy(p_cmd_interface->execution_file.path, cfg_value, FILE_PATH_MAX_STRING_LENGTH);
		} else

		if (memcmp(cfg_key, CFG_NAME_SCHEDULE_INTERVAL_REPORT_MS, length_key) == 0 && p_scheduling_interface != NULL) {
			char *ptr;
			p_scheduling_interface->report.interval = (u32)strtol(cfg_value, &ptr, 10);
		} else

		if (memcmp(cfg_key, CFG_NAME_SCHEDULE_INTERVAL_EVENT_MS, length_key) == 0 && p_scheduling_interface != NULL) {
			char *ptr;
			p_scheduling_interface->event.interval = (u32)strtol(cfg_value, &ptr, 10);
		} else

		if (memcmp(cfg_key, CFG_NAME_SCHEDULE_INTERVAL_CONFIG_MS, length_key) == 0 && p_scheduling_interface != NULL) {
			char *ptr;
			p_scheduling_interface->configuration.interval = (u32)strtol(cfg_value, &ptr, 10);
		} else 

		if (memcmp(cfg_key, CFG_NAME_LCD_ENABLE, length_key) == 0) {
			char *ptr;
			LCD_INIT((u32)strtol(cfg_value, &ptr, 10));
		}

		else {
			CFG_DEBUG_MSG("--- UNKNOWN CFG-KEY : %s\n", cfg_key);
		}
		
		NEXT_CONFIG_LINE:
		num_bytes = read_line(config_file_handle, line, 512);
	}

	return NO_ERR;
}