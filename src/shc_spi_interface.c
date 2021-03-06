/* ------------------------------------------------------------------------------
 *	Project	: Smart Home Client
 *	File	:
 *	Date	:
 *	Author	: Sebastian Lesse
 *	Brief	:
 * ------------------------------------------------------------------------------
 */
 
// ---- INCLUDES ----------------------------------------------------------------

#include "shc_project_configuration.h"
#include "shc_common_types.h"
#include "shc_common_string.h"
#include "shc_debug_interface.h"
#include "shc_gpio_interface.h"

#include "shc_spi_interface.h"

#include <sys/ioctl.h>
#include <linux/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

// ---- LOCAL DEFINITIONS -------------------------------------------------------

#define SPI_DEBUG_MSG				DEBUG_MSG

// ---- STATIC DATA -------------------------------------------------------------

GPIO_INTERFACE_BUILD_INOUT(SLAVE_CS, GPIO_CE0_PIN_NUM)

// ---- IMPLEMENTATION ----------------------------------------------------------

void spi_init(SPI_INTERFACE* p_spi_handle) {

	SPI_DEBUG_MSG("- Using SPI-Device: %s\n", p_spi_handle->device);

	p_spi_handle->_handle_id = open(p_spi_handle->device, O_RDWR);
	if (p_spi_handle->_handle_id < 0) {
		SPI_DEBUG_MSG("spi_init() - Cant open SPI device\n");
		return;
	}
	
	u8 err_code = ioctl(p_spi_handle->_handle_id, SPI_IOC_WR_MODE32, &p_spi_handle->mode);
	if (err_code) {
		SPI_DEBUG_MSG("spi_init() - Can't set spi mode - SPI_IOC_WR_MODE32 - error: %d\n", err_code);
		return;
	}

	err_code = ioctl(p_spi_handle->_handle_id, SPI_IOC_RD_MODE32, &p_spi_handle->mode);
	if (err_code) {
		SPI_DEBUG_MSG("spi_init() - Can't get spi mode - SPI_IOC_RD_MODE32\n");
		return;
	}
	
	err_code = ioctl(p_spi_handle->_handle_id, SPI_IOC_WR_BITS_PER_WORD, &p_spi_handle->bits_per_word);
	if (err_code) {
		SPI_DEBUG_MSG("spi_init() - Can't set bits per word\n");
		return;
	}

	err_code = ioctl(p_spi_handle->_handle_id, SPI_IOC_RD_BITS_PER_WORD, &p_spi_handle->bits_per_word);
	if (err_code) {
		SPI_DEBUG_MSG("spi_init() - Can't get bits per word - SPI_IOC_RD_BITS_PER_WORD\n");
		return;
	}
	
	err_code = ioctl(p_spi_handle->_handle_id, SPI_IOC_WR_MAX_SPEED_HZ, &p_spi_handle->speed_hz);
	if (err_code) {
		SPI_DEBUG_MSG("spi_init() - Can't set max speed hz - SPI_IOC_WR_MAX_SPEED_HZ\n");
		return;
	}

	err_code = ioctl(p_spi_handle->_handle_id, SPI_IOC_RD_MAX_SPEED_HZ, &p_spi_handle->speed_hz);
	if (err_code) {
		SPI_DEBUG_MSG("spi_init() - Can't get max speed hz - SPI_IOC_RD_MAX_SPEED_HZ\n");
		return;
	}
	
	//gpio_set_state(&ce_pin, GPIO_ON);
	SLAVE_CS_init();
	SLAVE_CS_pull_up();
}

void spi_deinit(SPI_INTERFACE* p_spi_handle) {
	close(p_spi_handle->_handle_id);
	p_spi_handle->_handle_id = -1;
}

u8 spi_transfer(SPI_INTERFACE* p_spi_handle, size_t num_bytes, const u8* p_buffer_from, u8* p_buffer_to) {

	if (p_spi_handle->_handle_id < 0) {
		SPI_DEBUG_MSG("SPI device not initailized!");
		return ERR_NOT_INITIALIZED;
	}
	
	if (num_bytes == 0) {
		SPI_DEBUG_MSG("---> Parameter num_bytes is zero! \n");
		return ERR_INVALID_ARGUMENT;
	}
	
	u8 tmp_tx_buffer[GENERAL_STRING_BUFFER_MAX_LENGTH];	
	u8 tmp_rx_buffer[GENERAL_STRING_BUFFER_MAX_LENGTH];

	if (p_buffer_from != NULL) {
		memcpy(tmp_tx_buffer, p_buffer_from, num_bytes);
	} else {
		memset(tmp_tx_buffer, 0x00, num_bytes);
	}

	struct spi_ioc_transfer spi_tr = {
		.tx_buf = (unsigned long)tmp_tx_buffer,
		.rx_buf = (unsigned long)tmp_rx_buffer,
		.len = num_bytes,
		.delay_usecs = p_spi_handle->delay,
		.speed_hz = p_spi_handle->speed_hz,
		.bits_per_word = p_spi_handle->bits_per_word,
		.tx_nbits = 0
	};

	/*
	if (p_spi_handle->mode & SPI_TX_QUAD) {
		spi_tr.tx_nbits = 4;
	
	} else if (p_spi_handle->mode & SPI_TX_DUAL) {
		spi_tr.tx_nbits = 2;
	}
	
	if (p_spi_handle->mode & SPI_RX_QUAD) {
		spi_tr.rx_nbits = 4;
	
	} else if (p_spi_handle->mode & SPI_RX_DUAL) {
		spi_tr.rx_nbits = 2;
	}	
	*/
	
	SLAVE_CS_drive_low(); //gpio_set_state(&ce_pin, GPIO_OFF);
	u8 err_code = ioctl(p_spi_handle->_handle_id, SPI_IOC_MESSAGE(1), &spi_tr);
	SLAVE_CS_pull_up(); //gpio_set_state(&ce_pin, GPIO_ON);

	//hex_dump((const void*)tmp_tx_buffer, num_bytes, 32, "TX");
	//hex_dump((const void*)tmp_rx_buffer, num_bytes, 32, "RX");

	if (p_buffer_to != NULL) {
		memcpy(p_buffer_to, tmp_rx_buffer, num_bytes);
	}
	
	if (err_code < 1) {
		SPI_DEBUG_MSG("-----> Can't send spi message (erro-code = %d)\n", err_code);
		return ERR_SEND_MSG;
	} else {
		return NO_ERR;
	}
}
