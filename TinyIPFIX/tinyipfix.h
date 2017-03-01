#include <stdint.h>

#ifndef TINYIPFIX_H_
#define TINYIPFIX_H_

//Length field is 10 bits long
#define MAX_MSG_SIZE 1023

#define MAX_SET_ID 4095
#define MAX_SEQ_NUM 65535

#define MAX_SEQ_SMALL 255
#define MAX_SEQ_LARGE 65535

#define TEMPLATE_SET_ID 2
#define DATA_SET_ID 256

#define SET_HEADER_SIZE 4

struct template_rec {

	//the enterprise bit will be the MSB of element_id, to not waste another byte
	uint16_t element_id;
	uint16_t field_len;
	uint32_t enterprise_num;

	void (* sens_val)(void*);
};

uint8_t initialize_tinyipfix(void);
uint8_t *get_template(void);
uint8_t *get_data(void);

#endif /* TINYIPFIX_H_ */
