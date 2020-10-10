#ifndef   _config_H_ /* parse include file only once */
#define   _config_H_

///-----------------------------------------------------------------------------

#define BOARD_DESCRIPTION_FILE                          "platine/board_RASPBERRYPI.h"
#include "platine/board_RASPBERRYPI.h"

//-------------------------------------------------------------------------

#define config_DEBUG_WELCOME_MESSAGE			"Welcome to RPi - Control Board V2"

//-------------------------------------------------------------------------

#define I2C_CLK_LIMIT 225000

//-------------------------------------------------------------------------

#define config_LOCAL_COMMAND_HANDLER_TABLE_FUNC_PROTO		\
	u8 cmd_handler_version(const COMMAND_BUFFER_INTERFACE* i_cmd_buffer, const ANSWER_BUFFER_INTERFACE* i_answ_buffer);

#define config_LOCAL_COMMAND_HANDLER_TABLE_FUNC_CALLBACK	\
	{CMD_VERSION, &cmd_handler_version},

//-------------------------------------------------------------------------

#include "../src/config_default.h"

#endif /* _config_H_ */
