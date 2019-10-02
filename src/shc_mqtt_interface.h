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
void connectionLost_Callback(void *context, char* cause);

/*!
 *
 */
int messageArrived_Callback(void* context, char* topicName, int topcLength, MQTTClient_message* message);

/*!
 *
 */
void deliveryComplete_Callback(void* context, MQTTClient_deliveryToken token);


#endif // _SHC_MQTT_INTERFACE_H_