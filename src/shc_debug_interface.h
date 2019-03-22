/* ------------------------------------------------------------------------------
 *	Project	: Smart Home Client
 *	File	:
 *	Date	:
 *	Author	: Sebastian Lesse
 *	Brief	:
 * ------------------------------------------------------------------------------
 */
 
#ifndef _SHC_DEBUG_ITNERFACE_H_
#define _SHC_DEBUG_ITNERFACE_H_

#include "shc_project_configuration.h"
#include "shc_common_types.h"
#include "shc_debug_interface.h"
#include "shc_file_interface.h"

#define ERR_LEVEL_INFO				1
#define ERR_LEVEL_WARNING			2
#define ERR_LEVEL_FATAL				3

//#define DEBUG_MSG(...)
#ifdef DEBUG_ENABLED
#define DEBUG_MSG(...)				printf(__VA_ARGS__)
#else
#define DEBUG_MSG(...)
#endif

#define noDEBUG_MSG(...)

#define LOG_MSG(level, p_file, ...)		{								\
							STRING_BUFFER log_msg;					\
							sprintf((char*)log_msg.payload, __VA_ARGS__);		\
							log_msg.length = string_length((char*)log_msg.payload);	\
							log_message(p_file, level, &log_msg);			\
						}

#define EVENT_DEBUG_MSG				noDEBUG_MSG
#define REPORT_DEBUG_MSG			noDEBUG_MSG
#define STRING_DEBUG_MSG			DEBUG_MSG
#define LOG_DEBUG_MSG				noDEBUG_MSG
#define CFG_DEBUG_MSG				noDEBUG_MSG

#define SET_MESSAGE(p_sb,p_msg,len_msg)		memcpy((p_sb)->payload, p_msg, len_msg); \
								(p_sb)->length = len_msg
								
#define DEBUG_DISABLE_REPORT_PROCESSING		0	
#define DEBUG_DISABLE_EVENT_PROCESSING		0	
#define DEBUG_DISABLE_COMMAND_PROCESSING	0

#if DEBUG_DISABLE_EVENT_PROCESSING == 1
#pragma WARNING_EVENT_PROCESSING_IS_DISABLED
#endif

/*!
 *
 */
void log_message(FILE_INTERFACE* p_file, u8 error_level, STRING_BUFFER* p_msg_from);

#endif // _SHC_DEBUG_ITNERFACE_H_