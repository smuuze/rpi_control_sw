
// -------- INCLUDES --------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>

#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <sys/ioctl.h>
#include <linux/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

#include <time.h>

#include <MQTTClient.h>

// -------- DEFINITIONS -----------------------------------------------------------------

#define V_MAJOR					3
#define V_MINOR					0

#define GENERAL_STRING_BUFFER_MAX_LENGTH	100
#define GENERAL_MAX_NUMBER_OF_MSG_IN_QEUE	10

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

#define COM_DEVICE_NAME_STRING_LENGTH		15

#define COMMAND_LINE_ARGUMENT_CFG_FILE		"-cfg"

#define FILE_PATH_MAX_STRING_LENGTH		64

#define CONFIGURATION_FILE_PATH			"smart_home_configuration_file.txt"
#define COMMAND_FILE_PATH			"smart_home_command_file.txt"
#define REPORT_FILE_PATH			"smart_home_report_file.txt"

#define CMD_RX_ANSWER_TIMEOUT_MS		1000
#define CMD_TX_COMMAND_TIMEOUT_MS		1000
#define CMD_ACTIVATE_TIMEOUT_MS			250

#define CMD_DEFAULT_RESEND_COUNTER		3

// -------- SCHEDULE INTERVAL DEFAULTS -------------------------------------------------------------------

#define REPORT_SCHEDULE_DEFAULT_INTERVAL_MS		60000
#define EVENT_SCHEDULE_DEFAULT_INTERVAL_MS		1500
#define CONFIGURATION_SCHEDULE_DEFAULT_INTERVAL_MS	60000

// -------- DEBUGGING -------------------------------------------------------------------

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

// -------- Command-Code ----------------------------------------------------------------
								
#define CMD_VERSION_STR				"version"
#define CMD_VERSION_LEN				7								

// -------- Error-Code ------------------------------------------------------------------
								
#define NO_ERR					0
#define ERR_INVALID_ARGUMENT			1
#define ERR_NOT_INITIALIZED			2
#define ERR_SEND_MSG				3
#define ERR_BAD_CMD				4
#define ERR_ANSWER_LENGTH			5
#define ERR_COMMUNICATION			6
#define ERR_FILE_OPEN				7
#define ERR_END_OF_FILE				8
#define ERR_CONNECT_TIMEOUT			9
#define ERR_QEUE_FULL				10
#define ERR_QEUE_EMPTY				10
#define ERR_QEUE_WRITE_FAULT			11
#define ERR_QEUE_READ_FAULT			12
#define ERR_NOT_EQUAL				13
#define ERR_GPIO				14
#define ERR_TIMEOUT				15
#define ERR_WRITE_FILE				16


// -------- GPIO-INTERFACE ------------------------------------------------------------

#define GPIO_ON					1
#define GPIO_OFF				0
#define GPIO_HIGH_Z				2

#define GPIO_INPUT				1
#define GPIO_OUTPUT				0

#define MY_GPIO_INTERFACE_WIRINGPI		0
#define MY_GPIO_INTERFACE_PIGPIO		1
#define MY_GPIO_INTERFACE 			MY_GPIO_INTERFACE_WIRINGPI

#if MY_GPIO_INTERFACE == MY_GPIO_INTERFACE_WIRINGPI

#include <wiringPi.h>

#define GPIO_IS_BUSY_PIN_NUM			0
#define GPIO_SAMPLE_PIN				29
#define GPIO_EVENT_PIN				3

#define GPIO_SPI_CS0_PIN_NUM			10
#define GPIO_SPI_SCK_PIN_NUM			14
#define GPIO_SPI_MOSI_PIN_NUM			12
#define GPIO_SPI_MISO_PIN_NUM			13

#define GPIO_READ_PIN(pin_num)			digitalRead(pin_num)
#define GPIO_WRITE_PIN(pin_num, state)		digitalWrite(pin_num, state)
#define GPIO_INITIALIZE()			wiringPiSetup()
#define GPIO_CONFIGURE_PIN(pin_num, is_input)	pinMode(pin_num, (is_input == GPIO_INPUT) ? INPUT : OUTPUT)
#define GPIO_PULL_UP_DOWN(pin_num, pull_down)	pullUpDnControl(pin_num, (pull_down == GPIO_OFF) ? PUD_DOWN : (pull_down == GPIO_ON) ? PUD_UP : PUD_OFF )

#define GPIO_OUTPUT_OFF				LOW
#define GPIO_OUTPUT_ON				HIGH

#else

#include <pigpio.h>
#define GPIO_IS_BUSY_PIN_NUM			17
#define GPIO_SAMPLE_PIN				26
#define GPIO_CONFIGURE_PIN(pin_num, is_input)	gpioSetMode(pin_num, is_input ? PI_INPUT : PI_OUTPUT)
#define GPIO_READ_PIN(pin_num)			gpioRead(pin_num)
#define GPIO_INITIALIZE()			gpioInitialise()

#endif			

#define IS_EXECUTION_COMMAND(msg)		(memcmp(msg.payload, "exe", 3) == 0 ? 1 : 0)
#define IS_COMMUNICATION_COMMAND(msg)		(memcmp(msg.payload, "cmd", 3) == 0 ? 1 : 0)					 		


// -------- TYPE-DEFINITIONS ------------------------------------------------------------


typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;

/*!
 *
 */
typedef struct {
	u8 length;
	u8 payload[GENERAL_STRING_BUFFER_MAX_LENGTH]; 
} STRING_BUFFER;

/*!
 *
 */
typedef struct {
	char path[FILE_PATH_MAX_STRING_LENGTH];
	FILE* handle;
	u32 last_file_pointer;
	u32 act_file_pointer;
	u32 timestamp_last_modified;
} FILE_INTERFACE;

/*!
 *
 */
typedef struct {
	STRING_BUFFER msg_list[GENERAL_MAX_NUMBER_OF_MSG_IN_QEUE];
	volatile u8 write_counter;
	volatile u8 read_counter;
	volatile u8 element_counter;
} MSG_QEUE;

/*!
 *
 */
typedef struct {
	u8 pin_num;
	u8 is_initialized;
	u8 is_input;
	u8 is_high_level;
	u8 match_event_level;
	u8 event_rised;
	u32 sample_time_reference;
	u32 sample_timeout;
	u32 event_ref_time;
	u32 event_timeout;
} GPIO_INTERFACE;

/*!
 *
 */
