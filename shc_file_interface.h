#ifndef _SHC_FILE_INTERFACE_H_
#define _SHC_FILE_INTERFACE_H_

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

#endif // _SHC_FILE_INTERFACE_H_