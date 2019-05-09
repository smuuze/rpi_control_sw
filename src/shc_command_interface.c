/* ------------------------------------------------------------------------------
 *	Project	: Smart Home Client
 *	File	:
 *	Date	:
 *	Author	: Sebastian Lesse
 *	Brief	:
 * ------------------------------------------------------------------------------
 */

// ---- INCLUDES ----------------------------------------------------------------

#include "shc_timer.h"
#include "shc_project_configuration.h"
#include "shc_common_types.h"
#include "shc_common_string.h"
#include "shc_debug_interface.h"

#include "shc_file_interface.h"
#include "shc_gpio_interface.h"
#include "shc_command_interface.h"

// ---- LOCAL DEFINITIONS -------------------------------------------------------

#define COMMAND_DEBUG_MSG				noDEBUG_MSG

#define COMMAND_INTERFACE_MAX_LENGTH_TEMP_BUFFER	64

// ---- STATIC DATA -------------------------------------------------------------


// ---- IMPLEMENTATION ----------------------------------------------------------

void restore_last_file_pointer(FILE_INTERFACE* p_file) {
	p_file->act_file_pointer = p_file->last_file_pointer;
}


u8 cmd_handler_prepare_command_from_file(COMMAND_INTERFACE* p_cmd, FILE_INTERFACE* p_file) {
		
	COMMAND_DEBUG_MSG("--- cmd_handler_prepare_command_from_file\n");
	
	if (p_file == NULL) {
		COMMAND_DEBUG_MSG("cmd_handler_prepare_command_from_file - INVALID_ARGUMENT !!! (p_file)\n");
		return ERR_INVALID_ARGUMENT;
	}
	
	if (p_cmd == NULL) {
		COMMAND_DEBUG_MSG("cmd_handler_prepare_command_from_file - INVALID_ARGUMENT !!! (p_cmd)\n");
		return ERR_INVALID_ARGUMENT;
	}

	p_file->handle = fopen(p_file->path, "r");
	if (p_file->handle == NULL) {
		COMMAND_DEBUG_MSG("--- Open Command-Map-File has FAILED !!! --- (FILE: %s / ERROR: %d)\n", p_file->path,  EXIT_FAILURE);
		return ERR_FILE_OPEN;
	}

	char file_line[config_MAX_LENGTH_OF_FILE_LINE];

	fseek(p_file->handle, p_file->act_file_pointer, SEEK_SET);
	u8 num_bytes = read_line(p_file->handle, file_line, config_MAX_LENGTH_OF_FILE_LINE);

	p_file->last_file_pointer = p_file->act_file_pointer;
	p_file->act_file_pointer = ftell(p_file->handle);

	fclose(p_file->handle);

	COMMAND_DEBUG_MSG("--- File-Line: %s\n", file_line);

	if (num_bytes == 0) {
		COMMAND_DEBUG_MSG("--- End of Command-File reached !!!\n");
		p_file->act_file_pointer = 0;
		p_file->last_file_pointer = 0;
		return ERR_END_OF_FILE;
	}

	COMMAND_DEBUG_MSG("--- File-Pointer : %d\n", p_file->act_file_pointer);

	char command_answer_string[2 * GENERAL_STRING_BUFFER_MAX_LENGTH];
	char command_string[GENERAL_STRING_BUFFER_MAX_LENGTH];
	char command_type_string[GENERAL_STRING_BUFFER_MAX_LENGTH];
	char command_data_string[GENERAL_STRING_BUFFER_MAX_LENGTH];
	char answer_string[GENERAL_STRING_BUFFER_MAX_LENGTH];

	COMMAND_DEBUG_MSG("cmd_handler_prepare_command_from_file() - Splitting command string 1 \n");
	COMMAND_DEBUG_MSG("cmd_handler_prepare_command_from_file() - file_line: %s\n", file_line);
	COMMAND_DEBUG_MSG("cmd_handler_prepare_command_from_file() - num_bytes: %d\n", num_bytes);
	COMMAND_DEBUG_MSG("cmd_handler_prepare_command_from_file() - p_cmd->message.payload: %s\n", (char*)p_cmd->message.payload);
	COMMAND_DEBUG_MSG("cmd_handler_prepare_command_from_file() - GENERAL_STRING_BUFFER_MAX_LENGTH: %d\n", GENERAL_STRING_BUFFER_MAX_LENGTH);
	
	// Split File-Line into name and value of command-definition
	split_string('=', file_line, num_bytes, (char*)p_cmd->message.payload, GENERAL_STRING_BUFFER_MAX_LENGTH, command_answer_string, 2 * GENERAL_STRING_BUFFER_MAX_LENGTH);
	
	// Split command value into command to perform and answer to match for if in event-mode
	split_string('=', command_answer_string, string_length(command_answer_string), command_string, GENERAL_STRING_BUFFER_MAX_LENGTH, answer_string, GENERAL_STRING_BUFFER_MAX_LENGTH);
	
	// Get Type of command by splitting command
	split_string(':', command_string, string_length(command_string), command_type_string, GENERAL_STRING_BUFFER_MAX_LENGTH, command_data_string, GENERAL_STRING_BUFFER_MAX_LENGTH);
	
	if (memcmp(command_type_string, COMMANAND_INTERFACE_TYPE_COMMUNICATION, COMMAND_INTERFACE_TYPE_STRING_LENGTH) == 0) {
	
		COMMAND_DEBUG_MSG("cmd_handler_prepare_command_from_file() - This is a communication command\n");
		p_cmd->type = COMMAND_TYPE_COMMUNICATION;
		COMMAND_DEBUG_MSG("cmd_handler_prepare_command_from_file() - Convert Hex-String to Byte-Array (Command) \n");
	
		p_cmd->command.length = hex_string_to_byte_array(command_data_string, string_length(command_data_string), p_cmd->command.payload, GENERAL_STRING_BUFFER_MAX_LENGTH);
		memset(p_cmd->command.payload + p_cmd->command.length, 0x00, GENERAL_STRING_BUFFER_MAX_LENGTH - p_cmd->command.length);

		COMMAND_DEBUG_MSG("cmd_handler_prepare_command_from_file() - Convert Hey-String to Byte-Array (Answer)\n");

		p_cmd->answer.length = hex_string_to_byte_array(answer_string, string_length(answer_string), p_cmd->answer.payload, GENERAL_STRING_BUFFER_MAX_LENGTH);
		memset(p_cmd->answer.payload + p_cmd->answer.length, 0x00, GENERAL_STRING_BUFFER_MAX_LENGTH - p_cmd->answer.length);
		
	} else if (memcmp(command_type_string, COMMAND_INTERFACE_TYPE_EXECUTION, COMMAND_INTERFACE_TYPE_STRING_LENGTH) == 0) {
	
		COMMAND_DEBUG_MSG("cmd_handler_prepare_command_from_file() - This is a execution command\n");
		p_cmd->type = COMMAND_TYPE_EXECUTION;
		
		p_cmd->command.length = string_length(command_data_string);
		memcpy(p_cmd->command.payload, command_data_string, p_cmd->command.length);
		memset(p_cmd->command.payload + p_cmd->command.length, 0x00, GENERAL_STRING_BUFFER_MAX_LENGTH - p_cmd->command.length);		
		
	} else {
		COMMAND_DEBUG_MSG("cmd_handler_prepare_command_from_file() - This is a UNKNOWN command !!! ---\n");
		p_cmd->type = COMMAND_TYPE_UNKNOWN;
	}

	COMMAND_DEBUG_MSG("cmd_handler_prepare_command_from_file() - Reset Message string\n");

	p_cmd->message.length = string_length((char*)p_cmd->message.payload);
	memset(p_cmd->message.payload + p_cmd->message.length, 0x00, GENERAL_STRING_BUFFER_MAX_LENGTH - p_cmd->message.length);

	if (p_cmd->type == COMMAND_TYPE_COMMUNICATION) {
	}

	COMMAND_DEBUG_MSG("--- Command: MSG=%s / CMD=%s / TYPE=%d / ANSW=%s\n", p_cmd->message.payload, command_data_string, p_cmd->type, answer_string);

	return NO_ERR;
}