typedef struct {

	u8 msg_arrived;
	u8 msg_delivered;
	u8 connection_lost;	
	u16 timeout_ms;
	u8 quality_of_service;
	
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
typedef struct {
	u32 _handle_id;
	char device[COM_DEVICE_NAME_STRING_LENGTH];
	u32 speed_hz;
	u8 bits_per_word;
	u16 delay;
	u32 mode;
} SPI_INTERFACE;

/*!
 *
 */
typedef enum {
	SPI = 0x00,
	I2C,
	USART
} COM_INTERFACE_TYPE;

/*!
 *
 */
typedef struct {
	
	COM_INTERFACE_TYPE type;
	union {
		SPI_INTERFACE spi;
	} data;
	
} COM_INTERFACE;

/*
 *
 */
typedef struct {
	u32 reference;
	u32 interval;
} SCHEDULING_TIME_VALUE;

/*
 *
 */
typedef struct {
	SCHEDULING_TIME_VALUE event;
	SCHEDULING_TIME_VALUE configuration;
	SCHEDULING_TIME_VALUE report;
} SCHEDULING_INTERFACE;

/*
 *
 */
typedef struct {
	
	u8 is_active;
	
	STRING_BUFFER message;
	STRING_BUFFER command;
	STRING_BUFFER answer;
	
	FILE_INTERFACE command_file;
	FILE_INTERFACE report_file;
	FILE_INTERFACE event_file;
	FILE_INTERFACE execution_file;
	
	char command_file_path[FILE_PATH_MAX_STRING_LENGTH];
	char report_file_path[FILE_PATH_MAX_STRING_LENGTH];
	char event_file_path[FILE_PATH_MAX_STRING_LENGTH];
} COMMAND_INTERFACE;

/*
 *
 */
typedef struct {
	FILE_INTERFACE cfg_file;
	FILE_INTERFACE log_file;
} CFG_INTERFACE;


// -------- STATIC FUNCTION PROTOTYPES --------------------------------------------------

/*
 *
 */
static u32 mstime_get_time(void) {
	struct timespec time_spec;
	
	if (clock_gettime(CLOCK_MONOTONIC, &time_spec) == 0) {
		return ((u32)(time_spec.tv_sec * 1e3) + (u32)(time_spec.tv_nsec / 1e6)); //time_spec.tv_nsec / 1000 / 1000;
	} else {
		return 0;
	}
}

/*
 *
 */
static u8 mstime_is_time_up(u32 reference_time, u32 interval_time) {	
	return (mstime_get_time() - reference_time) > interval_time ? 1 : 0;
}

/*
 *
 */
static void hex_dump(const void *src, size_t length, size_t line_size, char *prefix) {

	//#if STRING_DEBUG_MSG != noDEBUG_MSG	
	
	int i = 0;
	const unsigned char *address = src;
	const unsigned char *line = address;

	STRING_DEBUG_MSG("%s | ", prefix);
	
	while (length-- > 0) {
	
		STRING_DEBUG_MSG("%02X ", *address++);		
		if (!(++i % line_size) || (length == 0 && i % line_size)) {

			if (length == 0) {			
				while (i++ % line_size) {
					STRING_DEBUG_MSG("__ ");
				}
			}
			
			STRING_DEBUG_MSG(" | ");  /* right close */			
			while (line < address) {
				unsigned char c = *line++;
				STRING_DEBUG_MSG("%c", (c < 33 || c == 255) ? 0x2E : c);
			}
			
			STRING_DEBUG_MSG("\n");			
			if (length > 0) {
				STRING_DEBUG_MSG("%s | ", prefix);
			}
		}
	}
	
	/*
	#else
	
	(void ) src;
	(void ) length;
	(void ) line_size;
	(void ) prefix;
	
	#endif
	*/
}

/*!
 * returns number of bytes read
 */
static u16 read_line(FILE* file_handle, char* p_buffer_to, u16 num_max_bytes) {
	
	if (num_max_bytes == 0) {
		return 0;
	}
	
	char character;
	u16 num_bytes_read = 0;
	
	//STRING_DEBUG_MSG("--- Line: \n");
	
	while ((character = getc(file_handle)) != 255) {
		
		if (num_bytes_read == num_max_bytes - 1) {
			break;
		}
		
		if (character == '\n') {
			//STRING_DEBUG_MSG("----> End of line reached (LF)\n");
			break;
		}	
		
		if ((character  < 32 || character > 254)) {		
			//STRING_DEBUG_MSG("----> Character is not supported (%d)\n", character);
			continue;
		}
		
		//STRING_DEBUG_MSG("%d ", character);
		
		p_buffer_to[num_bytes_read] = character;
		num_bytes_read += 1;
	}
	
	//STRING_DEBUG_MSG("\n");
	
	p_buffer_to[num_bytes_read + 1] = '\0';
	
	return num_bytes_read;
}

//static u8 write_line(FILE* file_handle, char* p_buffer_from, u16 num_max_bytes) {
//
//	fputs(p_buffer_from, file_handle);
//	return ERR_WRITE_FILE;
//}

/*
 *
 */
static void split_string(char splitter, char* p_string_in, u16 string_in_len, char* p_string_out_1, u16 string_out_1_max_len, char* p_string_out_2, u16 string_out_2_max_len) {

	u8 i = 0;
	u8 j = 0;
	u8 splitter_detected = 0;
	
	for ( ; i < string_in_len; i++) {	
	
		if (p_string_in[i] == '\0') {
			break;
		}
	
		if (splitter_detected == 0 && p_string_in[i] == splitter) {
			p_string_out_1[j] = '\0';
			j = 0;
			splitter_detected = 1;
			continue;
		}
	
		if (splitter_detected != 0) {
		
			if (j < string_out_2_max_len - 1) {
				p_string_out_2[j] = p_string_in[i];
				j++;
			}
			
		} else {
		
			if (j < string_out_1_max_len - 1) {
				p_string_out_1[j] = p_string_in[i];
				j++;
			}			
		}
	}	
		
	p_string_out_2[j] = '\0';
}

/*
 *
 */
static u16 string_length(char* p_str) {
	
	return strlen(p_str);
	
	/*
	u16 i = 0;
	while (p_str[i] != '\0') {
		i++;
	}	
	return i;
	*/
}

static u8 hex_string_to_byte_array(char* hex_string, u16 hex_string_len, u8* byte_array, u8 byte_array_max_len) {
	
	if (hex_string_len < 2) {
		return 0;
	}
	
	u16 i = 0;
	u8 j = 0;
	u8 is_upper_nibble = 1;
	
	memset(byte_array, 0x00, byte_array_max_len);
	
	while (i < hex_string_len && j < byte_array_max_len) {
		
		char nibble = hex_string[i++];
		
		if (nibble == '\0') {
			//STRING_DEBUG_MSG("---> End of string reached\n");
			break;
		}
		
		u8 nibble_value = 0;
		u8 nibble_factor = is_upper_nibble != 0 ? 16 : 1;
		
		if (nibble > 47 && nibble < 57) {
			// is between 0 and 9
			nibble_value = nibble - '0';
			
		} else if (nibble > 64 && nibble < 91) {		
			// is between A and Z
			nibble_value = nibble - 'A' + 10;
			
		} else if (nibble > 97 && nibble < 122) {
			// is between a and z
			nibble_value = nibble - 'a' + 10;
			
		} else {
			//STRING_DEBUG_MSG("---> Character is not valid for HEX-String: %c\n", nibble);
			continue;
		}
		
		byte_array[j] += (nibble_value * nibble_factor);
		
		if (is_upper_nibble == 0) {
			is_upper_nibble = 1;
			j++;
		} else {
			is_upper_nibble = 0;
		}
	}
	
	return j;
}

static u8 byte_array_string_to_hex_string(u8* byte_array, u8 byte_array_len, char* hex_string, u16 hex_string_max_len) {
	
	if (hex_string_max_len < 2 || byte_array_len == 0) {
		return 0;
	}
	
	u16 i = 0;
	u8 j = 0;
	
	memset(hex_string, 0x00, hex_string_max_len);
	
	while (i < byte_array_len && j < hex_string_max_len - 2) {
		
		u8 nibble = byte_array[i] >> 4;
		char character = '0';
		
		if (nibble > 9) {
			// is between A and F
			character = nibble + 'A' - 10;
			
		} else {
			// is between 0 and 9
			character = nibble + '0';
		}
		
		hex_string[j++] = character;
		
		nibble = (byte_array[i] & 0x0F);
		
		if (nibble > 9) {
			// is between A and F
			character = nibble + 'A' - 10;
			
		} else {
			// is between 0 and 9
			character = nibble + '0';
		}
		
		hex_string[j++] = character;
		
		i++;
	}
	
	return j;
}

/*!
 *
 */
static void background_work(void) {
	
}

/*!
 *
 */
static u8 command_line_parser(int argc, char* argv[], CFG_INTERFACE* p_cfg_interface, COM_INTERFACE* p_com_interface, MQTT_INTERFACE* p_mqtt_interface, COMMAND_INTERFACE* p_cmd_interface, GPIO_INTERFACE* p_gpio_interface, SCHEDULING_INTERFACE* p_scheduling_interface);

/*!
 *
 */
static void command_line_usage(void);

/*!
 *
 */
static void restore_last_file_pointer(FILE_INTERFACE* p_file);

/*!
 *
 */
static u8 cmd_handler_prepare_command_from_file(COMMAND_INTERFACE* p_cmd, FILE_INTERFACE* p_file);

/*!
 *
 */
static u8 cmd_handler_match_event_answer(COMMAND_INTERFACE* p_cmd, COMMAND_INTERFACE* p_cmd_match);

/*!
 *
 */
static u8 cmd_handler_prepare_report_message(COMMAND_INTERFACE* p_cmd, u8 err_code);

/*!
 *
 */
static u8 cmd_handler_prepare_command(COMMAND_INTERFACE* p_cmd);

/*!
 *
 */
static u8 cmd_handler_prepare_execution(COMMAND_INTERFACE* p_cmd);

/*!
 *
 */
static u8 cmd_handler_run_execution(COMMAND_INTERFACE* p_cmd);

/*!
 *
 */
static u8 cmd_handler_send_command(COMMAND_INTERFACE* p_cmd, COM_INTERFACE* p_com, GPIO_INTERFACE* p_gpio);

/*!
 *
 */
static u8 cmd_handler_receive_answer(COMMAND_INTERFACE* p_cmd, COM_INTERFACE* p_com, GPIO_INTERFACE* p_gpio, u32 timeout_ms);

/*!
 *
 */
static u8 cmd_handler_get_error_code(COMMAND_INTERFACE* p_cmd);

/*!
 *
 */
static u8 mqtt_init(MQTT_INTERFACE* p_mqtt_interface);

/*!
 *
 */
static u8 mqtt_connect(MQTT_INTERFACE* p_mqtt_interface);

/*!
 *
 */
static u8 mqtt_send_message(MQTT_INTERFACE* p_mqtt_interface, STRING_BUFFER* p_msg_from);

/*!
 *
 */
static void connectionLost_Callback(void *context, char* cause);

/*!
 *
 */
static int messageArrived_Callback(void* context, char* topicName, int topcLength, MQTTClient_message* message);

/*!
 *
 */
static void deliveryComplete_Callback(void* context, MQTTClient_deliveryToken token);

/*!
 *
 */
static void spi_init(SPI_INTERFACE* p_spi_handle);

/*!
 *
 */
static u8 spi_transfer(SPI_INTERFACE* p_spi_handle, size_t num_bytes, const u8* p_buffer_from, u8* p_buffer_to);

/*!
 *
 */
static void spi_deinit(SPI_INTERFACE* p_spi_handle);

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

/*!
 *
 */
static u8 gpio_initialize(GPIO_INTERFACE* p_gpio);

/*!
 *
 */
static u8 gpio_is_event(GPIO_INTERFACE* p_gpio);

/*!
 *
 */
static void gpio_reset_pin(GPIO_INTERFACE* p_gpio);

/*!
 *
 */
static void gpio_set_state(GPIO_INTERFACE* p_gpio, u8 state);

/*!
 *
 */
static u8 file_has_changed(FILE_INTERFACE* p_file);

/*!
 *
 */
static u8 file_is_existing(FILE_INTERFACE* p_file);

/*!
 *
 */
static void log_message(FILE_INTERFACE* p_file, u8 error_level, STRING_BUFFER* p_msg_from);


// -------- STATIC DATA -----------------------------------------------------------------

/*!
 *
 */
static MSG_QEUE myCommandQeue;


// -------- MAIN ------------------------------------------------------------------------

int main(int argc, char* argv[]) {

	MAIN_DEBUG_MSG("Welcome to the SmartHomeClient v%d.%d", V_MAJOR, V_MINOR);
	
	qeue_init(&myCommandQeue);
	
	MQTT_INTERFACE myMqttInterface;
	COM_INTERFACE myComInterface;
	COMMAND_INTERFACE myCmdInterface;
	CFG_INTERFACE myCfgInterface;
	GPIO_INTERFACE myGpioInterface;
	SCHEDULING_INTERFACE mySchedulingInterface;
	
	#if DEBUG_DISABLE_REPORT_PROCESSING == 1
	MAIN_DEBUG_MSG("RPORT PROCESSING IS DISABLED !!!\n");	
	#endif
	
	#if DEBUG_DISABLE_EVENT_PROCESSING == 1
	MAIN_DEBUG_MSG("EVENT PROCESSING IS DISABLED !!!\n");	
	#endif

	MAIN_DEBUG_MSG("\n");	
	MAIN_DEBUG_MSG("---- Loading Configuration ---- \n");
	
	// --- Parsing Command-Line Arguments
	u8 err_code = command_line_parser(argc, argv, &myCfgInterface, &myComInterface, &myMqttInterface, &myCmdInterface, &myGpioInterface, &mySchedulingInterface);
	if (err_code != NO_ERR) {
		command_line_usage();
		return err_code;
	}
	
	LOG_MSG(NO_ERR, &myCfgInterface.log_file, "Starting SmartHomeClient Deamon v%d.%d", V_MAJOR, V_MINOR);
	
	MAIN_DEBUG_MSG("\n");	
	MAIN_DEBUG_MSG("---- Initialize GPIO-Interface---- \n");	
	LOG_MSG(NO_ERR, &myCfgInterface.log_file, "Initialize GPIO-Interface");
	
	gpio_initialize(&myGpioInterface);
				
	GPIO_INTERFACE is_busy_pin = {
		GPIO_IS_BUSY_PIN_NUM, //u8 pin_num ;
		0,  // u8 is_initialized;
		1, // u8 is_input;
		GPIO_OFF, //u8 is_high_level;
		1, //u8 match_event_level;
		0, //u8 event_rised;
		0, //u32 sample_time_reference;
		5, // u32 sample_timeout;
		0, //u32 event_ref_time;
		0, //u32 event_timeout;
	};
	
	err_code = gpio_initialize(&is_busy_pin);
	if (err_code != NO_ERR ) {
		LOG_MSG(ERR_LEVEL_FATAL, &myCfgInterface.log_file, "- Initializing Busy-Pin has FAILED !!! --- (error-code = %d)", err_code);
		return err_code;
	}

	MAIN_DEBUG_MSG("\n");	
	MAIN_DEBUG_MSG("---- Initialize COM-Interface---- \n");
	LOG_MSG(NO_ERR, &myCfgInterface.log_file, "Initialize Communication-Interface");
	
	spi_init(&myComInterface.data.spi);
	
	SET_MESSAGE(&myCmdInterface.message, CMD_VERSION_STR, CMD_VERSION_LEN);	
	
	if (cmd_handler_prepare_command(&myCmdInterface) != NO_ERR) {
		myCmdInterface.is_active = 0;
		
	} else if (cmd_handler_send_command(&myCmdInterface, &myComInterface, &is_busy_pin) != NO_ERR) {
		myCmdInterface.is_active = 0;
		
	} else if (cmd_handler_receive_answer(&myCmdInterface, &myComInterface, &is_busy_pin, CMD_RX_ANSWER_TIMEOUT_MS) != NO_ERR) {
		myCmdInterface.is_active = 0;
	}	
	
	char welcome_message[128];
	sprintf(welcome_message, "%s%s_v%d.%d", myMqttInterface.welcome_msg, myMqttInterface.client_id, myCmdInterface.answer.payload[3], myCmdInterface.answer.payload[2]);
	
	MAIN_DEBUG_MSG("%s\n", welcome_message);
	
	while (1) {

		// --- Initialize MQTT Interface
		MAIN_DEBUG_MSG("---- Initialize MQTT-Interface ---- \n");
		MAIN_DEBUG_MSG("- Host-Addr: \"%s\"\n", myMqttInterface.host_address);
		MAIN_DEBUG_MSG("- Client-ID: \"%s\"\n", myMqttInterface.client_id);
		
		LOG_MSG(NO_ERR, &myCfgInterface.log_file, "Initialize MQTT-Interface (Host-Addr: %s / Client-ID: %s)", myMqttInterface.host_address, myMqttInterface.client_id);
		SET_MESSAGE(&myMqttInterface.message, welcome_message, string_length(welcome_message));
		
		myMqttInterface.connection_lost = 1;
		
		if ((err_code = mqtt_init(&myMqttInterface)) != NO_ERR) {
		
			LOG_MSG(ERR_LEVEL_FATAL, &myCfgInterface.log_file, "Initializing MQTT-Client has FAILED !!! --- (error-code = %d)", err_code);
			
		} else 	if ((err_code = mqtt_connect(&myMqttInterface)) != NO_ERR) {
		
			LOG_MSG(ERR_LEVEL_FATAL, &myCfgInterface.log_file, "Connect to MQTT-Host has FAILED !!! --- (error-code = %d)", err_code);
			
		} else if ((err_code = mqtt_send_message(&myMqttInterface, &myMqttInterface.message)) != NO_ERR) {
		
			LOG_MSG(ERR_LEVEL_FATAL, &myCfgInterface.log_file, "Sending MQTT-Welcome-Message has FAILED !!! --- (error-code = %d)", err_code);
			
		} else {
		
			LOG_MSG(ERR_LEVEL_INFO, &myCfgInterface.log_file, "Connection to MQTT-Broker has established");
			
			myMqttInterface.connection_lost = 0;
			
			mySchedulingInterface.event.reference = mstime_get_time();
			mySchedulingInterface.report.reference = mstime_get_time();
			mySchedulingInterface.configuration.reference = mstime_get_time();
		}		

		while (myMqttInterface.connection_lost == 0) {
			
			usleep(50000);
			
			if (mstime_is_time_up(mySchedulingInterface.configuration.reference, mySchedulingInterface.configuration.interval) != 0) {
				mySchedulingInterface.configuration.reference = mstime_get_time();
				
				if (file_has_changed(&myCfgInterface.cfg_file) != 0) {
				
					MAIN_DEBUG_MSG("---- Configuration File has changed ----\n");
					
					err_code = command_line_parser(0, NULL, &myCfgInterface, &myComInterface, &myMqttInterface, &myCmdInterface, &myGpioInterface, &mySchedulingInterface);
					if (err_code != NO_ERR) {
						LOG_MSG(ERR_LEVEL_FATAL, &myCfgInterface.log_file, "- Reload of configuration has FAILED !!! --- (error-code = %d)", err_code);
					}
				}
			}
			
			// --- Command Handling ---
			if (qeue_is_empty(&myCommandQeue) == 0) {
			
				// only if a command-message has arrived
				err_code = qeue_deqeue(&myCommandQeue, &myCmdInterface.message);
				if (err_code != NO_ERR) {
					MAIN_DEBUG_MSG("ERROR, deqeue element has FAILED !!!  (err_code = %d) ------\n", err_code);
				}
				
				if (myCmdInterface.message.length == MQTT_EXIT_STRING_LEN && memcmp((const void*)myCmdInterface.message.payload, MQTT_EXIT_STRING, MQTT_EXIT_STRING_LEN) == 0) {
					LOG_MSG(ERR_LEVEL_INFO, &myCfgInterface.log_file, "Exit Message received - will quit program");
					break;
				}
				
				//mqtt_prepare_message(&myMqttInterface.message, &myCmdInterface.message);
				
				MAIN_DEBUG_MSG("---- Command Handling ---- (Message : %s)\n", myCmdInterface.message.payload);
				if (IS_EXECUTION_COMMAND(myCmdInterface.message)) {
					
					if ((err_code = cmd_handler_prepare_execution(&myCmdInterface)) != NO_ERR) {
						LOG_MSG(ERR_LEVEL_WARNING, &myCfgInterface.log_file, "Prepare Exec-Command has FAILED !!! --- (Exec:%s / Err:%d)", myCmdInterface.message.payload, err_code);
						
					} else if ((err_code = cmd_handler_run_execution(&myCmdInterface)) != NO_ERR) {
						LOG_MSG(ERR_LEVEL_WARNING, &myCfgInterface.log_file, "Executeion of Exec-Command has FAILED !!! --- (Exec:%s / Err:%d)", myCmdInterface.message.payload, err_code);
					}
					
				} else if (IS_COMMUNICATION_COMMAND(myCmdInterface.message)) {
				
					if (cmd_handler_prepare_command(&myCmdInterface) == NO_ERR) {
					
						u8 cmd_counter = 0;
						
						do {
							if ((err_code = cmd_handler_send_command(&myCmdInterface, &myComInterface, &is_busy_pin)) != NO_ERR) {
								LOG_MSG(ERR_LEVEL_WARNING, &myCfgInterface.log_file, "Prepare Com-Command has FAILED !!! --- (Com:%s / Err:%d)", myCmdInterface.message.payload, err_code);
												
							} else if ((err_code = cmd_handler_receive_answer(&myCmdInterface, &myComInterface, &is_busy_pin, CMD_RX_ANSWER_TIMEOUT_MS)) != NO_ERR) {
								LOG_MSG(ERR_LEVEL_WARNING, &myCfgInterface.log_file, "Executeion of Com-Command has FAILED !!! --- (Com:%s / Err:%d)", myCmdInterface.message.payload, err_code);
							
							} else if ((err_code = cmd_handler_get_error_code(&myCmdInterface)) != NO_ERR) {
								LOG_MSG(ERR_LEVEL_WARNING, &myCfgInterface.log_file, "Unexpected Status-Code --- (Com:%s / Err:%d)", myCmdInterface.message.payload, err_code);
							
							}
							
							if (++cmd_counter == CMD_DEFAULT_RESEND_COUNTER) {
								break;
							}
							
						} while (err_code != NO_ERR);				
					}
				}
			}
			
			// --- Report Handling ---
			if (myCmdInterface.is_active != 0
				&& myMqttInterface.msg_delivered != 0
				&& mstime_is_time_up(mySchedulingInterface.report.reference, mySchedulingInterface.report.interval) != 0) {
			
				MAIN_DEBUG_MSG("---- Report Handling ---- (Time : %d) \n", mstime_get_time());
				
				while ((err_code = cmd_handler_prepare_command_from_file(&myCmdInterface, &myCmdInterface.report_file)) == NO_ERR) {	

					#if DEBUG_DISABLE_REPORT_PROCESSING == 1
					err_code = ERR_END_OF_FILE;
					break;
					#endif
								
					memcpy(myCmdInterface.message.payload + myCmdInterface.message.length, "=", 1);
					myCmdInterface.message.length += 1;
					
					err_code = cmd_handler_send_command(&myCmdInterface, &myComInterface, &is_busy_pin);
					if (err_code != NO_ERR) {
						LOG_MSG(ERR_LEVEL_WARNING, &myCfgInterface.log_file, "- Sending Report-Command has FAILED !!! --- (error-code = %d / Command: %s)", err_code, (char*)myCmdInterface.message.payload);
						restore_last_file_pointer(&myCmdInterface.report_file);
						break;
					}		
					
					err_code = cmd_handler_receive_answer(&myCmdInterface, &myComInterface, &is_busy_pin, CMD_RX_ANSWER_TIMEOUT_MS);
					if (err_code != NO_ERR) {
						LOG_MSG(ERR_LEVEL_WARNING, &myCfgInterface.log_file, "- Receive Report-Answer has FAILED !!! --- (error-code = %d / Command: %s)", err_code, (char*)myCmdInterface.message.payload);
						restore_last_file_pointer(&myCmdInterface.report_file);
						break;
					}
					
					err_code = cmd_handler_get_error_code(&myCmdInterface);
					if (err_code != NO_ERR) {
						LOG_MSG(ERR_LEVEL_WARNING, &myCfgInterface.log_file, "- Status of Report-Answer unexpected --- (status-code = %d / Command: %s)", err_code, (char*)myCmdInterface.message.payload);
						restore_last_file_pointer(&myCmdInterface.report_file);
						DEBUG_MSG("-- Incorrect Status-Code --- (ERR: %d)\n", err_code);
						break;
					}
					
					err_code = cmd_handler_prepare_report_message(&myCmdInterface, err_code);
					if (err_code != NO_ERR) {
						LOG_MSG(ERR_LEVEL_WARNING, &myCfgInterface.log_file, "- Prepare Report-Message has FAILED !!! --- (error-code = %d / Command: %s)", err_code, (char*)myCmdInterface.message.payload);
						continue;
					}
					
					MAIN_DEBUG_MSG("Report-Answer : %s\n", myCmdInterface.message.payload);
					
					err_code = mqtt_send_message(&myMqttInterface, &myCmdInterface.message);
					if (err_code != NO_ERR) {
						LOG_MSG(ERR_LEVEL_FATAL, &myCfgInterface.log_file, "- Sending MQTT-Report-Message has FAILED !!! --- (error-code = %d / Command: %s)", err_code, (char*)myCmdInterface.message.payload);
						//return err_code;
					}
					
					if (gpio_is_event(&myGpioInterface) != 0) {
						break;
					}
					
					if (qeue_is_empty(&myCommandQeue) == 0) {
						break;
					}
				}
				
				if (err_code == ERR_END_OF_FILE) {
					mySchedulingInterface.report.reference = mstime_get_time();
				}
			}
			
			// --- Event Handling ---
			#if DEBUG_DISABLE_EVENT_PROCESSING == 0
			if (myCmdInterface.is_active != 0
				&& gpio_is_event(&myGpioInterface) != 0
				&& mstime_is_time_up(mySchedulingInterface.event.reference, mySchedulingInterface.event.interval) != 0) {
			
				while (myMqttInterface.msg_delivered == 0 ) {
					MAIN_DEBUG_MSG(".");
					usleep(5000);
				}
				
				gpio_reset_pin(&myGpioInterface);
				myGpioInterface.match_event_level = 1;
			
				mySchedulingInterface.event.reference = mstime_get_time();
				MAIN_DEBUG_MSG("---- Event Handling ---- (Time : %d)\n", mySchedulingInterface.event.reference);
				
				// holds the command and message to perform event matching
				COMMAND_INTERFACE cmd_match;
			
				while (cmd_handler_prepare_command_from_file(&myCmdInterface, &myCmdInterface.event_file) == NO_ERR) {
								
					// if command was send in last iteration we do not need to send it once again
					// just go directly to matching the answer
					// since this is not a time-critical application,
					// we can live with missed events between to event-handling-iterations
					
					if (memcmp(cmd_match.command.payload, myCmdInterface.command.payload, myCmdInterface.command.length) == 0) {
						EVENT_DEBUG_MSG("--- Same Event Command - Will directly match the answer --- \n");
						goto EVENT_HANDLING_MATCH_EVENT_ASNWER;
					}
					
					memcpy(cmd_match.command.payload, myCmdInterface.command.payload, GENERAL_STRING_BUFFER_MAX_LENGTH);
					cmd_match.command.length = myCmdInterface.command.length;
					
					err_code = cmd_handler_send_command(&cmd_match, &myComInterface, &is_busy_pin);
					if (err_code != NO_ERR) {
						LOG_MSG(ERR_LEVEL_WARNING, &myCfgInterface.log_file, "- Sending Event-Command has FAILED !!! --- (error-code = %d / Command: %s)", err_code, (char*)myCmdInterface.message.payload);
						restore_last_file_pointer(&myCmdInterface.report_file);
						continue;
					}
									
					err_code = cmd_handler_receive_answer(&cmd_match, &myComInterface, &is_busy_pin, CMD_RX_ANSWER_TIMEOUT_MS);
					if (err_code != NO_ERR) {
						LOG_MSG(ERR_LEVEL_WARNING, &myCfgInterface.log_file, "- Receiving Event-Answer has FAILED !!! --- (error-code = %d / Command: %s)", err_code, (char*)myCmdInterface.message.payload);
						restore_last_file_pointer(&myCmdInterface.report_file);
						continue;
					}
					
					err_code = cmd_handler_get_error_code(&myCmdInterface);
					if (err_code != NO_ERR) {
						LOG_MSG(ERR_LEVEL_WARNING, &myCfgInterface.log_file, "- Status of Report-Answer unexpected --- (status-code = %d / Command: %s)", err_code, (char*)myCmdInterface.message.payload);
						restore_last_file_pointer(&myCmdInterface.report_file);
						DEBUG_MSG("-- Incorrect Status-Code --- (ERR: %d)\n", err_code);
						continue;
					}
					
					EVENT_HANDLING_MATCH_EVENT_ASNWER:
					
					err_code = cmd_handler_match_event_answer(&myCmdInterface, &cmd_match);
					if (err_code != NO_ERR) {
						//LOG_MSG(ERR_LEVEL_WARNING, &myCfgInterface.log_file, "- Event not recognized !!! --- (error-code = %d)\n", err_code);
						continue;
					}
						
					err_code = mqtt_send_message(&myMqttInterface, &myCmdInterface.message);
					if (err_code != NO_ERR) {
						LOG_MSG(ERR_LEVEL_WARNING, &myCfgInterface.log_file, "- Sending MQTT-Report-Message has FAILED !!! --- (error-code = %d / Command: %s)", err_code, (char*)myCmdInterface.message.payload);
						continue;
					}			
				}
			}
			#endif		
		}
		
		MAIN_DEBUG_MSG("---- MQTT-CONNECTION LOST !!! ----\n");
		LOG_MSG(ERR_LEVEL_FATAL, &myCfgInterface.log_file, "Connection to MQTT-Broker lost - Trying to reconnect!");

		MQTTClient_disconnect(myMqttInterface.client, 10000);
		MQTTClient_destroy(&myMqttInterface.client);
		
		usleep(500000);
	}
	
	spi_deinit(&myComInterface.data.spi);
	
	DEBUG_MSG("- Disconnected from SmartHomeBroker.\n");

	return 0;
}

// -------- COMMAND-LINE PARSING --------------------------------------------------------

static u8 command_line_parser(int argc, char* argv[], CFG_INTERFACE* p_cfg_interface, COM_INTERFACE* p_com_interface, MQTT_INTERFACE* p_mqtt_interface, COMMAND_INTERFACE* p_cmd_interface, GPIO_INTERFACE* p_gpio_interface, SCHEDULING_INTERFACE* p_scheduling_interface) {
	
	// --- Initialize Communication-Interface
	
	p_scheduling_interface->report.interval = REPORT_SCHEDULE_DEFAULT_INTERVAL_MS;
	p_scheduling_interface->event.interval = EVENT_SCHEDULE_DEFAULT_INTERVAL_MS;
	p_scheduling_interface->configuration.interval = CONFIGURATION_SCHEDULE_DEFAULT_INTERVAL_MS;
	
	// 48000  4800 	9600 	38400 	115200
	p_com_interface->data.spi.speed_hz = 9600;
	p_com_interface->data.spi.bits_per_word = 8;
	p_com_interface->data.spi.delay = 0;
	p_com_interface->data.spi.mode = SPI_MODE_3;
	
	p_mqtt_interface->quality_of_service = MQTT_QOS;
	p_mqtt_interface->msg_delivered = 1;

	p_gpio_interface->pin_num = GPIO_EVENT_PIN;
	p_gpio_interface->is_input = 1;
	p_gpio_interface->is_high_level = 0;
	p_gpio_interface->match_event_level = 1;
	p_gpio_interface->event_rised = 0;
	p_gpio_interface->event_ref_time = 0;
	p_gpio_interface->event_timeout = 1000;
	p_gpio_interface->is_initialized = 0;
	p_gpio_interface->sample_time_reference = 0;
	p_gpio_interface->sample_timeout = 100;
	
	p_cmd_interface->command_file.handle = 0;
	p_cmd_interface->command_file.act_file_pointer = 0;
	p_cmd_interface->report_file.handle = 0;
	p_cmd_interface->report_file.act_file_pointer = 0;
	p_cmd_interface->event_file.handle = 0;
	p_cmd_interface->event_file.act_file_pointer = 0;
	p_cmd_interface->is_active = 1;
	
	p_cfg_interface->log_file.act_file_pointer = 0;		
	
	memset(p_cfg_interface->cfg_file.path, 0x00, FILE_PATH_MAX_STRING_LENGTH);
	memcpy(p_cfg_interface->cfg_file.path, CONFIGURATION_FILE_PATH, string_length(CONFIGURATION_FILE_PATH));

	u8 i = 0;
	for ( ; i < argc; i++) {
		
		if (memcmp(argv[i], COMMAND_LINE_ARGUMENT_CFG_FILE, string_length(COMMAND_LINE_ARGUMENT_CFG_FILE)) == 0) {
			
			if (i + 1 >= argc) {
				break;
			}
	
			memset(p_cfg_interface->cfg_file.path, 0x00, FILE_PATH_MAX_STRING_LENGTH);
			memcpy(p_cfg_interface->cfg_file.path, argv[i + 1], string_length(argv[i + 1]));
			
			CFG_DEBUG_MSG("Using Config-File: %s\n", p_cfg_interface->cfg_file.path);
		}
	}
	
	char path[64];
	sprintf(path, "%s", p_cfg_interface->cfg_file.path);
	
	FILE* config_file_handle = fopen((const char*)path, "r");
	if (config_file_handle == NULL) {
		CFG_DEBUG_MSG("--- Open Configuration-File has FAILED !!! --- (FILE: %s / ERROR: %d)\n", path,  EXIT_FAILURE);
		return ERR_FILE_OPEN;
	}
	
	struct stat file_attribute;
	stat(p_cfg_interface->cfg_file.path, &file_attribute);
	p_cfg_interface->cfg_file.timestamp_last_modified = file_attribute.st_mtime;
	
	char line[512];
	char cfg_key[GENERAL_STRING_BUFFER_MAX_LENGTH];
	char cfg_value[GENERAL_STRING_BUFFER_MAX_LENGTH];
	u16 num_bytes = 0;
	
	do  {				
		num_bytes = read_line(config_file_handle, line, 512);		
		split_string('=', line, num_bytes, cfg_key, GENERAL_STRING_BUFFER_MAX_LENGTH, cfg_value, GENERAL_STRING_BUFFER_MAX_LENGTH);
		
		u16 length_key = string_length(cfg_key);
		if (length_key == 0) {
			continue;
		}
		
		u16 length_value = string_length(cfg_value);
		if (length_value == 0) {
			continue;
		}
		
		memset(cfg_key + length_key, 0x00, GENERAL_STRING_BUFFER_MAX_LENGTH - length_key);
		memset(cfg_value + length_value, 0x00, GENERAL_STRING_BUFFER_MAX_LENGTH - length_value);
		
		CFG_DEBUG_MSG("key:%s : value:%s\n", cfg_key, cfg_value);
		
		if (memcmp(cfg_key, CFG_NAME_MQTT_HOST_ADDR, length_key) == 0) {
			memcpy(p_mqtt_interface->host_address, cfg_value, MQTT_HOST_ADDRESS_STRING_LENGTH);
		
		} else 
		
		if (memcmp(cfg_key, CFG_NAME_MQTT_CLINET_ID, length_key) == 0) {
			memcpy(p_mqtt_interface->client_id, cfg_value, MQTT_CLIENT_ID_STRING_LENGTH);
		
		} else 
		
		if (memcmp(cfg_key, CFG_NAME_MQTT_TOPIC_NAME, length_key) == 0) {
			memcpy(p_mqtt_interface->topic_name, cfg_value, MQTT_TOPIC_NAME_STRING_LENGTH);
		} else 
		
		if (memcmp(cfg_key, CFG_NAME_MQTT_WELCOME_MESSAGE, length_key) == 0) {
			memcpy(p_mqtt_interface->welcome_msg, cfg_value, MQTT_WELCOME_MSG_STRING_LENGTH);
		} else 
		
		if (memcmp(cfg_key, CFG_NAME_MQTT_TIMEOUT, length_key) == 0) {
			p_mqtt_interface->timeout_ms = MQTT_TIMEOUT;
		
		} else 
		
		if (memcmp(cfg_key, CFG_NAME_COMMUNICATION_TYPE, length_key) == 0) {
			p_com_interface->type = SPI;
		
		} else 
		
		if (memcmp(cfg_key, CFG_NAME_COM_SPI_BAUDRATE, length_key) == 0) {		
			char *ptr;
			p_com_interface->data.spi.speed_hz = (u32)strtol(cfg_value, &ptr, 10);			
		} else 
		
		if (memcmp(cfg_key, CFG_NAME_COM_SPI_DEVICE, length_key) == 0) {
			memcpy(p_com_interface->data.spi.device, cfg_value, COM_DEVICE_NAME_STRING_LENGTH);		
		} else 
		
		if (memcmp(cfg_key, CFG_NAME_LOG_FILE_PATH, length_key) == 0) {
			memcpy(p_cfg_interface->log_file.path, cfg_value, FILE_PATH_MAX_STRING_LENGTH);	
		} else 
		
		if (memcmp(cfg_key, CFG_NAME_COMMAND_FILE_PATH, length_key) == 0) {
			memcpy(p_cmd_interface->command_file.path, cfg_value, FILE_PATH_MAX_STRING_LENGTH);			
		} else 
		
		if (memcmp(cfg_key, CFG_NAME_REPORT_FILE_PATH, length_key) == 0) {
			memcpy(p_cmd_interface->report_file.path, cfg_value, FILE_PATH_MAX_STRING_LENGTH);		
		} else 
		
		if (memcmp(cfg_key, CFG_NAME_EVENT_FILE_PATH, length_key) == 0) {
			memcpy(p_cmd_interface->event_file.path, cfg_value, FILE_PATH_MAX_STRING_LENGTH);	
		} else 			
		
		if (memcmp(cfg_key, CFG_NAME_EXECUTION_FILE_PATH, length_key) == 0) {
			memcpy(p_cmd_interface->execution_file.path, cfg_value, FILE_PATH_MAX_STRING_LENGTH);
		} else 
		
		if (memcmp(cfg_key, CFG_NAME_SCHEDULE_INTERVAL_REPORT_MS, length_key) == 0) {
			char *ptr;
			p_scheduling_interface->report.interval = (u32)strtol(cfg_value, &ptr, 10);
		} else 
		
		if (memcmp(cfg_key, CFG_NAME_SCHEDULE_INTERVAL_EVENT_MS, length_key) == 0) {
			char *ptr;
			p_scheduling_interface->event.interval = (u32)strtol(cfg_value, &ptr, 10);
		} else 
		
		if (memcmp(cfg_key, CFG_NAME_SCHEDULE_INTERVAL_CONFIG_MS, length_key) == 0) {
			char *ptr;
			p_scheduling_interface->configuration.interval = (u32)strtol(cfg_value, &ptr, 10);
		}
		
		else {
			CFG_DEBUG_MSG("--- UNKNOWN CFG-KEY : %s\n", cfg_key);
		}
		
	} while (num_bytes != 0);
	
	return NO_ERR;
}

void command_line_usage(void) {
	
}

// -------- GPIO - HANDLER --------------------------------------------------------------

static void gpio_set_state(GPIO_INTERFACE* p_gpio, u8 state) {

	GPIO_CONFIGURE_PIN(p_gpio->pin_num, p_gpio->is_input);
		
	if (p_gpio->is_input != 0) {		
		GPIO_PULL_UP_DOWN(p_gpio->pin_num, state);		
	} else {
		GPIO_WRITE_PIN(p_gpio->pin_num, state);
	}
}

static u8 gpio_initialize(GPIO_INTERFACE* p_gpio) {
	
	static u8 is_initialized = 0;
	
	if (is_initialized == 0) {	
	
		int err_code = GPIO_INITIALIZE();
		if (err_code < 0) {
			GPIO_DEBUG_MSG("- Initializing GPIOD has FAILED !!! --- (err-code = %d)\n", err_code);
			return ERR_GPIO;
		}
	
		GPIO_DEBUG_MSG("- Initializing gpio has succeeded!\n");	
		is_initialized = 1;		
	}

	GPIO_CONFIGURE_PIN(p_gpio->pin_num, p_gpio->is_input);
	gpio_set_state(p_gpio, p_gpio->is_high_level);
	//gpioSetPullUpDown(p_gpio->pin_num, p_gpio->is_high_level ? PI_PUD_UP : PI_PUD_OFF);
	
	p_gpio->is_initialized = 1;
	return NO_ERR;
}

static u8 gpio_is_event(GPIO_INTERFACE* p_gpio) {
	
	if (p_gpio->is_initialized == 0) {
		GPIO_DEBUG_MSG("- GPIO-Interface is not initialized!!! ---\n");
		sleep(50000);
		return 0;
	}
	
	if (mstime_is_time_up(p_gpio->sample_time_reference, p_gpio->sample_timeout) == 0) {
		return p_gpio->event_rised != 0 ? 1 : 0;
	}
	
GPIO_CONFIGURE_PIN(GPIO_SAMPLE_PIN /* GPIO_22 */, 0 /* IS_OUTPUT */);
GPIO_WRITE_PIN(GPIO_SAMPLE_PIN /* GPIO_22 */, GPIO_ON);
	
	
	p_gpio->sample_time_reference = mstime_get_time();
	GPIO_DEBUG_MSG("- Sampling GPIO (number = %d / time : %d)\n", p_gpio->pin_num, p_gpio->sample_time_reference);

	if (p_gpio->event_rised != 0 && p_gpio->event_timeout != 0) {
		if (mstime_is_time_up(p_gpio->event_ref_time, p_gpio->event_timeout) == 0) {
			GPIO_DEBUG_MSG("- GPIO-Event still pending (number = %d / time : %d)\n", p_gpio->pin_num, p_gpio->event_ref_time);
GPIO_WRITE_PIN(GPIO_SAMPLE_PIN /* GPIO_22 */, GPIO_OFF);
			return 1;
		}
	}		
						
	p_gpio->event_rised = 0;

	int gpio_level = GPIO_READ_PIN(p_gpio->pin_num);
	
	if (gpio_level < 0 ){//== PI_BAD_GPIO) {
		GPIO_DEBUG_MSG("- Reading GPIO-Levl has FAILED !!! --- (pin number = %d)\n", p_gpio->pin_num);
GPIO_WRITE_PIN(GPIO_SAMPLE_PIN /* GPIO_22 */, GPIO_OFF);
		return 0;
	}
		
	GPIO_DEBUG_MSG("- GPIO Event  pin-level = %d\n", gpio_level);
		
	if (gpio_level == p_gpio->match_event_level) {
		
		GPIO_DEBUG_MSG("- GPIO Event raised\n");
		
		p_gpio->event_rised = 1;
		p_gpio->event_ref_time = mstime_get_time();
GPIO_WRITE_PIN(GPIO_SAMPLE_PIN /* GPIO_22 */, GPIO_OFF);
		return 1;
		
	} else {
GPIO_WRITE_PIN(GPIO_SAMPLE_PIN /* GPIO_22 */, GPIO_OFF);
		return 0;
	}
}


static void gpio_reset_pin(GPIO_INTERFACE* p_gpio) {	
	p_gpio->match_event_level = 0;
	p_gpio->event_rised = 0;
	p_gpio->event_timeout = 0;
	p_gpio->sample_time_reference = 0;
}


// -------- COMMAND - HANDLER -----------------------------------------------------------

static void restore_last_file_pointer(FILE_INTERFACE* p_file) {
	p_file->act_file_pointer = p_file->last_file_pointer;
}


static u8 cmd_handler_prepare_command_from_file(COMMAND_INTERFACE* p_cmd, FILE_INTERFACE* p_file) {
	
	p_file->handle = fopen(p_file->path, "r");
	if (p_file->handle == NULL) {
		COMMAND_DEBUG_MSG("--- Open Command-Map-File has FAILED !!! --- (FILE: %s / ERROR: %d)\n", p_file->path,  EXIT_FAILURE);
		return ERR_FILE_OPEN;
	}	
	
	char file_line[512];
		
	fseek(p_file->handle, p_file->act_file_pointer, SEEK_SET);
	u8 num_bytes = read_line(p_file->handle, file_line, 512);
	
	p_file->last_file_pointer = p_file->act_file_pointer;
	p_file->act_file_pointer = ftell(p_file->handle);
	
	fclose(p_file->handle);
	
	//COMMAND_DEBUG_MSG("--- File-Line: %s\n", file_line);
	
	if (num_bytes == 0) {
		COMMAND_DEBUG_MSG("--- End of Command-File reached !!!\n");
		p_file->act_file_pointer = 0;
		p_file->last_file_pointer = 0;
		return ERR_END_OF_FILE;
	}
		
	//COMMAND_DEBUG_MSG("--- File-Pointer : %d\n", p_file->act_file_pointer);
	
	char command_answer_string[2 * GENERAL_STRING_BUFFER_MAX_LENGTH];
	char command_string[GENERAL_STRING_BUFFER_MAX_LENGTH];	
	char answer_string[GENERAL_STRING_BUFFER_MAX_LENGTH];	
		
	split_string('=', file_line, num_bytes, (char*)p_cmd->message.payload, GENERAL_STRING_BUFFER_MAX_LENGTH, command_answer_string, 2 * GENERAL_STRING_BUFFER_MAX_LENGTH);
	split_string('=', command_answer_string, string_length(command_answer_string), command_string, GENERAL_STRING_BUFFER_MAX_LENGTH, answer_string, GENERAL_STRING_BUFFER_MAX_LENGTH);
		
	p_cmd->message.length = string_length((char*)p_cmd->message.payload);
	memset(p_cmd->message.payload + p_cmd->message.length, 0x00, GENERAL_STRING_BUFFER_MAX_LENGTH - p_cmd->message.length);
		
	p_cmd->command.length = hex_string_to_byte_array(command_string, string_length(command_string), p_cmd->command.payload, GENERAL_STRING_BUFFER_MAX_LENGTH);
	memset(p_cmd->command.payload + p_cmd->command.length, 0x00, GENERAL_STRING_BUFFER_MAX_LENGTH - p_cmd->command.length);
	
	p_cmd->answer.length = hex_string_to_byte_array(answer_string, string_length(answer_string), p_cmd->answer.payload, GENERAL_STRING_BUFFER_MAX_LENGTH);
	memset(p_cmd->answer.payload + p_cmd->answer.length, 0x00, GENERAL_STRING_BUFFER_MAX_LENGTH - p_cmd->answer.length);	
	
	COMMAND_DEBUG_MSG("--- Command: MSG=%s / CMD=%s / ANSW=%s\n", p_cmd->message.payload, command_string, answer_string);
	
	return NO_ERR;
}

static u8 cmd_handler_match_event_answer(COMMAND_INTERFACE* p_cmd, COMMAND_INTERFACE* p_cmd_match) {	

	#if EVENT_DEBUG_MSG == DEBUG_MSG
	u8 i = 0;
	for ( ; i < p_cmd->answer.length; i++) {
		COMMAND_DEBUG_MSG("---> MATCHING: %02x - %02x \n", p_cmd->answer.payload[i], p_cmd_match->answer.payload[i]);
	}

	#endif

	return memcmp(p_cmd->answer.payload, p_cmd_match->answer.payload, p_cmd->answer.length) == 0 ? NO_ERR : ERR_NOT_EQUAL;
}

static u8 cmd_handler_prepare_report_message(COMMAND_INTERFACE* p_cmd, u8 err_code) {
				
	if (err_code == NO_ERR) {					
		p_cmd->message.length +=
			byte_array_string_to_hex_string (
				p_cmd->answer.payload,
				p_cmd->answer.length,
				(char*)(p_cmd->message.payload + p_cmd->message.length),
				GENERAL_STRING_BUFFER_MAX_LENGTH - p_cmd->message.length
			);
	} else {					
		COMMAND_DEBUG_MSG("---> Receive Report-Command has FAILED !!! --- (ERROR:%d)\n", err_code);
		sprintf (
			(char*)(p_cmd->message.payload + p_cmd->message.length),
			"ERR(%d)", err_code 
		);
		p_cmd->message.length = string_length((char*)p_cmd->message.payload);
	}
	
	return NO_ERR;
}

static u8 cmd_handler_prepare_command(COMMAND_INTERFACE* p_cmd) {
	
	p_cmd->command.length = 0;
	p_cmd->answer.length = 0;
	
	char path[64];
	sprintf(path, "%s", p_cmd->command_file.path);
	
	FILE* command_file_handle = fopen((const char*)path, "r");
	if (command_file_handle == NULL) {
		COMMAND_DEBUG_MSG("--- Open Command-Map-File has FAILED !!! --- (FILE: %s / ERROR: %d)\n", path,  EXIT_FAILURE);
		return ERR_FILE_OPEN;
	}
	
	char command_line[512];
	char command_message[256];
	char command_data[256];
	u16 num_bytes = 0;
	
	do  {
		num_bytes = read_line(command_file_handle, command_line, 512);		
		split_string('=', command_line, num_bytes, command_message, 256, command_data, 256);
	
		if (memcmp(p_cmd->message.payload, command_message, string_length(command_message)) == 0) {	
			
			p_cmd->command.length = hex_string_to_byte_array(command_data, string_length(command_data), p_cmd->command.payload, GENERAL_STRING_BUFFER_MAX_LENGTH);		
			break;
		}
		
	} while (num_bytes != 0);
	
	fclose(command_file_handle);	
	
	if (p_cmd->command.length != 0) {	
		COMMAND_DEBUG_MSG("--- Command: %s (%s) \n", command_message, command_data);
		return NO_ERR;
		
	} else {
		COMMAND_DEBUG_MSG("--- ERROR: Unknown Command (\"%s\")\n", p_cmd->message.payload);
		return ERR_BAD_CMD;
	}
}

static u8 cmd_handler_prepare_execution(COMMAND_INTERFACE* p_cmd) {

	p_cmd->command.length = 0;
	p_cmd->answer.length = 0;
	
	char path[64];
	sprintf(path, "%s", p_cmd->execution_file.path);
	
	p_cmd->execution_file.handle = fopen((const char*)path, "r");
	if (p_cmd->execution_file.handle == NULL) {
		COMMAND_DEBUG_MSG("--- Open Execution-Map-File has FAILED !!! --- (FILE: %s / ERROR: %d)\n", path,  EXIT_FAILURE);
		return ERR_FILE_OPEN;
	}
	
	char command_line[512];
	char command_message[256];
	char command_data[256];
	u16 num_bytes = 0;
	
	do  {
		num_bytes = read_line(p_cmd->execution_file.handle, command_line, 512);
		split_string('=', command_line, num_bytes, command_message, 256, command_data, 256);
	
		if (memcmp(p_cmd->message.payload, command_message, string_length(command_message)) == 0) {
			memcpy(p_cmd->command.payload, command_data, string_length(command_data));
			memset(p_cmd->command.payload + string_length(command_data), 0, GENERAL_STRING_BUFFER_MAX_LENGTH - string_length(command_data));
			p_cmd->command.length = string_length(command_data);
			break;
		}
		
	} while (num_bytes != 0);
	
	fclose(p_cmd->execution_file.handle);
	
	if (p_cmd->command.length != 0) {	
		COMMAND_DEBUG_MSG("--- Execution: %s (%s) \n", command_message, command_data);
		return NO_ERR;
		
	} else {
		COMMAND_DEBUG_MSG("--- ERROR: Unknown Execution (\"%s\")\n", p_cmd->message.payload);
		return ERR_BAD_CMD;
	}
}

static u8 cmd_handler_run_execution(COMMAND_INTERFACE* p_cmd) {
	system((const char*)p_cmd->command.payload);
	return NO_ERR;
}
		
static u8 cmd_handler_send_command(COMMAND_INTERFACE* p_cmd, COM_INTERFACE* p_com, GPIO_INTERFACE* p_gpio) {

	if (p_cmd->command.length == 0) {
		return ERR_INVALID_ARGUMENT;
	}	
			
	// Set GPIO to low for at least 50ms to activate control-board
	gpio_reset_pin(p_gpio);
	p_gpio->match_event_level = 0;
	p_gpio->is_input = 1;
	gpio_set_state(p_gpio, GPIO_ON);
	
	u8 err_code = NO_ERR;
	u32 time_reference = mstime_get_time();
			
	while (gpio_is_event(p_gpio) == 0) {
			
		background_work();
		usleep(p_gpio->sample_timeout * 1000); // wait for HW to be ready

		if (mstime_is_time_up(time_reference, CMD_ACTIVATE_TIMEOUT_MS) != 0) {
			COMMAND_DEBUG_MSG("-- ERROR on sending command - wait for low-level of ready-pin has FAIELD !!! ---\n");
			err_code = ERR_COMMUNICATION;
			break;
		}
	}
			
	if (err_code != NO_ERR) {		
		gpio_set_state(p_gpio, GPIO_OFF);
		return err_code;
	}
	
	//spi_start_tx();
	
	switch (p_com->type) {
		case SPI :
			
			// Check if there is a old answer pending on the other side
			err_code = spi_transfer (
				&p_com->data.spi, 
				1, 
				(const u8*) p_cmd->command.payload, 
				(u8*)&p_cmd->answer.length
			);
			
			if (err_code != NO_ERR) {
				COMMAND_DEBUG_MSG("-- ERROR before sending command - reading old answer length has FAIELD !!! --- \n");
				break;
			}
			
			if (p_cmd->answer.length > GENERAL_STRING_BUFFER_MAX_LENGTH) {
				err_code = ERR_ANSWER_LENGTH;
				COMMAND_DEBUG_MSG("-- ERROR before sending command - old anser to long OVERFLOW !!! --- \n");
				break;
			}
			
			p_cmd->command.length -= 1;
			
			if (p_cmd->answer.length != 0) {
			
				u8 length = (p_cmd->answer.length > p_cmd->command.length) ? p_cmd->answer.length : p_cmd->command.length;
				err_code = spi_transfer(&p_com->data.spi, length, (const u8*)(p_cmd->command.payload) + 1, p_cmd->answer.payload);
				COMMAND_DEBUG_MSG("--- Need to read old answer from the interface (Length: %d)!!!\n", p_cmd->answer.length);
				
			} else {
				err_code = spi_transfer(&p_com->data.spi, p_cmd->command.length, (const u8*)(p_cmd->command.payload) + 1, NULL);
			}
					
			// while (p_cmd->answer.length != 0) {
					
				// COMMAND_DEBUG_MSG("--- Need to read old answer from the interface (Length: %d)!!!\n", p_cmd->answer.length);
					
				// u8 length = p_cmd->answer.length;
				// if (length > GENERAL_STRING_BUFFER_MAX_LENGTH) {
					// length = GENERAL_STRING_BUFFER_MAX_LENGTH;
				// }
					
				// err_code = spi_transfer (
					// &p_com->data.spi, 
					// length, 
					// NULL, 
					// p_cmd->answer.payload
				// );
					
				// p_cmd->answer.length -= length;
			// }
			
//			err_code = spi_transfer(&p_com->data.spi, p_cmd->command.length, (const u8*) p_cmd->command.payload, NULL);
			break;
			
		case I2C : 
			break;
				
		case USART : 
			break;
			
		default: 
			break;
	}

	gpio_reset_pin(p_gpio);
	p_gpio->match_event_level = 1;
	p_gpio->is_input = 1;
	gpio_set_state(p_gpio, GPIO_ON);
	
	time_reference = mstime_get_time();
	
	while (gpio_is_event(p_gpio) == 0) {
			
		background_work();
		usleep(p_gpio->sample_timeout * 1000);

		if (mstime_is_time_up(time_reference, CMD_ACTIVATE_TIMEOUT_MS) != 0) {
			COMMAND_DEBUG_MSG("-- ERROR after sending command - wait for high-level of ready-pin has FAIELD !!! ---\n");
			err_code = ERR_COMMUNICATION;
			break;
		}
	}
	
	//spi_stop_trx();	
	gpio_set_state(p_gpio, GPIO_OFF);
	
	return err_code;
}

static u8 cmd_handler_receive_answer(COMMAND_INTERFACE* p_cmd, COM_INTERFACE* p_com, GPIO_INTERFACE* p_gpio, u32 timeout_ms) {
	
	u8 err_code = NO_ERR;

	gpio_reset_pin(p_gpio);
	p_gpio->match_event_level = 0;
	p_gpio->is_input = 1;
	gpio_set_state(p_gpio, GPIO_ON);
	
	u32 time_reference = mstime_get_time();
	p_cmd->answer.length = 0;
			
	while (gpio_is_event(p_gpio) == 0) {
			
		background_work();
		usleep(p_gpio->sample_timeout * 1000); // wait a little bit for answer to be complete

		if (mstime_is_time_up(time_reference, CMD_ACTIVATE_TIMEOUT_MS) != 0) {
			COMMAND_DEBUG_MSG("-- ERROR on receiving answer - wait for low-level of ready-pin has FAIELD !!! ---\n");
			err_code = ERR_COMMUNICATION;
			break;
		}
	}
		
	gpio_set_state(p_gpio, GPIO_OFF);
			
	if (err_code != NO_ERR) {
		return err_code;
	}
	
	//spi_start_tx();
	
	switch (p_com->type) {
		case SPI :
				
			// length of answer	
			err_code = spi_transfer (
				&p_com->data.spi, 
				1, 
				NULL, 
				(u8*)&p_cmd->answer.length
			);
					
			if (err_code) {
				COMMAND_DEBUG_MSG("-- Receiving answer-length has FAILED !!! --- (ERR: %d)\n", err_code);
				err_code = ERR_ANSWER_LENGTH;
				break;
			}
				
			if (p_cmd->answer.length == 0) {
				COMMAND_DEBUG_MSG("-- Answer-Length is zero, nothing to receive. \n");
				break;
			}
			
			if (p_cmd->answer.length > GENERAL_STRING_BUFFER_MAX_LENGTH) {
				COMMAND_DEBUG_MSG("-- Answer-Length is too Long !!! --- (LENGTH: %d)\n", p_cmd->answer.length);
				err_code = ERR_ANSWER_LENGTH;
				break;
			}
			
			// answer
			err_code = spi_transfer (
				&p_com->data.spi, 
				p_cmd->answer.length, 
				NULL, 
				p_cmd->answer.payload
			);
				
			if (err_code) {
				COMMAND_DEBUG_MSG("-- Receiving answer has FAILED !!! --- (ERR: %d)\n", err_code);
				err_code = ERR_COMMUNICATION;
				break;
			}
			
			break;
		
		case I2C : 
			break;
			
		case USART : 
			break;
		
		default: 
			break;
	}
	
	#if COMMAND_DEBUG_MSG != noDEBUG_MSG
	command_handling_duration = mstime_get_time() - command_handling_duration;
	COMMAND_DEBUG_MSG("------> Command Execution-Time: %d ms\n", command_handling_duration);
	COMMAND_DEBUG_MSG("------> Low-Level-Time: %d ms\n", low_level_waiting_time);
	COMMAND_DEBUG_MSG("------> High-Level-Time: %d ms\n", high_level_waiting_time);
	COMMAND_DEBUG_MSG("------> Transfer-Time: %d ms\n", transfer_time);
	#endif

	gpio_reset_pin(p_gpio);
	p_gpio->match_event_level = 1;
	p_gpio->is_input = 1;
	gpio_set_state(p_gpio, GPIO_ON);
	
	time_reference = mstime_get_time();
	
	while (gpio_is_event(p_gpio) == 0) {
			
		background_work();
		usleep(p_gpio->sample_timeout * 1000); // wait a little bit for answer to be complete

		if (mstime_is_time_up(time_reference, CMD_ACTIVATE_TIMEOUT_MS) != 0) {
			COMMAND_DEBUG_MSG("-- ERROR after receiving answer - wait for high-level of ready-pin has FAIELD !!! ---\n");
			err_code = ERR_COMMUNICATION;
			break;
		}
	}
	
	//spi_stop_trx();	
	gpio_set_state(p_gpio, GPIO_OFF);
	
	return err_code;
}

static u8 cmd_handler_get_error_code(COMMAND_INTERFACE* p_cmd) {
	return p_cmd->answer.payload[1];
}

static u8 mqtt_init(MQTT_INTERFACE* p_mqtt_interface) {

	MQTT_DEBUG_MSG("--- Initialize MQTT-Client\n1");
	MQTTClient_create(&p_mqtt_interface->client, p_mqtt_interface->host_address, p_mqtt_interface->client_id, MQTTCLIENT_PERSISTENCE_NONE, NULL);
		
	MQTT_DEBUG_MSG("--- Set MQTT-Callbacks\n1");
	MQTTClient_setCallbacks(p_mqtt_interface->client, (void*)p_mqtt_interface, connectionLost_Callback, messageArrived_Callback, deliveryComplete_Callback);	
	
	return NO_ERR;
}

static u8 mqtt_connect(MQTT_INTERFACE* p_mqtt_interface) {	

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

static u8 mqtt_send_message(MQTT_INTERFACE* p_mqtt_interface, STRING_BUFFER* p_msg_from) {

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

// -------- MQTT CALLBACKs --------------------------------------------------------------

static void connectionLost_Callback(void *context, char* cause) {

	if (context == NULL) {
		return;		
	}
	
	MQTT_INTERFACE* ctx = (MQTT_INTERFACE*) context;
	
	if (ctx->connection_lost != 0) {
		return;		
	}
	
	MQTT_DEBUG_MSG("- MQTT-Connection has been lost !!! ---\n");
	
	ctx->connection_lost = 1;
}

static int messageArrived_Callback(void* context, char* topicName, int topcLength, MQTTClient_message* message) {

	if (context == NULL) {
	
		MQTT_DEBUG_MSG("- Context is zero !!! ---\n");
		return 1;		
	}
	
	if (memcmp((u8*)message->payload, "cmd", 3) == 0 || memcmp((u8*)message->payload, "exe", 3) == 0) {
		qeue_enqeue(&myCommandQeue, message);
	}	
	
	MQTTClient_free(topicName);
	MQTTClient_freeMessage(&message);
	
	return 1;
}

static void deliveryComplete_Callback(void* context, MQTTClient_deliveryToken token) {

	if (context == NULL) {
		return;		
	}

	MQTT_INTERFACE* ctx = (MQTT_INTERFACE*) context;
	
	if (ctx->msg_delivered != 0) {
		return;		
	}
	
	ctx->msg_delivered = 1;
	MQTT_DEBUG_MSG("- Message successful delivered !!! ---\n");
}

// -------- SPI DRIVER ------------------------------------------------------------------

static void spi_init(SPI_INTERFACE* p_spi_handle) {

	SPI_DEBUG_MSG("- Using SPI-Device: %s\n", p_spi_handle->device);

	p_spi_handle->_handle_id = open(p_spi_handle->device, O_RDWR);
	if (p_spi_handle->_handle_id < 0) {
		SPI_DEBUG_MSG("Cant open SPI device");
		return;
	}
	
	u8 err_code = ioctl(p_spi_handle->_handle_id, SPI_IOC_WR_MODE32, &p_spi_handle->mode);
	if (err_code) {
		SPI_DEBUG_MSG("Can't set spi mode");
		return;
	}

	err_code = ioctl(p_spi_handle->_handle_id, SPI_IOC_RD_MODE32, &p_spi_handle->mode);
	if (err_code) {
		SPI_DEBUG_MSG("Can't get spi mode");
		return;
	}
	
	err_code = ioctl(p_spi_handle->_handle_id, SPI_IOC_WR_BITS_PER_WORD, &p_spi_handle->bits_per_word);
	if (err_code) {
		SPI_DEBUG_MSG("Can't set bits per word");
		return;
	}

	err_code = ioctl(p_spi_handle->_handle_id, SPI_IOC_RD_BITS_PER_WORD, &p_spi_handle->bits_per_word);
	if (err_code) {
		SPI_DEBUG_MSG("Can't get bits per word");
		return;
	}
	
	err_code = ioctl(p_spi_handle->_handle_id, SPI_IOC_WR_MAX_SPEED_HZ, &p_spi_handle->speed_hz);
	if (err_code) {
		SPI_DEBUG_MSG("Can't set max speed hz");
		return;
	}

	err_code = ioctl(p_spi_handle->_handle_id, SPI_IOC_RD_MAX_SPEED_HZ, &p_spi_handle->speed_hz);
	if (err_code) {
		SPI_DEBUG_MSG("Can't get max speed hz");
		return;
	}		
}

static void spi_deinit(SPI_INTERFACE* p_spi_handle) {
	close(p_spi_handle->_handle_id);
	p_spi_handle->_handle_id = -1;
}

static u8 spi_transfer(SPI_INTERFACE* p_spi_handle, size_t num_bytes, const u8* p_buffer_from, u8* p_buffer_to) {

	if (p_spi_handle->_handle_id < 0) {
		SPI_DEBUG_MSG("SPI device not initailized!");
		return ERR_NOT_INITIALIZED;
	}
	
	if (num_bytes == 0) {
		SPI_DEBUG_MSG("---> Parameter num_bytes is zero! \n");
		return ERR_INVALID_ARGUMENT;
	}
	
	u8 tmp_tx_buffer[GENERAL_STRING_BUFFER_MAX_LENGTH];	
	u8 tmp_rx_buffer[GENERAL_STRING_BUFFER_MAX_LENGTH];

	if (p_buffer_from != NULL) {
		memcpy(tmp_tx_buffer, p_buffer_from, num_bytes);
	} else {
		memset(tmp_tx_buffer, 0x00, num_bytes);
	}

	struct spi_ioc_transfer spi_tr = {
		.tx_buf = (unsigned long)tmp_tx_buffer,
		.rx_buf = (unsigned long)tmp_rx_buffer,
		.len = num_bytes,
		.delay_usecs = p_spi_handle->delay,
		.speed_hz = p_spi_handle->speed_hz,
		.bits_per_word = p_spi_handle->bits_per_word,
		.tx_nbits = 0
	};

	/*
	if (p_spi_handle->mode & SPI_TX_QUAD) {
		spi_tr.tx_nbits = 4;
	
	} else if (p_spi_handle->mode & SPI_TX_DUAL) {
		spi_tr.tx_nbits = 2;
	}
	
	if (p_spi_handle->mode & SPI_RX_QUAD) {
		spi_tr.rx_nbits = 4;
	
	} else if (p_spi_handle->mode & SPI_RX_DUAL) {
		spi_tr.rx_nbits = 2;
	}	
	*/
	
	u8 err_code = ioctl(p_spi_handle->_handle_id, SPI_IOC_MESSAGE(1), &spi_tr);

	hex_dump((const void*)tmp_tx_buffer, num_bytes, 32, "TX");
	hex_dump((const void*)tmp_rx_buffer, num_bytes, 32, "RX");

	if (p_buffer_to != NULL) {
		memcpy(p_buffer_to, tmp_rx_buffer, num_bytes);
	}
	
	if (err_code < 1) {
		SPI_DEBUG_MSG("-----> Can't send spi message (erro-code = %d)\n", err_code);
		return ERR_SEND_MSG;
	} else {
		return NO_ERR;
	}
}

// -------- FILE HANDLING ------------------------------------------------------------------

static u8 file_has_changed(FILE_INTERFACE* p_file) {

	struct stat file_attribute;
	if (stat(p_file->path, &file_attribute) == 0) {
		return 0;
		
	} else if (file_attribute.st_mtime > p_file->timestamp_last_modified) {
		return 1;
		
	} else {
		return 0;
	}
}

static u8 file_is_existing(FILE_INTERFACE* p_file) {

	struct stat file_attribute;
	if (stat(p_file->path, &file_attribute) < 0) {
		return 0;
		
	} else {
		return 1;
	}
}

// -------- QEUE HANDLING ------------------------------------------------------------------

static void qeue_init(MSG_QEUE* p_qeue) {
	p_qeue->write_counter = 0;
	p_qeue->read_counter = 0;
	p_qeue->element_counter = 0;
}

static u8 qeue_enqeue(MSG_QEUE* p_qeue, MQTTClient_message* p_msg_from) {

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

static u8 qeue_deqeue(MSG_QEUE* p_qeue, STRING_BUFFER* p_msg_to) {

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

static u8 qeue_is_empty(MSG_QEUE* p_qeue) {
	return p_qeue->element_counter == 0 ? 1 : 0;
}

// -------- LOG HANDLING -------------------------------------------------------------------

/*!
 *
 */
static void log_message(FILE_INTERFACE* p_file, u8 error_level, STRING_BUFFER* p_msg_from) {
	
	char path[64];
	sprintf(path, "%s", p_file->path);
	LOG_DEBUG_MSG("LOG-DEBUG: Using Log-File: %s \n", path);
	
	if (file_is_existing(p_file) == 0) {
		p_file->handle = fopen((const char*)path, "w");
		LOG_DEBUG_MSG("LOG-DEBUG: Log-File does not exists, will create it\n");	
		
	} else {
		p_file->handle = fopen((const char*)path, "a");
		LOG_DEBUG_MSG("LOG-DEBUG: Appending Log-Message to existing file \n");	
	}	
	
	if (p_file->handle == NULL) {
		LOG_DEBUG_MSG("LOG-DEBUG: Open Log-File has FAILED !!! --- \n");
		return;
	}
	
	char date_string[128];
	memset(date_string, 0x00, 128);
	
	FILE* pipe = popen("date", "r");
	read_line(pipe, date_string, 128);
	fclose(pipe);
	
	int err_code = fprintf(p_file->handle, "%s \t %d \t %s \r\n", date_string, error_level, p_msg_from->payload);
	if (err_code < 0) {
		LOG_DEBUG_MSG("LOG-DEBUG: Writing File has FAILED !!! --- (ERROR: %d)\n", err_code);		
	}
		
	fclose(p_file->handle);
}
