
// -------- INCLUDES --------------------------------------------------------------------

#include "shc_project_configuration.h"
#include "shc_common_types.h"
#include "shc_common_string.h"
#include "shc_common_configuration.h"

#include "shc_timer.h"
#include "shc_spi_interface.h"
#include "shc_qeue_interface.h"
#include "shc_file_interface.h"
#include "shc_common_string.h"
#include "shc_command_interface.h"
#include "shc_mqtt_interface.h"
#include "shc_gpio_interface.h"
#include "shc_debug_interface.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <linux/spi/spidev.h>

#include <time.h>

// -------- SCHEDULE INTERVAL DEFAULTS -------------------------------------------------------------------

#define REPORT_SCHEDULE_DEFAULT_INTERVAL_MS		60000
#define EVENT_SCHEDULE_DEFAULT_INTERVAL_MS		1500
#define CONFIGURATION_SCHEDULE_DEFAULT_INTERVAL_MS	60000

// -------- DEBUGGING -------------------------------------------------------------------

#define MAIN_DEBUG_MSG					DEBUG_MSG
#define MAIN_CFG_DEBUG_MSG				DEBUG_MSG

// -------- Command-Code ----------------------------------------------------------------

#define CMD_VERSION_STR					"version"
#define CMD_VERSION_LEN					7


// -------- TYPE-DEFINITIONS ------------------------------------------------------------




// -------- STATIC FUNCTION PROTOTYPES --------------------------------------------------

static void reset_device(void);

/*!
 *
 */
u8 command_line_parser(int argc, char* argv[], CFG_INTERFACE* p_cfg_interface, COM_INTERFACE* p_com_interface, MQTT_INTERFACE* p_mqtt_interface, COMMAND_INTERFACE* p_cmd_interface, GPIO_INTERFACE* p_gpio_interface, SCHEDULING_INTERFACE* p_scheduling_interface);

/*!
 *
 */
void command_line_usage(void);


/*!
 *
 */
void log_message(FILE_INTERFACE* p_file, u8 error_level, STRING_BUFFER* p_msg_from);


// -------- STATIC DATA -----------------------------------------------------------------

/*!
 *
 */
MSG_QEUE myCommandQeue;

static MQTT_INTERFACE myMqttInterface;

/*!
 *
 */
static COM_INTERFACE myComInterface;

/*!
 *
 */
static COMMAND_INTERFACE myCmdInterface;

/*!
 *
 */
static CFG_INTERFACE myCfgInterface;

/*!
 *
 */
static GPIO_INTERFACE myGpioInterface;

/*!
 *
 */
static SCHEDULING_INTERFACE mySchedulingInterface;


// -------- MAIN ------------------------------------------------------------------------