u8 cmd_handler_match_event_answer(COMMAND_INTERFACE* p_cmd, COMMAND_INTERFACE* p_cmd_match) {

	#if EVENT_DEBUG_MSG == DEBUG_MSG
	u8 i = 0;
	for ( ; i < p_cmd->answer.length; i++) {
		COMMAND_DEBUG_MSG("---> MATCHING: %02x - %02x \n", p_cmd->answer.payload[i], p_cmd_match->answer.payload[i]);
	}

	#endif

	return memcmp(p_cmd->answer.payload, p_cmd_match->answer.payload, p_cmd->answer.length) == 0 ? NO_ERR : ERR_NOT_EQUAL;
}

u8 cmd_handler_prepare_report_message(COMMAND_INTERFACE* p_cmd, u8 err_code, u8 answer_is_byte_array) {

	if (err_code != NO_ERR) {
	
		COMMAND_DEBUG_MSG("---> Receive Report-Command has FAILED !!! --- (ERROR:%d)\n", err_code);
		
		sprintf (
			(char*)(p_cmd->message.payload + p_cmd->message.length),
			"ERR(%d)", err_code
		);
		
		p_cmd->message.length = string_length((char*)p_cmd->message.payload);
		return NO_ERR;
	}
	
	if (answer_is_byte_array != 0) {
	
		p_cmd->message.length +=
			byte_array_string_to_hex_string (
				p_cmd->answer.payload,
				p_cmd->answer.length,
				(char*)(p_cmd->message.payload + p_cmd->message.length),
				GENERAL_STRING_BUFFER_MAX_LENGTH - p_cmd->message.length
			);
			
	} else {
	
		COMMAND_DEBUG_MSG("cmd_handler_prepare_report_message() - Answer: %s (Length: %d)\n", p_cmd->answer.payload, p_cmd->answer.length);
	
		memcpy(p_cmd->message.payload + p_cmd->message.length, p_cmd->answer.payload, p_cmd->answer.length);
		p_cmd->message.length += p_cmd->answer.length;
		
		COMMAND_DEBUG_MSG("cmd_handler_prepare_report_message() - Message: %s (Length: %d)\n", p_cmd->message.payload, p_cmd->message.length);
	}

	return NO_ERR;
}

