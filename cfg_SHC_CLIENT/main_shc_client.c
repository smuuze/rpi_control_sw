/*! 
 * --------------------------------------------------------------------------------
 *
 * \file	main_shc_helper.c
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
#include "common/local_module_status.h"
#include "mcu_task_management/mcu_task_controller.h"
#include "ui/command_line/command_line_interface.h"
#include "ui/console/ui_console.h"
#include "ui/lcd/ui_lcd_interface.h"
#include "ui/cfg_file_parser/cfg_file_parser.h"
#include "time_management/time_management.h"

// --------------------------------------------------------------------------------------

#define MAIN_STATUS_EXIT_PROGRAM		(1 << 0)
#define MAIN_STATUS_CONSOLE_ACTIVE		(1 << 1)
#define MAIN_STATUS_CFG_FILE_SET		(1 << 2)

BUILD_MODULE_STATUS_U8(MAIN_STATUS)

//-----------------------------------------------------------------------------

TIME_MGMN_BUILD_STATIC_TIMER_U32(MAIN_TIMER)

//-----------------------------------------------------------------------------

static void main_CLI_HELP_REQUESTED_SLOT_CALLBACK(const void* p_argument) {
	(void) p_argument;

	console_write_line("Usage: shcClient [options]]\n\n");
	console_write_line("Options:");
	console_write_line("-file <cfg_file_path>   : using <cfg_file_path> for program configuration");
	console_write_line("-lcd                    : Enables 16x2 LCD dispaly-driver");
	console_write_line("-console                : Enables program output on console");
	console_write_line("-help / -h              : showing this help");

	MAIN_STATUS_set(MAIN_STATUS_EXIT_PROGRAM);
}

static void main_CLI_CONFIGURATION_SLOT_CALLBACK(const void* p_argument) {

	DEBUG_PASS("main_CLI_CONFIGURATION_SLOT_CALLBACK()");

	if (MAIN_STATUS_is_set(MAIN_STATUS_CONSOLE_ACTIVE)) {
		console_write_number(MAIN_TIMER_elapsed());
		console_write(" - ");
		console_write("Using configuration-file: ");
		console_write_line((const char*)p_argument);
	}

	MAIN_STATUS_set(MAIN_STATUS_CFG_FILE_SET);
}

static void main_CLI_INVALID_PARAMETER_SLOT_CALLBACK(const void* p_argument) {

	DEBUG_PASS("main_CLI_INVALID_PARAMETER_SLOT_CALLBACK");

	if (p_argument != NULL) {
		printf("Invalid parameter for arguemnt %s given!\n", (char*)p_argument);
	} else {
		console_write_number(MAIN_TIMER_elapsed());
		console_write(" - ");
		console_write_line("Invalid parameter given, check your input!");
	}
	
	lcd_write_line("SHC");
	lcd_write_line("- inv parameter");

	main_CLI_HELP_REQUESTED_SLOT_CALLBACK(NULL);
}

static void main_CLI_LCD_ACTIVATED_SLOT_CALLBACK(const void* p_argument) {

	DEBUG_PASS("main_CLI_LCD_ACTIVATED_SLOT_CALLBACK()");

	if (MAIN_STATUS_is_set(MAIN_STATUS_CONSOLE_ACTIVE)) {
		console_write_number(MAIN_TIMER_elapsed());
		console_write(" - ");
		console_write_line("LCD activated");
	}

	lcd_init();
	lcd_set_enabled(1);
}

static void main_CLI_CONSOLE_ACTIVATED_SLOT_CALLBACK(const void* p_argument) {

	DEBUG_PASS("main_CLI_CONSOLE_ACTIVATED_SLOT_CALLBACK()");
	MAIN_STATUS_set(MAIN_STATUS_CONSOLE_ACTIVE);

	if (MAIN_STATUS_is_set(MAIN_STATUS_CONSOLE_ACTIVE)) {
		console_write_number(MAIN_TIMER_elapsed());
		console_write(" - ");
		console_write_line("CONSOLE activated");
	}
}

static void main_CLI_EXECUTER_COMMAND_RESPONSE_SLOT_CALLBACK(const void* p_argument) {
	DEBUG_TRACE_STR((const char*)p_argument, "main_CLI_EXECUTER_COMMAND_RESPONSE_SLOT_CALLBACK()");

	if (MAIN_STATUS_is_set(MAIN_STATUS_CONSOLE_ACTIVE)) {
		console_write_number(MAIN_TIMER_elapsed());
		console_write(" - ");
		console_write_line("Execution Command response received");
	}
}

static void main_CLI_EXECUTER_COMMAND_TIMEOUT_CALLBACK(const void* p_argument) {
	DEBUG_PASS("main_CLI_EXECUTER_COMMAND_TIMEOUT_CALLBACK()");

	if (MAIN_STATUS_is_set(MAIN_STATUS_CONSOLE_ACTIVE)) {
		console_write_number(MAIN_TIMER_elapsed());
		console_write(" - ");
		console_write_line("--- TIMEOUT COMMAND_LINE_EXECUTER ---");
	}
}

static void main_CLI_EXECUTER_COMMAND_NOT_FOUND_CALLBACK(const void* p_argument) {
	DEBUG_PASS("main_CLI_EXECUTER_COMMAND_NOT_FOUND_CALLBACK()");

	if (MAIN_STATUS_is_set(MAIN_STATUS_CONSOLE_ACTIVE)) {
		console_write_number(MAIN_TIMER_elapsed());
		console_write(" - ");
		console_write("Execution Command not found: ");
		console_write_line((const char*) p_argument);
	}
}

static void main_CLI_EXECUTER_COMMAND_RECEIVED_SLOT_CALLBACK(const void* p_argument) {
	DEBUG_PASS("main_CLI_EXECUTER_COMMAND_RECEIVED_SLOT_CALLBACK()");

	if (MAIN_STATUS_is_set(MAIN_STATUS_CONSOLE_ACTIVE)) {
		console_write_number(MAIN_TIMER_elapsed());
		console_write(" - ");
		console_write("Execution Command: ");
		console_write_line((const char*) p_argument);
	}
}

static void main_MQTT_CONNECTION_ESTABLISHED_CALLBACK(const void* p_argument) {

	(void) p_argument;
	DEBUG_PASS("main_MQTT_CONNECTION_ESTABLISHED_CALLBACK()");

	if (MAIN_STATUS_is_set(MAIN_STATUS_CONSOLE_ACTIVE)) {
		console_write_number(MAIN_TIMER_elapsed());
		console_write(" - ");
		console_write_line("MQTT Connection established");
	}
}

static void main_MQTT_CONNECTION_LOST_CALLBACK(const void* p_argument) {

	(void) p_argument;
	DEBUG_PASS("main_MQTT_CONNECTION_LOST_CALLBACK()");

	if (MAIN_STATUS_is_set(MAIN_STATUS_CONSOLE_ACTIVE)) {
		console_write_number(MAIN_TIMER_elapsed());
		console_write(" - ");
		console_write_line("MQTT Connection Lost");
	}
}

static void main_MQTT_MESSAGE_RECEIVED_CALLBACK(const void* p_argument) {

	DEBUG_TRACE_STR((char*) p_argument, "main_MQTT_MESSAGE_RECEIVED_CALLBACK()");

	if (MAIN_STATUS_is_set(MAIN_STATUS_CONSOLE_ACTIVE)) {
		console_write_number(MAIN_TIMER_elapsed());
		console_write(" - ");
		console_write("MQTT Message received: ");
		console_write_line((char*) p_argument);
	}
}

static void main_MQTT_MESSAGE_SEND_FAILED_CALLBACK(const void* p_argument) {

	DEBUG_TRACE_STR((char*) p_argument, "main_MQTT_MESSAGE_SEND_FAILED_CALLBACK()");

	if (MAIN_STATUS_is_set(MAIN_STATUS_CONSOLE_ACTIVE)) {
		console_write_number(MAIN_TIMER_elapsed());
		console_write(" - ");
		console_write("Send MQTT-Message has FAILED !!!");
	}
}

static void main_MQTT_MESSAGE_SEND_SUCCEEDED_CALLBACK(const void* p_argument) {

	DEBUG_TRACE_STR((char*) p_argument, "main_MQTT_MESSAGE_SEND_SUCCEEDED_CALLBACK()");

	if (MAIN_STATUS_is_set(MAIN_STATUS_CONSOLE_ACTIVE)) {
		console_write_number(MAIN_TIMER_elapsed());
		console_write(" - ");
		console_write_line("Send MQTT-Message succeeded");
	}
}

static void main_MQTT_MESSAGE_TO_SEND_CALLBACK(const void* p_argument) {

	DEBUG_TRACE_STR((const char*) p_argument, "main_MQTT_MESSAGE_TO_SEND_CALLBACK()");

	if (MAIN_STATUS_is_set(MAIN_STATUS_CONSOLE_ACTIVE)) {
		console_write_number(MAIN_TIMER_elapsed());
		console_write(" - ");
		console_write("MQTT-Message to send: ");
		console_write_line((const char*) p_argument);
	}
}

static void main_MSG_EXECUTER_RESPONSE_TIMEOUT_SLOT_CALLBACK(const void* p_argument) {
	DEBUG_PASS("main_MSG_EXECUTER_RESPONSE_TIMEOUT_SLOT_CALLBACK()");

	if (MAIN_STATUS_is_set(MAIN_STATUS_CONSOLE_ACTIVE)) {
		console_write_number(MAIN_TIMER_elapsed());
		console_write(" - ");
		console_write("--- TIMEOUT MESSAGE_EXECUTER: ");
		console_write((const char*)p_argument);
		console_write_line(" --- ");
	}
}

static void main_MSG_EXECUTER_INVALID_COMMAND_SLOT_CALLBACK(const void* p_argument) {
	DEBUG_PASS("main_MSG_EXECUTER_INVALID_COMMAND_SLOT_CALLBACK()");

	if (MAIN_STATUS_is_set(MAIN_STATUS_CONSOLE_ACTIVE)) {
		console_write_number(MAIN_TIMER_elapsed());
		console_write(" - ");
		console_write_line("Process Command Message Failed - (unknown command)");
	}
}

static void main_MSG_EXECUTER_FILE_OPEN_FAILED_SLOT_CALLBACK(const void* p_argument) {

	__UNUSED__ const char* response_msg = (const char*)p_argument;
	DEBUG_TRACE_STR(response_msg, "main_MSG_EXECUTER_FILE_OPEN_FAILED_SLOT_CALLBACK() - File:");

	if (MAIN_STATUS_is_set(MAIN_STATUS_CONSOLE_ACTIVE)) {
		console_write_number(MAIN_TIMER_elapsed());
		console_write(" - ");
		console_write("File open Failed: ");
		console_write_line(response_msg);
	}
}

static void main_MSG_EXECUTER_RESPONSE_RECEIVED_SLOT_CALLBACK(const void* p_argument) {

	__UNUSED__ const char* response_msg = (const char*)p_argument;
	DEBUG_TRACE_STR(response_msg, "main_MSG_EXECUTER_RESPONSE_RECEIVED_SLOT_CALLBACK() - File:");

	if (MAIN_STATUS_is_set(MAIN_STATUS_CONSOLE_ACTIVE)) {
		console_write_number(MAIN_TIMER_elapsed());
		console_write(" - ");
		console_write("Process Command Message Succeeded: ");
		console_write_line(response_msg);
	}
}

static void main_RPI_HOST_COMMAND_RECEIVED_SLOT_CALLBACK(const void* p_argument) {

	DEBUG_PASS("main_RPI_HOST_COMMAND_RECEIVED_SLOT_CALLBACK()");

	if (MAIN_STATUS_is_set(MAIN_STATUS_CONSOLE_ACTIVE)) {
		console_write_number(MAIN_TIMER_elapsed());
		console_write(" - ");
		console_write_line("New COM-Command");
	}
}

static void main_RPI_HOST_RESPONSE_TIMEOUT_SLOT_CALLBACK(const void* p_argument) {

	DEBUG_PASS("main_RPI_HOST_RESPONSE_TIMEOUT_SLOT_CALLBACK()");

	if (MAIN_STATUS_is_set(MAIN_STATUS_CONSOLE_ACTIVE)) {
		console_write_number(MAIN_TIMER_elapsed());
		console_write(" - ");
		console_write_line("--- TIMEOUT RPI-HOST ---");
	}
}

// --------------------------------------------------------------------------------------

SIGNAL_SLOT_INTERFACE_CREATE_SLOT(CLI_CONFIGURATION_SIGNAL, MAIN_CLI_CONFIGURATION_SIGNAL_SLOT, main_CLI_CONFIGURATION_SLOT_CALLBACK)
SIGNAL_SLOT_INTERFACE_CREATE_SLOT(CLI_HELP_REQUESTED_SIGNAL, MAIN_CLI_HELP_REQUESTED_SLOT, main_CLI_HELP_REQUESTED_SLOT_CALLBACK)
SIGNAL_SLOT_INTERFACE_CREATE_SLOT(CLI_INVALID_PARAMETER_SIGNAL, MAIN_CLI_INVALID_PARAMETER_SLOT, main_CLI_INVALID_PARAMETER_SLOT_CALLBACK)
SIGNAL_SLOT_INTERFACE_CREATE_SLOT(CLI_LCD_ACTIVATED_SIGNAL, MAIN_CLI_LCD_ACTIVATED_SLOT, main_CLI_LCD_ACTIVATED_SLOT_CALLBACK)
SIGNAL_SLOT_INTERFACE_CREATE_SLOT(CLI_CONSOLE_ACTIVATED_SIGNAL, MAIN_CLI_CONSOLE_ACTIVATED_SLOT, main_CLI_CONSOLE_ACTIVATED_SLOT_CALLBACK)

SIGNAL_SLOT_INTERFACE_CREATE_SLOT(CLI_EXECUTER_COMMAND_RECEIVED_SIGNAL,	 MAIN_CLI_EXECUTER_COMMAND_RECEIVED_SLOT,  main_CLI_EXECUTER_COMMAND_RECEIVED_SLOT_CALLBACK)
SIGNAL_SLOT_INTERFACE_CREATE_SLOT(CLI_EXECUTER_COMMAND_RESPONSE_SIGNAL,	 MAIN_CLI_EXECUTER_COMMAND_RESPONSE_SLOT,  main_CLI_EXECUTER_COMMAND_RESPONSE_SLOT_CALLBACK)
SIGNAL_SLOT_INTERFACE_CREATE_SLOT(CLI_EXECUTER_COMMAND_TIMEOUT_SIGNAL,   MAIN_CLI_EXECUTER_COMMAND_TIMEOUT_SLOT,   main_CLI_EXECUTER_COMMAND_TIMEOUT_CALLBACK)
SIGNAL_SLOT_INTERFACE_CREATE_SLOT(CLI_EXECUTER_COMMAND_NOT_FOUND_SIGNAL, MAIN_CLI_EXECUTER_COMMAND_NOT_FOUND_SLOT, main_CLI_EXECUTER_COMMAND_NOT_FOUND_CALLBACK)

SIGNAL_SLOT_INTERFACE_CREATE_SLOT(MQTT_MESSAGE_RECEIVED_SIGNAL,     	MAIN_MQTT_MESSAGE_RECEIVED_SLOT,     	main_MQTT_MESSAGE_RECEIVED_CALLBACK)
SIGNAL_SLOT_INTERFACE_CREATE_SLOT(MQTT_CONNECTION_ESTABLISHED_SIGNAL, 	MAIN_MQTT_CONNECTION_ESTABLISHED_SLOT, 	main_MQTT_CONNECTION_ESTABLISHED_CALLBACK)
SIGNAL_SLOT_INTERFACE_CREATE_SLOT(MQTT_CONNECTION_LOST_SIGNAL,         	MAIN_MQTT_CONNECTION_LOST_SLOT, 	main_MQTT_CONNECTION_LOST_CALLBACK)
SIGNAL_SLOT_INTERFACE_CREATE_SLOT(MQTT_MESSAGE_SEND_FAILED_SIGNAL,      MAIN_MQTT_MESSAGE_SEND_FAILED_SLOT, 	main_MQTT_MESSAGE_SEND_FAILED_CALLBACK)
SIGNAL_SLOT_INTERFACE_CREATE_SLOT(MQTT_MESSAGE_SEND_SUCCEED_SIGNAL,     MAIN_MQTT_MESSAGE_SEND_SUCCEEDED_SLOT, 	main_MQTT_MESSAGE_SEND_SUCCEEDED_CALLBACK)
SIGNAL_SLOT_INTERFACE_CREATE_SLOT(MQTT_MESSAGE_TO_SEND_SIGNAL,     	MAIN_MQTT_MESSAGE_TO_SEND_SLOT, 	main_MQTT_MESSAGE_TO_SEND_CALLBACK)

SIGNAL_SLOT_INTERFACE_CREATE_SLOT(MSG_EXECUTER_FILE_OPEN_FAILED_SIGNAL, MAIN_MSG_EXECUTER_FILE_OPEN_FAILED_SLOT, main_MSG_EXECUTER_FILE_OPEN_FAILED_SLOT_CALLBACK)
SIGNAL_SLOT_INTERFACE_CREATE_SLOT(MSG_EXECUTER_RESPONSE_TIMEOUT_SIGNAL, MAIN_MSG_EXECUTER_RESPONSE_TIMEOUT_SLOT, main_MSG_EXECUTER_RESPONSE_TIMEOUT_SLOT_CALLBACK)
SIGNAL_SLOT_INTERFACE_CREATE_SLOT(MSG_EXECUTER_INVALID_COMMAND_SIGNAL, MAIN_MSG_EXECUTER_INVALID_COMMAND_SLOT, main_MSG_EXECUTER_INVALID_COMMAND_SLOT_CALLBACK)
SIGNAL_SLOT_INTERFACE_CREATE_SLOT(MSG_EXECUTER_RESPONSE_RECEIVED_SIGNAL, MAIN_MSG_EXECUTER_RESPONSE_RECEIVED_SLOT, main_MSG_EXECUTER_RESPONSE_RECEIVED_SLOT_CALLBACK)

SIGNAL_SLOT_INTERFACE_CREATE_SLOT(RPI_HOST_COMMAND_RECEIVED_SIGNAL, MAIN_RPI_HOST_COMMAND_RECEIVED_SLOT, main_RPI_HOST_COMMAND_RECEIVED_SLOT_CALLBACK)
SIGNAL_SLOT_INTERFACE_CREATE_SLOT(RPI_HOST_RESPONSE_TIMEOUT_SIGNAL, MAIN_RPI_HOST_RESPONSE_TIMEOUT_SLOT, main_RPI_HOST_RESPONSE_TIMEOUT_SLOT_CALLBACK)


// --------------------------------------------------------------------------------------

int main(int argc, char* argv[]) {

	ATOMIC_OPERATION
	(
		initialization();
	)

	MAIN_STATUS_clear_all();
	MAIN_TIMER_start();

	MAIN_CLI_HELP_REQUESTED_SLOT_connect();
	MAIN_CLI_INVALID_PARAMETER_SLOT_connect();
	MAIN_CLI_LCD_ACTIVATED_SLOT_connect();
	MAIN_CLI_CONSOLE_ACTIVATED_SLOT_connect();
	MAIN_CLI_CONFIGURATION_SIGNAL_SLOT_connect();

	MAIN_CLI_EXECUTER_COMMAND_RESPONSE_SLOT_connect();
	MAIN_CLI_EXECUTER_COMMAND_TIMEOUT_SLOT_connect();
	MAIN_CLI_EXECUTER_COMMAND_NOT_FOUND_SLOT_connect();
	MAIN_CLI_EXECUTER_COMMAND_RECEIVED_SLOT_connect();

	MAIN_MQTT_MESSAGE_RECEIVED_SLOT_connect();
	MAIN_MQTT_CONNECTION_ESTABLISHED_SLOT_connect();
	MAIN_MQTT_CONNECTION_LOST_SLOT_connect();
	MAIN_MQTT_MESSAGE_SEND_FAILED_SLOT_connect();
	MAIN_MQTT_MESSAGE_SEND_SUCCEEDED_SLOT_connect();

	MAIN_MSG_EXECUTER_FILE_OPEN_FAILED_SLOT_connect();
	MAIN_MSG_EXECUTER_RESPONSE_TIMEOUT_SLOT_connect();
	MAIN_MSG_EXECUTER_INVALID_COMMAND_SLOT_connect();
	MAIN_MSG_EXECUTER_RESPONSE_RECEIVED_SLOT_connect();

	MAIN_RPI_HOST_COMMAND_RECEIVED_SLOT_connect();
	MAIN_RPI_HOST_RESPONSE_TIMEOUT_SLOT_connect();

	command_line_interface(argc, argv);

	if (MAIN_STATUS_is_set(MAIN_STATUS_CONSOLE_ACTIVE)) {
		
		console_write("Welcome to the Smart-Home-Client v");
		console_write_number(VERSION_MAJOR);
		console_write(".");
		console_write_number(VERSION_MINOR);
		console_write_line("");
	}

	if (MAIN_STATUS_is_set(MAIN_STATUS_CFG_FILE_SET) == 0) {
		console_write_line("No configuration file given!");
		main_CLI_HELP_REQUESTED_SLOT_CALLBACK(NULL);
		return 1;
	}

	if (MAIN_STATUS_is_set(MAIN_STATUS_EXIT_PROGRAM)) {
		DEBUG_PASS("main() - initialization FAILED !!! --- ");
		return 1;
	}

	lcd_write_line("SHC");
	lcd_write_line("... started");

	for (;;) {

		if (MAIN_STATUS_is_set(MAIN_STATUS_EXIT_PROGRAM)) {
			break;
		}
		
		mcu_task_controller_schedule();
		mcu_task_controller_background_run();
		watchdog();
	}

	return 0;
}