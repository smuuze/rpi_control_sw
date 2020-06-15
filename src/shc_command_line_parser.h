#ifndef _SHC_COMMAND_LINE_PARSER_H_
#define _SHC_COMMAND_LINE_PARSER_H_

// --------------------------------------------------------------------------------------

#include "shc_file_interface.h"
#include "shc_common_string.h"
#include "shc_gpio_interface.h"
#include "shc_command_interface.h"
#include "shc_mqtt_interface.h"
#include "shc_spi_interface.h"

/*!
 *
 */
u8 command_line_parser_match_argument(char* p_argument, char* p_key);

// --------------------------------------------------------------------------------------

#endif // _SHC_COMMAND_LINE_PARSER_H_