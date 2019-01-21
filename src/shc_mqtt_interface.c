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

#include <MQTTClient.h>

// ---- LOCAL DEFINITIONS -------------------------------------------------------


// ---- STATIC DATA -------------------------------------------------------------


// ---- IMPLEMENTATION ----------------------------------------------------------

u8 mqtt_init(MQTT_INTERFACE* p_mqtt_interface) {

	MQTT_DEBUG_MSG("--- Initialize MQTT-Client\n1");
	MQTTClient_create(&p_mqtt_interface->client, p_mqtt_interface->host_address, p_mqtt_interface->client_id, MQTTCLIENT_PERSISTENCE_NONE, NULL);
		
	MQTT_DEBUG_MSG("--- Set MQTT-Callbacks\n1");
	MQTTClient_setCallbacks(p_mqtt_interface->client, (void*)p_mqtt_interface, connectionLost_Callback, messageArrived_Callback, deliveryComplete_Callback);	
	
	return NO_ERR;
}

u8 mqtt_connect(MQTT_INTERFACE* p_mqtt_interface) {	

	MQTTClient_connectOptions smartHomeConParam = MQTTClient_connectOptions_initializer;
	smartHomeConParam.keepAliveInterval = 20;
	smartHomeConParam.cleansession = 1;

	u8 err_code = MQTTClient_connect(p_mqtt_interface->client, &smartHomeConParam);
	if (err_code != MQTTCLIENT_SUCCESS) {
		MQTT_DEBUG_MSG("Connect to SmartHomeBroker has FAILED !!! (err_code = %d)\n", err_code);
		return ERR_CONNECT_TIMEOUT;

	} else {		
		MQTT_DEBUG_MSG("- Connection to SmartHomeBroker established.\n");
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

	MQTT_DEBUG_MSG("- Sending Message to Broker.\n");
	MQTTClient_publishMessage(p_mqtt_interface->client, p_mqtt_interface->topic_name, &mqtt_message, &message_token);

	MQTT_DEBUG_MSG("- Waiting for completion of message.\n");

	u8 err_code = MQTTClient_waitForCompletion(p_mqtt_interface->client, message_token, p_mqtt_interface->timeout_ms);
	if (err_code != MQTTCLIENT_SUCCESS) {
		MQTT_DEBUG_MSG("- Sending Message to SmartHomeBroker has FAILED !!! (err_code = %d)\n", err_code);
		return ERR_SEND_MSG;
	}
	
	MQTT_DEBUG_MSG("- Delivery of Message succeeds.\n");
	
	return NO_ERR;
}
