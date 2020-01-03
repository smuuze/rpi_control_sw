/* ------------------------------------------------------------------------------
 *	Project	: Smart Home Client
 *	File	:
 *	Date	:
 *	Author	: Sebastian Lesse
 *	Brief	:
 * ------------------------------------------------------------------------------
 */

#define TRACE_DEBUG_MSG				noDEBUG_MSG
 
// ---- INCLUDES ----------------------------------------------------------------

#include "shc_project_configuration.h"
#include "shc_common_types.h"
#include "shc_common_string.h"
#include "shc_debug_interface.h"
#include "shc_timer.h"
#include "shc_trace_object.h"
#include "shc_trace_parser.h"

// ---- LOCAL DEFINITIONS -------------------------------------------------------

// ---- STATIC DATA -------------------------------------------------------------


// ---- IMPLEMENTATION ----------------------------------------------------------

static u8 tracer_get_trace_type(TRACE_OBJECT_RAW* p_raw_object, TRACE_OBJECT* p_trace_obj){

	u8 raw_type = p_raw_object->data[TRACE_PARSER_INDEX_TRACE_TYPE];

	switch (raw_type) {
		default: return 0;
		case TRACER_TRACE_TYPE_RAW_PASS  : 

			p_trace_obj->type = TRACE_OBJECT_TYPE_PASS;
			p_trace_obj->data_length = 0;

			TRACE_DEBUG_MSG("tracer_get_trace_type() - TRACE_OBJECT_TYPE_PASS\n");
			break;

		case TRACER_TRACE_TYPE_RAW_BYTE  : 

			p_trace_obj->type = TRACE_OBJECT_TYPE_BYTE;  
			p_trace_obj->data_length = 1;

			TRACE_DEBUG_MSG("tracer_get_trace_type() - TRACE_OBJECT_TYPE_BYTE\n");
			break;

		case TRACER_TRACE_TYPE_RAW_WORD  : 

			p_trace_obj->type = TRACE_OBJECT_TYPE_WORD;  
			p_trace_obj->data_length = 2;

			TRACE_DEBUG_MSG("tracer_get_trace_type() - TRACE_OBJECT_TYPE_WORD\n");
			break;

		case TRACER_TRACE_TYPE_RAW_LONG  : 

			p_trace_obj->type = TRACE_OBJECT_TYPE_LONG;  
			p_trace_obj->data_length = 4;

			TRACE_DEBUG_MSG("tracer_get_trace_type() - TRACE_OBJECT_TYPE_LONG\n");
			break;

		case TRACER_TRACE_TYPE_RAW_ARRAY : 

			p_trace_obj->type = TRACE_OBJECT_TYPE_ARRAY; 
			p_trace_obj->data_length = p_raw_object->data[TRACE_PARSER_INDEX_TRACE_TYPE + 1];

			TRACE_DEBUG_MSG("tracer_get_trace_type() - TRACE_OBJECT_TYPE_ARRAY - Length: %d\n", p_trace_obj->data_length);
			break;
	}

	return 1;
}

static void tracer_get_line_number(TRACE_OBJECT_RAW* p_raw_object, TRACE_OBJECT* p_trace_obj) {

	u16 index = 0;

	switch (p_trace_obj->type) {
		default:
			break;

		case TRACE_OBJECT_TYPE_PASS  :
			index = TRACE_PARSER_INDEX_TRACE_TYPE + TRACE_PARSER_NUM_BYTES_TRACE_TYPE;
			break;

		case TRACE_OBJECT_TYPE_BYTE  : 
			index = TRACE_PARSER_INDEX_TRACE_TYPE + TRACE_PARSER_NUM_BYTES_TRACE_TYPE + 1;
			break;

		case TRACE_OBJECT_TYPE_WORD  : 
			index = TRACE_PARSER_INDEX_TRACE_TYPE + TRACE_PARSER_NUM_BYTES_TRACE_TYPE + 2;
			break;

		case TRACE_OBJECT_TYPE_LONG  : 
			index = TRACE_PARSER_INDEX_TRACE_TYPE + TRACE_PARSER_NUM_BYTES_TRACE_TYPE + 4;
			break;

		case TRACE_OBJECT_TYPE_ARRAY : 
			index = TRACE_PARSER_INDEX_TRACE_TYPE + TRACE_PARSER_NUM_BYTES_TRACE_TYPE + 1 + p_trace_obj->data_length;
			break;
	}

	TRACE_DEBUG_MSG("tracer_get_line_number() - Index: %d\n", index);
	p_trace_obj->line_number = readU16(p_raw_object->data + index);
}

