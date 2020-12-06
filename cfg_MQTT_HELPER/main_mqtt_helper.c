/*! 
 * --------------------------------------------------------------------------------
 *
 * \file	main_mqtt_helper.c
 * \brief
 * \author	sebastian lesse
 *
 * --------------------------------------------------------------------------------
 */

#define TRACER_OFF

// --------------------------------------------------------------------------------------

#include "config.h"

// --------------------------------------------------------------------------------------

#include "tracer.h"

// --------------------------------------------------------------------------------------

#include "cpu.h"

#include <stdio.h>

// --------------------------------------------------------------------------------------

#include "initialization/initialization.h"
#include "common/signal_slot_interface.h"
#include "common/common_types.h"
#include "mcu_task_management/mcu_task_controller.h"
#include "ui/command_line/command_line_interface.h"
#include "ui/console/ui_console.h"
#include "ui/lcd/ui_lcd_interface.h"
#include "ui/cfg_file_parser/cfg_file_parser.h"
#include "protocol_management/mqtt/mqtt_interface.h"

//-----------------------------------------------------------------------------

#define MAIN_MODULE_NAME		"MQTT Helper"
#define MAIN_USER_MSG_MAX_LENGTH	255

//-----------------------------------------------------------------------------

/*!
 *
 */
static void main_MQTT_CONNECTION_ESTABLISHED_SLOT_CALLBACK(const void* p_argument);

/*!
 *
 */
static void main_MQTT_CONNECTION_LOST_SLOT_CALLBACK(const void* p_argument);

/*!
 *
 */
static void main_MQTT_MESSAGE_RECEIVED_SLOT_CALLBACK(const void* p_argument);

/*!
 *
 */
static void main_MQTT_MESSAGE_SEND_SUCCEED_SLOT_CALLBACK(const void* p_argument);

/*!
 *
 */
static void main_CLI_HELP_REQUESTED_SLOT_CALLBACK(const void* p_argument);

/*!
 *
 */
static void main_CLI_INVALID_PARAMETER_SLOT_CALLBACK(const void* p_argument);

/*!
 *
 */
static void main_CLI_LCD_ACTIVATED_SLOT_CALLBACK(const void* p_argument);

/*!
 *
 */
static void main_CLI_MESSAGE_SLOT_CALLBACK(const void* p_argument);

/*!
 *
 */
static void main_CFG_OBJECT_RECEIVED_SLOT_CALLBACK(const void* p_argument);

// --------------------------------------------------------------------------------------

SIGNAL_SLOT_INTERFACE_CREATE_SLOT(MQTT_CONNECTION_ESTABLISHED_SIGNAL, MAIN_MQTT_CONNECTION_ESTABLISHED_SLOT, main_MQTT_CONNECTION_ESTABLISHED_SLOT_CALLBACK)
SIGNAL_SLOT_INTERFACE_CREATE_SLOT(MQTT_CONNECTION_LOST_SIGNAL, MAIN_MQTT_CONNECTION_LOST_SLOT, main_MQTT_CONNECTION_LOST_SLOT_CALLBACK)
SIGNAL_SLOT_INTERFACE_CREATE_SLOT(MQTT_MESSAGE_RECEIVED_SIGNAL, MAIN_MQTT_MESSAGE_RECEIVED_SLOT, main_MQTT_MESSAGE_RECEIVED_SLOT_CALLBACK)
SIGNAL_SLOT_INTERFACE_CREATE_SLOT(MQTT_MESSAGE_SEND_SUCCEED_SIGNAL, MAIN_MQTT_MESSAGE_SEND_SUCCEED_SLOT, main_MQTT_MESSAGE_SEND_SUCCEED_SLOT_CALLBACK)

