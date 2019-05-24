/* ------------------------------------------------------------------------------
 *	Project	: Smart Home Client
 *	File	:
 *	Date	:
 *	Author	: Sebastian Lesse
 *	Brief	:
 * ------------------------------------------------------------------------------
 */
 
// ---- INCLUDES ----------------------------------------------------------------

#include "shc_project_configuration.h"
#include "shc_common_types.h"
#include "shc_common_string.h"
#include "shc_debug_interface.h"

#include "shc_mqtt_interface.h"
#include "shc_qeue_interface.h"

// ---- LOCAL DEFINITIONS -------------------------------------------------------


#define MQTT_DEBUG_MSG				DEBUG_MSG

// ---- STATIC DATA -------------------------------------------------------------

extern MSG_QEUE myCommandQeue;

// ---- IMPLEMENTATION ----------------------------------------------------------

u8 mqtt_init(MQTT_INTERFACE* p_mqtt_interface) {

	MQTT_DEBUG_MSG("mqtt_init() - Initialize MQTT-Client\n");
	MQTTClient_create(&p_mqtt_interface->client, p_mqtt_interface->host_address, p_mqtt_interface->client_id, MQTTCLIENT_PERSISTENCE_NONE, NULL);
		
	MQTT_DEBUG_MSG("mqtt_init() - Set MQTT-Callbacks\n");
	MQTTClient_setCallbacks(p_mqtt_interface->client, (void*)p_mqtt_interface, connectionLost_Callback, messageArrived_Callback, deliveryComplete_Callback);	
	
	return NO_ERR;
}

u8 mqtt_connect(MQTT_INTERFACE* p_mqtt_interface) {	

	MQTTClient_connectOptions smartHomeConParam = MQTTClient_connectOptions_initializer;
	smartHomeConParam.keepAliveInterval = 20;
	smartHomeConParam.cleansession = 1;

	u8 err_code = MQTTClient_connect(p_mqtt_interface->client, &smartHomeConParam);
	if (err_code != MQTTCLIENT_SUCCESS) {
		MQTT_DEBUG_MSG("mqtt_connect() - Connect to SmartHomeBroker has FAILED !!! (err_code = %d)\n", err_code);
		return ERR_CONNECT_TIMEOUT;

	} else {		
		MQTT_DEBUG_MSG("mqtt_connect() - Connection to SmartHomeBroker established.\n");
	}
	
	MQTTClient_subscribe(p_mqtt_interface->client, p_mqtt_interface->topic_name, p_mqtt_interface->quality_of_service);
	p_mqtt_interface->connection_lost = 0;
	
	return NO_ERR;

}

u8 mqtt_send_message(MQTT_INTERFACE* p_mqtt_interface, STRING_BUFFER* p_msg_from) {

	MQTTClient_message mqtt_message = MQTTClient_message_initializer;
	mqtt_message.payload = p_msg_from->payload;
	mqtt_message.payloadlen = p_msg_from->length;
	mqtt_message.qos = MQTT_QOS;
	mqtt_message.retained = 0;
	
	MQTTClient_deliveryToken message_token;

	MQTT_DEBUG_MSG("mqtt_send_message() - Sending Message to Broker.\n");
	MQTTClient_publishMessage(p_mqtt_interface->client, p_mqtt_interface->topic_name, &mqtt_message, &message_token);

	MQTT_DEBUG_MSG("mqtt_send_message() - Waiting for completion of message.\n");

	u8 err_code = MQTTClient_waitForCompletion(p_mqtt_interface->client, message_token, p_mqtt_interface->timeout_ms);
	if (err_code != MQTTCLIENT_SUCCESS) {
		MQTT_DEBUG_MSG("mqtt_send_message() - Sending Message to SmartHomeBroker has FAILED !!! (err_code = %d)\n", err_code);
		return ERR_SEND_MSG;
	}
	
	MQTT_DEBUG_MSG("mqtt_send_message() - Delivery of Message succeeds.\n");
	
	return NO_ERR;
}
// -------- MQTT CALLBACKs --------------------------------------------------------------

void connectionLost_Callback(void *context, char* cause) {

	if (context == NULL) {
		return;		
	}
	
	MQTT_INTERFACE* ctx = (MQTT_INTERFACE*) context;
	
	if (ctx->connection_lost != 0) {
		return;		
	}
	
	MQTT_DEBUG_MSG("connectionLost_Callback() - MQTT-Connection has been lost !!! ---\n");
	
	ctx->connection_lost = 1;
}

int messageArrived_Callback(void* context, char* topicName, int topcLength, MQTTClient_message* message) {

	if (context == NULL) {
	
		MQTT_DEBUG_MSG("messageArrived_Callback() - Context is zero !!! ---\n");
		return 1;		
	}
	
	if (memcmp((u8*)message->payload, "cmd", 3) == 0 || memcmp((u8*)message->payload, "exe", 3) == 0) {
		qeue_enqeue(&myCommandQeue, message);
	}	
	
	MQTTClient_free(topicName);
	MQTTClient_freeMessage(&message);
	
	return 1;
}

void deliveryComplete_Callback(void* context, MQTTClient_deliveryToken token) {

	if (context == NULL) {
		return;		
	}

	MQTT_INTERFACE* ctx = (MQTT_INTERFACE*) context;
	
	if (ctx->msg_delivered != 0) {
		return;		
	}
	
	ctx->msg_delivered = 1;
	MQTT_DEBUG_MSG("deliveryComplete_Callback() - Message successful delivered !!! ---\n");
}
