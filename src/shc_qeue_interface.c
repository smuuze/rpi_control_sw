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

#define QEUE_DEBUG_MSG				noDEBUG_MSG

// ---- STATIC DATA -------------------------------------------------------------


// ---- IMPLEMENTATION ----------------------------------------------------------

void qeue_init(MSG_QEUE* p_qeue) {
	p_qeue->write_counter = 0;
	p_qeue->read_counter = 0;
	p_qeue->element_counter = 0;
}

u8 qeue_enqeue(MSG_QEUE* p_qeue, MQTTClient_message* p_msg_from) {

	if (p_qeue->element_counter == GENERAL_MAX_NUMBER_OF_MSG_IN_QEUE) {
		return ERR_QEUE_FULL;
	}
	
	if (p_qeue->write_counter > GENERAL_MAX_NUMBER_OF_MSG_IN_QEUE) {
		return ERR_QEUE_WRITE_FAULT;
	}
	
	memcpy(p_qeue->msg_list[p_qeue->write_counter].payload, (u8*)p_msg_from->payload, p_msg_from->payloadlen);
	p_qeue->write_counter += 1;
	p_qeue->element_counter += 1;
		
	if (p_qeue->write_counter == GENERAL_MAX_NUMBER_OF_MSG_IN_QEUE) {
		p_qeue->write_counter = 0;
	}
	
	QEUE_DEBUG_MSG("- Qeue Elements available : %d\n", p_qeue->element_counter);
	QEUE_DEBUG_MSG("- Qeue Write-Counter      : %d\n", p_qeue->write_counter);
	
	return NO_ERR;
}

u8 qeue_deqeue(MSG_QEUE* p_qeue, STRING_BUFFER* p_msg_to) {

	if (p_qeue->element_counter == 0) {
		return ERR_QEUE_EMPTY;
	}
	
	if (p_qeue->read_counter > GENERAL_MAX_NUMBER_OF_MSG_IN_QEUE) {
		return ERR_QEUE_READ_FAULT;
	}
	
	memcpy(p_msg_to->payload, p_qeue->msg_list[p_qeue->read_counter].payload, GENERAL_STRING_BUFFER_MAX_LENGTH);	
	p_msg_to->length = p_qeue->msg_list[p_qeue->read_counter].length;
	
	p_qeue->read_counter += 1;
	p_qeue->element_counter -= 1;
	
	if (p_qeue->read_counter == GENERAL_MAX_NUMBER_OF_MSG_IN_QEUE) {
		p_qeue->read_counter = 0;
	}
	
	QEUE_DEBUG_MSG("- Qeue Elements left : %d\n", p_qeue->element_counter);
	QEUE_DEBUG_MSG("- Qeue Read-Counter  : %d\n", p_qeue->read_counter);
	
	return NO_ERR;
}

u8 qeue_is_empty(MSG_QEUE* p_qeue) {
	return p_qeue->element_counter == 0 ? 1 : 0;
}