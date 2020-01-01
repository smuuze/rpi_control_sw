
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

#include "shc_usart_interface.h"

#include "shc_trace_object.h"
#include "shc_trace_parser.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <linux/spi/spidev.h>

#include <time.h>

// -------- DEBUGGING -------------------------------------------------------------------

#define MAIN_DEBUG_MSG					DEBUG_MSG
#define MAIN_CFG_DEBUG_MSG				DEBUG_MSG

// --------------------------------------------------------------------------------------

#define DEAFULT_USART_DEVICE 				"/dev/serial0"
#define DEFAULT_USART_TIMEOUT_MS			25

// -------- STATIC FUNCTION PROTOTYPES --------------------------------------------------

/*!
 *
 */
u8 command_line_parser(int argc, char* argv[], CFG_INTERFACE* p_cfg_interface);

/*!
 *
 */
u8 main_read_trace_object_raw(USART_INTERFACE* p_usart, TRACE_OBJECT_RAW* p_raw_obj);

/*!
 *
 */
u8 main_read_source_file_line(char* base_path, TRACE_OBJECT* p_trace_obj);


/*!
 *
 */
void command_line_usage(void);


// -------- STATIC DATA -----------------------------------------------------------------

TIME_MGMN_BUILD_TIMER(RESET_TIMER)
TIME_MGMN_BUILD_TIMER(MQTT_CONNECT_TIMER)
TIME_MGMN_BUILD_TIMER(BOARD_CONNECT_TIMER)
TIME_MGMN_BUILD_TIMER(REPORT_TIMER)

/*!
 *
 */
MSG_QEUE myCommandQeue;

/*!
 *
 */
static CFG_INTERFACE myCfgInterface;


// -------- MAIN ------------------------------------------------------------------------

int main(int argc, char* argv[]) {

	MAIN_DEBUG_MSG("Welcome to the SmartHomeClient v%d.%d\n", VERSION_MAJOR, VERSION_MINOR);

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

	USART_INTERFACE usart0;
	usart0._handle_id = -1;
	usart0.baudrate = USART_BAUDRATE_230400;
	memset(usart0.device_name, 0x00, USART_DEVICE_NAME_MAX_LENGTH);
	memcpy(usart0.device_name, DEAFULT_USART_DEVICE, string_length(DEAFULT_USART_DEVICE));

	if (usart_init(&usart0) == 0) {
		MAIN_DEBUG_MSG("main() - Init USART has FAILED !!!\n");
		return -1;
	}

	TRACE_OBJECT_RAW raw_obj;
	TRACE_OBJECT trace_obj;

	while (1) {

		usleep(50000); // reduce cpu-load

		if (main_read_trace_object_raw(&usart0, &raw_obj) == 0) {
			continue;
		}
		
		if (tracer_parse_object(&raw_obj, &trace_obj) == 0) {
			//MAIN_DEBUG_MSG("main() - Parsing Trace-Object has FAILED !!!\n");
			continue;
		}

		if (main_read_source_file_line(myCfgInterface.base_path.path, &trace_obj) == 0) {
			//MAIN_DEBUG_MSG("main() - Reading Source-File has FAILED !!!\n");
			continue;
		}

		char trace_line[config_MAX_LENGTH_OF_FILE_LINE];
		sprintf(trace_line, "Length: %03d | Type: %d | Line: %04d | FILE: %s", trace_obj.length, trace_obj.type, trace_obj.line_number, trace_obj.file_name);
		file_append_line(&myCfgInterface.trace_file, trace_line);


		MAIN_DEBUG_MSG("%s:%d - %s\n", trace_obj.file_name, trace_obj.line_number, trace_obj.source_line);
	}

	return 0;
}

