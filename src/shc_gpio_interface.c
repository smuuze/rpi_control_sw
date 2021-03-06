/* ------------------------------------------------------------------------------
 *	Project	: Smart Home Client
 *	File	:
 *	Date	:
 *	Author	: Sebastian Lesse
 *	Brief	:
 * ------------------------------------------------------------------------------
 */

// ---- INCLUDES ----------------------------------------------------------------

#include "shc_timer.h"
#include "shc_project_configuration.h"
#include "shc_common_types.h"
#include "shc_common_string.h"
#include "shc_debug_interface.h"

#include "shc_gpio_interface.h"

// ---- LOCAL DEFINITIONS -------------------------------------------------------

#define GPIO_DEBUG_MSG				noDEBUG_MSG

// ---- STATIC DATA -------------------------------------------------------------


// ---- IMPLEMENTATION ----------------------------------------------------------

void gpio_set_state(GPIO_INTERFACE* p_gpio, u8 state) {

	GPIO_CONFIGURE_PIN(p_gpio->pin_num, p_gpio->is_input);

	if (p_gpio->is_input != 0) {
		GPIO_PULL_UP_DOWN(p_gpio->pin_num, state);
	} else {
		GPIO_WRITE_PIN(p_gpio->pin_num, state);
	}
}

u8 gpio_initialize(GPIO_INTERFACE* p_gpio) {

	static u8 is_initialized = 0;

	if (is_initialized == 0) {

		int err_code = GPIO_INITIALIZE();
		if (err_code < 0) {
			GPIO_DEBUG_MSG("- Initializing GPIOD has FAILED !!! --- (err-code = %d)\n", err_code);
			return ERR_GPIO;
		}

		GPIO_DEBUG_MSG("- Initializing gpio has succeeded!\n");
		is_initialized = 1;
	}

	if (p_gpio->is_initialized) {
		return NO_ERR;
	}

	GPIO_CONFIGURE_PIN(p_gpio->pin_num, p_gpio->is_input);
	gpio_set_state(p_gpio, p_gpio->is_high_level);
	//gpioSetPullUpDown(p_gpio->pin_num, p_gpio->is_high_level ? PI_PUD_UP : PI_PUD_OFF);

	p_gpio->is_initialized = 1;
	return NO_ERR;
}

u8 gpio_is_event(GPIO_INTERFACE* p_gpio) {

	if (p_gpio->is_initialized == 0) {
		GPIO_DEBUG_MSG("- GPIO-Interface is not initialized!!! ---\n");
		sleep(50000);
		return 0;
	}

	if (mstime_is_time_up(p_gpio->sample_time_reference, p_gpio->sample_timeout) == 0) {
		return p_gpio->event_rised != 0 ? 1 : 0;
	}

	GPIO_CONFIGURE_PIN(GPIO_SAMPLE_PIN /* GPIO_22 */, 0 /* IS_OUTPUT */);
	GPIO_WRITE_PIN(GPIO_SAMPLE_PIN /* GPIO_22 */, GPIO_ON);


	p_gpio->sample_time_reference = mstime_get_time();
	GPIO_DEBUG_MSG("- Sampling GPIO (number = %d / time : %d)\n", p_gpio->pin_num, p_gpio->sample_time_reference);

	if (p_gpio->event_rised != 0 && p_gpio->event_timeout != 0) {
		if (mstime_is_time_up(p_gpio->event_ref_time, p_gpio->event_timeout) == 0) {
			GPIO_DEBUG_MSG("- GPIO-Event still pending (number = %d / time : %d)\n", p_gpio->pin_num, p_gpio->event_ref_time);
			GPIO_WRITE_PIN(GPIO_SAMPLE_PIN /* GPIO_22 */, GPIO_OFF);
			return 1;
		}
	}

	p_gpio->event_rised = 0;

	int gpio_level = GPIO_READ_PIN(p_gpio->pin_num);

	if (gpio_level < 0 ){//== PI_BAD_GPIO) {
		GPIO_DEBUG_MSG("- Reading GPIO-Levl has FAILED !!! --- (pin number = %d)\n", p_gpio->pin_num);
		GPIO_WRITE_PIN(GPIO_SAMPLE_PIN /* GPIO_22 */, GPIO_OFF);
		return 0;
	}

	GPIO_DEBUG_MSG("- GPIO Event  pin-level = %d\n", gpio_level);

	if (gpio_level == p_gpio->match_event_level) {

		GPIO_DEBUG_MSG("- GPIO Event raised\n");

		p_gpio->event_rised = 1;
		p_gpio->event_ref_time = mstime_get_time();
		GPIO_WRITE_PIN(GPIO_SAMPLE_PIN /* GPIO_22 */, GPIO_OFF);
		return 1;

	} else {
		GPIO_WRITE_PIN(GPIO_SAMPLE_PIN /* GPIO_22 */, GPIO_OFF);
		return 0;
	}
}