int main(int argc, char* argv[]) {

	MAIN_DEBUG_MSG("Welcome to the SmartHomeClient v%d.%d", VERSION_MAJOR, VERSION_MINOR);

	qeue_init(&myCommandQeue);

	#if DEBUG_DISABLE_REPORT_PROCESSING == 1
	MAIN_DEBUG_MSG("RPORT PROCESSING IS DISABLED !!!\n");
	#endif

	MAIN_DEBUG_MSG("\n");
	MAIN_DEBUG_MSG("---- Loading Configuration ---- \n");

	// --- Parsing Command-Line Arguments
	u8 err_code = command_line_parser(argc, argv, &myCfgInterface, &myComInterface, &myMqttInterface, &myCmdInterface, &myGpioInterface, &mySchedulingInterface);
	if (err_code != NO_ERR) {
		command_line_usage();
		return err_code;
	}

	LOG_MSG(NO_ERR, &myCfgInterface.log_file, "Starting SmartHomeClient Deamon v%d.%d", VERSION_MAJOR, VERSION_MINOR);

	MAIN_DEBUG_MSG("\n");
	MAIN_DEBUG_MSG("---- Initialize GPIO-Interface---- \n");
	LOG_MSG(NO_ERR, &myCfgInterface.log_file, "Initialize GPIO-Interface");

	gpio_initialize(&myGpioInterface);
	
	reset_device();

	GPIO_INTERFACE is_busy_pin = {
		GPIO_IS_BUSY_PIN_NUM, //u8 pin_num ;
		0,  // u8 is_initialized;
		1, // u8 is_input;
		GPIO_OFF, //u8 is_high_level;
		1, //u8 match_event_level;
		0, //u8 event_rised;
		0, //u32 sample_time_reference;
		5, // u32 sample_timeout;
		0, //u32 event_ref_time;
		0, //u32 event_timeout;
	};

	err_code = gpio_initialize(&is_busy_pin);
	if (err_code != NO_ERR ) {
		LOG_MSG(ERR_LEVEL_FATAL, &myCfgInterface.log_file, "- Initializing Busy-Pin has FAILED !!! --- (error-code = %d)", err_code);
		return err_code;
	}

	MAIN_DEBUG_MSG("\n");
	MAIN_DEBUG_MSG("---- Initialize COM-Interface---- \n");
	LOG_MSG(NO_ERR, &myCfgInterface.log_file, "Initialize Communication-Interface");

	spi_init(&myComInterface.data.spi);

	SET_MESSAGE(&myCmdInterface.message, CMD_VERSION_STR, CMD_VERSION_LEN);

	if (cmd_handler_prepare_command(&myCmdInterface) != NO_ERR) {
		myCmdInterface.is_active = 0;

	} else if (cmd_handler_send_command(&myCmdInterface, &myComInterface, &is_busy_pin) != NO_ERR) {
		myCmdInterface.is_active = 0;

	} else if (cmd_handler_receive_answer(&myCmdInterface, &myComInterface, &is_busy_pin, CMD_RX_ANSWER_TIMEOUT_MS) != NO_ERR) {
		myCmdInterface.is_active = 0;
	}

	char welcome_message[128];
	sprintf(welcome_message, "%s%s_v%d.%d", myMqttInterface.welcome_msg, myMqttInterface.client_id, myCmdInterface.answer.payload[3], myCmdInterface.answer.payload[2]);

	MAIN_DEBUG_MSG("%s\n", welcome_message);

	while (1) {

		// --- Initialize MQTT Interface
		MAIN_DEBUG_MSG("---- Initialize MQTT-Interface ---- \n");
		MAIN_DEBUG_MSG("- Host-Addr: \"%s\"\n", myMqttInterface.host_address);
		MAIN_DEBUG_MSG("- Client-ID: \"%s\"\n", myMqttInterface.client_id);

		LOG_MSG(NO_ERR, &myCfgInterface.log_file, "Initialize MQTT-Interface (Host-Addr: %s / Client-ID: %s)", myMqttInterface.host_address, myMqttInterface.client_id);
		SET_MESSAGE(&myMqttInterface.message, welcome_message, string_length(welcome_message));

		myMqttInterface.connection_lost = 1;

		if ((err_code = mqtt_init(&myMqttInterface)) != NO_ERR) {

			LOG_MSG(ERR_LEVEL_FATAL, &myCfgInterface.log_file, "Initializing MQTT-Client has FAILED !!! --- (error-code = %d)", err_code);

		} else 	if ((err_code = mqtt_connect(&myMqttInterface)) != NO_ERR) {

			LOG_MSG(ERR_LEVEL_FATAL, &myCfgInterface.log_file, "Connect to MQTT-Host has FAILED !!! --- (error-code = %d)", err_code);

		} else if ((err_code = mqtt_send_message(&myMqttInterface, &myMqttInterface.message)) != NO_ERR) {

			LOG_MSG(ERR_LEVEL_FATAL, &myCfgInterface.log_file, "Sending MQTT-Welcome-Message has FAILED !!! --- (error-code = %d)", err_code);

		} else {

			LOG_MSG(ERR_LEVEL_INFO, &myCfgInterface.log_file, "Connection to MQTT-Broker has established");

			myMqttInterface.connection_lost = 0;

			mySchedulingInterface.event.reference = mstime_get_time();
			mySchedulingInterface.report.reference = mstime_get_time();
			mySchedulingInterface.configuration.reference = mstime_get_time();
		}

		while (myMqttInterface.connection_lost == 0) {

			usleep(50000);

			if (mstime_is_time_up(mySchedulingInterface.configuration.reference, mySchedulingInterface.configuration.interval) != 0) {
				mySchedulingInterface.configuration.reference = mstime_get_time();

				if (file_has_changed(&myCfgInterface.cfg_file) != 0) {

					MAIN_DEBUG_MSG("---- Configuration File has changed ----\n");

					err_code = command_line_parser(0, NULL, &myCfgInterface, &myComInterface, &myMqttInterface, &myCmdInterface, &myGpioInterface, &mySchedulingInterface);
					if (err_code != NO_ERR) {
						LOG_MSG(ERR_LEVEL_FATAL, &myCfgInterface.log_file, "- Reload of configuration has FAILED !!! --- (error-code = %d)", err_code);
					}
				}
			}

			// --- Command Handling ---
			if (qeue_is_empty(&myCommandQeue) == 0) {

				// only if a command-message has arrived
				err_code = qeue_deqeue(&myCommandQeue, &myCmdInterface.message);
				if (err_code != NO_ERR) {
					MAIN_DEBUG_MSG("ERROR, deqeue element has FAILED !!!  (err_code = %d) ------\n", err_code);
				}

				if (myCmdInterface.message.length == MQTT_EXIT_STRING_LEN && memcmp((const void*)myCmdInterface.message.payload, MQTT_EXIT_STRING, MQTT_EXIT_STRING_LEN) == 0) {
					MAIN_DEBUG_MSG("Exit Message received - will quit program");
					LOG_MSG(ERR_LEVEL_INFO, &myCfgInterface.log_file, "Exit Message received - will quit program");
					break;
				}

				//mqtt_prepare_message(&myMqttInterface.message, &myCmdInterface.message);

				MAIN_DEBUG_MSG("---- Command Handling ---- (Message : %s)\n", myCmdInterface.message.payload);
				if (IS_EXECUTION_COMMAND(myCmdInterface.message)) {

					if ((err_code = cmd_handler_prepare_execution(&myCmdInterface)) != NO_ERR) {
						LOG_MSG(ERR_LEVEL_WARNING, &myCfgInterface.log_file, "Prepare Exec-Command has FAILED !!! --- (Exec:%s / Err:%d)", myCmdInterface.message.payload, err_code);

					} else if ((err_code = cmd_handler_run_execution(&myCmdInterface, COMMAND_INTERFACE_IGNORE_OUTPUT)) != NO_ERR) {
						LOG_MSG(ERR_LEVEL_WARNING, &myCfgInterface.log_file, "Executeion of Exec-Command has FAILED !!! --- (Exec:%s / Err:%d)", myCmdInterface.message.payload, err_code);
					}

				} else if (IS_COMMUNICATION_COMMAND(myCmdInterface.message)) {

					if (cmd_handler_prepare_command(&myCmdInterface) == NO_ERR) {

						u8 cmd_counter = 0;

						do {
							if ((err_code = cmd_handler_send_command(&myCmdInterface, &myComInterface, &is_busy_pin)) != NO_ERR) {
								LOG_MSG(ERR_LEVEL_WARNING, &myCfgInterface.log_file, "Prepare Com-Command has FAILED !!! --- (Com:%s / Err:%d)", myCmdInterface.message.payload, err_code);

							} else if ((err_code = cmd_handler_receive_answer(&myCmdInterface, &myComInterface, &is_busy_pin, CMD_RX_ANSWER_TIMEOUT_MS)) != NO_ERR) {
								LOG_MSG(ERR_LEVEL_WARNING, &myCfgInterface.log_file, "Executeion of Com-Command has FAILED !!! --- (Com:%s / Err:%d)", myCmdInterface.message.payload, err_code);

							} else if ((err_code = cmd_handler_get_error_code(&myCmdInterface)) != NO_ERR) {
								LOG_MSG(ERR_LEVEL_WARNING, &myCfgInterface.log_file, "Unexpected Status-Code --- (Com:%s / Err:%d)", myCmdInterface.message.payload, err_code);

							}

							if (++cmd_counter == CMD_DEFAULT_RESEND_COUNTER) {
								break;
							}

						} while (err_code != NO_ERR);
					}
				}
			}

			// --- Report Handling ---
			if (myCmdInterface.is_active != 0
				&& myMqttInterface.msg_delivered != 0
				&& mstime_is_time_up(mySchedulingInterface.report.reference, mySchedulingInterface.report.interval) != 0) {

				MAIN_DEBUG_MSG("---- Report Handling ---- (Time : %d) \n", mstime_get_time());
				
				while ((err_code = cmd_handler_prepare_command_from_file(&myCmdInterface, &myCmdInterface.report_file)) == NO_ERR) {

					#if DEBUG_DISABLE_REPORT_PROCESSING == 1
					MAIN_DEBUG_MSG("DEBUG_DISABLE_REPORT_PROCESSING == 1 / exit program");
					err_code = ERR_END_OF_FILE;
					break;
					#endif

					memcpy(myCmdInterface.message.payload + myCmdInterface.message.length, "=", 1);
					myCmdInterface.message.length += 1;
					
					if (cmd_handler_is_communication_command(&myCmdInterface) != 0) {
					
						err_code = cmd_handler_send_command(&myCmdInterface, &myComInterface, &is_busy_pin);
						if (err_code != NO_ERR) {
							LOG_MSG(ERR_LEVEL_WARNING, &myCfgInterface.log_file, "- Sending Report-Command has FAILED !!! --- (error-code = %d / Command: %s)", err_code, (char*)myCmdInterface.message.payload);
							restore_last_file_pointer(&myCmdInterface.report_file);
							break;
						}

						err_code = cmd_handler_receive_answer(&myCmdInterface, &myComInterface, &is_busy_pin, CMD_RX_ANSWER_TIMEOUT_MS);
						if (err_code != NO_ERR) {
							LOG_MSG(ERR_LEVEL_WARNING, &myCfgInterface.log_file, "- Receive Report-Answer has FAILED !!! --- (error-code = %d / Command: %s)", err_code, (char*)myCmdInterface.message.payload);
							restore_last_file_pointer(&myCmdInterface.report_file);
							break;
						}

						err_code = cmd_handler_get_error_code(&myCmdInterface);
						if (err_code != NO_ERR) {
							LOG_MSG(ERR_LEVEL_WARNING, &myCfgInterface.log_file, "- Status of Report-Answer unexpected --- (status-code = %d / Command: %s)", err_code, (char*)myCmdInterface.message.payload);
							restore_last_file_pointer(&myCmdInterface.report_file);
							MAIN_DEBUG_MSG("main() - Incorrect Status-Code --- (ERR: %d)\n", err_code);
							break;
						}
					
					} else if (cmd_handler_is_execution_command(&myCmdInterface) != 0) {

						if ((err_code = cmd_handler_run_execution(&myCmdInterface, COMMAND_INTERFACE_CATCH_OUTPUT)) != NO_ERR) {
							LOG_MSG(ERR_LEVEL_WARNING, &myCfgInterface.log_file, "Executeion of Exec-Command has FAILED !!! --- (Exec:%s / Err:%d)", myCmdInterface.message.payload, err_code);
						}
					}

					u8 string_is_byte_array = cmd_handler_is_communication_command(&myCmdInterface) ? 1 : 0;					
					err_code = cmd_handler_prepare_report_message(&myCmdInterface, err_code, string_is_byte_array);
					
					if (err_code != NO_ERR) {
						LOG_MSG(ERR_LEVEL_WARNING, &myCfgInterface.log_file, "- Prepare Report-Message has FAILED !!! --- (error-code = %d / Command: %s)", err_code, (char*)myCmdInterface.message.payload);
						continue;
					}

					MAIN_DEBUG_MSG("main() - Report-Answer : %s\n", myCmdInterface.message.payload);

					err_code = mqtt_send_message(&myMqttInterface, &myCmdInterface.message);
					if (err_code != NO_ERR) {
						LOG_MSG(ERR_LEVEL_FATAL, &myCfgInterface.log_file, "- Sending MQTT-Report-Message has FAILED !!! --- (error-code = %d / Command: %s)", err_code, (char*)myCmdInterface.message.payload);
						//return err_code;
					}

					if (gpio_is_event(&myGpioInterface) != 0) {
						break;
					}

					if (qeue_is_empty(&myCommandQeue) == 0) {
						break;
					}
				}

				if (err_code == ERR_FILE_OPEN) {
					MAIN_DEBUG_MSG("---- Error opening Report-File\n");
					LOG_MSG(ERR_LEVEL_FATAL, &myCfgInterface.log_file, "- File Open FAILED !!! --- (error-code = %d / File: %s)", err_code, (char*)myCmdInterface.report_file.path);						
					break;
				}
				
				if (err_code == ERR_INVALID_ARGUMENT) {
					MAIN_DEBUG_MSG("---- Invalid Argument Exception on Report-Handling\n");
					LOG_MSG(ERR_LEVEL_FATAL, &myCfgInterface.log_file, "- Report-Handling failed because of INVALID_ARGUMENT_EXCEPTION !!!");						
					break;
				}

				if (err_code == ERR_END_OF_FILE) {
					mySchedulingInterface.report.reference = mstime_get_time();
				}
			}

			// --- Event Handling ---
			if (myCmdInterface.is_active != 0
				&& gpio_is_event(&myGpioInterface) != 0
				&& mstime_is_time_up(mySchedulingInterface.event.reference, mySchedulingInterface.event.interval) != 0) {

				MAIN_DEBUG_MSG("---- Event Handling ---- (Time : %d)\n", mySchedulingInterface.event.reference);
				
				while (myMqttInterface.msg_delivered == 0 ) {
					MAIN_DEBUG_MSG(".");
					usleep(5000);
				}

				mySchedulingInterface.event.reference = mstime_get_time();

				gpio_reset_pin(&myGpioInterface);
				myGpioInterface.match_event_level = 1;

				// holds the command and message to perform event matching
				COMMAND_INTERFACE cmd_match;

				while (cmd_handler_prepare_command_from_file(&myCmdInterface, &myCmdInterface.event_file) == NO_ERR) {

					// if command was send in last iteration we do not need to send it once again
					// just go directly to matching the answer
					// since this is not a time-critical application,
					// we can live with missed events between to event-handling-iterations

					if (memcmp(cmd_match.command.payload, myCmdInterface.command.payload, myCmdInterface.command.length) == 0) {
						MAIN_DEBUG_MSG("--- Same Event Command - Will directly match the answer --- \n");
						goto EVENT_HANDLING_MATCH_EVENT_ASNWER;
					}

					memcpy(cmd_match.command.payload, myCmdInterface.command.payload, GENERAL_STRING_BUFFER_MAX_LENGTH);
					cmd_match.command.length = myCmdInterface.command.length;

					err_code = cmd_handler_send_command(&cmd_match, &myComInterface, &is_busy_pin);
					if (err_code != NO_ERR) {
						LOG_MSG(ERR_LEVEL_WARNING, &myCfgInterface.log_file, "- Sending Event-Command has FAILED !!! --- (error-code = %d / Command: %s)", err_code, (char*)myCmdInterface.message.payload);
						restore_last_file_pointer(&myCmdInterface.report_file);
						continue;
					}

					err_code = cmd_handler_receive_answer(&cmd_match, &myComInterface, &is_busy_pin, CMD_RX_ANSWER_TIMEOUT_MS);
					if (err_code != NO_ERR) {
						LOG_MSG(ERR_LEVEL_WARNING, &myCfgInterface.log_file, "- Receiving Event-Answer has FAILED !!! --- (error-code = %d / Command: %s)", err_code, (char*)myCmdInterface.message.payload);
						restore_last_file_pointer(&myCmdInterface.report_file);
						continue;
					}

					err_code = cmd_handler_get_error_code(&myCmdInterface);
					if (err_code != NO_ERR) {
						LOG_MSG(ERR_LEVEL_WARNING, &myCfgInterface.log_file, "- Status of Report-Answer unexpected --- (status-code = %d / Command: %s)", err_code, (char*)myCmdInterface.message.payload);
						restore_last_file_pointer(&myCmdInterface.report_file);
						MAIN_DEBUG_MSG("-- Incorrect Status-Code --- (ERR: %d)\n", err_code);
						continue;
					}

					EVENT_HANDLING_MATCH_EVENT_ASNWER:

					err_code = cmd_handler_match_event_answer(&myCmdInterface, &cmd_match);
					if (err_code != NO_ERR) {
						//LOG_MSG(ERR_LEVEL_WARNING, &myCfgInterface.log_file, "- Event not recognized !!! --- (error-code = %d)\n", err_code);
						continue;
					}

					err_code = mqtt_send_message(&myMqttInterface, &myCmdInterface.message);
					if (err_code != NO_ERR) {
						LOG_MSG(ERR_LEVEL_WARNING, &myCfgInterface.log_file, "- Sending MQTT-Report-Message has FAILED !!! --- (error-code = %d / Command: %s)", err_code, (char*)myCmdInterface.message.payload);
						continue;
					}
				}
			}
			
			if (myMqttInterface.connection_lost != 0) {
				MAIN_DEBUG_MSG("---- MQTT-CONNECTION LOST !!! ----\n");
				LOG_MSG(ERR_LEVEL_FATAL, &myCfgInterface.log_file, "Connection to MQTT-Broker lost - Trying to reconnect!");
			}
		}		
			
		if (myMqttInterface.connection_lost != 0) {
			LOG_MSG(ERR_LEVEL_FATAL, &myCfgInterface.log_file, "Try reconnecting to MQTT-Broker!");

			MQTTClient_disconnect(myMqttInterface.client, 10000);
			MQTTClient_destroy(&myMqttInterface.client);

			usleep(500000);
			
		} else {
		
			MAIN_DEBUG_MSG("Fatal Error - Exit Program");
			LOG_MSG(ERR_LEVEL_FATAL, &myCfgInterface.log_file, "Fatal Error - Exit Program");
			break;
		}
	}

	spi_deinit(&myComInterface.data.spi);

	MAIN_DEBUG_MSG("- Disconnected from SmartHomeBroker.\n");

	return 0;
}