SIGNAL_SLOT_INTERFACE_CREATE_SLOT(CLI_HELP_REQUESTED_SIGNAL, MAIN_CLI_HELP_REQUESTED_SLOT, main_CLI_HELP_REQUESTED_SLOT_CALLBACK)
SIGNAL_SLOT_INTERFACE_CREATE_SLOT(CLI_INVALID_PARAMETER_SIGNAL, MAIN_CLI_INVALID_PARAMETER_SLOT, main_CLI_INVALID_PARAMETER_SLOT_CALLBACK)
SIGNAL_SLOT_INTERFACE_CREATE_SLOT(CLI_LCD_ACTIVATED_SIGNAL, MAIN_CLI_LCD_ACTIVATED_SLOT, main_CLI_LCD_ACTIVATED_SLOT_CALLBACK)
SIGNAL_SLOT_INTERFACE_CREATE_SLOT(CLI_MESSAGE_SIGNAL, MAIN_CLI_MESSAGE_SLOT, main_CLI_MESSAGE_SLOT_CALLBACK)

SIGNAL_SLOT_INTERFACE_CREATE_SLOT(CFG_PARSER_NEW_CFG_OBJECT_SIGNAL, MAIN_CFG_OBJECT_RECEIVED_SLOT, main_CFG_OBJECT_RECEIVED_SLOT_CALLBACK)

// --------------------------------------------------------------------------------------

/*!
 *
 */
static u8 exit_program = 0;

/*!
 *
 */
static u8 user_msg_pending = 0;

/*!
 *
 */
static u8 connection_established = 0;

/*!
 *
 */
static char user_msg[MAIN_USER_MSG_MAX_LENGTH];

// --------------------------------------------------------------------------------------

int main(int argc, char* argv[]) {

	ATOMIC_OPERATION
	(
		initialization();
	)

	DEBUG_PASS("main() - MAIN_CLI_HELP_REQUESTED_SLOT_connect()");
	MAIN_CLI_HELP_REQUESTED_SLOT_connect();

	DEBUG_PASS("main() - MAIN_CLI_INVALID_PARAMETER_SLOT_connect()");
	MAIN_CLI_INVALID_PARAMETER_SLOT_connect();

	DEBUG_PASS("main() - MAIN_CLI_LCD_ACTIVATED_SLOT_connect()");
	MAIN_CLI_LCD_ACTIVATED_SLOT_connect();

	DEBUG_PASS("main() - MAIN_CFG_OBJECT_RECEIVED_SLOT_connect()");
	MAIN_CFG_OBJECT_RECEIVED_SLOT_connect();

	DEBUG_PASS("main() - MAIN_MQTT_CONNECTION_ESTABLISHED_SLOT_connect()");
	MAIN_MQTT_CONNECTION_ESTABLISHED_SLOT_connect();

	DEBUG_PASS("main() - MAIN_MQTT_MESSAGE_SEND_SUCCEED_SLOT_connect()");
	MAIN_MQTT_MESSAGE_SEND_SUCCEED_SLOT_connect();

	DEBUG_PASS("main() - MAIN_MQTT_CONNECTION_LOST_SLOT_connect()");
	MAIN_MQTT_CONNECTION_LOST_SLOT_connect();

	DEBUG_PASS("main() - MAIN_MQTT_MESSAGE_RECEIVED_SLOT_connect()");
	MAIN_MQTT_MESSAGE_RECEIVED_SLOT_connect();

	DEBUG_PASS("main() - MAIN_CLI_MESSAGE_SLOT_connect()");
	MAIN_CLI_MESSAGE_SLOT_connect();

	printf("Welcome to %s Version %d.%d\n\n", MAIN_MODULE_NAME, VERSION_MAJOR, VERSION_MINOR);

	command_line_interface(argc, argv);

	if (exit_program) {
		DEBUG_PASS("main() - initialization FAILED !!! --- ");
		return 1;
	}

	lcd_write_line(MAIN_MODULE_NAME);
	lcd_write_line("... started");

	CFG_PARSER_CFG_COMPLETE_SIGNAL_send(NULL);

	for (;;) {

		if (exit_program) {
			break;
		}

		if (user_msg_pending && connection_established) {
			user_msg_pending = 0;
			MQTT_MESSAGE_TO_SEND_SIGNAL_send(user_msg);
		}
		
		mcu_task_controller_schedule();
		mcu_task_controller_background_run();
		watchdog();
	}

	console_write_line("Closing Connection");

	mcu_task_controller_terminate_all();

	return 0;
}

// --------------------------------------------------------------------------------------

