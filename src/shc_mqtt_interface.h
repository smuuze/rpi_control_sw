/* ------------------------------------------------------------------------------
 *	Project	: Smart Home Client
 *	File	:
 *	Date	:
 *	Author	: Sebastian Lesse
 *	Brief	:
 * ------------------------------------------------------------------------------
 */
 
#ifndef _SHC_MQTT_INTERFACE_H_
#define _SHC_MQTT_INTERFACE_H_

#include "shc_project_configuration.h"
#include "shc_common_types.h"

#include <MQTTClient.h>


//#define MQTT_HOST_ADDR			"tcp://192.168.2.85:1883"
#define MQTT_HOST_ADDR				"tcp://192.168.110.12:1883"
#define MQTT_CLIENT_ID				"Smart_Home_Dev_Client"
#define MQTT_TOPIC				"myflat/bedroom"
#define MQTT_WELCOME_MSG			"Hello Smart Home!"
#define MQTT_QOS					1
#define MQTT_TIMEOUT				1000L

#define MQTT_EXIT_STRING			"cmd_exit"
#define MQTT_EXIT_STRING_LEN			8

#define MQTT_HOST_ADDRESS_STRING_LENGTH		26
#define MQTT_TOPIC_NAME_STRING_LENGTH		64
#define MQTT_CLIENT_ID_STRING_LENGTH		26
#define MQTT_WELCOME_MSG_STRING_LENGTH		26

/*!
 *
 */
typedef struct {

	u8 msg_arrived;
	u8 msg_delivered;
	u8 connection_lost;	
	u16 timeout_ms;
	u8 quality_of_service;
	u8 initialized;
	
	char host_address[MQTT_HOST_ADDRESS_STRING_LENGTH];
	char topic_name[MQTT_TOPIC_NAME_STRING_LENGTH];
	char client_id[MQTT_CLIENT_ID_STRING_LENGTH];
	char welcome_msg[MQTT_WELCOME_MSG_STRING_LENGTH];	
	
	STRING_BUFFER message;
	MQTTClient client;
	
} MQTT_INTERFACE;

/*!
 *
 */
void mqtt_configure(MQTT_INTERFACE* p_mqtt_interface, char* p_host_addr, char* p_topic_name, char* p_client_name);

/*!
 *
 */
u8 mqtt_init(MQTT_INTERFACE* p_mqtt_interface);

/*!
 *
 */
u8 mqtt_connect(MQTT_INTERFACE* p_mqtt_interface);

/*!
 *
 */
u8 mqtt_send_message(MQTT_INTERFACE* p_mqtt_interface, STRING_BUFFER* p_msg_from);

/*!
 *
 */
void mqtt_keep_alive(void);

/*!
 *
 */
void connectionLost_Callback(void *context, char* cause);

/*!
 *
 */
int messageArrived_Callback(void* context, char* topicName, int topcLength, MQTTClient_message* message);

/*!
 *
 */
void deliveryComplete_Callback(void* context, MQTTClient_deliveryToken token);

#define MQTT_INTERFACE_BUILD_HOST(name)												\
	static MQTT_INTERFACE __##name##_mqtt_interface;									\
																\
	void name##_configure(char* p_host_addr, char* p_topic_name, char* p_client_name) {					\
		mqtt_configure(&__##name##_mqtt_interface, p_host_addr, p_topic_name, p_client_name);				\
	}															\
																\
	u8 name##_init(void) {													\
		return mqtt_init(&__##name##_mqtt_interface);									\
	}															\
																\
	u8 name##_connect(void) {												\
		return mqtt_connect(&__##name##_mqtt_interface);								\
	}															\
																\
	u8 name##_send_message(STRING_BUFFER* p_msg_from) {									\
		return mqtt_send_message(&__##name##_mqtt_interface, p_msg_from);						\
	}															\
																\
	u8 name##_delivery_complete(void) {											\
		return __##name##_mqtt_interface.msg_delivered != 0 ? 1 : 0;							\
	}															\
																\
	u8 name##_connection_lost(void) {											\
		return __##name##_mqtt_interface.connection_lost != 0 ? 1 : 0;							\
	}															\
																\
	u8 name##_is_initialized(void) {											\
		return __##name##_mqtt_interface.initialized != 0 ? 1 : 0;							\
	}															\
																\
	u8 name##_disconnect(void) {												\
		return 0;													\
	}

#define MQTT_INTERFACE_INCLUDE_HOST(name)											\
	void name##_configure(char* p_host_addr, char* p_topic_name, char* p_client_name);					\
	u8 name##_init(void);													\
	u8 name##_connect(void);												\
	u8 name##_send_message(STRING_BUFFER* p_msg_from);									\
	u8 name##_delivery_complete(void);											\
	u8 name##_connection_lost(void);											\
	u8 name##_is_initialized(void) 								

#endif // _SHC_MQTT_INTERFACE_H_