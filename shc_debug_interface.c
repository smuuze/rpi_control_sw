

#include "shc_project_configuration.h"
#include "shc_common_types.h"

/*!
 *
 */
void log_message(FILE_INTERFACE* p_file, u8 error_level, STRING_BUFFER* p_msg_from) {
	
	char path[64];
	sprintf(path, "%s", p_file->path);
	LOG_DEBUG_MSG("LOG-DEBUG: Using Log-File: %s \n", path);
	
	if (file_is_existing(p_file) == 0) {
		p_file->handle = fopen((const char*)path, "w");
		LOG_DEBUG_MSG("LOG-DEBUG: Log-File does not exists, will create it\n");	
		
	} else {
		p_file->handle = fopen((const char*)path, "a");
		LOG_DEBUG_MSG("LOG-DEBUG: Appending Log-Message to existing file \n");	
	}	
	
	if (p_file->handle == NULL) {
		LOG_DEBUG_MSG("LOG-DEBUG: Open Log-File has FAILED !!! --- \n");
		return;
	}
	
	char date_string[128];
	memset(date_string, 0x00, 128);
	
	FILE* pipe = popen("date", "r");
	read_line(pipe, date_string, 128);
	fclose(pipe);
	
	int err_code = fprintf(p_file->handle, "%s \t %d \t %s \r\n", date_string, error_level, p_msg_from->payload);
	if (err_code < 0) {
		LOG_DEBUG_MSG("LOG-DEBUG: Writing File has FAILED !!! --- (ERROR: %d)\n", err_code);		
	}
		
	fclose(p_file->handle);
}