// -------- COMMAND-LINE PARSING --------------------------------------------------------

u8 command_line_parser(int argc, char* argv[], CFG_INTERFACE* p_cfg_interface, COM_INTERFACE* p_com_interface, MQTT_INTERFACE* p_mqtt_interface, COMMAND_INTERFACE* p_cmd_interface, GPIO_INTERFACE* p_gpio_interface, SCHEDULING_INTERFACE* p_scheduling_interface) {

	// --- Initialize Communication-Interface

	p_scheduling_interface->report.interval = REPORT_SCHEDULE_DEFAULT_INTERVAL_MS;
	p_scheduling_interface->event.interval = EVENT_SCHEDULE_DEFAULT_INTERVAL_MS;
	p_scheduling_interface->configuration.interval = CONFIGURATION_SCHEDULE_DEFAULT_INTERVAL_MS;

	// 48000  4800 	9600 	38400 	115200
	p_com_interface->data.spi.speed_hz = 9600;
	p_com_interface->data.spi.bits_per_word = 8;
	p_com_interface->data.spi.delay = 0;
	p_com_interface->data.spi.mode = SPI_MODE_3;

	p_mqtt_interface->quality_of_service = MQTT_QOS;
	p_mqtt_interface->msg_delivered = 1;

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

	p_cmd_interface->command_file.handle = 0;
	p_cmd_interface->command_file.act_file_pointer = 0;
	p_cmd_interface->report_file.handle = 0;
	p_cmd_interface->report_file.act_file_pointer = 0;
	p_cmd_interface->event_file.handle = 0;
	p_cmd_interface->event_file.act_file_pointer = 0;
	p_cmd_interface->is_active = 1;

	p_cfg_interface->log_file.act_file_pointer = 0;

	memset(p_cfg_interface->cfg_file.path, 0x00, FILE_PATH_MAX_STRING_LENGTH);
	memcpy(p_cfg_interface->cfg_file.path, CONFIGURATION_FILE_PATH, string_length(CONFIGURATION_FILE_PATH));

	u8 i = 0;
	for ( ; i < argc; i++) {

		if (memcmp(argv[i], COMMAND_LINE_ARGUMENT_CFG_FILE, string_length(COMMAND_LINE_ARGUMENT_CFG_FILE)) == 0) {

			if (i + 1 >= argc) {
				break;
			}

			memset(p_cfg_interface->cfg_file.path, 0x00, FILE_PATH_MAX_STRING_LENGTH);
			memcpy(p_cfg_interface->cfg_file.path, argv[i + 1], string_length(argv[i + 1]));

			MAIN_CFG_DEBUG_MSG("Using Config-File: %s\n", p_cfg_interface->cfg_file.path);
		}
	}

	char path[64];
	sprintf(path, "%s", p_cfg_interface->cfg_file.path);

	FILE* config_file_handle = fopen((const char*)path, "r");
	if (config_file_handle == NULL) {
		MAIN_CFG_DEBUG_MSG("--- Open Configuration-File has FAILED !!! --- (FILE: %s / ERROR: %d)\n", path,  EXIT_FAILURE);
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

		MAIN_CFG_DEBUG_MSG("key:%s : value:%s\n", cfg_key, cfg_value);

		if (memcmp(cfg_key, CFG_NAME_MQTT_HOST_ADDR, length_key) == 0) {
			memcpy(p_mqtt_interface->host_address, cfg_value, MQTT_HOST_ADDRESS_STRING_LENGTH);

		} else

		if (memcmp(cfg_key, CFG_NAME_MQTT_CLINET_ID, length_key) == 0) {
			memcpy(p_mqtt_interface->client_id, cfg_value, MQTT_CLIENT_ID_STRING_LENGTH);

		} else

		if (memcmp(cfg_key, CFG_NAME_MQTT_TOPIC_NAME, length_key) == 0) {
			memcpy(p_mqtt_interface->topic_name, cfg_value, MQTT_TOPIC_NAME_STRING_LENGTH);
		} else

		if (memcmp(cfg_key, CFG_NAME_MQTT_WELCOME_MESSAGE, length_key) == 0) {
			memcpy(p_mqtt_interface->welcome_msg, cfg_value, MQTT_WELCOME_MSG_STRING_LENGTH);
		} else

		if (memcmp(cfg_key, CFG_NAME_MQTT_TIMEOUT, length_key) == 0) {
			p_mqtt_interface->timeout_ms = MQTT_TIMEOUT;

		} else

		if (memcmp(cfg_key, CFG_NAME_COMMUNICATION_TYPE, length_key) == 0) {
			p_com_interface->type = SPI;

		} else

		if (memcmp(cfg_key, CFG_NAME_COM_SPI_BAUDRATE, length_key) == 0) {
			char *ptr;
			p_com_interface->data.spi.speed_hz = (u32)strtol(cfg_value, &ptr, 10);
		} else

		if (memcmp(cfg_key, CFG_NAME_COM_SPI_DEVICE, length_key) == 0) {
			memcpy(p_com_interface->data.spi.device, cfg_value, COM_DEVICE_NAME_STRING_LENGTH);
		} else

		if (memcmp(cfg_key, CFG_NAME_LOG_FILE_PATH, length_key) == 0) {
			memcpy(p_cfg_interface->log_file.path, cfg_value, FILE_PATH_MAX_STRING_LENGTH);
		} else

		if (memcmp(cfg_key, CFG_NAME_COMMAND_FILE_PATH, length_key) == 0) {
			memcpy(p_cmd_interface->command_file.path, cfg_value, FILE_PATH_MAX_STRING_LENGTH);
		} else

		if (memcmp(cfg_key, CFG_NAME_REPORT_FILE_PATH, length_key) == 0) {
			memcpy(p_cmd_interface->report_file.path, cfg_value, FILE_PATH_MAX_STRING_LENGTH);
		} else

		if (memcmp(cfg_key, CFG_NAME_EVENT_FILE_PATH, length_key) == 0) {
			memcpy(p_cmd_interface->event_file.path, cfg_value, FILE_PATH_MAX_STRING_LENGTH);
		} else

		if (memcmp(cfg_key, CFG_NAME_EXECUTION_FILE_PATH, length_key) == 0) {
			memcpy(p_cmd_interface->execution_file.path, cfg_value, FILE_PATH_MAX_STRING_LENGTH);
		} else

		if (memcmp(cfg_key, CFG_NAME_SCHEDULE_INTERVAL_REPORT_MS, length_key) == 0) {
			char *ptr;
			p_scheduling_interface->report.interval = (u32)strtol(cfg_value, &ptr, 10);
		} else

		if (memcmp(cfg_key, CFG_NAME_SCHEDULE_INTERVAL_EVENT_MS, length_key) == 0) {
			char *ptr;
			p_scheduling_interface->event.interval = (u32)strtol(cfg_value, &ptr, 10);
		} else

		if (memcmp(cfg_key, CFG_NAME_SCHEDULE_INTERVAL_CONFIG_MS, length_key) == 0) {
			char *ptr;
			p_scheduling_interface->configuration.interval = (u32)strtol(cfg_value, &ptr, 10);
		}

		else {
			MAIN_CFG_DEBUG_MSG("--- UNKNOWN CFG-KEY : %s\n", cfg_key);
		}
		
		NEXT_CONFIG_LINE:
		num_bytes = read_line(config_file_handle, line, 512);
	}

	return NO_ERR;
}

