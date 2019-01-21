/* ------------------------------------------------------------------------------
 *	Project	: Smart Home Client
 *	File	:
 *	Date	:
 *	Author	: Sebastian Lesse
 *	Brief	:
 * ------------------------------------------------------------------------------
 */
 
#ifndef _SHC_COMMON_TYPES_H_
#define _SHC_COMMON_TYPES_H_

#include "shc_project_configuration.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>			

// -------- Error-Code ------------------------------------------------------------------
								
#define NO_ERR					0
#define ERR_INVALID_ARGUMENT			1
#define ERR_NOT_INITIALIZED			2
#define ERR_SEND_MSG				3
#define ERR_BAD_CMD				4
#define ERR_ANSWER_LENGTH			5
#define ERR_COMMUNICATION			6
#define ERR_FILE_OPEN				7
#define ERR_END_OF_FILE				8
#define ERR_CONNECT_TIMEOUT			9
#define ERR_QEUE_FULL				10
#define ERR_QEUE_EMPTY				10
#define ERR_QEUE_WRITE_FAULT			11
#define ERR_QEUE_READ_FAULT			12
#define ERR_NOT_EQUAL				13
#define ERR_GPIO				14
#define ERR_TIMEOUT				15
#define ERR_WRITE_FILE				16

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;

/*!
 *
 */
typedef struct {
	u8 length;
	u8 payload[GENERAL_STRING_BUFFER_MAX_LENGTH]; 
} STRING_BUFFER;

/*!
 *
 */
typedef enum {
	SPI = 0x00,
	I2C,
	USART
} COM_INTERFACE_TYPE;

/*!
 *
 */
typedef struct {
	
	COM_INTERFACE_TYPE type;
	union {
		SPI_INTERFACE spi;
	} data;
	
} COM_INTERFACE;

/*
 *
 */
typedef struct {
	u32 reference;
	u32 interval;
} SCHEDULING_TIME_VALUE;

/*
 *
 */
typedef struct {
	SCHEDULING_TIME_VALUE event;
	SCHEDULING_TIME_VALUE configuration;
	SCHEDULING_TIME_VALUE report;
} SCHEDULING_INTERFACE;


#endif // _SHC_COMMON_TYPES_H_