/*
 * shc_timer.c
 *
 *  Created on: 12.03.2019
 *      Author: sebastian.lesse
 */

#include <time.h>

#include "shc_project_configuration.h"
#include "shc_common_types.h"
#include "shc_common_string.h"
#include "shc_common_configuration.h"

/*
 *
 */
u32 mstime_get_time(void) {
	struct timespec time_spec;

	if (clock_gettime(CLOCK_MONOTONIC, &time_spec) == 0) {
		return ((u32)(time_spec.tv_sec * 1e3) + (u32)(time_spec.tv_nsec / 1e6)); //time_spec.tv_nsec / 1000 / 1000;
	} else {
		return 0;
	}
}

/*
 *
 */
u8 mstime_is_time_up(u32 reference_time, u32 interval_time) {
	return (mstime_get_time() - reference_time) > interval_time ? 1 : 0;
}

