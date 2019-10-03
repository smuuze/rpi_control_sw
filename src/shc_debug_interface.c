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


// ---- STATIC DATA -------------------------------------------------------------


// ---- IMPLEMENTATION ----------------------------------------------------------

/*!
 *
 */
void log_message(FILE_INTERFACE* p_file, u8 error_level, STRING_BUFFER_BIG* p_msg_from) {
	
	char path[128];
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
	
	if (file_is_existing(p_file) == 0) {
		p_file->handle = fopen((const char*)path, "w");
		LOG_DEBUG_MSG("log_message() - Log-File does not exists, will create it\n");	
		
	} else {
		p_file->handle = fopen((const char*)path, "a");
		//LOG_DEBUG_MSG("log_message() - Appending Log-Message to existing file \n");	
	}	
	
	if (p_file->handle == NULL) {
		LOG_DEBUG_MSG("log_message() - Open Log-File has FAILED !!! --- \n");
		return;
	}
	
	char date_string[128];
	memset(date_string, 0x00, 128);
	
	FILE* pipe = popen("date", "r");
	read_line(pipe, date_string, 128);
	fclose(pipe);
	
	int err_code = fprintf(p_file->handle, "%s \t %d \t %s \r\n", date_string, error_level, p_msg_from->payload);
	if (err_code < 0) {
		LOG_DEBUG_MSG("log_message() - Writing File has FAILED !!! --- (ERROR: %d)\n", err_code);		
	}
		
	fclose(p_file->handle);
}