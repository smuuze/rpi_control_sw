/* ------------------------------------------------------------------------------
 *	Project	: Smart Home Client
 *	File	:
 *	Date	:
 *	Author	: Sebastian Lesse
 *	Brief	:
 * ------------------------------------------------------------------------------
 */

#define THREAD_DEBUG_MSG				DEBUG_MSG
 
// ---- INCLUDES ----------------------------------------------------------------

#include "shc_project_configuration.h"
#include "shc_common_types.h"
#include "shc_common_string.h"
#include "shc_debug_interface.h"

#include "shc_thread_interface.h"

// ---- LOCAL DEFINITIONS -------------------------------------------------------


// ---- STATIC DATA -------------------------------------------------------------


// ---- IMPLEMENTATION ----------------------------------------------------------

u8 thread_interface_create_thread(THREAD_INTERFACE_TYPE* p_thread) { 
	
	int err = pthread_create(&p_thread->id, NULL, p_thread->run, NULL);
        if (err != 0) {
		THREAD_DEBUG_MSG("thread_interface_create_thread() - Starting thread has FAILED !!! --- (err:%d)\n", err);
		return 0;
	}

	THREAD_DEBUG_MSG("thread_interface_create_thread() - Thread started\n");
	    
	return 0;
}

u8 thread_interface_terminate(THREAD_INTERFACE_TYPE* p_thread) {
	return 0;
}