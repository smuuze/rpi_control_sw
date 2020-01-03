/* ------------------------------------------------------------------------------
 *	Project	: Smart Home Client
 *	File	:
 *	Date	:
 *	Author	: Sebastian Lesse
 *	Brief	:
 * ------------------------------------------------------------------------------
 */
 
#ifndef _SHC_THREAD_PARSE_TRACE_OBJECT_H_
#define _SHC_THREAD_PARSE_TRACE_OBJECT_H_

#include "shc_project_configuration.h"
#include "shc_common_types.h"
#include "shc_thread_interface.h"

/*!
 *
 */
void* thread_parse_trace_object_run(void* p_arg);

/*!
 *
 */
void thread_parse_trace_object_terminate(void);

/*!
 *
 */
THREAD_INTERFACE_INCLUDE_THREAD(PARSE_TRACE_OBJECT_THREAD)


#endif // _SHC_THREAD_PARSE_TRACE_OBJECT_H_