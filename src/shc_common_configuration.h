/* ------------------------------------------------------------------------------
 *	Project	: Smart Home Client
 *	File	:
 *	Date	:
 *	Author	: Sebastian Lesse
 *	Brief	:
 * ------------------------------------------------------------------------------
 */
 
#ifndef _SHC_COMMON_CONFIGURATION_H_
#define _SHC_COMMON_CONFIGURATION_H_

#include "shc_project_configuration.h"
#include "shc_file_interface.h"

// -------- Configuration Keynames ------------------------------------------------------

#define CFG_NAME_MQTT_HOST_ADDR			"MQTT_HOST_ADDRESS"
#define CFG_NAME_MQTT_CLINET_ID			"MQTT_CLIENT_ID"
#define CFG_NAME_MQTT_TOPIC_NAME		"MQTT_TOPIC_NAME"
#define CFG_NAME_MQTT_TIMEOUT			"MQTT_TIMEOUT"
#define CFG_NAME_MQTT_WELCOME_MESSAGE		"MQTT_WELCOME_MESSAGE"

#define CFG_NAME_COMMUNICATION_TYPE		"COMMUNICATION_TYPE"
#define CFG_NAME_COM_SPI_BAUDRATE		"COM_SPI_BAUDRATE"
#define CFG_NAME_COM_SPI_DEVICE			"COM_SPI_DEVICE"

#define CFG_NAME_COMMAND_FILE_PATH		"COMMAND_FILE_PATH"
#define CFG_NAME_REPORT_FILE_PATH		"REPORT_FILE_PATH"
#define CFG_NAME_EVENT_FILE_PATH		"EVENT_FILE_PATH"
#define CFG_NAME_EXECUTION_FILE_PATH		"EXECUTION_FILE_PATH"
#define CFG_NAME_LOG_FILE_PATH			"LOG_FILE_PATH"				   

#define CFG_NAME_SCHEDULE_INTERVAL_REPORT_MS	"SCHEDULE_INTERVAL_REPORT_MS"
#define CFG_NAME_SCHEDULE_INTERVAL_EVENT_MS	"SCHEDULE_INTERVAL_EVENT_MS"
#define CFG_NAME_SCHEDULE_INTERVAL_CONFIG_MS	"SCHEDULE_INTERVAL_CONFIG_MS"

/*
 *
 */
typedef struct {
	FILE_INTERFACE cfg_file;
	FILE_INTERFACE log_file;
} CFG_INTERFACE;

#endif