/* ------------------------------------------------------------------------------
 *	Project	: Smart Home Client
 *	File	:
 *	Date	:
 *	Author	: Sebastian Lesse
 *	Brief	:
 * ------------------------------------------------------------------------------
 */
 
#ifndef _SHC_SPI_INTERFACE_H_
#define _SHC_SPI_INTERFACE_H_

#include "shc_project_configuration.h"

/*!
 * 
 */
typedef struct {
	u32 _handle_id;
	char device[COM_DEVICE_NAME_STRING_LENGTH];
	u32 speed_hz;
	u8 bits_per_word;
	u16 delay;
	u32 mode;
} SPI_INTERFACE;

/*!
 *
 */
void spi_init(SPI_INTERFACE* p_spi_handle);

/*!
 *
 */
u8 spi_transfer(SPI_INTERFACE* p_spi_handle, size_t num_bytes, const u8* p_buffer_from, u8* p_buffer_to);

/*!
 *
 */
void spi_deinit(SPI_INTERFACE* p_spi_handle);

#endif // _SHC_SPI_INTERFACE_H_