u8 cmd_handler_prepare_command(COMMAND_INTERFACE* p_cmd) {

	p_cmd->command.length = 0;
	p_cmd->answer.length = 0;

	char path[64];
	sprintf(path, "%s", p_cmd->command_file.path);

	FILE* command_file_handle = fopen((const char*)path, "r");
	if (command_file_handle == NULL) {
		COMMAND_DEBUG_MSG("--- Open Command-Map-File has FAILED !!! --- (FILE: %s / ERROR: %d)\n", path,  EXIT_FAILURE);
		return ERR_FILE_OPEN;
	}

	char command_line[512];
	char command_message[256];
	char command_data[256];
	u16 num_bytes = 0;

	do  {
		num_bytes = read_line(command_file_handle, command_line, 512);
		split_string('=', command_line, num_bytes, command_message, 256, command_data, 256);

		if (memcmp(p_cmd->message.payload, command_message, string_length(command_message)) == 0) {

			p_cmd->command.length = hex_string_to_byte_array(command_data, string_length(command_data), p_cmd->command.payload, GENERAL_STRING_BUFFER_MAX_LENGTH);
			break;
		}

	} while (num_bytes != 0);

	fclose(command_file_handle);

	if (p_cmd->command.length != 0) {
		COMMAND_DEBUG_MSG("--- Command: %s (%s) \n", command_message, command_data);
		return NO_ERR;

	} else {
		COMMAND_DEBUG_MSG("--- ERROR: Unknown Command (\"%s\")\n", p_cmd->message.payload);
		return ERR_BAD_CMD;
	}
}

