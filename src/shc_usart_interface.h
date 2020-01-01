/* ------------------------------------------------------------------------------
 *	Project	: Smart Home Client
 *	File	:
 *	Date	:
 *	Author	: Sebastian Lesse
 *	Brief	:
 * ------------------------------------------------------------------------------
 */
 
#ifndef _SHC_USART_INTERFACE_H_
#define _SHC_USART_INTERFACE_H_

#include "shc_project_configuration.h"
#include "shc_common_types.h"

#ifndef USART_DEVICE_NAME_MAX_LENGTH
#define USART_DEVICE_NAME_MAX_LENGTH			128
#endif

typedef enum {
	USART_BAUDRATE_9600 = 0,
	USART_BAUDRATE_230400
} USART_BAUDRATE;

typedef enum {
	USART_PARITY_NONE = 0
} USART_PARITY;

typedef enum {
	USART_DATABITS_8 = 0
} USART_DATABITS;

/*!
 * 
 */
typedef struct {
	int _handle_id;
	char device_name[USART_DEVICE_NAME_MAX_LENGTH];
	USART_BAUDRATE baudrate;
	USART_PARITY parity;
	USART_DATABITS databits;
} USART_INTERFACE;

/*!
 *
 */
u8 usart_init(USART_INTERFACE* p_device);

/*!
 *
 */
u16 usart_read_bytes(USART_INTERFACE* p_device, u16 max_num_bytes, u8* p_buffer, u16 timeout_ms);

/*!
 *
 */
u16 usart_read_line(USART_INTERFACE* p_device, u16 max_num_bytes, u8* p_buffer, u16 timeout_ms);

/*!
 *
 */
u8 usart_write_line(USART_INTERFACE* p_device, u16 num_bytes, const u8* p_buffer);

/*!
 *
 */
void usart_deinit(USART_INTERFACE* p_device);

#endif // _SHC_USART_INTERFACE_H_