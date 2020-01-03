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

QEUE_INTERFACE_INCLUDE_QEUE(RAW_TRACE_OBJECT_QEUE)
QEUE_INTERFACE_INCLUDE_QEUE(TRACE_OBJECT_QEUE)

// ---- LOCAL DEFINITIONS -------------------------------------------------------


// ---- STATIC DATA -------------------------------------------------------------


// ---- IMPLEMENTATION ----------------------------------------------------------

void* thread_parse_trace_object_run(void* p_arg) {

	THREAD_DEBUG_MSG("thread_parse_trace_object_run() - Thread started\n");
	    
	TRACE_OBJECT_RAW raw_obj;
	TRACE_OBJECT trace_obj;

	while (1) {

		usleep(50000); // reduce cpu-load

		if (RAW_TRACE_OBJECT_QEUE_is_empty()) {
			continue;
		}

		if (RAW_TRACE_OBJECT_QEUE_mutex_get() == 0) {
			continue;
		}
			
		u8 object_available = RAW_TRACE_OBJECT_QEUE_deqeue(&raw_obj);
		RAW_TRACE_OBJECT_QEUE_mutex_release();

		if (object_available == 0) {
			continue;
		}

		if (tracer_parse_object(&raw_obj, &trace_obj) == 0) {
			THREAD_DEBUG_MSG("main() - Parsing Trace-Object has FAILED !!!\n");
			continue;
		}
			
		while (TRACE_OBJECT_QEUE_mutex_get() == 0) {
			//THREAD_DEBUG_MSG("thread_parse_trace_object_run() - TRACE_OBJECT_QEUE is busy\n");
			//continue;
			usleep(50000); // reduce cpu-load
		}
			
		if (TRACE_OBJECT_QEUE_is_full()) {
			THREAD_DEBUG_MSG("thread_parse_trace_object_run() - TRACE_OBJECT_QEUE is full\n");
		} else if (TRACE_OBJECT_QEUE_enqeue(&trace_obj)) {
			//THREAD_DEBUG_MSG("thread_parse_trace_object_run() - TRACE_OBJECT enqeued <<<\n");
		} else {
			THREAD_DEBUG_MSG("thread_parse_trace_object_run() - TRACE_OBJECT enqeued has FAILED !!!\n");
		}
		
		TRACE_OBJECT_QEUE_mutex_release();
	}

	return NULL;
}

void thread_parse_trace_object_terminate(void) {

}

// ------------------------------------------------------------------------------

THREAD_INTERFACE_BUILD_THREAD(PARSE_TRACE_OBJECT_THREAD, THREAD_PRIORITY_MIDDLE, thread_parse_trace_object_run, thread_parse_trace_object_terminate)