u8 cmd_handler_prepare_execution(COMMAND_INTERFACE* p_cmd) {

	p_cmd->command.length = 0;
	p_cmd->answer.length = 0;

	char path[64];
	sprintf(path, "%s", p_cmd->execution_file.path);

	p_cmd->execution_file.handle = fopen((const char*)path, "r");
	if (p_cmd->execution_file.handle == NULL) {
		COMMAND_DEBUG_MSG("--- Open Execution-Map-File has FAILED !!! --- (FILE: %s / ERROR: %d)\n", path,  EXIT_FAILURE);
		return ERR_FILE_OPEN;
	}

	char command_line[512];
	char command_message[256];
	char command_data[256];
	u16 num_bytes = 0;

	do  {
		num_bytes = read_line(p_cmd->execution_file.handle, command_line, 512);
		split_string('=', command_line, num_bytes, command_message, 256, command_data, 256);

		if (memcmp(p_cmd->message.payload, command_message, string_length(command_message)) == 0) {
			memcpy(p_cmd->command.payload, command_data, string_length(command_data));
			memset(p_cmd->command.payload + string_length(command_data), 0, GENERAL_STRING_BUFFER_MAX_LENGTH - string_length(command_data));
			p_cmd->command.length = string_length(command_data);
			break;
		}

	} while (num_bytes != 0);

	fclose(p_cmd->execution_file.handle);

	if (p_cmd->command.length != 0) {
		COMMAND_DEBUG_MSG("--- Execution: %s (%s) \n", command_message, command_data);
		return NO_ERR;

	} else {
		COMMAND_DEBUG_MSG("--- ERROR: Unknown Execution (\"%s\")\n", p_cmd->message.payload);
		return ERR_BAD_CMD;
	}
}

u8 cmd_handler_run_execution(COMMAND_INTERFACE* p_cmd, u8 get_output) {

	if (get_output == COMMAND_INTERFACE_IGNORE_OUTPUT) {
		system((const char*)p_cmd->command.payload);
		return NO_ERR;
	}
	
	p_cmd->answer.length = 0;
	
	FILE* p_pipe = popen((const char*)p_cmd->command.payload, "r");		//Send the command, popen exits immediately
	if (!p_pipe) {
		COMMAND_DEBUG_MSG("cmd_handler_run_execution() - popen has FAILED !!! --- error: Command not found\n");
		return ERR_BAD_CMD;
	}
	
	while( !feof(p_pipe) ) {
	
		char t_buffer[COMMAND_INTERFACE_MAX_LENGTH_TEMP_BUFFER];		
		if( fgets(t_buffer, COMMAND_INTERFACE_MAX_LENGTH_TEMP_BUFFER, p_pipe) == NULL) {
			continue;
		}
		
		u8 byte_count = string_length(t_buffer);
		memcpy(p_cmd->answer.payload + p_cmd->answer.length, t_buffer, byte_count);
		p_cmd->answer.length += byte_count;
	}
	
	pclose(p_pipe);
	
	p_cmd->answer.length = string_trim(p_cmd->answer.payload, GENERAL_STRING_BUFFER_MAX_LENGTH);
	
	if (p_cmd->answer.length < GENERAL_STRING_BUFFER_MAX_LENGTH) {
		memset(p_cmd->answer.payload + p_cmd->answer.length, '\0', GENERAL_STRING_BUFFER_MAX_LENGTH - p_cmd->answer.length);
	}
	
	COMMAND_DEBUG_MSG("cmd_handler_run_execution() - popen output: %s\n", p_cmd->answer.payload);
	return NO_ERR;
}

