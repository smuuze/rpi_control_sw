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

// ---- LOCAL DEFINITIONS -------------------------------------------------------


// ---- STATIC DATA -------------------------------------------------------------

extern CFG_INTERFACE myCfgInterface;

// ---- IMPLEMENTATION ----------------------------------------------------------

static u8 main_read_source_file_line(char* base_path, TRACE_OBJECT* p_trace_obj) {

	FILE_INTERFACE source_file;
	sprintf(source_file.path, "%s%s", base_path, p_trace_obj->file_name);
	//THREAD_DEBUG_MSG("Source-File: %s\n", source_file.path);

	if (file_open(&source_file) == 0) {
		THREAD_DEBUG_MSG("main_read_source_file_line() - File not found !!! (FILE:%s)\n", source_file.path);
		return 0;
	}

	file_read_specific_line(&source_file, p_trace_obj->line_number, p_trace_obj->source_line, TRACE_OBJECT_SOURCE_LINE_LENGTH);
	//THREAD_DEBUG_MSG("Source-Line: %s\n", p_trace_obj->source_line);

	return 1;
}

void* thread_print_trace_object_run(void* p_arg) {

	THREAD_DEBUG_MSG("thread_print_trace_object_run() - Thread started\n");
	        
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

		char trace_line[config_MAX_LENGTH_OF_FILE_LINE];
		sprintf(trace_line, "Length: %03d | Type: %d | Line: %04d | FILE: %s", trace_obj.length, trace_obj.type, trace_obj.line_number, trace_obj.file_name);
		file_append_line(&myCfgInterface.trace_file, trace_line);

		THREAD_DEBUG_MSG("%s:%d - %s\n", trace_obj.file_name, trace_obj.line_number, trace_obj.source_line);
	}
	return NULL;
}

void thread_print_trace_object_terminate(void) {

}

// ------------------------------------------------------------------------------

THREAD_INTERFACE_BUILD_THREAD(PRINT_TRACE_OBJECT_THREAD, THREAD_PRIORITY_MIDDLE, thread_print_trace_object_run, thread_print_trace_object_terminate)