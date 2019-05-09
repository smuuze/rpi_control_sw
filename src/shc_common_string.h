/* ------------------------------------------------------------------------------
 *	Project	: Smart Home Client
 *	File	:
 *	Date	:
 *	Author	: Sebastian Lesse
 *	Brief	:
 * ------------------------------------------------------------------------------
 */
 
#ifndef _SHC_COMMON_STRING_H_
#define _SHC_COMMON_STRING_H_

#include "shc_project_configuration.h"
#include "shc_common_types.h"

/*
 *
 */
void hex_dump(const void *src, size_t length, size_t line_size, char *prefix);

/*!
 * returns number of bytes read
 */
u16 read_line(FILE* file_handle, char* p_buffer_to, u16 num_max_bytes);

/*
 *
 */
u8 write_line(FILE* file_handle, char* p_buffer_from, u16 num_max_bytes);

/*
 *
 */
void split_string(char splitter, char* p_string_in, u16 string_in_len, char* p_string_out_1, u16 string_out_1_max_len, char* p_string_out_2, u16 string_out_2_max_len);

/*
 *
 */
u16 string_length(char* p_str);

/*
 *
 */
u16 string_trim(u8* p_string, u8 max_length);

/*
 *
 */
u8 hex_string_to_byte_array(char* hex_string, u16 hex_string_len, u8* byte_array, u8 byte_array_max_len);

/*
 *
 */
u8 byte_array_string_to_hex_string(u8* byte_array, u8 byte_array_len, char* hex_string, u16 hex_string_max_len);

#endif // _SHC_COMMON_STRING_H_