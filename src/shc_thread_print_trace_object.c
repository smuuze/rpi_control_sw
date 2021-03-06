/* ------------------------------------------------------------------------------
 *	Project	: Smart Home Client
 *	File	:
 *	Date	:
 *	Author	: Sebastian Lesse
 *	Brief	:
 * ------------------------------------------------------------------------------
 */

#define THREAD_DEBUG_MSG				DEBUG_MSG
 
// ---- INCLUDES ----------------------------------------------------------------

#include "shc_project_configuration.h"
#include "shc_common_configuration.h"
#include "shc_common_types.h"
#include "shc_common_string.h"
#include "shc_debug_interface.h"
#include "shc_trace_object.h"
#include "shc_trace_parser.h"
#include "shc_thread_interface.h"
#include "shc_qeue_interface.h"

// ------------------------------------------------------------------------------

QEUE_INTERFACE_INCLUDE_QEUE(TRACE_OBJECT_QEUE)
//MQTT_INTERFACE_INCLUDE_HOST(MQTT_TRACER)
//TIME_MGMN_BUILD_TIMER(MQTT_CONNECT_TIMER)

// ---- LOCAL DEFINITIONS -------------------------------------------------------

#ifndef config_MAX_LENGTH_OF_TRACE_OUTPUT_LINE
#define config_MAX_LENGTH_OF_TRACE_OUTPUT_LINE		2048
#endif

// ---- STATIC DATA -------------------------------------------------------------

extern CFG_INTERFACE myCfgInterface;

// ---- IMPLEMENTATION ----------------------------------------------------------

static void main_get_traceoutput_from_source_line(char* p_string_to, char* p_string_from) {

	u16 length = string_length(p_string_from);
	u16 i = 0;

	u16 start = 0;

	for ( ; i < length; i++) {
		if (p_string_from[i] == '\"') {
			start = i;
			break;
		}
	}

	u16 end = start;

	for ( i = start + 1 ; i < length; i++) {
		if (p_string_from[i] == '\"') {
			end = i;
			break;
		}
	}

	length = end - start;

	memcpy(p_string_to, p_string_from + start, length);
}

static u8 main_read_source_file_line(char* base_path, TRACE_OBJECT* p_trace_obj) {

	FILE_INTERFACE source_file;
	sprintf(source_file.path, "%s%s", base_path, p_trace_obj->file_name);
	//THREAD_DEBUG_MSG("Source-File: %s\n", source_file.path);

	if (file_open(&source_file) == 0) {
		THREAD_DEBUG_MSG("main_read_source_file_line() - File not found !!! (FILE:%s)\n", source_file.path);
		return 0;
	}

	char trace_line[TRACE_OBJECT_SOURCE_LINE_LENGTH];
	memset(trace_line, '\0', TRACE_OBJECT_SOURCE_LINE_LENGTH);

	file_read_specific_line(&source_file, p_trace_obj->line_number, trace_line , TRACE_OBJECT_SOURCE_LINE_LENGTH);
	main_get_traceoutput_from_source_line(p_trace_obj->source_line, trace_line);

	//THREAD_DEBUG_MSG("Source-Line: %s\n", p_trace_obj->source_line);

	return 1;
}

/*
static void main_connect_mqtt_host(CFG_INTERFACE* p_cfgInterface) {

	if (MQTT_TRACER_connection_lost() == 0) {
		return;
	}

	if (MQTT_CONNECT_TIMER_is_active()) {

		if (MQTT_CONNECT_TIMER_is_up(MQTT_CONNECTION_INTERVAL_TIMEOUT_MS) == 0) {
			//MAIN_DEBUG_MSG("main_connect_mqtt_host() - Waiting for MQTT_CONNECTION_INTERVAL_TIMEOUT_MS\n");
			return;
		}
	}

	// --- Initialize MQTT Interface
	THREAD_DEBUG_MSG("main_connect_mqtt_host() - INITIALIZE MQTT INTERFACE\n");
	THREAD_DEBUG_MSG("                         - Host-Addr: \"%s\"\n", p_mqttInterface->host_address);
	THREAD_DEBUG_MSG("                         - Client-ID: \"%s\"\n", p_mqttInterface->client_id);

	LOG_MSG(NO_ERR, &myCfgInterface.log_file, "Initialize MQTT-Interface (Host-Addr: %s / Client-ID: %s)", p_mqttInterface->host_address, p_mqttInterface->client_id);

	u8 err_code = 0;

	if ((err_code = MQTT_TRACER_init()) != NO_ERR) {
			
		THREAD_DEBUG_MSG("main_connect_mqtt_host() - Initializing MQTT-Client has FAILED !!! - error-code = %d\n", err_code);
		LOG_MSG(ERR_LEVEL_FATAL, &p_cfgInterface->log_file, "Initializing MQTT-Client has FAILED !!! --- (error-code = %d)", err_code);

	} else 	if ((err_code = MQTT_TRACER_connect()) != NO_ERR) {

		THREAD_DEBUG_MSG("main_connect_mqtt_host() - Connect to MQTT-Host has FAILED !!! - error-code = %d\n", err_code);
		LOG_MSG(ERR_LEVEL_FATAL, &p_cfgInterface->log_file, "Connect to MQTT-Host has FAILED !!! --- (error-code = %d)", err_code);

	} else {

		THREAD_DEBUG_MSG("main_connect_mqtt_host() - Connection to MQTT-Broker has benn established\n");
		LOG_MSG(ERR_LEVEL_INFO, &p_cfgInterface->log_file, "Connection to MQTT-Broker has been established");

		p_mqttInterface->connection_lost = 0;
		p_mqttInterface->initialized = 1;
	}
			
	MQTT_CONNECT_TIMER_start();
}
*/

