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

void lcd_set_enabled(u8 enabled);

#ifdef LCD_AVAILABLE
#define LCD_INIT(a)		lcd_set_enabled(a)
#define LCD_PRINTF(...)		{					\
					char lcd_msg[64];		\
					sprintf(lcd_msg, __VA_ARGS__);	\
					lcd_write_line(lcd_msg);	\
				}
#else
#define LCD_INIT(a)		do{}while(0)
#define LCD_PRINTF(...)		do{}while(0)
#endif

#endif // _SHC_LCD_INTERFACE_H_