u8 cmd_handler_send_command(COMMAND_INTERFACE* p_cmd, COM_INTERFACE* p_com, GPIO_INTERFACE* p_gpio) {

	if (p_cmd->command.length == 0) {
		return ERR_INVALID_ARGUMENT;
	}

	// Set GPIO to low for at least 50ms to activate control-board
	gpio_reset_pin(p_gpio);
	p_gpio->match_event_level = 0;
	p_gpio->is_input = 1;
	gpio_set_state(p_gpio, GPIO_ON);

	u8 err_code = NO_ERR;
	u32 time_reference = mstime_get_time();

	while (gpio_is_event(p_gpio) == 0) {

		usleep(p_gpio->sample_timeout * 1000); // wait for HW to be ready

		if (mstime_is_time_up(time_reference, CMD_ACTIVATE_TIMEOUT_MS) != 0) {
			COMMAND_DEBUG_MSG("-- ERROR on sending command - wait for low-level of ready-pin has FAIELD !!! ---\n");
			err_code = ERR_COMMUNICATION;
			break;
		}
	}

	if (err_code != NO_ERR) {
		gpio_set_state(p_gpio, GPIO_OFF);
		return err_code;
	}

	//spi_start_tx();

	switch (p_com->type) {
		case SPI :

			// Check if there is a old answer pending on the other side
			err_code = spi_transfer (
				&p_com->data.spi,
				1,
				(const u8*) p_cmd->command.payload,
				(u8*)&p_cmd->answer.length
			);

			if (err_code != NO_ERR) {
				COMMAND_DEBUG_MSG("-- ERROR before sending command - reading old answer length has FAIELD !!! --- \n");
				break;
			}

			if (p_cmd->answer.length > GENERAL_STRING_BUFFER_MAX_LENGTH) {
				err_code = ERR_ANSWER_LENGTH;
				COMMAND_DEBUG_MSG("-- ERROR before sending command - old anser to long OVERFLOW !!! --- \n");
				break;
			}

			p_cmd->command.length -= 1;

			if (p_cmd->answer.length != 0) {

				u8 length = (p_cmd->answer.length > p_cmd->command.length) ? p_cmd->answer.length : p_cmd->command.length;
				err_code = spi_transfer(&p_com->data.spi, length, (const u8*)(p_cmd->command.payload) + 1, p_cmd->answer.payload);
				COMMAND_DEBUG_MSG("--- Need to read old answer from the interface (Length: %d)!!!\n", p_cmd->answer.length);

			} else {
				err_code = spi_transfer(&p_com->data.spi, p_cmd->command.length, (const u8*)(p_cmd->command.payload) + 1, NULL);
			}

			// while (p_cmd->answer.length != 0) {

				// COMMAND_DEBUG_MSG("--- Need to read old answer from the interface (Length: %d)!!!\n", p_cmd->answer.length);

				// u8 length = p_cmd->answer.length;
				// if (length > GENERAL_STRING_BUFFER_MAX_LENGTH) {
					// length = GENERAL_STRING_BUFFER_MAX_LENGTH;
				// }

				// err_code = spi_transfer (
					// &p_com->data.spi,
					// length,
					// NULL,
					// p_cmd->answer.payload
				// );

				// p_cmd->answer.length -= length;
			// }

//			err_code = spi_transfer(&p_com->data.spi, p_cmd->command.length, (const u8*) p_cmd->command.payload, NULL);
			break;

		case I2C :
			break;

		case USART :
			break;

		default:
			break;
	}

	gpio_reset_pin(p_gpio);
	p_gpio->match_event_level = 1;
	p_gpio->is_input = 1;
	gpio_set_state(p_gpio, GPIO_ON);

	time_reference = mstime_get_time();

	while (gpio_is_event(p_gpio) == 0) {

		usleep(p_gpio->sample_timeout * 1000);

		if (mstime_is_time_up(time_reference, CMD_ACTIVATE_TIMEOUT_MS) != 0) {
			COMMAND_DEBUG_MSG("-- ERROR after sending command - wait for high-level of ready-pin has FAIELD !!! ---\n");
			err_code = ERR_COMMUNICATION;
			break;
		}
	}

	//spi_stop_trx();
	gpio_set_state(p_gpio, GPIO_OFF);

	return err_code;
}