void command_line_usage(void) {

}


static void reset_device(void) {

	static GPIO_INTERFACE reset_pin = {
		GPIO_RESET_PIN_NUM, //u8 pin_num ;
		0,  // u8 is_initialized;
		0, // u8 is_input;
		0, //u8 is_high_level;
		0, //u8 match_event_level;
		0, //u8 event_rised;
		0, //u32 sample_time_reference;
		5, // u32 sample_timeout;
		0, //u32 event_ref_time;
		0, //u32 event_timeout;
	};
	
	if (reset_pin.is_initialized == 0) {
	
		u8 err_code = gpio_initialize(&reset_pin);
		if (err_code != NO_ERR ) {
			MAIN_DEBUG_MSG("- Initializing Reset-Pin has FAILED !!! --- (error-code = %d)", err_code);
			return;
		}
	}

	MAIN_DEBUG_MSG("---- RESETTING DEVICE ----\n");

	gpio_set_state(&reset_pin, GPIO_OFF);
	
	u32 time_reference_ms = mstime_get_time();	
	while (mstime_is_time_up(time_reference_ms, DEVICE_RESET_TIME_MS) != 0);
	
	gpio_set_state(&reset_pin, GPIO_ON);
	
	time_reference_ms = mstime_get_time();	
	while (mstime_is_time_up(time_reference_ms, DEVICE_STARTUP_TIME_MS) != 0);
}

