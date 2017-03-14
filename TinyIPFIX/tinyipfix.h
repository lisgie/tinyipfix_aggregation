#ifndef TINYIPFIX_H_
#define TINYIPFIX_H_

#include <stdint.h>


//Length field is 10 bits long
#define MAX_MSG_SIZE 1023

#define MAX_SET_ID 4095
#define MAX_SEQ_NUM 65535

#define MAX_SEQ_SMALL 255
#define MAX_SEQ_LARGE 65535

#define TEMPLATE_SET_ID 2
#define DATA_SET_ID 256

#define SET_HEADER_SIZE 4

//Depending on config defined in [platform].h define the correct header size
#ifndef EXTENDED_HEADER_SEQ
	#define EXTENDED_HEADER_SEQ 0
#endif
#ifndef EXTENDED_HEADER_SET_ID
	#define EXTENDED_HEADER_SET_ID 0
#endif
#if EXTENDED_HEADER_SET_ID == 1
	#if EXTENDED_HEADER_SEQ == 1
		#define MSG_HEADER_SIZE 5
	#else
		#define MSG_HEADER_SIZE 4
	#endif
#else
	#if EXTENDED_HEADER_SEQ == 1
		#define MSG_HEADER_SIZE 4
	#else
		#define MSG_HEADER_SIZE 3
	#endif
#endif

struct tinyipfix_packet {
	uint8_t header[MSG_HEADER_SIZE];
	uint8_t *payload;
};

struct template_rec {

	//the enterprise bit will be the MSB of element_id, to not waste another byte
	uint16_t element_id;
	uint16_t field_len;
	uint32_t enterprise_num;

	void (* sens_val)(void*);
};

struct tinyipfix_packet *split_packet(const uint8_t* data);
uint8_t build_msg_header(uint8_t* buf, uint16_t set_id, uint16_t length, uint16_t seq_num);
uint8_t initialize_tinyipfix(uint8_t);
uint8_t *get_template(void);
uint8_t *get_data(void);

#endif /* TINYIPFIX_H_ */
