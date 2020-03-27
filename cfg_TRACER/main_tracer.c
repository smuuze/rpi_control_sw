
// -------- INCLUDES --------------------------------------------------------------------

#include "shc_project_configuration.h"
#include "shc_common_types.h"
#include "shc_common_string.h"
#include "shc_common_configuration.h"

#include "shc_timer.h"
#include "shc_qeue_interface.h"
#include "shc_file_interface.h"
#include "shc_common_string.h"
#include "shc_debug_interface.h"
#include "shc_trace_object.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <linux/spi/spidev.h>

#include "shc_thread_read_trace_object.h"
#include "shc_thread_parse_trace_object.h"
#include "shc_thread_print_trace_object.h"

#include <time.h>

// -------- DEBUGGING -------------------------------------------------------------------

#define MAIN_DEBUG_MSG					DEBUG_MSG
#define MAIN_CFG_DEBUG_MSG				DEBUG_MSG

// -------- STATIC FUNCTION PROTOTYPES --------------------------------------------------

/*!
 *
 */
u8 command_line_parser(int argc, char* argv[], CFG_INTERFACE* p_cfg_interface);

/*!
 *
 */
void command_line_usage(void);

// -------- STATIC DATA -----------------------------------------------------------------

TIME_MGMN_BUILD_TIMER(RESET_TIMER)
TIME_MGMN_BUILD_TIMER(MQTT_CONNECT_TIMER)
TIME_MGMN_BUILD_TIMER(BOARD_CONNECT_TIMER)
TIME_MGMN_BUILD_TIMER(REPORT_TIMER)

QEUE_INTERFACE_BUILD_QEUE(RAW_TRACE_OBJECT_QEUE, TRACE_OBJECT_RAW, 48)
QEUE_INTERFACE_BUILD_QEUE(TRACE_OBJECT_QEUE, TRACE_OBJECT, 48)

/*!
 *
 */
MSG_QEUE myCommandQeue;

/*!
 *
 */
CFG_INTERFACE myCfgInterface;


// -------- MAIN ------------------------------------------------------------------------


int main(int argc, char* argv[]) {

	MAIN_DEBUG_MSG("Welcome to the SHC-Tracer v%d.%d\n", VERSION_MAJOR, VERSION_MINOR);

	qeue_init(&myCommandQeue);

	MAIN_DEBUG_MSG("main() - LOADING CONFIGURATION\n");

	// --- Parsing Command-Line Arguments
	u8 err_code = command_line_parser(argc, argv, &myCfgInterface);
	if (err_code != NO_ERR) {
		command_line_usage();
		return err_code;
	}

	char welcome_message[128];
	sprintf(welcome_message, "Welcome to Tracer v%d.%d", VERSION_MAJOR, VERSION_MINOR);
	MAIN_DEBUG_MSG("main() - Welcome message: \"%s\"\n", welcome_message);
	
	LOG_MSG(NO_ERR, &myCfgInterface.log_file, "Starting SmartHomeClient Deamon v%d.%d", VERSION_MAJOR, VERSION_MINOR);
	
	RAW_TRACE_OBJECT_QEUE_init();
	TRACE_OBJECT_QEUE_init();

	READ_TRACE_OBJECT_THREAD_start();
	PARSE_TRACE_OBJECT_THREAD_start();
	PRINT_TRACE_OBJECT_THREAD_start();

	while (1) {

		usleep(500000); // reduce cpu-load

	}

	return 0;
}


// -------- COMMAND-LINE PARSING --------------------------------------------------------


