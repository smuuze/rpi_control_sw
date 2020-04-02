/* ------------------------------------------------------------------------------
 *	Project	: Smart Home Client
 *	File	:
 *	Date	:
 *	Author	: Sebastian Lesse
 *	Brief	:
 * ------------------------------------------------------------------------------
 */
 
#ifndef _SHC_COMMAND_INTERFACE_H_
#define _SHC_COMMAND_INTERFACE_H_

#include "shc_project_configuration.h"
#include "shc_common_configuration.h"
#include "shc_common_types.h"
#include "shc_file_interface.h"
#include "shc_gpio_interface.h"
#include "shc_spi_interface.h"


#define COMMAND_INTERFACE_TYPE_STRING_LENGTH	3
#define COMMANAND_INTERFACE_TYPE_COMMUNICATION	"com"
#define COMMAND_INTERFACE_TYPE_EXECUTION	"exe"

#define IS_EXECUTION_COMMAND(msg)		(memcmp(msg.payload, "exe", 3) == 0 ? 1 : 0)
#define IS_COMMUNICATION_COMMAND(msg)		(memcmp(msg.payload, "cmd", 3) == 0 ? 1 : 0)	

#define COMMAND_INTERFACE_IGNORE_OUTPUT		0
#define COMMAND_INTERFACE_CATCH_OUTPUT		1

/*
 *
 */
typedef enum {
	COMMAND_TYPE_UNKNOWN		= 0x00,
	COMMAND_TYPE_COMMUNICATION,
	COMMAND_TYPE_EXECUTION
} COMMAND_INTERFACE_TYPE;

/*
 *
 */
typedef struct {

	u8 is_active;
	u8 fail_counter;
	
	COMMAND_INTERFACE_TYPE type;
	
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
void cmd_handler_init(CFG_INTERFACE* p_cfgInterface);

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
u8 cmd_handler_prepare_report_message(COMMAND_INTERFACE* p_cmd, u8 err_code, u8 answer_is_byte_array);

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
u8 cmd_handler_run_execution(COMMAND_INTERFACE* p_cmd, u8 get_output);

/*!
 *
 */
u8 cmd_handler_send_command(COMMAND_INTERFACE* p_cmd, COM_INTERFACE* p_com);

/*!
 *
 */
u8 cmd_handler_receive_answer(COMMAND_INTERFACE* p_cmd, COM_INTERFACE* p_com, u32 timeout_ms);

/*!
 *
 */
u8 cmd_handler_is_communication_command(COMMAND_INTERFACE* p_cmd);

/*!
 *
 */
u8 cmd_handler_is_execution_command(COMMAND_INTERFACE* p_cmd);

/*!
 *
 */
u8 cmd_handler_get_error_code(COMMAND_INTERFACE* p_cmd);

#endif // _SHC_COMMAND_INTERFACE_H_