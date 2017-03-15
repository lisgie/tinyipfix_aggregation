#include "aggr.h"

uint8_t aggr_template_buf[MAX_MSG_SIZE];
uint8_t aggr_data_buf[MAX_MSG_SIZE];

uint8_t *get_aggr_template() {

	initialize_tinyipfix(DEGREE_OF_AGGREGATION);
	memcpy(aggr_template_buf, get_template(), MAX_MSG_SIZE);

	return aggr_template_buf;
}

void calc_aggr_payload(uint8_t *data, uint16_t datalen) {

	struct tinyipfix_packet *tinyipfix_msg;
	static uint8_t aggr_count = 0;
	static uint16_t position = MSG_HEADER_SIZE, seq_num = 0;

	//I guess we have to copy the data because data is const
	tinyipfix_msg = split_packet(data);

	aggr_count++;

	memcpy(aggr_data_buf+position,tinyipfix_msg->payload,datalen-MSG_HEADER_SIZE);
	position += (datalen-MSG_HEADER_SIZE);

	if(aggr_count == DEGREE_OF_AGGREGATION) {
		//construct new header
		build_msg_header(aggr_data_buf,DATA_SET_ID,position,seq_num);
		msg_send(BORDER_COMM,aggr_data_buf,position);
		aggr_count = 0;
		seq_num++;
		position = MSG_HEADER_SIZE;
	}
}