void gpio_reset_pin(GPIO_INTERFACE* p_gpio) {
	p_gpio->match_event_level = 0;
	p_gpio->event_rised = 0;
	p_gpio->event_timeout = 0;
	p_gpio->sample_time_reference = 0;
}


// -------------------------------------------------------------------------------------------------------------------------

void gpio_interface_init(GPIO_INTERFACE* p_gpio) {

	static u8 is_initialized = 0;

	if (is_initialized == 0) {

		int err_code = GPIO_INITIALIZE();
		if (err_code < 0) {
			GPIO_DEBUG_MSG("- Initializing GPIOD has FAILED !!! --- (err-code = %d)\n", err_code);
		}

		GPIO_DEBUG_MSG("- Initializing gpio has succeeded!\n");
		is_initialized = 1;
	}

	if (p_gpio->is_initialized) {
		return;
	}

	GPIO_CONFIGURE_PIN(p_gpio->pin_num, p_gpio->is_input);

	if (p_gpio->is_input != 0) {
	
		if (p_gpio->is_high_level != 0) {
			gpio_interface_pull_up(p_gpio);
		} else {
			gpio_interface_pull_down(p_gpio);
		}
		
	} else {
	
		if (p_gpio->is_high_level != 0) {
			gpio_interface_drive_high(p_gpio);
		} else {
			gpio_interface_drive_low(p_gpio);
		}
	}

	p_gpio->is_initialized = 1;
}

void gpio_interface_drive_high(GPIO_INTERFACE* p_gpio) {

	GPIO_CONFIGURE_PIN(p_gpio->pin_num, GPIO_OUTPUT);
	GPIO_WRITE_PIN(p_gpio->pin_num, GPIO_ON);
}

void gpio_interface_drive_low(GPIO_INTERFACE* p_gpio) {

	GPIO_CONFIGURE_PIN(p_gpio->pin_num, GPIO_OUTPUT);
	GPIO_WRITE_PIN(p_gpio->pin_num, GPIO_OFF);
}

void gpio_interface_no_drive(GPIO_INTERFACE* p_gpio) {

	GPIO_CONFIGURE_PIN(p_gpio->pin_num, GPIO_INPUT);
	GPIO_PULL_UP_DOWN(p_gpio->pin_num, GPIO_HIGH_Z);
}

void gpio_interface_toggle_level(GPIO_INTERFACE* p_gpio) {

	if (p_gpio->is_input != 0) {
		GPIO_CONFIGURE_PIN(p_gpio->pin_num, GPIO_OUTPUT);
	} 
	
	if (p_gpio->is_high_level != 0) {
		GPIO_WRITE_PIN(p_gpio->pin_num, GPIO_OFF);
		p_gpio->is_high_level = 0;
		
	} else {
		GPIO_WRITE_PIN(p_gpio->pin_num, GPIO_ON);
		p_gpio->is_high_level = 1;
	}
}

void gpio_interface_pull_up(GPIO_INTERFACE* p_gpio) {

	GPIO_CONFIGURE_PIN(p_gpio->pin_num, GPIO_INPUT);
	GPIO_PULL_UP_DOWN(p_gpio->pin_num, GPIO_ON);
}

void gpio_interface_pull_down(GPIO_INTERFACE* p_gpio) {

	GPIO_CONFIGURE_PIN(p_gpio->pin_num, GPIO_INPUT);
	GPIO_PULL_UP_DOWN(p_gpio->pin_num, GPIO_OFF);
}

void gpio_interface_no_pull(GPIO_INTERFACE* p_gpio) {

	GPIO_CONFIGURE_PIN(p_gpio->pin_num, GPIO_INPUT);
	GPIO_PULL_UP_DOWN(p_gpio->pin_num, GPIO_HIGH_Z);
}

u8 gpio_interface_is_high_level(GPIO_INTERFACE* p_gpio) {
	return GPIO_READ_PIN(p_gpio->pin_num) == GPIO_ON ? 1 : 0;
}

u8 gpio_interface_is_low_level(GPIO_INTERFACE* p_gpio) {
	return GPIO_READ_PIN(p_gpio->pin_num) == GPIO_OFF ? 1 : 0;
}
