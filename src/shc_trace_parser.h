/* ------------------------------------------------------------------------------
 *	Project	: Smart Home Client
 *	File	:
 *	Date	:
 *	Author	: Sebastian Lesse
 *	Brief	:
 * ------------------------------------------------------------------------------
 */
 
#ifndef _SHC_TRACE_PARSER_H_
#define _SHC_TRACE_PARSER_H_

#include "shc_project_configuration.h"
#include "shc_common_types.h"
#include "shc_trace_object.h"

#define TRACE_PARSER_HEADER_BYTE_ARRAY		0xFA,0xFA,0xFA

#define TRACE_PARSER_NUM_BYTES_HEADER		3
#define TRACE_PARSER_NUM_BYTES_BYTE_COUNT	2
#define TRACE_PARSER_NUM_BYTES_TRACE_TYPE	1
#define TRACE_PARSER_NUM_BYTES_LINE_NUMBER	2
#define TRACER_PARSER_NUM_BYTES_FOOTER		1

#define TRACE_PARSER_INDEX_BYTE_COUNT		(TRACE_PARSER_NUM_BYTES_HEADER)
#define TRACE_PARSER_INDEX_TRACE_TYPE		(TRACE_PARSER_NUM_BYTES_HEADER + TRACE_PARSER_NUM_BYTES_BYTE_COUNT)
#define TRACE_PARSER_INDEX_OF_TYPE
#define TRACE_PARSER_INDEX_OF_LINE_NUMBER

#define TRACER_TRACE_TYPE_RAW_PASS		0x01
#define TRACER_TRACE_TYPE_RAW_BYTE		0x02
#define TRACER_TRACE_TYPE_RAW_WORD		0x03
#define TRACER_TRACE_TYPE_RAW_LONG		0x04
#define TRACER_TRACE_TYPE_RAW_ARRAY		0x05


/*!
 *
 */
u8 tracer_parse_object(TRACE_OBJECT_RAW* p_raw_object, TRACE_OBJECT* p_object);

#endif // _SHC_TRACE_PARSER_H_