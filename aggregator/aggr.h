#ifndef AGGR_H_
#define AGGR_H_

#include "../TinyIPFIX/tinyipfix.h"
#include "comm.h"

//because of HUMID_POS we need to include the hw_module header, should be somehow changed
#define SENSOR TEMP_POS
//AVG - 0, MAX - 1, MIN - 2
#define AGGR_FUNC 2
//How many values to be aggregated
#define NUMBER_OF_AGGR_FIELDS 1
//---

//hardcoded for testing
#define DEGREE_OF_AGGREGATION 3

//is this even needed?
#define MSG_AGGREGATION 0
#define DATA_AGGREGATION 1

uint8_t *get_aggr_template();
void calc_aggr_payload(uint8_t*,uint16_t);

#endif /* AGGR_H_ */
