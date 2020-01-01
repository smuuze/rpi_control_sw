/* ------------------------------------------------------------------------------
 *	Project	: Smart Home Client
 *	File	:
 *	Date	:
 *	Author	: Sebastian Lesse
 *	Brief	:
 * ------------------------------------------------------------------------------
 */
 

#include "shc_common_types.h"
			

u16 readU16_LSB(u8* p_buffer) {
	return ((u16)p_buffer[1] << 8) + (u16)(p_buffer[0]);
}

u16 readU16_MSB(u8* p_buffer) {
	return ((u16)p_buffer[0] << 8) + (u16)(p_buffer[1]);
}