#ifndef _SHC_DEBUG_ITNERFACE_H_
#define _SHC_DEBUG_ITNERFACE_H_

#include "shc_project_configuration.h"

#define ERR_LEVEL_INFO				1
#define ERR_LEVEL_WARNING			2
#define ERR_LEVEL_FATAL				3

//#define DEBUG_MSG(...)
#define DEBUG_MSG(...)				printf(__VA_ARGS__)
#define LOG_MSG(level, p_file, ...)		{								\
							STRING_BUFFER log_msg;					\
							sprintf((char*)log_msg.payload, __VA_ARGS__);		\
							log_msg.length = string_length((char*)log_msg.payload);	\
							log_message(p_file, level, &log_msg);			\
						}

#define noDEBUG_MSG(...)

#define SPI_DEBUG_MSG				DEBUG_MSG
#define MQTT_DEBUG_MSG				noDEBUG_MSG
#define COMMAND_DEBUG_MSG			DEBUG_MSG
#define EVENT_DEBUG_MSG				noDEBUG_MSG
#define REPORT_DEBUG_MSG			noDEBUG_MSG
#define QEUE_DEBUG_MSG				noDEBUG_MSG
#define STRING_DEBUG_MSG			DEBUG_MSG
#define MAIN_DEBUG_MSG				DEBUG_MSG
#define GPIO_DEBUG_MSG				noDEBUG_MSG
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