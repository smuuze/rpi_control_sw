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
#define LCD_PIN_RS				(1 << 7)
#define LCD_PIN_D4				(1 << 0)
#define LCD_PIN_D5				(1 << 1)
#define LCD_PIN_D6				(1 << 2)
#define LCD_PIN_D7				(1 << 3)

// ---- STATIC DATA -------------------------------------------------------------

GPIO_INTERFACE_BUILD_INOUT(LCD_RS, LCD_PIN_NUMBER_RS)
GPIO_INTERFACE_BUILD_INOUT(LCD_EN, LCD_PIN_NUMBER_EN)
GPIO_INTERFACE_BUILD_INOUT(LCD_D4, LCD_PIN_NUMBER_D4)
GPIO_INTERFACE_BUILD_INOUT(LCD_D5, LCD_PIN_NUMBER_D5)
GPIO_INTERFACE_BUILD_INOUT(LCD_D6, LCD_PIN_NUMBER_D6)
GPIO_INTERFACE_BUILD_INOUT(LCD_D7, LCD_PIN_NUMBER_D7)

static char line_buffer[LCD_NUM_LINES][LCD_NUM_CHARS + 1];

// ---- IMPLEMENTATION ----------------------------------------------------------

static void lcd_set_pins(u8 pins) {
	
	if (pins & LCD_PIN_RS) LCD_RS_drive_high();  else  LCD_RS_drive_low();

	if (pins & LCD_PIN_D4) LCD_D4_drive_high();  else  LCD_D4_drive_low();
	if (pins & LCD_PIN_D5) LCD_D5_drive_high();  else  LCD_D5_drive_low();
	if (pins & LCD_PIN_D6) LCD_D6_drive_high();  else  LCD_D6_drive_low();
	if (pins & LCD_PIN_D7) LCD_D7_drive_high();  else  LCD_D7_drive_low();

	usleep(50); // reduce cpu-load

	LCD_EN_drive_high();
	usleep(50); // reduce cpu-load
	LCD_EN_drive_low();
}

static void lcd_select_line(u8 line_index) {

	switch (line_index) {
		default :
		case 0 : // Line 1
			lcd_set_pins(LCD_PIN_D7 | 0x00);
			lcd_set_pins(0x00);
			break;

		case 1 : // Line 2
			lcd_set_pins(LCD_PIN_D7 | LCD_PIN_D6);
			lcd_set_pins(0);
			break;
	}
}

void lcd_write_char(char character) {
	lcd_set_pins(LCD_PIN_RS | (u8)(character >> 4));
	lcd_set_pins(LCD_PIN_RS | (u8)(character & 0x0F));
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

	// -----------------------------------------------------

	usleep(15 * 1000); // wait for LCD controller power-up

	lcd_set_pins(LCD_PIN_D4 | LCD_PIN_D5); 	usleep(5 * 1000);
	lcd_set_pins(LCD_PIN_D4 | LCD_PIN_D5); 	usleep(100);
	lcd_set_pins(LCD_PIN_D4 | LCD_PIN_D5); 	usleep(5 * 1000);

	lcd_set_pins(LCD_PIN_D5); 		usleep(5 * 1000); 

	// -----------------------------------------------------	

	lcd_set_pins(LCD_PIN_D5);					// switch to 4-Bit interface
	lcd_set_pins(LCD_PIN_D7);					// 2 Lines / 5x8 Font

	lcd_set_pins(0);						// cmd control
	lcd_set_pins(LCD_PIN_D7 | LCD_PIN_D6);				// enable display	

	lcd_set_pins(0);						
	lcd_set_pins(LCD_PIN_D4);					// display clear

	lcd_set_pins(0);						// Entry mode set
	lcd_set_pins(LCD_PIN_D6 | LCD_PIN_D5);				// cursor increment

	usleep(2 * 1000);

	u8 line_cnt = 0;
	for ( ; line_cnt < LCD_NUM_LINES; line_cnt += 1) {
		memset(line_buffer[line_cnt], ' ', LCD_NUM_CHARS);
		line_buffer[line_cnt][LCD_NUM_CHARS] = '\0';
	}
}

void lcd_deinit(void) {
	
}

void lcd_write_line(char* message) {

	LCD_DEBUG_MSG("lcd_write_line() - New Line: %s\n", message);

	u8 line_cnt = 0;
	u8 char_cnt = 0;

	for ( ; line_cnt < LCD_NUM_LINES - 1; line_cnt += 1) {
		for ( ; char_cnt < LCD_NUM_CHARS; char_cnt += 1) {
			line_buffer[line_cnt][char_cnt] = line_buffer[line_cnt + 1][char_cnt];
		}
	}

	u8 length = string_length(message);
	if (length > LCD_NUM_CHARS) {
		length = LCD_NUM_CHARS;
	}
		
	for (char_cnt = 0 ; char_cnt < length; char_cnt += 1) {
		line_buffer[LCD_NUM_LINES - 1][char_cnt] = message[char_cnt];
	}
		
	for ( ; char_cnt < LCD_NUM_CHARS; char_cnt += 1) {
		line_buffer[LCD_NUM_LINES - 1][char_cnt] = ' ';
	}

	LCD_DEBUG_MSG("lcd_write_line() - LCD-Content:\n");

	for (line_cnt = 0 ; line_cnt < LCD_NUM_LINES; line_cnt += 1) {
		LCD_DEBUG_MSG("\t | %s |\n", line_buffer[line_cnt]);
	}

	for (line_cnt = 0; line_cnt < LCD_NUM_LINES; line_cnt += 1) {
		
		lcd_select_line(line_cnt);

		for (char_cnt = 0 ; char_cnt < LCD_NUM_CHARS; char_cnt += 1) {
			lcd_write_char(line_buffer[line_cnt][char_cnt]);
		}
	}
}
