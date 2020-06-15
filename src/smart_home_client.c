
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
#include "shc_lcd_interface.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <linux/spi/spidev.h>

#include <time.h>

// -------- SCHEDULE INTERVAL DEFAULTS --------------------------------------------------

#define REPORT_SCHEDULE_DEFAULT_INTERVAL_MS		60000
#define EVENT_SCHEDULE_DEFAULT_INTERVAL_MS		1500
#define CONFIGURATION_SCHEDULE_DEFAULT_INTERVAL_MS	60000

// -------- DEBUGGING -------------------------------------------------------------------

#define MAIN_DEBUG_MSG					DEBUG_MSG
#define MAIN_CFG_DEBUG_MSG				noDEBUG_MSG

// -------- Command-Code ----------------------------------------------------------------

#define CMD_VERSION_STR					"version"
#define CMD_VERSION_LEN					7

// --------------------------------------------------------------------------------------

static void main_reset_control_board(CFG_INTERFACE* p_cfgInterface);


/*!
 *
 */
static void main_connect_control_board(CFG_INTERFACE* p_cfgInterface, COMMAND_INTERFACE* p_cmdInterface, COM_INTERFACE* p_comInterface);


/*!
 *
 */
static void main_connect_mqtt_host(MQTT_INTERFACE* p_mqttInterface, CFG_INTERFACE* p_cfgInterface);


/*!
 *
 */
u8 command_line_parser(int argc, char* argv[], CFG_INTERFACE* p_cfg_interface, COM_INTERFACE* p_com_interface, MQTT_INTERFACE* p_mqtt_interface, COMMAND_INTERFACE* p_cmd_interface, GPIO_INTERFACE* p_gpio_interface, SCHEDULING_INTERFACE* p_scheduling_interface);


/*!
 *
 */
void command_line_usage(void);


// -------- STATIC DATA -----------------------------------------------------------------


GPIO_INTERFACE_BUILD_INOUT(RESET_PIN, GPIO_RESET_PIN_NUM)
GPIO_INTERFACE_BUILD_INOUT(REQUEST_PIN, GPIO_REQUEST_PIN_NUM)
GPIO_INTERFACE_BUILD_INOUT(EVENT_PIN, GPIO_REQUEST_PIN_NUM)

