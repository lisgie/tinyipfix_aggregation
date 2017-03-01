//Used for memcpy(...)
#include <string.h>
//Used for malloc(...)
#include <stdlib.h>
#include <stdint.h>

#include "tinyipfix.h"

//Corresponds to the hardware used
#include "../hw_module/openmote.h"

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

//This translation functions are needed for switching endianness before writing to byte buffer
#define SWITCH_ENDIAN_16(n) (uint16_t)((((uint16_t) (n)) << 8) | (((uint16_t) (n)) >> 8))
#define SWITCH_ENDIAN_32(n) (((uint32_t)SWITCH_ENDIAN_16(n) << 16) | SWITCH_ENDIAN_16((uint32_t)(n) >> 16))
#define SWITCH_ENDIAN_64(n) (((uint64_t)SWITCH_ENDIAN_32(n) << 32) | SWITCH_ENDIAN_32((uint64_t)(n) >> 32))

//Every TinyIPFIX message consists of a message header
uint8_t build_msg_header(uint8_t *buf, uint16_t set_id, uint16_t length, uint16_t seq_num);

//The whole template can be built statically, no distinction between header/payload necessary
uint8_t build_template(void);

//Updates the data records
uint8_t build_data_payload(void);
//Most of the data header can be computed statically, done at the beginning
//for optimization reasons. Sequence number needs to be updated in every msg, though
uint8_t build_data_header(void);

uint16_t calc_template_size();
uint16_t calc_data_size();

//TinyIPFIX messages can only be used, if the system has been initialized
uint8_t is_initialized = 0;

uint8_t *template_buf;
uint8_t *data_buf;

uint16_t template_size;
uint16_t data_size;

//Defines field ID, length, enterprise num and function pointer
struct template_rec *sensor;

uint8_t initialize_tinyipfix(void) {

	sensor = init_template();

	template_size = calc_template_size();
	template_buf = (uint8_t*)malloc(template_size*sizeof(uint8_t));
	data_size = calc_data_size();
	data_buf = (uint8_t*)malloc(data_size*sizeof(uint8_t));

	//Everything needs to be properly set up in order to continue, else errorcode
	if( build_template() == -1 || build_data_header() == -1 )
		return -1;

	is_initialized = 1;

	return 0;
}

uint16_t calc_template_size() {

	uint8_t i;
	uint16_t template_size = MSG_HEADER_SIZE;

	//Calculate length first to be able to build the message header
	for(i = 0; i < NUM_ENTRIES; i++) {

		//Check first bit to know if enterprise field is set or not
		if( ((sensor[i].element_id) | 0x8000) == sensor[i].element_id) {
			template_size += 4;
		}
	}

	//4 = sizeof(element_id) + sizeof(field_len) = 2 + 2
	template_size += 4*NUM_ENTRIES;
	//Template Set Header
	template_size += SET_HEADER_SIZE;

	return template_size;
}

uint16_t calc_data_size() {

	uint8_t i;
	uint16_t data_size = MSG_HEADER_SIZE;

	for(i = 0; i < NUM_ENTRIES; i++) {

		data_size += sensor[i].field_len;
	}

	return data_size;
}

