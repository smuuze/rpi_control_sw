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

#define GENERAL_STRING_BUFFER_MAX_LENGTH	100
#define GENERAL_MAX_NUMBER_OF_MSG_IN_QEUE	10

#define COM_DEVICE_NAME_STRING_LENGTH		15

#define COMMAND_LINE_ARGUMENT_CFG_FILE		"-cfg"

#define FILE_PATH_MAX_STRING_LENGTH		64

#define CONFIGURATION_FILE_PATH			"smart_home_configuration_file.txt"
#define COMMAND_FILE_PATH			"smart_home_command_file.txt"
#define REPORT_FILE_PATH			"smart_home_report_file.txt"

#define CMD_RX_ANSWER_TIMEOUT_MS		1000
#define CMD_TX_COMMAND_TIMEOUT_MS		1000
#define CMD_ACTIVATE_TIMEOUT_MS			250

#define CMD_DEFAULT_RESEND_COUNTER		3

#define GPIO_CE0_PIN_NUM			6

#endif // _SHC_PROJECT_CONFIGURATION_H_