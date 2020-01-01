/* ------------------------------------------------------------------------------
 *	Project	: Smart Home Client
 *	File	:
 *	Date	:
 *	Author	: Sebastian Lesse
 *	Brief	:
 * ------------------------------------------------------------------------------
 */

#define USART_DEBUG_MSG				DEBUG_MSG
 
// ---- INCLUDES ----------------------------------------------------------------

#include "shc_project_configuration.h"
#include "shc_common_types.h"
#include "shc_common_string.h"
#include "shc_debug_interface.h"
#include "shc_timer.h"

#include "shc_usart_interface.h"

#include <stdio.h>
#include <unistd.h>		//Used for UART
#include <fcntl.h>		//Used for UART
#include <termios.h>		//Used for UART

// ---- LOCAL DEFINITIONS -------------------------------------------------------


// ---- STATIC DATA -------------------------------------------------------------

TIME_MGMN_BUILD_TIMER(RX_TIMER)

// ---- IMPLEMENTATION ----------------------------------------------------------

u8 usart_init(USART_INTERFACE* p_device) {
		
	USART_DEBUG_MSG("usart_init()\n");

	p_device->_handle_id = open(p_device->device_name, O_RDWR | O_NOCTTY | O_NDELAY);		//Open in non blocking read/write mode
	if (p_device->_handle_id == -1) {
		USART_DEBUG_MSG("usart_init() - Open device %s has FAILED !!!\n", p_device->device_name);
		return 0;
	}
	
	//CONFIGURE THE UART
	//The flags (defined in /usr/include/termios.h - see http://pubs.opengroup.org/onlinepubs/007908799/xsh/termios.h.html):
	//	Baud rate:- B1200, B2400, B4800, B9600, B19200, B38400, B57600, B115200, B230400, B460800, B500000, B576000, B921600, B1000000, B1152000, B1500000, B2000000, B2500000, B3000000, B3500000, B4000000
	//	CSIZE:- CS5, CS6, CS7, CS8
	//	CLOCAL - Ignore modem status lines
	//	CREAD - Enable receiver
	//	IGNPAR = Ignore characters with parity errors
	//	ICRNL - Map CR to NL on input (Use for ASCII comms where you want to auto correct end of line characters - don't use for bianry comms!)
	//	PARENB - Parity enable
	//	PARODD - Odd parity (else even)
	struct termios options;
	tcgetattr(p_device->_handle_id, &options);

	options.c_cflag = CLOCAL | CREAD;		//<Set baud rate
	options.c_iflag = IGNPAR;
	options.c_oflag = 0;
	options.c_lflag = 0;

	switch (p_device->baudrate) {
		default :
		case USART_BAUDRATE_9600 :	options.c_cflag |= B9600; break;
		case USART_BAUDRATE_230400 :	options.c_cflag |= B230400; break;
	}

	switch (p_device->parity) {
		default :
		case USART_PARITY_NONE :	break;
	}

	switch (p_device->databits) {
		default :
		case USART_DATABITS_8 :		options.c_cflag |= CS8; break;
	}

	tcflush(p_device->_handle_id, TCIFLUSH);
	tcsetattr(p_device->_handle_id, TCSANOW, &options);

	return 1;
}

u16 usart_read_bytes(USART_INTERFACE* p_device, u16 max_num_bytes, u8* p_buffer, u16 timeout_ms) {

	if (p_device->_handle_id == -1) {
		USART_DEBUG_MSG("usart_read_bytes() - Device %s is NOT READY !!!\n", p_device->device_name);
		return 0;
	}

	u16 bytes_read = 0;

	RX_TIMER_start();

	while (bytes_read < max_num_bytes) {

		unsigned char byte = 0;
		int length = read(p_device->_handle_id, (void*)(&byte), 1);

		if (length > 0) {
			//USART_DEBUG_MSG("usart_read_bytes() - Byte read: %02X \n", byte);
			p_buffer[bytes_read] = (u8)byte;
			bytes_read += 1;

			RX_TIMER_start();
		}

		if (RX_TIMER_is_up(timeout_ms)) {
			break;
		}
	}

	return bytes_read;

}


u16 usart_read_line(USART_INTERFACE* p_device, u16 max_num_bytes, u8* p_buffer, u16 timeout_ms) {

	if (p_device->_handle_id == -1) {
		USART_DEBUG_MSG("usart_read_line() - Device %s is NOT READY !!!\n", p_device->device_name);
		return 0;
	}

	u16 bytes_read = 0;

	RX_TIMER_start();

	while (bytes_read < max_num_bytes) {

		unsigned char byte = 0;
		int length = read(p_device->_handle_id, (void*)(&byte), 1);

		if (length > 0) {
			p_buffer[bytes_read] = (u8)byte;
			bytes_read += 1;

			if (byte == '\n') {
				break;
			}
		}

		if (RX_TIMER_is_up(timeout_ms)) {
			break;
		}
	}

	return bytes_read;
}

u8 usart_write_line(USART_INTERFACE* p_device, u16 num_bytes, const u8* p_buffer) {

	if (p_device->_handle_id == -1) {
		USART_DEBUG_MSG("usart_write_line() - Device %s is NOT READY !!!\n", p_device->device_name);
		return 0;
	}
	
	int bytes_left = (int)num_bytes;

	while (bytes_left) {
	
		int bytes_written = write(p_device->_handle_id, p_buffer, bytes_left);		//Filestream, bytes to write, number of bytes to write
		if (bytes_written < 0) {
			USART_DEBUG_MSG("usart_write_line() - Writing data has FAILED !!!\n");
			return 0;
		}

		if (bytes_written < bytes_left) {
			bytes_left -= bytes_written;
		} else {
			bytes_left = 0;
		}
	}

	return 1;
}

void usart_deinit(USART_INTERFACE* p_device) {

}