u8 cmd_handler_receive_answer(COMMAND_INTERFACE* p_cmd, COM_INTERFACE* p_com, GPIO_INTERFACE* p_gpio, u32 timeout_ms) {

	u8 err_code = NO_ERR;

	gpio_reset_pin(p_gpio);
	p_gpio->match_event_level = 0;
	p_gpio->is_input = 1;
	gpio_set_state(p_gpio, GPIO_ON);

	u32 time_reference = mstime_get_time();
	p_cmd->answer.length = 0;

	while (gpio_is_event(p_gpio) == 0) {

		usleep(p_gpio->sample_timeout * 1000); // wait a little bit for answer to be complete

		if (mstime_is_time_up(time_reference, CMD_ACTIVATE_TIMEOUT_MS) != 0) {
			COMMAND_DEBUG_MSG("-- ERROR on receiving answer - wait for low-level of ready-pin has FAIELD !!! ---\n");
			err_code = ERR_COMMUNICATION;
			break;
		}
	}

	gpio_set_state(p_gpio, GPIO_OFF);

	if (err_code != NO_ERR) {
		return err_code;
	}

	//spi_start_tx();

	switch (p_com->type) {
		case SPI :

			// length of answer
			err_code = spi_transfer (
				&p_com->data.spi,
				1,
				NULL,
				(u8*)&p_cmd->answer.length
			);

			if (err_code) {
				COMMAND_DEBUG_MSG("-- Receiving answer-length has FAILED !!! --- (ERR: %d)\n", err_code);
				err_code = ERR_ANSWER_LENGTH;
				break;
			}

			if (p_cmd->answer.length == 0) {
				COMMAND_DEBUG_MSG("-- Answer-Length is zero, nothing to receive. \n");
				break;
			}

			if (p_cmd->answer.length > GENERAL_STRING_BUFFER_MAX_LENGTH) {
				COMMAND_DEBUG_MSG("-- Answer-Length is too Long !!! --- (LENGTH: %d)\n", p_cmd->answer.length);
				err_code = ERR_ANSWER_LENGTH;
				break;
			}

			// answer
			err_code = spi_transfer (
				&p_com->data.spi,
				p_cmd->answer.length,
				NULL,
				p_cmd->answer.payload
			);

			if (err_code) {
				COMMAND_DEBUG_MSG("-- Receiving answer has FAILED !!! --- (ERR: %d)\n", err_code);
				err_code = ERR_COMMUNICATION;
				break;
			}

			break;

		case I2C :
			break;

		case USART :
			break;

		default:
			break;
	}

	#if COMMAND_DEBUG_MSG != noDEBUG_MSG
	command_handling_duration = mstime_get_time() - command_handling_duration;
	COMMAND_DEBUG_MSG("------> Command Execution-Time: %d ms\n", command_handling_duration);
	COMMAND_DEBUG_MSG("------> Low-Level-Time: %d ms\n", low_level_waiting_time);
	COMMAND_DEBUG_MSG("------> High-Level-Time: %d ms\n", high_level_waiting_time);
	COMMAND_DEBUG_MSG("------> Transfer-Time: %d ms\n", transfer_time);
	#endif

	gpio_reset_pin(p_gpio);
	p_gpio->match_event_level = 1;
	p_gpio->is_input = 1;
	gpio_set_state(p_gpio, GPIO_ON);

	time_reference = mstime_get_time();

	while (gpio_is_event(p_gpio) == 0) {

		usleep(p_gpio->sample_timeout * 1000); // wait a little bit for answer to be complete

		if (mstime_is_time_up(time_reference, CMD_ACTIVATE_TIMEOUT_MS) != 0) {
			COMMAND_DEBUG_MSG("-- ERROR after receiving answer - wait for high-level of ready-pin has FAIELD !!! ---\n");
			err_code = ERR_COMMUNICATION;
			break;
		}
	}

	//spi_stop_trx();
	gpio_set_state(p_gpio, GPIO_OFF);

	return err_code;
}

u8 cmd_handler_get_error_code(COMMAND_INTERFACE* p_cmd) {
	return p_cmd->answer.payload[1];
}

u8 cmd_handler_is_communication_command(COMMAND_INTERFACE* p_cmd) {
	return p_cmd->type == COMMAND_TYPE_COMMUNICATION ? 1 : 0;
}

u8 cmd_handler_is_execution_command(COMMAND_INTERFACE* p_cmd) {
	return p_cmd->type == COMMAND_TYPE_EXECUTION ? 1 : 0;

}