static void tracer_get_file_name(TRACE_OBJECT_RAW* p_raw_object, TRACE_OBJECT* p_trace_obj) {

	u16 index = TRACE_PARSER_INDEX_TRACE_TYPE + TRACE_PARSER_NUM_BYTES_TRACE_TYPE + TRACE_PARSER_NUM_BYTES_LINE_NUMBER;
	u16 length = p_raw_object->length - TRACE_PARSER_NUM_BYTES_HEADER - TRACE_PARSER_NUM_BYTES_BYTE_COUNT - TRACE_PARSER_NUM_BYTES_TRACE_TYPE - TRACE_PARSER_NUM_BYTES_LINE_NUMBER - TRACER_PARSER_NUM_BYTES_FOOTER;
	u16 offset = 0;

	switch (p_trace_obj->type) {
		default:
			break;

		case TRACE_OBJECT_TYPE_PASS  :
			break;

		case TRACE_OBJECT_TYPE_BYTE  : 
			index += 1;
			length -= 1;
			break;

		case TRACE_OBJECT_TYPE_WORD  : 
			index += 2;
			length -= 2;
			break;

		case TRACE_OBJECT_TYPE_LONG  : 
			index += 4;
			length -= 4;
			break;

		case TRACE_OBJECT_TYPE_ARRAY : 
			index = TRACE_PARSER_INDEX_TRACE_TYPE + TRACE_PARSER_NUM_BYTES_TRACE_TYPE + 2 + 1 + p_trace_obj->data_length;
			length -= (1 + p_trace_obj->data_length);
			break;
	}


	if (length > TRACE_OBJECT_FILE_NAME_LENGTH - 1) {
		TRACE_DEBUG_MSG("tracer_get_file_name() - OVERFLOW !! length > TRACE_OBJECT_FILE_NAME_LENGTH (%d > %d)!!!\n", length, TRACE_OBJECT_FILE_NAME_LENGTH);
		offset = length - (TRACE_OBJECT_FILE_NAME_LENGTH - 1);
		length = TRACE_OBJECT_FILE_NAME_LENGTH - 1;
	}

	if (index + length > p_raw_object->length) {
		TRACE_DEBUG_MSG("tracer_get_file_name() - OVERFLOW !! length + index > p_raw_object->length (%d + %d > %d)!!!\n", length, index, p_raw_object->length);
		return;
	}
		
	TRACE_DEBUG_MSG("tracer_get_file_name() - Index: %d | Length: %d\n", index, length);
	memcpy(p_trace_obj->file_name, p_raw_object->data + index + offset, length);
	p_trace_obj->file_name[length] = '\0';
}

static void tracer_get_trace_data(TRACE_OBJECT_RAW* p_raw_object, TRACE_OBJECT* p_trace_obj) {

}

u8 tracer_parse_object(TRACE_OBJECT_RAW* p_raw_object, TRACE_OBJECT* p_object) {

	//hex_dump((const void*)p_raw_object->data, p_raw_object->length, 16, "RX");

	p_object->type = TRACE_OBJECT_TYPE_INVALID;

	u8 header[] = {TRACE_PARSER_HEADER_BYTE_ARRAY};

	if (memcmp(p_raw_object->data, header, TRACE_PARSER_NUM_BYTES_HEADER) != 0) {
		TRACE_DEBUG_MSG("tracer_parse_object() - Incorrect header\n");
		return 0;
	}

	p_object->length = readU16_MSB(p_raw_object->data + TRACE_PARSER_INDEX_BYTE_COUNT);
	if (p_object->length == 0) {
		TRACE_DEBUG_MSG("tracer_parse_object() - Length is zero\n");
		return 0;
	}

	if (tracer_get_trace_type(p_raw_object, p_object) == 0) {
		TRACE_DEBUG_MSG("tracer_parse_object() - Invalid Tracetype\n");
		return 0;
	}

	tracer_get_line_number(p_raw_object, p_object);
	tracer_get_file_name(p_raw_object, p_object);
	tracer_get_trace_data(p_raw_object, p_object);

	return 1;

}