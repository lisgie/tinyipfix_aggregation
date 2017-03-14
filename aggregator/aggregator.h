#ifndef AGGREGATOR_H_
#define AGGREGATOR_H_

//0 - Message Aggr., 1 - Data Aggr.
#define AGGREGATION_MODE 1

//Message Aggregation
#define DEGREE_OF_MSG_AGGREGATION 3
//---

//Data Aggregation
#define DEGREE_OF_DATA_AGGREGATION 5
//because of HUMID_POS we need to include the hw_module header, should be somehow changed
#define SENSOR TEMP_POS
//AVG - 0, MAX - 1, MIN - 2
#define AGGR_FUNC 1
//---

//can be inlined
uint32_t max(uint32_t a, uint32_t b) { return a > b ? a : b; }
uint32_t min(uint32_t a, uint32_t b) { return a < b ? a : b; }

#endif /* AGGREGATOR_H_ */
