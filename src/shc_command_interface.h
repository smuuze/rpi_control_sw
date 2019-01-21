#ifndef _SHC_COMMAND_INTERFACE_H_
#define _SHC_COMMAND_INTERFACE_H_

#include "shc_common_types.h"
#include "shc_file_interface.h"
#include "shc_gpio_interface.h"

/*
 *
 */
typedef struct {
	
	u8 is_active;
	
	STRING_BUFFER message;
	STRING_BUFFER command;
	STRING_BUFFER answer;
	
	FILE_INTERFACE command_file;
	FILE_INTERFACE report_file;
	FILE_INTERFACE event_file;
	FILE_INTERFACE execution_file;
	
	char command_file_path[FILE_PATH_MAX_STRING_LENGTH];
	char report_file_path[FILE_PATH_MAX_STRING_LENGTH];
	char event_file_path[FILE_PATH_MAX_STRING_LENGTH];
} COMMAND_INTERFACE;

/*!
 *
 */
void restore_last_file_pointer(FILE_INTERFACE* p_file);

/*!
 *
 */
u8 cmd_handler_prepare_command_from_file(COMMAND_INTERFACE* p_cmd, FILE_INTERFACE* p_file);

/*!
 *
 */
u8 cmd_handler_match_event_answer(COMMAND_INTERFACE* p_cmd, COMMAND_INTERFACE* p_cmd_match);

/*!
 *
 */
u8 cmd_handler_prepare_report_message(COMMAND_INTERFACE* p_cmd, u8 err_code);

/*!
 *
 */
u8 cmd_handler_prepare_command(COMMAND_INTERFACE* p_cmd);

/*!
 *
 */
u8 cmd_handler_prepare_execution(COMMAND_INTERFACE* p_cmd);

/*!
 *
 */
u8 cmd_handler_run_execution(COMMAND_INTERFACE* p_cmd);

/*!
 *
 */
u8 cmd_handler_send_command(COMMAND_INTERFACE* p_cmd, COM_INTERFACE* p_com, GPIO_INTERFACE* p_gpio);

/*!
 *
 */
u8 cmd_handler_receive_answer(COMMAND_INTERFACE* p_cmd, COM_INTERFACE* p_com, GPIO_INTERFACE* p_gpio, u32 timeout_ms);

/*!
 *
 */
u8 cmd_handler_get_error_code(COMMAND_INTERFACE* p_cmd);

#endif // _SHC_COMMAND_INTERFACE_H_