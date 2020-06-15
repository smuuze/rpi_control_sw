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

#include "shc_file_interface.h"
#include "shc_debug_interface.h"

// -------- DEBUGGING -------------------------------------------------------------------

#define LOG_DEBUG_MSG				noDEBUG_MSG

// ---- LOCAL DEFINITIONS -------------------------------------------------------

/*!
 *
 */
//void log_message(FILE_INTERFACE* p_file, u8 error_level, STRING_BUFFER_BIG* p_msg_from)
void log_message(FILE_INTERFACE* p_file, u8 error_level, char* p_msg_from) {
	
	if (string_length(p_file->path) == 0) {
		return;
	}

	char path[1024];
	sprintf(path, "%s", p_file->path);
	//LOG_DEBUG_MSG("LOG-DEBUG: Using Log-File: %s \n", path);
	
	if (file_get_size(p_file) > LOG_FILE_MAX_SIZE_BYTES) {
				
		LOG_DEBUG_MSG("log_message() - Limit of file size reached\n");
		
		FILE_INTERFACE backup_file;
		
		memcpy(backup_file.path, p_file->path, FILE_PATH_MAX_STRING_LENGTH);		
		string_append(backup_file.path, ".old", FILE_PATH_MAX_STRING_LENGTH);
		
		u8 err = 0;
		
		if (file_is_existing(&backup_file) != 0) {
		
			LOG_DEBUG_MSG("log_message() - Old log-file exists, will delete it\n");
			
			err = file_delete(&backup_file);
			if (err != 0) {
				LOG_DEBUG_MSG("log_message() - Deleting old file has FAILED !!! --- (error: %d)\n", err);
			}
		}
		
		LOG_DEBUG_MSG("log_message() - Renaming old file \"%s\" -> \"%s\"\n", p_file->path, backup_file.path);
		
		err = file_rename(p_file, &backup_file);
		if (err != 0) {
				LOG_DEBUG_MSG("log_message() - Renaming old file has FAILED !!! --- (error: %d)\n", err);
		}
	}
		
	LOG_DEBUG_MSG("log_message() - check for existing Log-File\n");
	
	if (file_is_existing(p_file) == 0) {
				
		LOG_DEBUG_MSG("log_message() - Log-File not existing\n");

		if (file_create(p_file)) {
			LOG_DEBUG_MSG("log_message() - file_create() has FAILED !!! --- \n");
			return;
		}	
	}
	
	char date_string[128];
	memset(date_string, 0x00, 128);
	
	// get the actual date-time
	FILE* pipe = popen("date", "r");
	read_line(pipe, date_string, 128);
	fclose(pipe);
	
	char new_line[1024];
	sprintf(new_line, "%s \t %d \t %s \r\n", date_string, error_level, p_msg_from /*->payload*/);

	if (file_append_line(p_file, new_line) == 0) {
		LOG_DEBUG_MSG("log_message() - file_append_line() has FAILED !!! --- \n");
	}
}