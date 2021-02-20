/*! 
 * ----------------------------------------------------------------------------
 *
 * \file	main_time_helper.c
 * \brief
 * \author	sebastian lesse
 *
 * ----------------------------------------------------------------------------
 */

#define TRACER_OFF

//-----------------------------------------------------------------------------

#include "config.h"

//-----------------------------------------------------------------------------

#include "tracer.h"

//-----------------------------------------------------------------------------

#include "cpu.h"
#include <stdio.h>

//-----------------------------------------------------------------------------

#include "initialization/initialization.h"
#include "time_management/time_management.h"

//-----------------------------------------------------------------------------

TIME_MGMN_BUILD_STATIC_TIMER_U8(MAIN_U8_TIMER)
TIME_MGMN_BUILD_STATIC_TIMER_U16(MAIN_U16_TIMER)
TIME_MGMN_BUILD_STATIC_TIMER_U32(MAIN_U32_TIMER)

//-----------------------------------------------------------------------------

int main(int argc, char* argv[]) {

	(void) argc;
	(void) argv;

	ATOMIC_OPERATION
	(
		initialization();
	)

	MAIN_U8_TIMER_start();
	MAIN_U16_TIMER_start();
	MAIN_U32_TIMER_start();

	u8  start_time_u8  = MAIN_U8_TIMER_start_time();
	u16 start_time_u16 = MAIN_U16_TIMER_start_time();
	u32 start_time_u32 = MAIN_U32_TIMER_start_time();

	printf("Start-Time u8  - Signed: %d - Unsigned %u \n", start_time_u8,  start_time_u8);
	printf("Start-Time u16 - Signed: %d - Unsigned %u \n", start_time_u16, start_time_u16);
	printf("Start-Time u32 - Signed: %d - Unsigned %u \n", start_time_u32, start_time_u32);

	return 0;
}
