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

#include "shc_lcd_interface.h"

#include <sys/ioctl.h>
#include <linux/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

// ---- LOCAL DEFINITIONS -------------------------------------------------------

#define LCD_DEBUG_MSG				DEBUG_MSG

// helping definitions for fast access to pins via function lcd_set_pins()
#define LCD_PIN_RS				(1 << 0)
#define LCD_PIN_D4				(1 << 1)
#define LCD_PIN_D5				(1 << 2)
#define LCD_PIN_D6				(1 << 3)
#define LCD_PIN_D7				(1 << 4)

// ---- STATIC DATA -------------------------------------------------------------

GPIO_INTERFACE_BUILD_INOUT(LCD_RS, LCD_PIN_NUMBER_RS)
GPIO_INTERFACE_BUILD_INOUT(LCD_EN, LCD_PIN_NUMBER_EN)
GPIO_INTERFACE_BUILD_INOUT(LCD_D4, LCD_PIN_NUMBER_D4)
GPIO_INTERFACE_BUILD_INOUT(LCD_D5, LCD_PIN_NUMBER_D5)
GPIO_INTERFACE_BUILD_INOUT(LCD_D6, LCD_PIN_NUMBER_D6)
GPIO_INTERFACE_BUILD_INOUT(LCD_D7, LCD_PIN_NUMBER_D7)

static u8 line_buffer[LCD_NUM_LINES][LCD_NUM_CHARS];

// ---- IMPLEMENTATION ----------------------------------------------------------

static void lcd_set_pins(u8 pins) {
	
	if (pins & LCD_PIN_RS) LCD_RS_drive_high();  else  LCD_RS_drive_low();

	if (pins & LCD_PIN_D4) LCD_D4_drive_high();  else  LCD_D4_drive_low();
	if (pins & LCD_PIN_D5) LCD_D5_drive_high();  else  LCD_D5_drive_low();
	if (pins & LCD_PIN_D6) LCD_D6_drive_high();  else  LCD_D6_drive_low();
	if (pins & LCD_PIN_D7) LCD_D7_drive_high();  else  LCD_D7_drive_low();

	LCD_EN_drive_high();
	usleep(40); // reduce cpu-load
	LCD_EN_drive_low();

}

static void lcd_return_cursor(void) {
	lcd_set_pins(LCD_PIN_D5);
	usleep(2 * 1000); // reduce cpu-load
}

void lcd_init(void) {
	
	LCD_DEBUG_MSG("lcd_init()\n");

	LCD_DEBUG_MSG("lcd_init() - init pins\n");

	LCD_RS_init();
	LCD_EN_init();

	LCD_D4_init();
	LCD_D5_init();
	LCD_D6_init();
	LCD_D7_init();

	LCD_DEBUG_MSG("lcd_init() - set initial state \n");

	LCD_RS_drive_low();
	LCD_EN_drive_low();

	LCD_D4_drive_low();
	LCD_D5_drive_low();
	LCD_D6_drive_low();
	LCD_D7_drive_low();

	LCD_DEBUG_MSG("lcd_init() - running init sequence \n");

	usleep(15 * 1000); // wait for LCD controller power-up

	lcd_set_pins(LCD_PIN_D4 | LCD_PIN_D5); 	usleep(5 * 1000);
	lcd_set_pins(LCD_PIN_D4 | LCD_PIN_D5); 	usleep(100);
	lcd_set_pins(LCD_PIN_D4 | LCD_PIN_D5); 	usleep(5 * 1000);

	lcd_set_pins(LCD_PIN_D5); 		usleep(5 * 1000); 	// switch to 4-Bit interface

	lcd_set_pins(LCD_PIN_D5);					// Function set	
	lcd_set_pins(LCD_PIN_D7);		usleep(2 * 1000); 	// 2 Lines / 5x8 Font

	lcd_set_pins(0);
	lcd_set_pins(LCD_PIN_D4);		usleep(2 * 1000);	// clear display	

	lcd_set_pins(0);						
	lcd_set_pins(LCD_PIN_D7 | LCD_PIN_D6);				// display on / no cursor / no cursor blinking

	lcd_set_pins(0);						// Entry mode set
	lcd_set_pins(LCD_PIN_D6 | LCD_PIN_D5);				// DD-Ram Address auto increment

	lcd_return_cursor();
}

void lcd_deinit(void) {
	
}

void lcd_write_char(char character) {
	lcd_set_pins(LCD_PIN_RS | (u8)(character >> 4));
	lcd_set_pins(LCD_PIN_RS | (u8)(character & 0x0F));
	lcd_return_cursor();
}

void lcd_write_line(char* message) {

	u8 line_cnt = 0;
	for ( ; line_cnt < LCD_NUM_LINES - 1; line_cnt += 1) {
		memcpy(line_buffer[line_cnt], line_buffer[line_cnt + 1], LCD_NUM_CHARS);
	}

	memset(line_buffer[LCD_NUM_LINES - 1], 0x00, LCD_NUM_CHARS);
	u8 length = string_length(message);
	memcpy(line_buffer[LCD_NUM_LINES - 1], (u8*)message, length);

	for (line_cnt = 0; line_cnt < LCD_NUM_LINES; line_cnt += 1) {

		u8 char_cnt = 0;
		for ( ; char_cnt < LCD_NUM_CHARS; char_cnt += 1) {
			lcd_write_char((char)line_buffer[line_cnt][char_cnt]);
		}
	} 
}