static void thread_print_trace_object_get_hex_string(char* p_string, const void *src, size_t length, size_t line_size) {
	
	int i = 0;
	const unsigned char *address = src;
	const unsigned char *line = address;
	char* p_string_addr = p_string;

	sprintf(p_string_addr, "   ");
	p_string_addr += 3;
	
	while (length-- > 0) {
	
		sprintf(p_string_addr, "%02X ", *address++);
		p_string_addr += 3;

		if (!(++i % line_size) || (length == 0 && i % line_size)) {

			if (length == 0) {			
				while (i++ % line_size) {
					sprintf(p_string_addr, "__ ");
					p_string_addr += 3;
				}
			}
			
			sprintf(p_string_addr, " | "); // right close
			p_string_addr += 3;

			while (line < address) {
				
				unsigned char c = *line++;
				sprintf(p_string_addr, "%c", (c < 33 || c == 255) ? 0x2E : c);
				p_string_addr += 1;
			}
			
			sprintf(p_string_addr, "\n");
			p_string_addr += 1;

			if (length > 0 ) {
				sprintf(p_string_addr, "   ");
				p_string_addr += 3;
			}
		}
	}
}

void thread_print_trace_object_prepare_output(TRACE_OBJECT* p_trace_obj, char* p_string, u8 output_level) {

	switch (p_trace_obj->type) {
		default:
		case TRACE_OBJECT_TYPE_PASS:
			sprintf(p_string, "%s:%d - %s\n", p_trace_obj->file_name, p_trace_obj->line_number, p_trace_obj->source_line);
			break;

		case TRACE_OBJECT_TYPE_BYTE:
			sprintf(p_string, "%s:%d - %s\n\t - Data: %d (0x%02X)\n", p_trace_obj->file_name, p_trace_obj->line_number, p_trace_obj->source_line, p_trace_obj->data.byte, p_trace_obj->data.byte);
			break;

		case TRACE_OBJECT_TYPE_WORD:
			sprintf(p_string, "%s:%d - %s\n\t - Data: %d (0x%04X)\n", p_trace_obj->file_name, p_trace_obj->line_number, p_trace_obj->source_line, p_trace_obj->data.word, p_trace_obj->data.word);
			break;

		case TRACE_OBJECT_TYPE_LONG:
			sprintf(p_string, "%s:%d - %s\n\t - Data: %d (0x%08X)\n", p_trace_obj->file_name, p_trace_obj->line_number, p_trace_obj->source_line, p_trace_obj->data.integer, p_trace_obj->data.integer);
			break;

		case TRACE_OBJECT_TYPE_ARRAY:
			sprintf(p_string, "%s:%d - %s\n", p_trace_obj->file_name, p_trace_obj->line_number, p_trace_obj->source_line);
			thread_print_trace_object_get_hex_string(p_string + string_length(p_string), (const void*) p_trace_obj->data.array, p_trace_obj->data_length, 16);
			break;
	}
}

void* thread_print_trace_object_run(void* p_arg) {

	THREAD_DEBUG_MSG("thread_print_trace_object_run() - Thread started\n");

	if (myCfgInterface.output.mqtt != 0) {
		//main_connect_mqtt_host(&myCfgInterface);
	}
	        
	TRACE_OBJECT trace_obj;

	while (1) {

		usleep(50000); // reduce cpu-load

		if (TRACE_OBJECT_QEUE_is_empty()) {
			continue;
		}

		if (TRACE_OBJECT_QEUE_mutex_get() == 0) {
			continue;
		}
			
		u8 object_available = TRACE_OBJECT_QEUE_deqeue(&trace_obj);
		TRACE_OBJECT_QEUE_mutex_release();

		if (object_available == 0) {
			continue;
		}

		if (main_read_source_file_line(myCfgInterface.base_path.path, &trace_obj) == 0) {
			//THREAD_DEBUG_MSG("main() - Reading Source-File has FAILED !!!\n");
			continue;
		}

		char trace_line[config_MAX_LENGTH_OF_TRACE_OUTPUT_LINE];
		memset(trace_line, '\0', config_MAX_LENGTH_OF_TRACE_OUTPUT_LINE);

		thread_print_trace_object_prepare_output(&trace_obj, trace_line, 1);
		//sprintf(trace_line, "Length: %03d | Type: %d | Line: %04d | FILE: %s", trace_obj.length, trace_obj.type, trace_obj.line_number, trace_obj.file_name);

		// if console is activated
		if (myCfgInterface.output.console != 0) {
			printf("%s", trace_line);
		}

		if (myCfgInterface.output.file != 0) {
			file_append_line(&myCfgInterface.trace_file, trace_line);
		}

		if (myCfgInterface.output.mqtt != 0) {
			//MQTT_TRACER_send_message();

			//while ( !MQTT_TRACER_delivery_complete() ) { usleep(50000); };
		}
	}
	return NULL;
}

void thread_print_trace_object_terminate(void) {

}

// ------------------------------------------------------------------------------

THREAD_INTERFACE_BUILD_THREAD(PRINT_TRACE_OBJECT_THREAD, THREAD_PRIORITY_MIDDLE, thread_print_trace_object_run, thread_print_trace_object_terminate)