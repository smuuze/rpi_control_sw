/* ------------------------------------------------------------------------------
 *	Project	: Smart Home Client
 *	File	:
 *	Date	:
 *	Author	: Sebastian Lesse
 *	Brief	:
 * ------------------------------------------------------------------------------
 */
 
#ifndef _SHC_QEUE_INTERFACE_H_
#define _SHC_QEUE_INTERFACE_H_

#include "shc_project_configuration.h"
#include "shc_common_types.h"
#include "shc_mqtt_interface.h"


/*!
 *
 */
typedef struct {
	STRING_BUFFER msg_list[GENERAL_MAX_NUMBER_OF_MSG_IN_QEUE];
	volatile u8 write_counter;
	volatile u8 read_counter;
	volatile u8 element_counter;
} MSG_QEUE;

#define QEUE_INTERFACE_BUILD_QEUE(name, object_type, max_size)							\
														\
	typedef struct {											\
		object_type list[max_size];									\
		volatile u8 write_counter;									\
		volatile u8 read_counter;									\
		volatile u8 element_counter;									\
	} QEUE_INTERFACE_OBECT_##name;										\
														\
	static QEUE_INTERFACE_OBECT_##name __##name##_qeue;							\
	static u8 __##name##_mutex;										\
														\
	void name##_init(void) {										\
		__##name##_qeue.write_counter = 0;								\
		__##name##_qeue.read_counter = 0;								\
		__##name##_qeue.element_counter = 0;								\
		__##name##_mutex = 0;										\
	}													\
														\
	u8 name##_enqeue(void* p_object) {									\
		/*DEBUG_MSG("QEUE_ADD() - SIZE: %d\n", __##name##_qeue.element_counter);*/			\
														\
		if (__##name##_qeue.element_counter == max_size) {						\
			return 0;										\
		}												\
														\
														\
		memcpy((u8*)&__##name##_qeue.list[__##name##_qeue.write_counter], (u8*)p_object, sizeof(object_type));	\
														\
		__##name##_qeue.write_counter += 1;								\
		__##name##_qeue.element_counter += 1;								\
														\
		if (__##name##_qeue.write_counter == max_size) {						\
			__##name##_qeue.write_counter = 0;							\
		}												\
														\
		return 1;											\
	}													\
														\
	u8 name##_deqeue(void* p_object) {									\
		/*DEBUG_MSG("QEUE_GET() - SIZE: %d\n", __##name##_qeue.element_counter);*/			\
		if (__##name##_qeue.element_counter == 0) {							\
			return 0;										\
		}												\
														\
		memcpy((u8*)p_object, (u8*)&__##name##_qeue.list[__##name##_qeue.read_counter], sizeof(object_type));	\
														\
		__##name##_qeue.read_counter += 1;								\
		__##name##_qeue.element_counter -= 1;								\
														\
		if (__##name##_qeue.read_counter == max_size) {							\
			__##name##_qeue.read_counter = 0;							\
		}												\
														\
		return 1;											\
	}													\
														\
	u8 name##_mutex_get(void) {										\
		/*DEBUG_MSG("MUTEX_GET() - MUTEX: %d\n", __##name##_mutex);*/					\
		if (__##name##_mutex != 0) {									\
			return 0;										\
		}												\
														\
		__##name##_mutex = 1;										\
		return 1;											\
	}													\
														\
	void name##_mutex_release(void) {									\
		/*DEBUG_MSG("MUTEX_RELEASE() - MUTEX: %d\n", __##name##_mutex);	*/				\
		__##name##_mutex = 0;										\
	}													\
														\
	u8 name##_is_empty(void) {										\
		return __##name##_qeue.element_counter == 0 ? 1 : 0;						\
	}													\
														\
	u8 name##_is_full(void) {										\
		return __##name##_qeue.element_counter == max_size ? 1 : 0;					\
	}


#define QEUE_INTERFACE_INCLUDE_QEUE(name)									\
	void name##_init(void);											\
	u8 name##_enqeue(void* p_object);									\
	u8 name##_deqeue(void* p_object);									\
	u8 name##_mutex_get(void);										\
	void name##_mutex_release(void);									\
	u8 name##_is_empty(void);										\
	u8 name##_is_full(void);
/*
 *
 */
void qeue_init(MSG_QEUE* p_qeue);

/*!
 *
 */
u8 qeue_enqeue(MSG_QEUE* p_qeue, MQTTClient_message* p_msg_from);

/*!
 *
 */
u8 qeue_deqeue(MSG_QEUE* p_qeue, STRING_BUFFER* p_msg_to);

/*!
 *
 */
u8 qeue_is_empty(MSG_QEUE* p_qeue);

#endif // _SHC_QEUE_INTERFACE_H_