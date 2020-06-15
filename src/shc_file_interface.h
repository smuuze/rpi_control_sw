/* ------------------------------------------------------------------------------
 *	Project	: Smart Home Client
 *	File	:
 *	Date	:
 *	Author	: Sebastian Lesse
 *	Brief	:
 * ------------------------------------------------------------------------------
 */
 
#ifndef _SHC_FILE_INTERFACE_H_
#define _SHC_FILE_INTERFACE_H_

#include "shc_project_configuration.h"
#include "shc_common_types.h"
/*!
 *
 */
typedef struct {
	char path[FILE_PATH_MAX_STRING_LENGTH];
	FILE* handle;
	u32 last_file_pointer;
	u32 act_file_pointer;
	u32 timestamp_last_modified;
} FILE_INTERFACE;

/*!
 *
 */
u8 file_has_changed(FILE_INTERFACE* p_file);

/*!
 *
 */
u8 file_is_existing(FILE_INTERFACE* p_file);

/*!
 *
 */
u32 file_get_size(FILE_INTERFACE* p_file);

/*!
 *
 */
u8 file_create(FILE_INTERFACE* p_file);

/*!
 *
 */
u8 file_delete(FILE_INTERFACE* p_file);

/*!
 *
 */
u8 file_rename(FILE_INTERFACE* p_old_file, FILE_INTERFACE* p_new_file);

/*!
 *
 */
u8 file_open(FILE_INTERFACE* p_file);

/*!
 *
 */
void file_close(FILE_INTERFACE* p_file);

/*!
 *
 */
u16 file_read_next_line(FILE_INTERFACE* p_file, char* next_line, u16 max_length);

/*!
 *
 */
u16 file_read_specific_line(FILE_INTERFACE* p_file, u16 line_number, char* next_line, u16 max_length);

/*!
 *
 */
u8 file_append_line(FILE_INTERFACE* p_file, char* new_line);

#endif // _SHC_FILE_INTERFACE_H_