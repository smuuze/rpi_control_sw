/* ------------------------------------------------------------------------------
 *	Project	: Smart Home Client
 *	File	:
 *	Date	:
 *	Author	: Sebastian Lesse
 *	Brief	:
 * ------------------------------------------------------------------------------
 */

#define FILE_DEBUG_MSG					noDEBUG_MSG
 
// ---- INCLUDES ----------------------------------------------------------------

#include "shc_project_configuration.h"
#include "shc_common_types.h"
#include "shc_common_string.h"
#include "shc_debug_interface.h"

#include "shc_file_interface.h"

#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>

// ---- LOCAL DEFINITIONS -------------------------------------------------------


// ---- STATIC DATA -------------------------------------------------------------


// ---- IMPLEMENTATION ----------------------------------------------------------


u8 file_has_changed(FILE_INTERFACE* p_file) {

	struct stat file_attribute;
	if (stat(p_file->path, &file_attribute) == 0) {
		return 0;
		
	} else if (file_attribute.st_mtime > p_file->timestamp_last_modified) {
		return 1;
		
	} else {
		return 0;
	}
}

u8 file_is_existing(FILE_INTERFACE* p_file) {

	struct stat file_attribute;
	if (stat(p_file->path, &file_attribute) < 0) {
		return 0;
		
	} else {
		return 1;
	}
}

u32 file_get_size(FILE_INTERFACE* p_file) {

	struct stat file_attribute;
	if (stat(p_file->path, &file_attribute) < 0) {
		return 0;		
	}
	
	return file_attribute.st_size;
}

u8 file_delete(FILE_INTERFACE* p_file) {
	return remove(p_file->path);
}

u8 file_rename(FILE_INTERFACE* p_old_file, FILE_INTERFACE* p_new_file) {
	return rename(p_old_file->path, p_new_file->path);
}

u8 file_create(FILE_INTERFACE* p_file) {
	p_file->handle = fopen((const char*)p_file->path, "w");

	if (p_file->handle != NULL) {
		file_close(p_file);
		return 1;
	} 

	return 0;
}

u8 file_open(FILE_INTERFACE* p_file) {

	file_close(p_file);

	p_file->handle = fopen((const char*)p_file->path, "r");
	if (p_file->handle == NULL) {
		FILE_DEBUG_MSG("file_open() - Open file has FAILED !!! --- (FILE: %s / ERROR: %d)\n", p_file->path,  EXIT_FAILURE);
		return 0;
	}

	//FILE_DEBUG_MSG("file_open() - FILE: %s\n", p_file->path);
	return 1;
}

void file_close(FILE_INTERFACE* p_file) {

	if (p_file->handle == NULL) {
		return;
	}

	//FILE_DEBUG_MSG("file_close() - FILE: %s\n", p_file->path);
	fclose(p_file->handle);
	p_file->handle = NULL;
}

u16 file_read_line(FILE* file_handle, char* p_buffer_to, u16 num_max_bytes) {
	
	if (num_max_bytes == 0) {
		return 0;
	}
	
	char character;
	u16 num_bytes_read = 0;
	
	//FILE_DEBUG_MSG("read_line() - Line: ");
	
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
		
		//FILE_DEBUG_MSG("%d ", character);		
		p_buffer_to[num_bytes_read++] = character;
	}
	
	//FILE_DEBUG_MSG("\n");
	
	p_buffer_to[num_bytes_read] = '\0';
	return num_bytes_read;
}

u16 file_read_next_line(FILE_INTERFACE* p_file, char* next_line, u16 max_length) {
	
	if (p_file->handle == NULL) {
		FILE_DEBUG_MSG("file_read_next_line() - File is not open !!! --- (FILE: %s)\n", p_file->path);
		return 0;
	}
	
	u16 num_bytes = file_read_line(p_file->handle, next_line, max_length);

	//FILE_DEBUG_MSG("file_read_next_line() - FILE: %s - LINE: %s\n", p_file->path, next_line);
	return num_bytes;
}

u16 file_read_specific_line(FILE_INTERFACE* p_file, u16 line_number, char* next_line, u16 max_length) {
	
	if (line_number == 0) {
		FILE_DEBUG_MSG("file_read_specific_line() - Invalid line-number given !!!");
		return 0;
	}

	if (p_file->handle == NULL) {
		FILE_DEBUG_MSG("file_read_next_line() - File is not open !!! --- (FILE: %s)\n", p_file->path);
		return 0;
	}
	
	u16 num_bytes_line = 0;

	do {
		num_bytes_line = file_read_line(p_file->handle, next_line, max_length);
	} while (--line_number != 0);

	return num_bytes_line;
}

u8 file_append_line(FILE_INTERFACE* p_file, char* new_line) {

	file_close(p_file);
	
	p_file->handle = fopen((const char*)p_file->path, "a");
	if (p_file->handle == NULL) {
		FILE_DEBUG_MSG("file_append_line() - Open file has FAILED !!! --- (FILE: %s)\n", p_file->path);
		return ERR_FILE_OPEN;
	}

	int err_code = fprintf(p_file->handle, "%s\r\n", new_line);
	fclose(p_file->handle);
	p_file->handle = NULL;

	if (err_code < 0) {
		FILE_DEBUG_MSG("file_append_line() - Writing File has FAILED !!! --- (FILE: %s)\n", p_file->path);
		return 0;
	} else {
		return 1;
	}

}
