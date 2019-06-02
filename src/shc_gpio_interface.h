/* ------------------------------------------------------------------------------
 *	Project	: Smart Home Client
 *	File	:
 *	Date	:
 *	Author	: Sebastian Lesse
 *	Brief	:
 * ------------------------------------------------------------------------------
 */
 
#ifndef _SHC_GPIO_INTERFACE_H_
#define _SHC_GPIO_INTERFACE_H_

#include "shc_project_configuration.h"
#include "shc_common_types.h"

// ----------------------------------------------------------------------------------------------------------------------

#define GPIO_ON					1
#define GPIO_OFF				0
#define GPIO_HIGH_Z				2

#define GPIO_INPUT				1
#define GPIO_OUTPUT				0

#define MY_GPIO_INTERFACE_WIRINGPI		0
#define MY_GPIO_INTERFACE_PIGPIO		1
#define MY_GPIO_INTERFACE 			MY_GPIO_INTERFACE_WIRINGPI

#if MY_GPIO_INTERFACE == MY_GPIO_INTERFACE_WIRINGPI

#include <wiringPi.h>

#define GPIO_SPI_CS0_PIN_NUM			10
#define GPIO_SPI_SCK_PIN_NUM			14
#define GPIO_SPI_MOSI_PIN_NUM			12
#define GPIO_SPI_MISO_PIN_NUM			13

#define GPIO_READ_PIN(pin_num)			digitalRead(pin_num)
#define GPIO_WRITE_PIN(pin_num, state)		digitalWrite(pin_num, state)
#define GPIO_INITIALIZE()			wiringPiSetup()
#define GPIO_CONFIGURE_PIN(pin_num, is_input)	pinMode(pin_num, (is_input == GPIO_INPUT) ? INPUT : OUTPUT)
#define GPIO_PULL_UP_DOWN(pin_num, pull_down)	pullUpDnControl(pin_num, (pull_down == GPIO_OFF) ? PUD_DOWN : (pull_down == GPIO_ON) ? PUD_UP : PUD_OFF )

#define GPIO_OUTPUT_OFF				LOW
#define GPIO_OUTPUT_ON				HIGH

#else

#include <pigpio.h>
#define GPIO_IS_BUSY_PIN_NUM			17
#define GPIO_SAMPLE_PIN				26
#define GPIO_CONFIGURE_PIN(pin_num, is_input)	gpioSetMode(pin_num, is_input ? PI_INPUT : PI_OUTPUT)
#define GPIO_READ_PIN(pin_num)			gpioRead(pin_num)
#define GPIO_INITIALIZE()			gpioInitialise()

#endif

// ----------------------------------------------------------------------------------------------------------------------

#define GPIO_INTERFACE_BUILD_INOUT(name, pin_number)									\
															\
	static GPIO_INTERFACE __##name##_pin_descirptor = {								\
		.pin_num = pin_number,											\
		.is_initialized = 0,											\
		.is_input = 1,												\
		.is_high_level = 1,											\
		.match_event_level = 0,											\
		.event_rised = 0,											\
		.sample_time_reference = 0,										\
		.sample_timeout = 0,											\
		.event_ref_time = 0,											\
		.event_timeout = 0											\
	};														\
															\
	void name##_init(void) {											\
		gpio_interface_init(&__##name##_pin_descirptor);							\
	}														\
															\
	void name##_drive_high(void) {											\
		gpio_interface_drive_high(&__##name##_pin_descirptor);							\
	}														\
															\
	void name##_drive_low(void) {											\
		gpio_interface_drive_low(&__##name##_pin_descirptor);							\
	}														\
															\
	void name##_no_drive(void) {											\
		gpio_interface_no_drive(&__##name##_pin_descirptor);							\
	}														\
															\
	void name##_toggle_level(void) {										\
		gpio_interface_toggle_level(&__##name##_pin_descirptor);						\
	}														\
															\
	void name##_pull_up(void) {											\
		gpio_interface_pull_up(&__##name##_pin_descirptor);							\
	}														\
															\
	void name##_pull_down(void) {											\
		gpio_interface_pull_down(&__##name##_pin_descirptor);							\
	}														\
															\
	void name##_no_pull(void) {											\
		gpio_interface_no_pull(&__##name##_pin_descirptor);							\
	}														\
															\
	u8 name##_is_high_level(void) {											\
		return gpio_interface_is_high_level(&__##name##_pin_descirptor);					\
	}														\
															\
	u8 name##_is_low_level(void) {											\
		return gpio_interface_is_low_level(&__##name##_pin_descirptor);						\
	}														\

#define GPIO_INTERFACE_INCLUDE_INOUT(name)										\
	void name##_drive_high(void);											\
	void name##_drive_low(void);											\
	void name##_no_drive(void);											\
	void name##_pull_up(void);											\
	void name##_pull_down(void);											\
	void name##_no_pull(void);											\
	u8 name##_is_low_level(void);											\
	u8 name##_is_high_level(void);
	
// ----------------------------------------------------------------------------------------------------------------------		

/*!
 *
 */
typedef struct {
	u8 pin_num;
	u8 is_initialized;
	u8 is_input;
	u8 is_high_level;
	u8 match_event_level;
	u8 event_rised;
	u32 sample_time_reference;
	u32 sample_timeout;
	u32 event_ref_time;
	u32 event_timeout;
} GPIO_INTERFACE;

// ----------------------------------------------------------------------------------------------------------------------

/*!
 *
 */
void gpio_interface_init(GPIO_INTERFACE* p_descriptor);

/*!
 *
 */
void gpio_interface_drive_high(GPIO_INTERFACE* p_descriptor);

/*!
 *
 */
void gpio_interface_drive_low(GPIO_INTERFACE* p_descriptor);

/*!
 *
 */
void gpio_interface_no_drive(GPIO_INTERFACE* p_descriptor);

/*!
 *
 */
void gpio_interface_toggle_level(GPIO_INTERFACE* p_descriptor);

/*!
 *
 */
void gpio_interface_pull_up(GPIO_INTERFACE* p_descriptor);

/*!
 *
 */
void gpio_interface_pull_down(GPIO_INTERFACE* p_descriptor);

/*!
 *
 */
void gpio_interface_no_pull(GPIO_INTERFACE* p_descriptor);

/*!
 *
 */
u8 gpio_interface_is_high_level(GPIO_INTERFACE* p_descriptor);

/*!
 *
 */
u8 gpio_interface_is_low_level(GPIO_INTERFACE* p_gpio);

// ----------------------------------------------------------------------------------------------------------------------

/*!
 *
 */
u8 gpio_initialize(GPIO_INTERFACE* p_gpio);

/*!
 *
 */
u8 gpio_is_event(GPIO_INTERFACE* p_gpio);

/*!
 *
 */
void gpio_reset_pin(GPIO_INTERFACE* p_gpio);

/*!
 *
 */
void gpio_set_state(GPIO_INTERFACE* p_gpio, u8 state);

#endif // _SHC_GPIO_INTERFACE_H_