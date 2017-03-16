#include "aggr.h"

//In contrast to msg aggr. we only need one template version
#define TEMPLATE_REPLICA 1

//not placed in header for information hiding reasons (also different aggr. techniques
//might need different amount of memory
uint8_t *aggr_template_buf;
uint8_t *aggr_data_buf;

uint16_t aggr_field_len;
uint16_t payload_extr_pos;

uint32_t max(uint32_t a, uint32_t b) { return a > b ? a : b; }
uint32_t min(uint32_t a, uint32_t b) { return a < b ? a : b; }

uint8_t *get_aggr_template() {
	uint8_t i;
	uint16_t position, end_of_block;
	uint16_t element_id, field_len = 0;

	aggr_template_buf = (uint8_t*)malloc(sizeof(uint8_t)*MAX_MSG_SIZE);
	aggr_data_buf = (uint8_t*)malloc(sizeof(uint8_t)*MAX_MSG_SIZE);

	initialize_tinyipfix(TEMPLATE_REPLICA);
	//this might not necessarily be needed

	aggr_template_buf = get_template();
	//modify the header such that the desired template field is in there
	//we need to count the length until we get our desired value
	position = MSG_HEADER_SIZE+SET_HEADER_SIZE;
	end_of_block = position;
	for(i = 0; i < SENSOR+1; i++) {
		position = end_of_block;
		//we need to check if enterprise bit is set
		element_id = aggr_template_buf[position];
		element_id <<= 8;
		element_id |= aggr_template_buf[position+1];
		if( (element_id | 0x8000) == element_id) {
			end_of_block = position+8;
		} else {
			end_of_block = position+4;
		}
		payload_extr_pos += field_len;
		field_len = aggr_template_buf[position+2];
		field_len <<= 8;
		field_len |= aggr_template_buf[position+3];
	}

	//our target field len is loc_field_len;
	aggr_field_len = field_len;

	//if I switch the sequence here, it doesn't work, why?
	memcpy(aggr_template_buf+MSG_HEADER_SIZE+SET_HEADER_SIZE,get_template()+position, end_of_block-position);
	memcpy(aggr_template_buf, get_template(), MSG_HEADER_SIZE+SET_HEADER_SIZE);

	//length
	aggr_template_buf[1] = MSG_HEADER_SIZE+SET_HEADER_SIZE+(end_of_block-position);
	//length field of the set header, we only have one aggregated value
	aggr_template_buf[MSG_HEADER_SIZE+SET_HEADER_SIZE-1] = NUMBER_OF_AGGR_FIELDS;

	return aggr_template_buf;
}

void calc_aggr_payload(uint8_t* data,uint16_t datalen) {

	struct tinyipfix_packet* header_and_payload;
	static uint8_t aggr_count = 0;
	static uint16_t seq_num = 0;
	static uint32_t data_aggr_state = 0;
	//Using 0 as start value doesn't work for minimizing function
	static uint32_t data_aggr_state_min = 0xffff;
	uint8_t i;
	uint32_t converted = 0;
	uint32_t tmp;

	printf("*");
	header_and_payload = split_packet(data);
	aggr_count++;

	//need to extract multiple bytes and put it in one variable
	for(i = 0; i < aggr_field_len; i++) {
		tmp = header_and_payload->payload[payload_extr_pos+i];
		tmp <<= (8*(aggr_field_len-1-i));
		converted |= tmp;
		printf("#0x%x#, ", converted);
	}

	//see the meaning of the constants in aggr.h
	if(AGGR_FUNC == 0) {
		data_aggr_state += converted;
	} else if(AGGR_FUNC == 1) {
		data_aggr_state = max(data_aggr_state, converted);
	} else if(AGGR_FUNC == 2) {
		data_aggr_state_min = min(data_aggr_state_min, converted);
		data_aggr_state = data_aggr_state_min;
	}
	if(aggr_count == DEGREE_OF_AGGREGATION) {
		if(AGGR_FUNC == 0)
			data_aggr_state /= DEGREE_OF_AGGREGATION;
		build_msg_header(aggr_data_buf,DATA_SET_ID,MSG_HEADER_SIZE+aggr_field_len,seq_num);

		//write the data back to the buffer
		for(i = 0; i < aggr_field_len; i++) {
			aggr_data_buf[MSG_HEADER_SIZE+i] = (data_aggr_state >> (8*(aggr_field_len-1-i)));
		}
		msg_send(BORDER_COMM,aggr_data_buf,extr_len_from_header(aggr_data_buf));
		data_aggr_state = 0;
		data_aggr_state_min = 0xffff;
		aggr_count = 0;
		seq_num++;
	}
}
