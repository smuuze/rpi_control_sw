#ifndef _SHC_COMMAND_LINE_PARSER_H_
#define _SHC_COMMAND_LINE_PARSER_H_

// -------- INCLUDES --------------------------------------------------------------------

#include "shc_file_interface.h"
#include "shc_common_string.h"
#include "shc_gpio_interface.h"
#include "shc_command_interface.h"
#include "shc_mqtt_interface.h"
#include "shc_spi_interface.h"

/*!
 *
 */
u8 command_line_parser(int argc, char* argv[], CFG_INTERFACE* p_cfg_interface, COM_INTERFACE* p_com_interface, MQTT_INTERFACE* p_mqtt_interface, COMMAND_INTERFACE* p_cmd_interface, GPIO_INTERFACE* p_gpio_interface, SCHEDULING_INTERFACE* p_scheduling_interface);


/*!
 *
 */
void command_line_usage(void);

#endif // _SHC_COMMAND_LINE_PARSER_H_