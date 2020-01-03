/* ------------------------------------------------------------------------------
 *	Project	: Tracer
 *	File	:
 *	Date	:
 *	Author	: Sebastian Lesse
 *	Brief	:
 * ------------------------------------------------------------------------------
 */
 
#ifndef _SHC_TRACE_OBJECT_H_
#define _SHC_TRACE_OBJECT_H_

#include "shc_project_configuration.h"
#include "shc_common_types.h"

// ------------------------------------------------------------------------------

#ifndef TRACE_OBJECT_FILE_NAME_LENGTH
#define TRACE_OBJECT_FILE_NAME_LENGTH		256
#endif
#ifndef TRACE_OBJECT_SOURCE_LINE_LENGTH
#define TRACE_OBJECT_SOURCE_LINE_LENGTH		512
#endif

#ifndef TRACE_OBJECT_ARRAY_LENGTH		
#define TRACE_OBJECT_ARRAY_LENGTH		256
#endif

#ifndef TRACE_OBJECT_RAW_DATA_LENGTH
#define TRACE_OBJECT_RAW_DATA_LENGTH		1024
#endif

// ------------------------------------------------------------------------------

/*!
 *
 */
typedef enum {
	TRACE_OBJECT_TYPE_INVALID = 0x00,
	TRACE_OBJECT_TYPE_PASS = 0x01,
	TRACE_OBJECT_TYPE_BYTE,
	TRACE_OBJECT_TYPE_WORD,
	TRACE_OBJECT_TYPE_LONG,
	TRACE_OBJECT_TYPE_ARRAY
} TRACE_OBJECT_TYPE;

/*!
 *
 */
typedef struct {

	TRACE_OBJECT_TYPE type;
	u16 length;
	u16 line_number;
	u8 data_length;
	char file_name[TRACE_OBJECT_FILE_NAME_LENGTH];
	char source_line[TRACE_OBJECT_SOURCE_LINE_LENGTH];

	union {
		u8 byte;
		u16 word;
		u32 integer;
		u8 array[TRACE_OBJECT_ARRAY_LENGTH];
	} data;

} TRACE_OBJECT;

/*!
 *
 */
typedef struct {
	u8 data[TRACE_OBJECT_RAW_DATA_LENGTH];
	u16 length;
} TRACE_OBJECT_RAW;

#endif // _SHC_TRACE_OBJECT_H_