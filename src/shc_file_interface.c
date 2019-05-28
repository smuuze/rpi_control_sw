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
