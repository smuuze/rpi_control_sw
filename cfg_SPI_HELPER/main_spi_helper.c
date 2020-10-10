
#define TRACER_ON

// --------------------------------------------------------------------------------------

#include "shc_project_configuration.h"
#include "shc_common_types.h"
#include "shc_common_string.h"
#include "shc_common_configuration.h"
#include "shc_debug_interface.h"

#include "shc_command_interface.h"
#include "shc_spi_interface.h"
#include "shc_gpio_interface.h"
#include "shc_command_line_parser.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <linux/spi/spidev.h>

#include <time.h>

// --------------------------------------------------------------------------------

#include "tracer.h"

// --------------------------------------------------------------------------------------

#define MAIN_DEBUG_MSG					DEBUG_MSG
#define MAIN_CFG_DEBUG_MSG				DEBUG_MSG

// --------------------------------------------------------------------------------------

/*!
 *
 */
void command_line_usage(void);

// --------------------------------------------------------------------------------------

GPIO_INTERFACE_BUILD_INOUT(RESET_PIN, GPIO_RESET_PIN_NUM)
GPIO_INTERFACE_BUILD_INOUT(REQUEST_PIN, GPIO_REQUEST_PIN_NUM)

// --------------------------------------------------------------------------------------

/*!
 *
 */
static CFG_INTERFACE myCfgInterface;

/*!
 *
 */
static COMMAND_INTERFACE myCmdInterface;

/*!
 *
 */
static COM_INTERFACE myComInterface;

// --------------------------------------------------------------------------------------


int main(int argc, char* argv[]) {

	printf("Welcome to the SHC-SPI-Helper v%d.%d\n\n", VERSION_MAJOR, VERSION_MINOR);

	// --- Parsing Command-Line Arguments
	u8 err_code = command_line_parser(argc, argv, &myCfgInterface, &myComInterface, NULL, &myCmdInterface, NULL, NULL);
	if (err_code != NO_ERR) {

		DEBUG_PASS("main() - command_line_parser() has FAILED !!! ---\n");
		command_line_usage();
		return err_code;
	}

	cmd_handler_init(&myCfgInterface);

	spi_init(&myComInterface.data.spi);

	REQUEST_PIN_init();
	REQUEST_PIN_pull_up();

	if ((err_code = cmd_handler_send_command(&myCmdInterface, &myComInterface)) != NO_ERR) {

		DEBUG_TRACE_byte(err_code, "main() - cmd_handler_send_command() has FAILED !!! -- -- (error)");
		command_line_usage();
		return -2;					
	} 
	
	if ((err_code = cmd_handler_receive_answer(&myCmdInterface, &myComInterface, CMD_RX_ANSWER_TIMEOUT_MS)) != NO_ERR) {

		DEBUG_TRACE_byte(err_code, "main() - cmd_handler_receive_answer() has FAILED !!! -- -- (error)");
		command_line_usage();
		return -3;
	}

	hex_dump(myCmdInterface.command.payload, myCmdInterface.command.length, 32, "TX");
	hex_dump(myCmdInterface.answer.payload, myCmdInterface.answer.length, 32, "RX");
	
	return 0;
}

// --------------------------------------------------------------------------------------

void command_line_usage(void) {
	printf("\nUsage: spiHelper [options]]\n\n");
	printf("Options:\n");
	printf("-dev <device>                        : SPI-device to use for communication\t\n");
	printf("-cmd <command>                       : command to send in hexadecimal form (e.g. 0101)\t\n");
}