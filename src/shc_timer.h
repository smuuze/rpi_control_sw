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
