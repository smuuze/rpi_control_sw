/* ------------------------------------------------------------------------------
 *	Project	: Smart Home Client
 *	File	:
 *	Date	:
 *	Author	: Sebastian Lesse
 *	Brief	:
 * ------------------------------------------------------------------------------
 */
 
#ifndef _SHC_LCD_INTERFACE_H_
#define _SHC_LCD_INTERFACE_H_

#include "shc_project_configuration.h"
#include "shc_common_types.h"

void lcd_init(void);

void lcd_deinit(void);

void lcd_write_line(char* message);

#endif // _SHC_LCD_INTERFACE_H_