uint8_t build_msg_header(uint8_t* buf, uint16_t set_id, uint16_t length, uint16_t seq_num) {

	//Basic checks
	if(set_id > MAX_SET_ID || seq_num > MAX_SEQ_NUM || length > MAX_MSG_SIZE)
		return -1;

	//Zeroing out, can't rely on zero in mem
	buf[0] = 0;

	//Shortcut for certain SetIDs, check TinyIPFIX draft
	if(set_id == TEMPLATE_SET_ID) {
		set_id = 1;
	} else if(set_id == DATA_SET_ID) {
		set_id = 2;
	}

	//SetID occupies only bit 2..5, counting from 0
	if(set_id < 16) {
		buf[0] |= (set_id << 2);
	}

	//Length is part of the message header at a fixed location, known at compile time
	//length += MSG_HEADER_SIZE;
	buf[0] |= (uint8_t)(length >> 8);
	buf[1] = (uint8_t)(length);

	buf[2] = (uint8_t)(seq_num);

	//Case distinction depending on header configuration
	if(MSG_HEADER_SIZE == 3) {

	} else if(EXTENDED_HEADER_SET_ID == 0 && EXTENDED_HEADER_SEQ == 1) {

		buf[0] |= 0x40;
		buf[2] = (uint8_t)(seq_num >> 8);
		buf[3] = (uint8_t)(seq_num);
	} else if(EXTENDED_HEADER_SET_ID == 1 && EXTENDED_HEADER_SEQ == 0) {

		buf[0] |= 0x80;
		buf[0] |= ((set_id >> 8) << 2);
		buf[3] = (uint8_t)(set_id);

	} else if(MSG_HEADER_SIZE == 5) {

		buf[0] |= 0xc0;

		buf[0] |= ((set_id >> 8) << 2);
		buf[4] = (uint8_t)(set_id);

		buf[2] = (uint8_t)(seq_num >> 8);
		buf[3] = (uint8_t)(seq_num);
	}

	return 0;
}

 uint8_t build_template(void) {

	uint8_t i;
	uint16_t element_id, field_len;
	uint32_t enterprise_num;

	//In case the template doesn't reference data set 256, ref_set_id points to the right number
	uint16_t ref_set_id = DATA_SET_ID;
	//Can be found at [platform].h
	uint16_t field_count = NUM_ENTRIES;

	//Used to know where we are within our buffer
	uint16_t template_tmp_len = MSG_HEADER_SIZE;

	if( build_msg_header(template_buf, TEMPLATE_SET_ID, template_size, 0xFFFF) == -1)
		return -1;

	//Before writing into byte buf, always change endiannsess
	ref_set_id = SWITCH_ENDIAN_16(ref_set_id);
	field_count = SWITCH_ENDIAN_16(field_count);

	//Write into byte buffer
	memcpy(&template_buf[template_tmp_len], &ref_set_id, sizeof(ref_set_id));
	template_tmp_len += sizeof(ref_set_id);
	memcpy(&template_buf[template_tmp_len], &field_count, sizeof(field_count));
	template_tmp_len += sizeof(field_count);

	//Filling up the template with records
	for(i = 0; i < NUM_ENTRIES; i++) {

		element_id = SWITCH_ENDIAN_16(sensor[i].element_id);
		memcpy(&template_buf[template_tmp_len], &element_id, sizeof(element_id));
		template_tmp_len += sizeof(element_id);

		field_len = SWITCH_ENDIAN_16(sensor[i].field_len);
		memcpy(&template_buf[template_tmp_len], &field_len, sizeof(field_len));
		template_tmp_len += sizeof(field_len);

		//Check if enterprise bit exists
		if( ((sensor[i].element_id) | 0x8000) == sensor[i].element_id) {

			enterprise_num = SWITCH_ENDIAN_32(sensor[i].enterprise_num);
			memcpy(&template_buf[template_tmp_len], &enterprise_num, sizeof(enterprise_num));
			template_tmp_len += sizeof(enterprise_num);
		}
	}

	return 0;
}

//Everything known at compile time except changing sequence number
uint8_t build_data_header(void) {

	//Sequence number starts at 0x00
	if( build_msg_header(data_buf, DATA_SET_ID, data_size, 0x00) == -1 )
		return -1;
	else
		return 0;
}

uint8_t build_data_payload(void) {

	//Value is preserved between function calls
	static uint16_t data_seq_num = 0;

	uint8_t i, val8;
	uint16_t val16;
	uint32_t val32;
	uint64_t val64;

	uint16_t data_tmp_len = MSG_HEADER_SIZE;

	//Adjust the sequence number without completely rebuilding the header
	data_seq_num++;
	if(EXTENDED_HEADER_SEQ == 1) {
		if(data_seq_num > MAX_SEQ_LARGE)
			data_seq_num = 0;

		data_buf[2] = (uint8_t)(data_seq_num >> 8);
		data_buf[3] = (uint8_t)(data_seq_num);
	} else {
		if(data_seq_num > MAX_SEQ_SMALL) {
			data_seq_num = 0;
		}
		data_buf[2] = (uint8_t)(data_seq_num);
	}

	//Read out sensor/field values, supports up to 64 bit precision
	for(i = 0; i < NUM_ENTRIES; i++) {

		switch(sensor[i].field_len) {

			case 1:
				sensor[i].sens_val(&val8);
				memcpy(&data_buf[data_tmp_len], &val8, sizeof(uint8_t));
				data_tmp_len += sizeof(uint8_t);
				break;
			case 2:
				sensor[i].sens_val(&val16);
				val16 = SWITCH_ENDIAN_16(val16);
				memcpy(&data_buf[data_tmp_len], &val16, sizeof(uint16_t));
				data_tmp_len += sizeof(uint16_t);
				break;
			case 4:
				sensor[i].sens_val(&val32);
				val32 = SWITCH_ENDIAN_32(val32);
				memcpy(&data_buf[data_tmp_len], &val32, sizeof(uint32_t));
				data_tmp_len += sizeof(uint32_t);
				break;
			case 8:
				sensor[i].sens_val(&val64);
				val64 = SWITCH_ENDIAN_64(val64);
				memcpy(&data_buf[data_tmp_len], &val64, sizeof(uint64_t));
				data_tmp_len += sizeof(uint64_t);
				break;
		}
	}

	return 0;
}

//Pass template buffer to main application
uint8_t *get_template(void) {

	if(!is_initialized)
		return NULL;

	return template_buf;
}

//Pass data buffer to main application
//Error code -1 for build_data_payload not used yet, memcpy only returns pointer to dest
//Adding more safeguards might be not worth it efficiency-wise, because bogus data can
//easily be spotted in the application
uint8_t *get_data(void) {

	//In case statically built stuff or payload generation failed

	if(!is_initialized || build_data_payload() == -1)
		return NULL;

	return data_buf;
}


