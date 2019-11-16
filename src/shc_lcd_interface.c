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

#define LCD_DEBUG_MSG				noDEBUG_MSG

// ---- STATIC DATA -------------------------------------------------------------

GPIO_INTERFACE_BUILD_INOUT(LCD_RS, LCD_PIN_NUMBER_RS)
GPIO_INTERFACE_BUILD_INOUT(LCD_EN, LCD_PIN_NUMBER_EN)
GPIO_INTERFACE_BUILD_INOUT(LCD_D4, LCD_PIN_NUMBER_D4)
GPIO_INTERFACE_BUILD_INOUT(LCD_D5, LCD_PIN_NUMBER_D5)
GPIO_INTERFACE_BUILD_INOUT(LCD_D6, LCD_PIN_NUMBER_D6)
GPIO_INTERFACE_BUILD_INOUT(LCD_D7, LCD_PIN_NUMBER_D7)

// ---- IMPLEMENTATION ----------------------------------------------------------

void lcd_init(void) {
	
	LCD_RS_init();
	LCD_EN_init();

	LCD_D4_init();
	LCD_D5_init();
	LCD_D6_init();
	LCD_D7_init();

	LCD_D4_drive_low();
	LCD_D5_drive_low();
	LCD_D6_drive_low();
	LCD_D7_drive_low();
}

void lcd_deinit(void) {
	
}

u8 lcd_write_line(unisgned char* message) {

}