u8 command_line_parser(int argc, char* argv[], CFG_INTERFACE* p_cfg_interface) {

	p_cfg_interface->log_file.act_file_pointer = 0;

	memset(p_cfg_interface->cfg_file.path, 0x00, FILE_PATH_MAX_STRING_LENGTH);
	memcpy(p_cfg_interface->cfg_file.path, CONFIGURATION_FILE_PATH, string_length(CONFIGURATION_FILE_PATH));
	memcpy(p_cfg_interface->trace_file.path, TRACE_FILE_DEFAULT_PATH, string_length(TRACE_FILE_DEFAULT_PATH));
	memcpy(p_cfg_interface->trace_file.path, TRACE_DEFAULT_BASE_PATH, string_length(TRACE_DEFAULT_BASE_PATH));

	p_cfg_interface->output.console = 0;
	p_cfg_interface->output.file = 0;
	p_cfg_interface->output.mqtt = 0;

	u8 i = 0;
	for ( ; i < argc; i++) {

		MAIN_CFG_DEBUG_MSG("command_line_parser() - Parsing cli-argument: %s\n", argv[i]);

		if (memcmp(argv[i], COMMAND_LINE_ARGUMENT_CFG_FILE, string_length(COMMAND_LINE_ARGUMENT_CFG_FILE)) == 0) {

			if (i + 1 >= argc) {
				break;
			}

			memset(p_cfg_interface->cfg_file.path, 0x00, FILE_PATH_MAX_STRING_LENGTH);
			memcpy(p_cfg_interface->cfg_file.path, argv[i + 1], string_length(argv[i + 1]));

			MAIN_CFG_DEBUG_MSG("command_line_parser() - Using Config-File: %s\n", p_cfg_interface->cfg_file.path);
			
			// do not process the parameter as a new argument
			i += 1;

		} else if (memcmp(argv[i], COMMAND_LINE_ARGUMENT_FILE, string_length(COMMAND_LINE_ARGUMENT_FILE)) == 0) {

			if (i + 1 >= argc) {
				break;
			}

			memset(p_cfg_interface->trace_file.path, 0x00, FILE_PATH_MAX_STRING_LENGTH);
			memcpy(p_cfg_interface->trace_file.path, argv[i + 1], string_length(argv[i + 1]));
			p_cfg_interface->output.file = 1;

			MAIN_CFG_DEBUG_MSG("command_line_parser() - Using Trace-File: %s\n", p_cfg_interface->trace_file.path);
			

		} else if (memcmp(argv[i], COMMAND_LINE_ARGUMENT_MQTT, string_length(COMMAND_LINE_ARGUMENT_MQTT)) == 0) {

		} else if (memcmp(argv[i], COMMAND_LINE_ARGUMENT_CONSOLE, string_length(COMMAND_LINE_ARGUMENT_CONSOLE)) == 0) {
			p_cfg_interface->output.console = 1;

		} else if (memcmp(argv[i], COMMAND_LINE_ARGUMENT_PATH, string_length(COMMAND_LINE_ARGUMENT_PATH)) == 0) {

			if (i + 1 >= argc) {
				break;
			}

			memset(p_cfg_interface->base_path.path, 0x00, FILE_PATH_MAX_STRING_LENGTH);
			memcpy(p_cfg_interface->base_path.path, argv[i + 1], string_length(argv[i + 1]));

			MAIN_CFG_DEBUG_MSG("command_line_parser() - Using Trace-Path: %s\n", p_cfg_interface->base_path.path);

		} else if (memcmp(argv[i], COMMAND_LINE_ARGUMENT_HELP, string_length(COMMAND_LINE_ARGUMENT_HELP)) == 0) {
			return ERR_INVALID_ARGUMENT;
		} 

		
	}

	return NO_ERR;
}

void command_line_usage(void) {
	printf("Tracer v%d.%d\n\n", VERSION_MAJOR, VERSION_MINOR);
	printf("Usage: shcTracer [options]]\n\n");
	printf("Options:\n");
	printf("-path <path>                       : path to directory that includes your makefile\t\n");
	printf("-file <path>                       : traceoutput will be stored into this file\t\n");
	printf("-console                           : traceoutput will be shown on console\t\n");
	printf("-mqtt <topic>@<servicer_ip:port>   : traceoutput will be shown on console\t\n");
}