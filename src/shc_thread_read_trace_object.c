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
#include "shc_usart_interface.h"
#include "shc_trace_object.h"
#include "shc_trace_parser.h"
#include "shc_thread_interface.h"
#include "shc_qeue_interface.h"

// ------------------------------------------------------------------------------

QEUE_INTERFACE_INCLUDE_QEUE(RAW_TRACE_OBJECT_QEUE)

// ---- LOCAL DEFINITIONS -------------------------------------------------------


// ---- STATIC DATA -------------------------------------------------------------


// ---- IMPLEMENTATION ----------------------------------------------------------

static u8 main_read_trace_object_raw(USART_INTERFACE* p_usart, TRACE_OBJECT_RAW* p_raw_obj) {

	u16 length = usart_read_bytes(p_usart, TRACE_PARSER_NUM_BYTES_HEADER, p_raw_obj->data, DEFAULT_USART_TIMEOUT_MS);
	if (length == 0) {
		//THREAD_DEBUG_MSG("main_read_trace_object_raw() - usart_read_bytes(HEADER) has FAILED !!!\n");
		return 0;
	}

	p_raw_obj->length = length;

	u8 header[] = {TRACE_PARSER_HEADER_BYTE_ARRAY};
	if (memcmp(p_raw_obj->data, header, TRACE_PARSER_NUM_BYTES_HEADER) != 0) {
		THREAD_DEBUG_MSG("main_read_trace_object_raw() - Matching header has FAILED !!!\n");
		return 0;
	}

	length = usart_read_bytes(p_usart, TRACE_PARSER_NUM_BYTES_BYTE_COUNT, p_raw_obj->data + TRACE_PARSER_NUM_BYTES_HEADER, DEFAULT_USART_TIMEOUT_MS);
	if (length == 0) {
		THREAD_DEBUG_MSG("main_read_trace_object_raw() - usart_read_bytes(BYTE_COUNT) has FAILED !!!\n");
		return 0;
	}

	p_raw_obj->length += length;
	length = readU16_MSB(p_raw_obj->data + TRACE_PARSER_NUM_BYTES_HEADER);

	length = usart_read_bytes(p_usart, length, p_raw_obj->data + TRACE_PARSER_NUM_BYTES_HEADER + TRACE_PARSER_NUM_BYTES_BYTE_COUNT, DEFAULT_USART_TIMEOUT_MS);
	if (length == 0) {
		THREAD_DEBUG_MSG("main_read_trace_object_raw() - usart_read_bytes(PAYLOAD) has FAILED !!!\n");
		return 0;
	}

	p_raw_obj->length += length;
	return 1;
}

void* thread_read_trace_object_run(void* p_arg) {

	THREAD_DEBUG_MSG("thread_read_trace_object_run() - Thread started\n");
	
	USART_INTERFACE usart0;
	usart0._handle_id = -1;
	usart0.baudrate = USART_BAUDRATE_230400;
	memset(usart0.device_name, 0x00, USART_DEVICE_NAME_MAX_LENGTH);
	memcpy(usart0.device_name, DEAFULT_USART_DEVICE, string_length(DEAFULT_USART_DEVICE));

	if (usart_init(&usart0) == 0) {
		THREAD_DEBUG_MSG("main() - Init USART has FAILED !!!\n");
		return NULL;
	}

	TRACE_OBJECT_RAW raw_obj;

	while (1) {

		usleep(50000); // reduce cpu-load

		if (main_read_trace_object_raw(&usart0, &raw_obj) == 0) {
			continue;
		}

		if (RAW_TRACE_OBJECT_QEUE_mutex_get() == 0) {
			THREAD_DEBUG_MSG("thread_read_trace_object_run() - QEUE is busy\n");
			continue;
		}
		
		if (RAW_TRACE_OBJECT_QEUE_is_full()) {
			THREAD_DEBUG_MSG("thread_read_trace_object_run() - QEUE is full\n");
		} else if (RAW_TRACE_OBJECT_QEUE_enqeue(&raw_obj)) {
			//THREAD_DEBUG_MSG("thread_read_trace_object_run() - Object enqeued <<<\n");
		} else {
			THREAD_DEBUG_MSG("thread_read_trace_object_run() - Object enqeued has FAILED !!!\n");
		}

		RAW_TRACE_OBJECT_QEUE_mutex_release();
	}

	return NULL;
}

void thread_read_trace_object_terminate(void) {

}

// ------------------------------------------------------------------------------

THREAD_INTERFACE_BUILD_THREAD(READ_TRACE_OBJECT_THREAD, THREAD_PRIORITY_MIDDLE, thread_read_trace_object_run, thread_read_trace_object_terminate)