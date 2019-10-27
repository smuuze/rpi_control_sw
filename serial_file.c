#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

int main() {

	int uart0_filestream = -1;
	uart0_filestream = open("/dev/ttyAMA0", O_RDWR | O_NOCTTY | O_NDELAY);
	if (uart0_filestream == -1) {
		printf("[ERROR] UART open()\n");
	}

	struct termios options;
	tcgetattr(uart0_filestream, &options);
	options.c_cflag = B230400 | CS8 | CLOCAL | CREAD; 
	options.c_iflag = IGNPAR;
	options.c_oflag = 0;
	options.c_lflag = 0;

	tcflush(uart0_filestream, TCIFLUSH);
	tcsetattr(uart0_filestream, TCSANOW, &options);

	// Bytes empfangen
	while (uart0_filestream != -1) {
		unsigned char BUF_RX[50];
		int rx_length = read(uart0_filestream, (void*)BUF_RX, 50);

		if (rx_length < 0) {
			printf("[ERROR] UART RX\n");
		} else if (rx_length == 0) {
			//printf("[ERROR] UART RX - no data\n");
		} else {
			BUF_RX[rx_length] = '\0';
			printf("[STATUS: RX %i Bytes] %s\n", rx_length, BUF_RX);
		} //rx_length check

		usleep(5);
	} //if uart0

	close(uart0_filestream);

	return 0;
}