u8 main_read_trace_object_raw(USART_INTERFACE* p_usart, TRACE_OBJECT_RAW* p_raw_obj) {

	u16 length = usart_read_bytes(p_usart, TRACE_PARSER_NUM_BYTES_HEADER, p_raw_obj->data, DEFAULT_USART_TIMEOUT_MS);
	if (length == 0) {
		//MAIN_DEBUG_MSG("main_read_trace_object_raw() - usart_read_bytes(HEADER) has FAILED !!!\n");
		return 0;
	}

	p_raw_obj->length = length;

	u8 header[] = {TRACE_PARSER_HEADER_BYTE_ARRAY};
	if (memcmp(p_raw_obj->data, header, TRACE_PARSER_NUM_BYTES_HEADER) != 0) {
		MAIN_DEBUG_MSG("main_read_trace_object_raw() - Matching header has FAILED !!!\n");
		return 0;
	}

	length = usart_read_bytes(p_usart, TRACE_PARSER_NUM_BYTES_BYTE_COUNT, p_raw_obj->data + TRACE_PARSER_NUM_BYTES_HEADER, DEFAULT_USART_TIMEOUT_MS);
	if (length == 0) {
		MAIN_DEBUG_MSG("main_read_trace_object_raw() - usart_read_bytes(BYTE_COUNT) has FAILED !!!\n");
		return 0;
	}

	p_raw_obj->length += length;
	length = readU16_MSB(p_raw_obj->data + TRACE_PARSER_NUM_BYTES_HEADER);

	length = usart_read_bytes(p_usart, length, p_raw_obj->data + TRACE_PARSER_NUM_BYTES_HEADER + TRACE_PARSER_NUM_BYTES_BYTE_COUNT, DEFAULT_USART_TIMEOUT_MS);
	if (length == 0) {
		MAIN_DEBUG_MSG("main_read_trace_object_raw() - usart_read_bytes(PAYLOAD) has FAILED !!!\n");
		return 0;
	}

	p_raw_obj->length += length;
	return 1;
}

u8 main_read_source_file_line(char* base_path, TRACE_OBJECT* p_trace_obj) {

	FILE_INTERFACE source_file;
	sprintf(source_file.path, "%s%s", base_path, p_trace_obj->file_name);
	//MAIN_DEBUG_MSG("Source-File: %s\n", source_file.path);

	if (file_open(&source_file) == 0) {
		MAIN_DEBUG_MSG("main_read_source_file_line() - File not found !!! (FILE:%s)\n", source_file.path);
		return 0;
	}

	file_read_specific_line(&source_file, p_trace_obj->line_number, p_trace_obj->source_line, TRACE_OBJECT_SOURCE_LINE_LENGTH);
	//MAIN_DEBUG_MSG("Source-Line: %s\n", p_trace_obj->source_line);

	return 1;
}

// -------- COMMAND-LINE PARSING --------------------------------------------------------

u8 command_line_parser(int argc, char* argv[], CFG_INTERFACE* p_cfg_interface) {

	p_cfg_interface->log_file.act_file_pointer = 0;

	memset(p_cfg_interface->cfg_file.path, 0x00, FILE_PATH_MAX_STRING_LENGTH);
	memcpy(p_cfg_interface->cfg_file.path, CONFIGURATION_FILE_PATH, string_length(CONFIGURATION_FILE_PATH));
	memcpy(p_cfg_interface->trace_file.path, TRACE_FILE_DEFAULT_PATH, string_length(TRACE_FILE_DEFAULT_PATH));
	memcpy(p_cfg_interface->trace_file.path, TRACE_DEFAULT_BASE_PATH, string_length(TRACE_DEFAULT_BASE_PATH));

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

			MAIN_CFG_DEBUG_MSG("command_line_parser() - Using Trace-File: %s\n", p_cfg_interface->trace_file.path);
			

		} else if (memcmp(argv[i], COMMAND_LINE_ARGUMENT_MQTT, string_length(COMMAND_LINE_ARGUMENT_MQTT)) == 0) {

		} else if (memcmp(argv[i], COMMAND_LINE_ARGUMENT_PATH, string_length(COMMAND_LINE_ARGUMENT_PATH)) == 0) {

			if (i + 1 >= argc) {
				break;
			}

			memset(p_cfg_interface->base_path.path, 0x00, FILE_PATH_MAX_STRING_LENGTH);
			memcpy(p_cfg_interface->base_path.path, argv[i + 1], string_length(argv[i + 1]));

			MAIN_CFG_DEBUG_MSG("command_line_parser() - Using Trace-File: %s\n", p_cfg_interface->base_path.path);

		}
	}
/*
	char path[128];
	sprintf(path, "%s", p_cfg_interface->cfg_file.path);

	FILE* config_file_handle = fopen((const char*)p_cfg_interface->cfg_file.path, "r");
	if (config_file_handle == NULL) {
		MAIN_CFG_DEBUG_MSG("command_line_parser() - Open Configuration-File has FAILED !!! --- (FILE: %s / ERROR: %d)\n", p_cfg_interface->cfg_file.path,  EXIT_FAILURE);
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

		if (memcmp(cfg_key, CFG_NAME_LOG_FILE_PATH, length_key) == 0) {
			memcpy(p_cfg_interface->log_file.path, cfg_value, FILE_PATH_MAX_STRING_LENGTH);
		}
		else {
			MAIN_CFG_DEBUG_MSG("--- UNKNOWN CFG-KEY : %s\n", cfg_key);
		}
		
		NEXT_CONFIG_LINE:
		num_bytes = read_line(config_file_handle, line, 512);
	}
*/
	return NO_ERR;
}

void command_line_usage(void) {

}