static void main_MQTT_CONNECTION_ESTABLISHED_SLOT_CALLBACK(const void* p_argument) {
	console_write_line("Connection to MQTT-Broker has been established");
	
	lcd_write_line(MAIN_MODULE_NAME);
	lcd_write_line("connected");

	connection_established = 1;
}

static void main_MQTT_CONNECTION_LOST_SLOT_CALLBACK(const void* p_argument) {
	console_write_line("Connection to MQTT-Broker lost");

	lcd_write_line(MAIN_MODULE_NAME);
	lcd_write_line("connection lost");
}

static void main_MQTT_MESSAGE_RECEIVED_SLOT_CALLBACK(const void* p_argument){


	if (p_argument == NULL) {
		DEBUG_PASS("main_MQTT_MESSAGE_RECEIVED_SLOT_CALLBACK() - NULL_POINTER_EXEPTIO !!! ---");
		return;
	}

	DEBUG_PASS("main_MQTT_MESSAGE_RECEIVED_SLOT_CALLBACK()");

	console_write_line((const char*)p_argument);

	lcd_write_line(MAIN_MODULE_NAME);
	lcd_write_line("msg received");
}

static void main_MQTT_MESSAGE_SEND_SUCCEED_SLOT_CALLBACK(const void* p_argument) {
	console_write_line("Message has been send");

	lcd_write_line(MAIN_MODULE_NAME);
	lcd_write_line("message send");

	exit_program = 1;
}

// --------------------------------------------------------------------------------------

static void main_CLI_INVALID_PARAMETER_SLOT_CALLBACK(const void* p_argument) {

	DEBUG_PASS("main_CLI_INVALID_PARAMETER_SLOT_CALLBACK");

	if (p_argument != NULL) {
		printf("Invalid parameter for arguemnt %s given!\n", (char*)p_argument);
	} else {
		console_write_line("Invalid parameter given, check your input!");
	}
	
	lcd_write_line(MAIN_MODULE_NAME);
	lcd_write_line("- inv parameter");

	main_CLI_HELP_REQUESTED_SLOT_CALLBACK(NULL);
}

static void main_CLI_LCD_ACTIVATED_SLOT_CALLBACK(const void* p_argument) {

	DEBUG_PASS("main_CLI_LCD_ACTIVATED_SLOT_CALLBACK()");

	lcd_init();
	lcd_set_enabled(1);
}

static void main_CLI_HELP_REQUESTED_SLOT_CALLBACK(const void* p_argument) {
	(void) p_argument;

	console_write_line("Usage: mqttHelper [options]]\n\n");
	console_write_line("Options:");
	console_write_line("-topic <topic_name>               : Name of the Topic to listen to");
	console_write_line("-host <host_name>:<port>          : Host-Address and Port of the MQTT-Broker (e.g. 192.168.1.100:1883)");
	console_write_line("-client <client_name>             : Name that is used to identify the mqttListener");
	console_write_line("-msg \"<cmessage>\"               : Message to send, programm will exit after message has been send");

	exit_program = 1;
}

static void main_CLI_MESSAGE_SLOT_CALLBACK(const void* p_argument) {

	if (p_argument == NULL) {
		console_write_line("Invalid argument for -msg given!");
		exit_program = 1;
		return;
	}

	const char* message_to_send = (const char*) p_argument;

	if (strlen(message_to_send) > MAIN_USER_MSG_MAX_LENGTH) {
		console_write_line("Your message is too long, sorry! (maximum 255 characters)");
		exit_program = 1;
		return;
	}

	memset(user_msg, '\0', MAIN_USER_MSG_MAX_LENGTH);
	memcpy(user_msg, message_to_send, strlen(message_to_send));

	user_msg_pending = 1;
}

// --------------------------------------------------------------------------------------

static void main_CFG_OBJECT_RECEIVED_SLOT_CALLBACK(const void* p_argument) {

	__UNUSED__ CFG_FILE_PARSER_CFG_OBJECT_TYPE* p_cfg_obj = (CFG_FILE_PARSER_CFG_OBJECT_TYPE*)p_argument;

	DEBUG_PASS("main_CFG_OBJECT_RECEIVED_SLOT_CALLBACK()");
	DEBUG_TRACE_STR(p_cfg_obj->value, p_cfg_obj->key);
}