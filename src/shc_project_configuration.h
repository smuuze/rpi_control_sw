/* ------------------------------------------------------------------------------
 *	Project	: Smart Home Client
 *	File	:
 *	Date	:
 *	Author	: Sebastian Lesse
 *	Brief	:
 * ------------------------------------------------------------------------------
 */

#ifndef _SHC_PROJECT_CONFIGURATION_H_
#define _SHC_PROJECT_CONFIGURATION_H_

// -------- DEFINITIONS -----------------------------------------------------------------
#ifndef VERSION_MAJOR
#define VERSION_MAJOR				1
#endif

#ifndef VERSION_MINOR
#define VERSION_MINOR				0
#endif

#define GENERAL_STRING_BUFFER_MAX_LENGTH	256
#define GENERAL_STRING_BIG_BUFFER_MAX_LENGTH	256
#define GENERAL_MAX_NUMBER_OF_MSG_IN_QEUE	10

#define COM_DEVICE_NAME_STRING_LENGTH		15

#define COMMAND_LINE_ARGUMENT_CFG_FILE		"-cfg"
#define COMMAND_LINE_ARGUMENT_LCD		"-lcd"
#define COMMAND_LINE_ARGUMENT_CONTROLLER	"-control"
#define COMMAND_LINE_ARGUMENT_FILE		"-file"
#define COMMAND_LINE_ARGUMENT_MQTT		"-mgtt"
#define COMMAND_LINE_ARGUMENT_PATH		"-path"
#define COMMAND_LINE_ARGUMENT_CONSOLE		"-console"

#define FILE_PATH_MAX_STRING_LENGTH		256

#define LOG_FILE_MAX_SIZE_BYTES			(10 * 1024 * 1024)

#define CONFIGURATION_FILE_PATH			"smart_home_configuration_file.txt"
#define COMMAND_FILE_PATH			"smart_home_command_file.txt"
#define REPORT_FILE_PATH			"smart_home_report_file.txt"
#define TRACE_FILE_DEFAULT_PATH			"trace_file.txt"
#define TRACE_DEFAULT_BASE_PATH			"../"

#define CMD_RX_ANSWER_TIMEOUT_MS		1000
#define CMD_TX_COMMAND_TIMEOUT_MS		1000
#define CMD_ACTIVATE_TIMEOUT_MS			250
#define CMD_REQUEST_TIME_MS			60
#define CMD_MAXIMUM_COM_FAIL_COUNT		5

#define MQTT_CONNECTION_LOST_TIMEOUT_MS		30000
#define MQTT_CONNECTION_INTERVAL_TIMEOUT_MS	300000

#define BOARD_CONNECTION_INTERVAL_TIMEOUT_MS	15000

#define DEVICE_RESET_TIME_MS			10
#define DEVICE_STARTUP_TIME_MS			150
#define DEVICE_STARTUP_TIMEOUT_MS		5000

#define CMD_DEFAULT_RESEND_COUNTER		3

#define GPIO_CE0_PIN_NUM			6

#define GPIO_RESET_PIN_NUM			5

#define GPIO_IS_BUSY_PIN_NUM			0
#define GPIO_REQUEST_PIN_NUM			0
#define GPIO_SAMPLE_PIN				29
#define GPIO_EVENT_PIN				21

#define config_MAX_LENGTH_OF_FILE_LINE		1024

// -------- LCD-Display -----------------------------------------------------------------

#define LCD_NUM_LINES				2
#define LCD_NUM_CHARS				16
#define LCD_PIN_NUMBER_RS			24 /* 19 */
#define LCD_PIN_NUMBER_EN			25 /* 26 */
#define LCD_PIN_NUMBER_D4			26 /* 12 */
#define LCD_PIN_NUMBER_D5			27 /* 16 */
#define LCD_PIN_NUMBER_D6			28 /* 20 */
#define LCD_PIN_NUMBER_D7			29 /* 21 */

#endif // _SHC_PROJECT_CONFIGURATION_H_
