/*
 * shc_timer.h
 *
 *  Created on: 12.03.2019
 *      Author: sebastian.lesse
 */

#ifndef SHC_TIMER_H_
#define SHC_TIMER_H_


#include "shc_common_types.h"

#define TIMER_MAX_TIME_U32			0xFFFFFFFF


#define TIME_MGMN_BUILD_TIMER(name)										\
														\
	static u32 _##name##_time_reference = 0;								\
														\
	void name##_start(void) {										\
		_##name##_time_reference = mstime_get_time();							\
	}													\
														\
	void name##_stop(void) {										\
		_##name##_time_reference = 0;									\
	}													\
														\
	u8 name##_is_up(u32 time_interval) {									\
		return mstime_is_time_up(_##name##_time_reference, time_interval);				\
	}													\
														\
	u32 name##_start_time(void) {										\
		return _##name##_time_reference;								\
	}													\
														\
	u32 name##_elapsed(void) {										\
		return mstime_elapsed(_##name##_time_reference);						\
	}


/*!
 *
 * @return
 */
u32 mstime_get_time(void);


/*!
 *
 * @param reference_time
 * @param interval_time
 * @return
 */
u8 mstime_is_time_up(u32 reference_time, u32 interval_time);


/*!
 *
 * @return
 */
u32 mstime_elapsed(u32 reference_time);


#endif /* SHC_TIMER_H_ */
