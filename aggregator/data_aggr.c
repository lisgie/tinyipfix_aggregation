#include "aggr.h"

//In contrast to msg aggr. we only need one template version
#define TEMPLATE_REPLICA 1

//not placed in header for information hiding reasons (also different aggr. techniques
//might need different amount of memory
uint8_t *aggr_template_buf;
uint8_t *aggr_data_buf;

uint16_t aggr_field_len;
uint16_t payload_extr_pos;

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

	//template_buf[] stores length
	aggr_template_buf[1] = MSG_HEADER_SIZE+SET_HEADER_SIZE+(end_of_block-position);
	//length field of the set header
	aggr_template_buf[MSG_HEADER_SIZE+SET_HEADER_SIZE-1] = 0x01;

	return aggr_template_buf;
}

void calc_aggr_payload(uint8_t* data,uint16_t datalen) {

	/*struct tinyipfix_packet* header_and_payload;
		uint8_t i, tmp_8;
		uint32_t converted = 0;
		uint32_t tmp;

		header_and_payload = split_packet(data);
		count++;

		leds_on(LEDS_RED);
		clock_delay(1000);
		leds_off(LEDS_RED);

		//check with SWITCH_ENDIAN from tinyipfix code
		for(i = 0; i < target_field_len; i++) {
			tmp = header_and_payload->payload[extraction_within_payload+i];
			tmp <<= (8*(target_field_len-1-i));
			converted |= tmp;
		}

		if(AGGR_FUNC == 0) {
			//AVG
			data_aggr_state += converted;
		} else if(AGGR_FUNC == 1) {
			//MAX
			data_aggr_state = max(data_aggr_state, converted);

		} else if(AGGR_FUNC == 2) {
			//MIN
			data_aggr_state = min(data_aggr_state, converted);
		}

		if(count == DEGREE_OF_DATA_AGGREGATION) {
			leds_on(LEDS_YELLOW);
			clock_delay(1000);
			leds_off(LEDS_YELLOW);
			if(AGGR_FUNC == 0)
				data_aggr_state /= DEGREE_OF_DATA_AGGREGATION;
			//memcpy(buf,header_and_payload->header,MSG_HEADER_SIZE);
			build_msg_header(buf,DATA_SET_ID,MSG_HEADER_SIZE+target_field_len,seq_num);
			//buf[1] = MSG_HEADER_SIZE+target_field_len;

			for(i = 0; i < target_field_len; i++) {
				tmp_8 = (converted >> (8*(target_field_len-1-i)));
				buf[MSG_HEADER_SIZE+i] = tmp_8;
			}

			msg_send(BORDER_COMM,buf,buf[1]);
			data_aggr_state = 0;
			count = 0;
			seq_num++;
		}*/
}