TIME_MGMN_BUILD_TIMER(RESET_TIMER)
TIME_MGMN_BUILD_TIMER(MQTT_CONNECT_TIMER)
TIME_MGMN_BUILD_TIMER(BOARD_CONNECT_TIMER)
TIME_MGMN_BUILD_TIMER(REPORT_TIMER)
TIME_MGMN_BUILD_TIMER(DATETIME_TIMER)

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

	MAIN_DEBUG_MSG("Welcome to the SmartHomeClient v%d.%d\n", VERSION_MAJOR, VERSION_MINOR);

	qeue_init(&myCommandQeue);

	#if DEBUG_DISABLE_REPORT_PROCESSING == 1
	MAIN_DEBUG_MSG("main() - REPORT PROCESSING IS DISABLED !!! ---\n");
	#endif

	MAIN_DEBUG_MSG("main() - LOADING CONFIGURATION\n");

	// --- Parsing Command-Line Arguments
	u8 err_code = command_line_parser(argc, argv, &myCfgInterface, &myComInterface, &myMqttInterface, &myCmdInterface, &myGpioInterface, &mySchedulingInterface);
	if (err_code != NO_ERR) {
		command_line_usage();
		return err_code;
	}

	char welcome_message[128];
	sprintf(welcome_message, "%s%s_v%d.%d", myMqttInterface.welcome_msg, myMqttInterface.client_id, myCmdInterface.answer.payload[2], myCmdInterface.answer.payload[3]);
	MAIN_DEBUG_MSG("main() - Welcome message: \"%s\"\n", welcome_message);
	
	LOG_MSG(NO_ERR, &myCfgInterface.log_file, "Starting SmartHomeClient Deamon v%d.%d", VERSION_MAJOR, VERSION_MINOR);
	
	LCD_PRINTF("Welcome to:");
	LCD_PRINTF("SHC v%d.%d", VERSION_MAJOR, VERSION_MINOR);

	MAIN_DEBUG_MSG("main() - INITIALIZE GPIO INTERFACE\n");
	LOG_MSG(NO_ERR, &myCfgInterface.log_file, "Initialize GPIO-Interface");
	LCD_PRINTF("INIT: GPIO");

	gpio_initialize(&myGpioInterface);

	LCD_PRINTF("... OK");

	MAIN_DEBUG_MSG("main() - INITIALIZE CFG INTERFACE\n");
	LOG_MSG(NO_ERR, &myCfgInterface.log_file, "Initialize Command-Interface");
	LCD_PRINTF("INIT: CMD");

	cmd_handler_init(&myCfgInterface);

	LCD_PRINTF("... OK");

	myMqttInterface.initialized = 0;
	myMqttInterface.connection_lost = 1;

	LOG_MSG(NO_ERR, &myCfgInterface.log_file, "Starting Application");

	REPORT_TIMER_start();
	DATETIME_TIMER_start();

	while (1) {

		usleep(50000); // reduce cpu-load

		if (myCmdInterface.is_active == 0) {			
			main_connect_control_board(&myCfgInterface, &myCmdInterface, &myComInterface);
		}

		if (myMqttInterface.connection_lost) {
			SET_MESSAGE(&myMqttInterface.message, welcome_message, string_length(welcome_message));
			main_connect_mqtt_host(&myMqttInterface, &myCfgInterface);
		}		

		while (myMqttInterface.connection_lost == 0) {

			usleep(50000); // reduce cpu-load

			mqtt_keep_alive();

			if (mstime_is_time_up(mySchedulingInterface.configuration.reference, mySchedulingInterface.configuration.interval) != 0) {
				mySchedulingInterface.configuration.reference = mstime_get_time();

				if (file_has_changed(&myCfgInterface.cfg_file) != 0) {

					MAIN_DEBUG_MSG("main() - Configuration File has changed\n");

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
					MAIN_DEBUG_MSG("main() - deqeue element has FAILED !!!  (err_code = %d) ------\n", err_code);
				}

				if (myCmdInterface.message.length == MQTT_EXIT_STRING_LEN && memcmp((const void*)myCmdInterface.message.payload, MQTT_EXIT_STRING, MQTT_EXIT_STRING_LEN) == 0) {
					MAIN_DEBUG_MSG("main() - Exit Message received - will quit program");
					LOG_MSG(ERR_LEVEL_INFO, &myCfgInterface.log_file, "Exit Message received - will quit program");
					break;
				}

				//mqtt_prepare_message(&myMqttInterface.message, &myCmdInterface.message);

				MAIN_DEBUG_MSG("main() - COMMAND HANDLING - Message : \"%s\"\n", myCmdInterface.message.payload);
				LCD_PRINTF("Command Handling");

				if (IS_EXECUTION_COMMAND(myCmdInterface.message)) {

					if ((err_code = cmd_handler_prepare_execution(&myCmdInterface)) != NO_ERR) {
						LOG_MSG(ERR_LEVEL_WARNING, &myCfgInterface.log_file, "Prepare Exec-Command has FAILED !!! --- (Exec:%s / Err:%d)", myCmdInterface.message.payload, err_code);

					} else if ((err_code = cmd_handler_run_execution(&myCmdInterface, COMMAND_INTERFACE_IGNORE_OUTPUT)) != NO_ERR) {
						LOG_MSG(ERR_LEVEL_WARNING, &myCfgInterface.log_file, "Executeion of Exec-Command has FAILED !!! --- (Exec:%s / Err:%d)", myCmdInterface.message.payload, err_code);
					}

				} else if ( (IS_COMMUNICATION_COMMAND(myCmdInterface.message) != 0) && (myCmdInterface.is_active != 0) ) {

					if (cmd_handler_prepare_command(&myCmdInterface) == NO_ERR) {

						u8 cmd_counter = 0;

						do {
							if ((err_code = cmd_handler_send_command(&myCmdInterface, &myComInterface)) != NO_ERR) {
								LOG_MSG(ERR_LEVEL_WARNING, &myCfgInterface.log_file, "Prepare Com-Command has FAILED !!! --- (Com:%s / Err:%d)", myCmdInterface.message.payload, err_code);
								LCD_PRINTF("... COM ERR !!!");
							
							} else if ((err_code = cmd_handler_receive_answer(&myCmdInterface, &myComInterface, CMD_RX_ANSWER_TIMEOUT_MS)) != NO_ERR) {
								LOG_MSG(ERR_LEVEL_WARNING, &myCfgInterface.log_file, "Executeion of Com-Command has FAILED !!! --- (Com:%s / Err:%d)", myCmdInterface.message.payload, err_code);
								LCD_PRINTF("... COM ERR !!!");

							} else if ((err_code = cmd_handler_get_error_code(&myCmdInterface)) != NO_ERR) {
								LOG_MSG(ERR_LEVEL_WARNING, &myCfgInterface.log_file, "Unexpected Status-Code --- (Com:%s / Err:%d)", myCmdInterface.message.payload, err_code);
								LCD_PRINTF("... COM ERR !!!");

							} else {

								LCD_PRINTF("... OK");
							}

							if (++cmd_counter == CMD_DEFAULT_RESEND_COUNTER) {
								break;
							}

						} while (err_code != NO_ERR);
					}
				}
			}

			// --- Report Handling ---
			if (myMqttInterface.msg_delivered && REPORT_TIMER_is_up(mySchedulingInterface.report.interval)) {

				MAIN_DEBUG_MSG("main() - Report Handling - Time : %d \n", mstime_get_time());
				//LOG_MSG(NO_ERR, &myCfgInterface.log_file, "Start Report-Handling");
				LCD_PRINTF("Report Handling");
				
				while ((err_code = cmd_handler_prepare_command_from_file(&myCmdInterface, &myCmdInterface.report_file)) == NO_ERR) {

					#if DEBUG_DISABLE_REPORT_PROCESSING == 1
					MAIN_DEBUG_MSG("main() - DEBUG_DISABLE_REPORT_PROCESSING == 1 / exit program");
					err_code = ERR_END_OF_FILE;
					break;
					#endif

					memcpy(myCmdInterface.message.payload + myCmdInterface.message.length, "=", 1);
					myCmdInterface.message.length += 1;

					//LOG_MSG(ERR_LEVEL_WARNING, &myCfgInterface.log_file, "Performing Command: %s)", (char*)myCmdInterface.message.payload);
					
					if ( (cmd_handler_is_communication_command(&myCmdInterface) != 0) && (myCmdInterface.is_active != 0) ) {
					
						err_code = cmd_handler_send_command(&myCmdInterface, &myComInterface);
						if (err_code != NO_ERR) {
							MAIN_DEBUG_MSG("main() - cmd_handler_send_command() has FAILED !!! --- (ERR: %d)\n", err_code);
							LOG_MSG(ERR_LEVEL_WARNING, &myCfgInterface.log_file, "- Sending Report-Command has FAILED !!! --- (error-code = %d / Command: %s)", err_code, (char*)myCmdInterface.message.payload);
							LCD_PRINTF("... COM ERR !!!");
							restore_last_file_pointer(&myCmdInterface.report_file);
							break;
						}

						err_code = cmd_handler_receive_answer(&myCmdInterface, &myComInterface, CMD_RX_ANSWER_TIMEOUT_MS);
						if (err_code != NO_ERR) {
							MAIN_DEBUG_MSG("main() - cmd_handler_receive_answer() has FAILED !!! --- (ERR: %d)\n", err_code);
							LOG_MSG(ERR_LEVEL_WARNING, &myCfgInterface.log_file, "- Receive Report-Answer has FAILED !!! --- (error-code = %d / Command: %s)", err_code, (char*)myCmdInterface.message.payload);
							LCD_PRINTF("... COM ERR !!!");
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

					MAIN_DEBUG_MSG("main() - Error opening Report-File\n");
					LCD_PRINTF("... NO FILE !!!");

					LOG_MSG(ERR_LEVEL_FATAL, &myCfgInterface.log_file, "- File Open FAILED !!! --- (error-code = %d / File: %s)", err_code, (char*)myCmdInterface.report_file.path);
					REPORT_TIMER_start();

					break;
				}
				
				if (err_code == ERR_INVALID_ARGUMENT) {

					MAIN_DEBUG_MSG("main() - Invalid Argument Exception on Report-Handling\n");
					LCD_PRINTF("... INV ARG !!!");

					LOG_MSG(ERR_LEVEL_FATAL, &myCfgInterface.log_file, "- Report-Handling failed because of INVALID_ARGUMENT_EXCEPTION !!!");
					REPORT_TIMER_start();

					break;
				}

				if (err_code == ERR_END_OF_FILE) {
					// start timer after all report-commands have been handled
					// because there is only one command-handled within one loop-itereation
					REPORT_TIMER_start();
					LCD_PRINTF("... OK");
				}
			}

			// --- Event Handling ---
			if (myCmdInterface.is_active != 0
				&& gpio_is_event(&myGpioInterface) != 0
				&& mstime_is_time_up(mySchedulingInterface.event.reference, mySchedulingInterface.event.interval) != 0) {

				MAIN_DEBUG_MSG("main() - Event Handling (Time : %d)\n", mySchedulingInterface.event.reference);
				
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
						MAIN_DEBUG_MSG("main() - Same Event Command - Will directly match the answer --- \n");
						goto EVENT_HANDLING_MATCH_EVENT_ASNWER;
					}

					memcpy(cmd_match.command.payload, myCmdInterface.command.payload, GENERAL_STRING_BUFFER_MAX_LENGTH);
					cmd_match.command.length = myCmdInterface.command.length;

					err_code = cmd_handler_send_command(&cmd_match, &myComInterface);
					if (err_code != NO_ERR) {
						LOG_MSG(ERR_LEVEL_WARNING, &myCfgInterface.log_file, "- Sending Event-Command has FAILED !!! --- (error-code = %d / Command: %s)", err_code, (char*)myCmdInterface.message.payload);
						restore_last_file_pointer(&myCmdInterface.report_file);
						continue;
					}

					err_code = cmd_handler_receive_answer(&cmd_match, &myComInterface, CMD_RX_ANSWER_TIMEOUT_MS);
					if (err_code != NO_ERR) {
						LOG_MSG(ERR_LEVEL_WARNING, &myCfgInterface.log_file, "- Receiving Event-Answer has FAILED !!! --- (error-code = %d / Command: %s)", err_code, (char*)myCmdInterface.message.payload);
						restore_last_file_pointer(&myCmdInterface.report_file);
						continue;
					}

					err_code = cmd_handler_get_error_code(&myCmdInterface);
					if (err_code != NO_ERR) {
						LOG_MSG(ERR_LEVEL_WARNING, &myCfgInterface.log_file, "- Status of Report-Answer unexpected --- (status-code = %d / Command: %s)", err_code, (char*)myCmdInterface.message.payload);
						restore_last_file_pointer(&myCmdInterface.report_file);
						MAIN_DEBUG_MSG("main() - Incorrect Status-Code --- (ERR: %d)\n", err_code);
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

			if ((myCmdInterface.is_active != 0) && (myCmdInterface.fail_counter > CMD_MAXIMUM_COM_FAIL_COUNT - 1)) {
				MAIN_DEBUG_MSG("main() - FAIL_COUNTER = %d - Connection to Control-Board has been lost !!! ---\n", myCmdInterface.fail_counter);
				LOG_MSG(ERR_LEVEL_WARNING, &myCfgInterface.log_file, "- Connection to Control-Board has been lost !!! ---");
				LCD_PRINTF("Board lost!!!");

				myCmdInterface.is_active = 0;
				myCmdInterface.fail_counter = 0;
				break;
			}
			
			if (myMqttInterface.connection_lost != 0) {
				MAIN_DEBUG_MSG("main() - MQTT-CONNECTION LOST - ! - ! - ! -\n");
				LOG_MSG(ERR_LEVEL_FATAL, &myCfgInterface.log_file, "Connection to MQTT-Broker lost!");
				LCD_PRINTF("Mqtt lost!!!");
				break;
			}
		}
	}		
			
	if (myMqttInterface.connection_lost != 0) {
		//LOG_MSG(ERR_LEVEL_FATAL, &myCfgInterface.log_file, "Try reconnecting to MQTT-Broker!");

		MQTTClient_disconnect(myMqttInterface.client, 10000);
		MQTTClient_destroy(&myMqttInterface.client);		
	}

	spi_deinit(&myComInterface.data.spi);

	MAIN_DEBUG_MSG("main() - Disconnected from SmartHomeBroker.\n");

	return 0;
}

// -------- COMMAND-LINE PARSING --------------------------------------------------------

u8 command_line_parser(int argc, char* argv[], CFG_INTERFACE* p_cfg_interface, COM_INTERFACE* p_com_interface, MQTT_INTERFACE* p_mqtt_interface, COMMAND_INTERFACE* p_cmd_interface, GPIO_INTERFACE* p_gpio_interface, SCHEDULING_INTERFACE* p_scheduling_interface) {

	// --- Initialize Communication-Interface

	p_scheduling_interface->report.interval = REPORT_SCHEDULE_DEFAULT_INTERVAL_MS;
	p_scheduling_interface->event.interval = EVENT_SCHEDULE_DEFAULT_INTERVAL_MS;
	p_scheduling_interface->configuration.interval = CONFIGURATION_SCHEDULE_DEFAULT_INTERVAL_MS;

	// 48000  4800 	9600 	38400 	115200
	p_com_interface->is_enabled = 0;
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
	p_cmd_interface->is_active = 0;
	p_cmd_interface->fail_counter = 0;

	p_cfg_interface->log_file.act_file_pointer = 0;

	memset(p_cfg_interface->cfg_file.path, 0x00, FILE_PATH_MAX_STRING_LENGTH);
	memcpy(p_cfg_interface->cfg_file.path, CONFIGURATION_FILE_PATH, string_length(CONFIGURATION_FILE_PATH));

	u8 i = 0;
	for ( ; i < argc; i++) {

		MAIN_CFG_DEBUG_MSG("command_line_parser() - Parsing cli-argument: %s\n", argv[i]);

		if (memcmp(argv[i], COMMAND_LINE_ARGUMENT_CFG_FILE, string_length(COMMAND_LINE_ARGUMENT_CFG_FILE)) == 0) {

			if (i + 1 >= argc) {
				break;
			}

			memset(p_cfg_interface->cfg_file.path, 0x00, FILE_PATH_MAX_STRING_LENGTH);
			memcpy(p_cfg_interface->cfg_file.path, argv[i + 1], string_length(argv[i + 1]));

			MAIN_CFG_DEBUG_MSG("Using Config-File: %s\n", p_cfg_interface->cfg_file.path);
			
			// do not process the parameter as a new argument
			i += 1;

		} else if (memcmp(argv[i], COMMAND_LINE_ARGUMENT_LCD, string_length(COMMAND_LINE_ARGUMENT_LCD)) == 0) {
			MAIN_CFG_DEBUG_MSG("command_line_parser() - Enabling LCD\n");
			lcd_set_enabled(1);

		} else if (memcmp(argv[i], COMMAND_LINE_ARGUMENT_CONTROLLER, string_length(COMMAND_LINE_ARGUMENT_CONTROLLER)) == 0) {
			MAIN_CFG_DEBUG_MSG("command_line_parser() - Enabling Control-Board\n");
		}
	}

	char path[128];
	sprintf(path, "%s", p_cfg_interface->cfg_file.path);

	FILE* config_file_handle = fopen((const char*)p_cfg_interface->cfg_file.path, "r");
	if (config_file_handle == NULL) {
		MAIN_CFG_DEBUG_MSG("--- Open Configuration-File has FAILED !!! --- (FILE: %s / ERROR: %d)\n", p_cfg_interface->cfg_file.path,  EXIT_FAILURE);
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
			MAIN_CFG_DEBUG_MSG("command_line_parser() - Ignoring line: %s\n", line);
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

		MAIN_CFG_DEBUG_MSG("command_line_parser() - key:%s : value:%s\n", cfg_key, cfg_value);

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
			p_com_interface->is_enabled = 1;
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
		} else

		if (memcmp(cfg_key, CFG_NAME_LCD_ENABLE, length_key) == 0) {
			char *ptr;
			lcd_set_enabled((u32)strtol(cfg_value, &ptr, 10));
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


static void main_reset_control_board(CFG_INTERFACE* p_cfgInterface) {

	MAIN_DEBUG_MSG("main_reset_control_board() - RESETTING DEVICE !!!\n");
	LOG_MSG(NO_ERR, &p_cfgInterface->log_file, "Going to reset Control-Board");

	//RESET_PIN_drive_low();	
	//RESET_TIMER_start();	
	
	while (RESET_TIMER_is_up(DEVICE_RESET_TIME_MS) == 0) { usleep(5000); }

	//REQUEST_PIN_no_pull();
	//RESET_PIN_pull_up();

	RESET_TIMER_start();
	
	while (RESET_TIMER_is_up(DEVICE_STARTUP_TIME_MS) == 0) { usleep(5000); }
	
	while (REQUEST_PIN_is_low_level()) {
					
		usleep(5000);

		if (RESET_TIMER_is_up(DEVICE_STARTUP_TIMEOUT_MS) != 0) {
			MAIN_DEBUG_MSG("main_reset_control_board() - Reset Control-Board  has FAILED !!! --- (Timeout)\n");
			break;
		}
	}
	
	MAIN_DEBUG_MSG("main_reset_control_board() - Device ready after %d ms\n", RESET_TIMER_elapsed());
	LOG_MSG(NO_ERR, &p_cfgInterface->log_file, "Reset of Control-Board done");
}

static void main_connect_control_board(CFG_INTERFACE* p_cfgInterface, COMMAND_INTERFACE* p_cmdInterface, COM_INTERFACE* p_comInterface) {

	if (BOARD_CONNECT_TIMER_is_active() == 0) {
		BOARD_CONNECT_TIMER_start();
	}

	if (BOARD_CONNECT_TIMER_is_up(BOARD_CONNECTION_INTERVAL_TIMEOUT_MS) == 0) {
		//MAIN_DEBUG_MSG("main_connect_control_board() - Waiting for BOARD_CONNECTION_INTERVAL_TIMEOUT_MS\n");
		return;
	}
		
	BOARD_CONNECT_TIMER_start();
		
	if (p_comInterface->is_enabled == 0) {
		MAIN_DEBUG_MSG("main_connect_control_board() - COMMUNICATION INTERFACE is not enabled!\n");
		LOG_MSG(NO_ERR, &p_cfgInterface->log_file, "Communication-Interface is not active");
		return;
	}
	
	LOG_MSG(NO_ERR, &myCfgInterface.log_file, "Initialize SPI-Interface");
	LCD_PRINTF("INIT: SPI");

	spi_init(&myComInterface.data.spi);

	LCD_PRINTF("... OK");

	REQUEST_PIN_init();
	REQUEST_PIN_pull_up();

	RESET_PIN_init();
	RESET_PIN_pull_up();

	main_reset_control_board(p_cfgInterface);

	p_cmdInterface->is_active = 1;

	MAIN_DEBUG_MSG("main_connect_control_board() - INITIALIZE COMMUNICATION INTERFACE\n");
	LOG_MSG(NO_ERR, &p_cfgInterface->log_file, "Initialize Communication-Interface");
	LCD_PRINTF("INIT: BOARD");

	SET_MESSAGE(&p_cmdInterface->message, CMD_VERSION_STR, CMD_VERSION_LEN);

	if (cmd_handler_prepare_command(p_cmdInterface) != NO_ERR) {

		MAIN_DEBUG_MSG("main_connect_control_board() - Set Command-Interface to inactive - preparing command has FAILED !!! ---  \n");
		LOG_MSG(ERR_LEVEL_FATAL, &p_cfgInterface->log_file, "Set Command-Interface to inactive - preparing command has FAILED");
		LCD_PRINTF("err: PREPARE");

		p_cmdInterface->is_active = 0;
		return;
	}

	LOG_MSG(NO_ERR, &p_cfgInterface->log_file, "Sending Version-Command");

	if (cmd_handler_send_command(p_cmdInterface, p_comInterface) != NO_ERR) {

		MAIN_DEBUG_MSG("main_connect_control_board() - Set Command-Interface to inactive - sending command has FAILED !!! ---  \n");
		LOG_MSG(ERR_LEVEL_FATAL, &p_cfgInterface->log_file, "Set Command-Interface to inactive - sending command has FAILED");
		LCD_PRINTF("err: SEND");

		p_cmdInterface->is_active = 0;
		return;
	}

	LOG_MSG(NO_ERR, &p_cfgInterface->log_file, "Receiving Answer of Version-Command");
	
	if (cmd_handler_receive_answer(p_cmdInterface, p_comInterface, CMD_RX_ANSWER_TIMEOUT_MS) != NO_ERR) {

		MAIN_DEBUG_MSG("main_connect_control_board() - Set Command-Interface to inactive - receiving answer has FAILED !!! ---  \n");
		LOG_MSG(ERR_LEVEL_FATAL, &p_cfgInterface->log_file, "Set Command-Interface to inactive - receiving answer has FAILED");
		LCD_PRINTF("err: RECEIVE");

		p_cmdInterface->is_active = 0;
		return;
	}

	MAIN_DEBUG_MSG("main_connect_control_board() - Control-Board is connected\n");
	LCD_PRINTF("... OK");
	LOG_MSG(NO_ERR, &myCfgInterface.log_file, "Control-Board is connected");

	BOARD_CONNECT_TIMER_stop();
}

static void main_connect_mqtt_host(MQTT_INTERFACE* p_mqttInterface, CFG_INTERFACE* p_cfgInterface) {

	if (p_mqttInterface->connection_lost == 0) {
		return;
	}

	if (MQTT_CONNECT_TIMER_is_active() == 0) {
		if (MQTT_CONNECT_TIMER_is_up(MQTT_CONNECTION_INTERVAL_TIMEOUT_MS) == 0) {
			//MAIN_DEBUG_MSG("main_connect_mqtt_host() - Waiting for MQTT_CONNECTION_INTERVAL_TIMEOUT_MS\n");
			return;
		}
	}
			
	MQTT_CONNECT_TIMER_start();

	// --- Initialize MQTT Interface
	MAIN_DEBUG_MSG("main_connect_mqtt_host() - INITIALIZE MQTT INTERFACE\n");
	MAIN_DEBUG_MSG("                         - Host-Addr: \"%s\"\n", p_mqttInterface->host_address);
	MAIN_DEBUG_MSG("                         - Client-ID: \"%s\"\n", p_mqttInterface->client_id);

	LOG_MSG(NO_ERR, &myCfgInterface.log_file, "Initialize MQTT-Interface (Host-Addr: %s / Client-ID: %s)", p_mqttInterface->host_address, p_mqttInterface->client_id);
	LCD_PRINTF("INIT: MQTT");

	u8 err_code = 0;

	if ((err_code = mqtt_init(p_mqttInterface)) != NO_ERR) {
			
		MAIN_DEBUG_MSG("main_connect_mqtt_host() - Initializing MQTT-Client has FAILED !!! - error-code = %d\n", err_code);
		LOG_MSG(ERR_LEVEL_FATAL, &p_cfgInterface->log_file, "Initializing MQTT-Client has FAILED !!! --- (error-code = %d)", err_code);
		LCD_PRINTF("err: INIT");

	} else 	if ((err_code = mqtt_connect(p_mqttInterface)) != NO_ERR) {

		MAIN_DEBUG_MSG("main_connect_mqtt_host() - Connect to MQTT-Host has FAILED !!! - error-code = %d\n", err_code);
		LOG_MSG(ERR_LEVEL_FATAL, &p_cfgInterface->log_file, "Connect to MQTT-Host has FAILED !!! --- (error-code = %d)", err_code);
		LCD_PRINTF("err: CONNECT");

	} else if ((err_code = mqtt_send_message(&myMqttInterface, &p_mqttInterface->message)) != NO_ERR) {

		MAIN_DEBUG_MSG("main_connect_mqtt_host() - Sending MQTT-Welcome-Message has FAILED !!! - error-code = %d\n", err_code);
		LOG_MSG(ERR_LEVEL_FATAL, &p_cfgInterface->log_file, "Sending MQTT-Welcome-Message has FAILED !!! --- (error-code = %d)", err_code);
		LCD_PRINTF("err: SEND");

	} else {

		MAIN_DEBUG_MSG("main_connect_mqtt_host() - Connection to MQTT-Broker has benn established\n");
		LOG_MSG(ERR_LEVEL_INFO, &p_cfgInterface->log_file, "Connection to MQTT-Broker has been established");
		LCD_PRINTF("... OK");

		p_mqttInterface->connection_lost = 0;
		p_mqttInterface->initialized = 1;

		mySchedulingInterface.event.reference = mstime_get_time();
		mySchedulingInterface.report.reference = mstime_get_time();
		mySchedulingInterface.configuration.reference = mstime_get_time();
			
		MQTT_CONNECT_TIMER_stop();
	}
}
