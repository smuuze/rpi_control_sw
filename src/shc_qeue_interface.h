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

/*!
 *
 */
typedef struct {
	STRING_BUFFER msg_list[GENERAL_MAX_NUMBER_OF_MSG_IN_QEUE];
	volatile u8 write_counter;
	volatile u8 read_counter;
	volatile u8 element_counter;
} MSG_QEUE;

/*
 *
 */
static void qeue_init(MSG_QEUE* p_qeue);

/*!
 *
 */
static u8 qeue_enqeue(MSG_QEUE* p_qeue, MQTTClient_message* p_msg_from);

/*!
 *
 */
static u8 qeue_deqeue(MSG_QEUE* p_qeue, STRING_BUFFER* p_msg_to);

/*!
 *
 */
static u8 qeue_is_empty(MSG_QEUE* p_qeue);

#endif _SHC_QEUE_INTERFACE_H_