/* ------------------------------------------------------------------------------
 *	Project	: Smart Home Client
 *	File	:
 *	Date	:
 *	Author	: Sebastian Lesse
 *	Brief	:
 * ------------------------------------------------------------------------------
 */
 
#ifndef _SHC_THREAD_INTERFACE_H_
#define _SHC_THREAD_INTERFACE_H_

#include "shc_project_configuration.h"
#include "shc_common_types.h"

#include "pthread.h"

#ifndef _POSIX_THREADS 
#error THREADS_ARE_NOT_AVAILABLE
#endif

/*!
 *
 */
typedef enum {
	THREAD_PRIORITY_HIGH = 0x01,
	THREAD_PRIORITY_MIDDLE,
	THREAD_PRIORITY_LOW
} THREAD_INTERFACE_PRITORITY;

/*!
 *
 */
typedef pthread_t THREAD_INTERFACE_ID;

/*!
 *
 */
typedef void* (*THREAD_INTERFACE_RUN_FUNCTION_CALLBACK)		(void* arguments);

/*!
 *
 */
typedef void (*THREAD_INTERFACE_TERMINATE_CALLBACK)		(void);

/*!
 *
 */
typedef struct {
	THREAD_INTERFACE_ID id;
	THREAD_INTERFACE_PRITORITY priority;
	THREAD_INTERFACE_RUN_FUNCTION_CALLBACK run;
	THREAD_INTERFACE_TERMINATE_CALLBACK terminate;
} THREAD_INTERFACE_TYPE;

#define THREAD_INTERFACE_BUILD_THREAD(name, prio, p_run, p_terminate)			\
	static THREAD_INTERFACE_TYPE __##name##_thread_obj = {				\
		.priority = prio,							\
		.run = &p_run,								\
		.terminate = &p_terminate						\
	};										\
											\
	void name##_init(void) {}							\
											\
	u8 name##_start(void) {								\
		return thread_interface_create_thread(&__##name##_thread_obj);		\
	}										\
											\
	void name##_terminate(void) {							\
		thread_interface_terminate(&__##name##_thread_obj);			\
	}

#define THREAD_INTERFACE_INCLUDE_THREAD(name)						\
	void name##_init(void);								\
	u8 name##_start(void);								\
	void name##_terminate(void);
/*!
 *
 */
u8 thread_interface_create_thread(THREAD_INTERFACE_TYPE* p_thread);

/*!
 *
 */
u8 thread_interface_terminate(THREAD_INTERFACE_TYPE* p_thread);

#endif // _SHC_THREAD_INTERFACE_H_