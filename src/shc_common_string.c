/* ------------------------------------------------------------------------------
 *	Project	: Smart Home Client
 *	File	:
 *	Date	:
 *	Author	: Sebastian Lesse
 *	Brief	:
 * ------------------------------------------------------------------------------
 */
 
// ---- INCLUDES ----------------------------------------------------------------

#include "shc_project_configuration.h"
#include "shc_common_types.h"
#include "shc_common_string.h"

// ---- LOCAL DEFINITIONS -------------------------------------------------------


// ---- STATIC DATA -------------------------------------------------------------


// ---- IMPLEMENTATION ----------------------------------------------------------

/*
 *
 */
void hex_dump(const void *src, size_t length, size_t line_size, char *prefix) {

	//#if STRING_DEBUG_MSG != noDEBUG_MSG	
	
	int i = 0;
	const unsigned char *address = src;
	const unsigned char *line = address;

	STRING_DEBUG_MSG("%s | ", prefix);
	
	while (length-- > 0) {
	
		STRING_DEBUG_MSG("%02X ", *address++);		
		if (!(++i % line_size) || (length == 0 && i % line_size)) {

			if (length == 0) {			
				while (i++ % line_size) {
					STRING_DEBUG_MSG("__ ");
				}
			}
			
			STRING_DEBUG_MSG(" | ");  /* right close */			
			while (line < address) {
				unsigned char c = *line++;
				STRING_DEBUG_MSG("%c", (c < 33 || c == 255) ? 0x2E : c);
			}
			
			STRING_DEBUG_MSG("\n");			
			if (length > 0) {
				STRING_DEBUG_MSG("%s | ", prefix);
			}
		}
	}
	
	/*
	#else
	
	(void ) src;
	(void ) length;
	(void ) line_size;
	(void ) prefix;
	
	#endif
	*/
}

/*!
 * returns number of bytes read
 */
u16 read_line(FILE* file_handle, char* p_buffer_to, u16 num_max_bytes) {
	
	if (num_max_bytes == 0) {
		return 0;
	}
	
	char character;
	u16 num_bytes_read = 0;
	
	//STRING_DEBUG_MSG("--- Line: \n");
	
	while ((character = getc(file_handle)) != 255) {
		
		if (num_bytes_read == num_max_bytes - 1) {
			break;
		}
		
		if (character == '\n') {
			//STRING_DEBUG_MSG("----> End of line reached (LF)\n");
			break;
		}	
		
		if ((character  < 32 || character > 254)) {		
			//STRING_DEBUG_MSG("----> Character is not supported (%d)\n", character);
			continue;
		}
		
		//STRING_DEBUG_MSG("%d ", character);
		
		p_buffer_to[num_bytes_read] = character;
		num_bytes_read += 1;
	}
	
	//STRING_DEBUG_MSG("\n");
	
	p_buffer_to[num_bytes_read + 1] = '\0';
	
	return num_bytes_read;
}

//u8 write_line(FILE* file_handle, char* p_buffer_from, u16 num_max_bytes) {
//
//	fputs(p_buffer_from, file_handle);
//	return ERR_WRITE_FILE;
//}

/*
 *
 */
void split_string(char splitter, char* p_string_in, u16 string_in_len, char* p_string_out_1, u16 string_out_1_max_len, char* p_string_out_2, u16 string_out_2_max_len) {

	u8 i = 0;
	u8 j = 0;
	u8 splitter_detected = 0;
	
	for ( ; i < string_in_len; i++) {	
	
		if (p_string_in[i] == '\0') {
			break;
		}
	
		if (splitter_detected == 0 && p_string_in[i] == splitter) {
			p_string_out_1[j] = '\0';
			j = 0;
			splitter_detected = 1;
			continue;
		}
	
		if (splitter_detected != 0) {
		
			if (j < string_out_2_max_len - 1) {
				p_string_out_2[j] = p_string_in[i];
				j++;
			}
			
		} else {
		
			if (j < string_out_1_max_len - 1) {
				p_string_out_1[j] = p_string_in[i];
				j++;
			}			
		}
	}	
		
	p_string_out_2[j] = '\0';
}

/*
 *
 */
u16 string_length(char* p_str) {
	
	return strlen(p_str);
	
	/*
	u16 i = 0;
	while (p_str[i] != '\0') {
		i++;
	}	
	return i;
	*/
}

u8 hex_string_to_byte_array(char* hex_string, u16 hex_string_len, u8* byte_array, u8 byte_array_max_len) {
	
	if (hex_string_len < 2) {
		return 0;
	}
	
	u16 i = 0;
	u8 j = 0;
	u8 is_upper_nibble = 1;
	
	memset(byte_array, 0x00, byte_array_max_len);
	
	while (i < hex_string_len && j < byte_array_max_len) {
		
		char nibble = hex_string[i++];
		
		if (nibble == '\0') {
			//STRING_DEBUG_MSG("---> End of string reached\n");
			break;
		}
		
		u8 nibble_value = 0;
		u8 nibble_factor = is_upper_nibble != 0 ? 16 : 1;
		
		if (nibble > 47 && nibble < 57) {
			// is between 0 and 9
			nibble_value = nibble - '0';
			
		} else if (nibble > 64 && nibble < 91) {		
			// is between A and Z
			nibble_value = nibble - 'A' + 10;
			
		} else if (nibble > 97 && nibble < 122) {
			// is between a and z
			nibble_value = nibble - 'a' + 10;
			
		} else {
			//STRING_DEBUG_MSG("---> Character is not valid for HEX-String: %c\n", nibble);
			continue;
		}
		
		byte_array[j] += (nibble_value * nibble_factor);
		
		if (is_upper_nibble == 0) {
			is_upper_nibble = 1;
			j++;
		} else {
			is_upper_nibble = 0;
		}
	}
	
	return j;
}

u8 byte_array_string_to_hex_string(u8* byte_array, u8 byte_array_len, char* hex_string, u16 hex_string_max_len) {
	
	if (hex_string_max_len < 2 || byte_array_len == 0) {
		return 0;
	}
	
	u16 i = 0;
	u8 j = 0;
	
	memset(hex_string, 0x00, hex_string_max_len);
	
	while (i < byte_array_len && j < hex_string_max_len - 2) {
		
		u8 nibble = byte_array[i] >> 4;
		char character = '0';
		
		if (nibble > 9) {
			// is between A and F
			character = nibble + 'A' - 10;
			
		} else {
			// is between 0 and 9
			character = nibble + '0';
		}
		
		hex_string[j++] = character;
		
		nibble = (byte_array[i] & 0x0F);
		
		if (nibble > 9) {
			// is between A and F
			character = nibble + 'A' - 10;
			
		} else {
			// is between 0 and 9
			character = nibble + '0';
		}
		
		hex_string[j++] = character;
		
		i++;
	}
	
	return j;
}