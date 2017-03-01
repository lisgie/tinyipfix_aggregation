//This include specifies the corresponding platform
#include "openmote.h"

#include <stdint.h>

#include "../TinyIPFIX/tinyipfix.h"

#include "net/ip/uip.h"

#include "dev/button-sensor.h"
//Temperature/Humidity sensor
#include "dev/sht21.h"
//Light sensor
#include "dev/max44009.h"

//Initialising struct, used to fill records with relevant data
struct template_rec set_fields(uint8_t, uint16_t, uint16_t, uint32_t, void(*sens_val)(void*));

//Every data record to be sent has to be read by a specified function
void read_temp(void*);
void read_humid(void*);
void read_light(void*);
void read_time(void*);
void read_id(void*);
void read_pull(void*);

//Main datastructure, struct array of type template_rec
struct template_rec records[NUM_ENTRIES];

//Values are defined in [platform].h
struct template_rec *init_template() {

	records[0] = set_fields(E_BIT_TEMP, ELEMENT_ID_TEMP, LEN_TEMP, ENTERPRISE_ID_TEMP, &read_temp);
	records[1] = set_fields(E_BIT_HUMID, ELEMENT_ID_HUMID, LEN_HUMID, ENTERPRISE_ID_HUMID, &read_humid);
	records[2] = set_fields(E_BIT_LIGHT, ELEMENT_ID_LIGHT, LEN_LIGHT, ENTERPRISE_ID_LIGHT, &read_light);
	records[3] = set_fields(E_BIT_TIME, ELEMENT_ID_TIME, LEN_TIME, ENTERPRISE_ID_TIME, &read_time);
	records[4] = set_fields(E_BIT_ID, ELEMENT_ID_ID, LEN_ID, ENTERPRISE_ID_ID, &read_id);
	records[5] = set_fields(E_BIT_PULL, ELEMENT_ID_PULL, LEN_PULL, ENTERPRISE_ID_PULL, &read_pull);

	return records;
}

struct template_rec set_fields(uint8_t e_bit, uint16_t element_id,
		uint16_t field_len, uint32_t enterprise_id, void (*sens_val)(void*)) {

	struct template_rec rec;

	rec.element_id = element_id;
	rec.field_len = field_len;
	rec.enterprise_num = enterprise_id;

	//The first bit of the field_id is reserved for the enterprise bit
	//0 = no enterprise bit set, 1 = enterprise bit set
	if(e_bit == 1)
		rec.element_id |= 0x8000;

	//Setting the function pointer
	rec.sens_val = sens_val;

	return rec;
}

//FUNCTIONS TO READ VALUES ---------------------------------------
void read_temp(void* temp) {

	SENSORS_ACTIVATE(sht21);
	*(uint16_t*)(temp) = sht21.value(SHT21_READ_TEMP);
	SENSORS_DEACTIVATE(sht21);
}

void read_humid(void* humid) {

	SENSORS_ACTIVATE(sht21);
	*(uint16_t*)(humid) = sht21.value(SHT21_READ_RHUM);
	SENSORS_DEACTIVATE(sht21);
}


void read_light (void* light) {

	 SENSORS_ACTIVATE(max44009);
	 max44009.value(MAX44009_READ_LIGHT);
	 SENSORS_DEACTIVATE(max44009);
}

void read_time(void* time) {

	*(uint32_t*)(time) = clock_seconds();
}

//last two octets of the MAC address, not necessarily unique?!
void read_id(void* id) {

	*((int16_t*)(id)) = uip_lladdr.addr[6] << 8;
	*((int16_t*)(id)) |= uip_lladdr.addr[7];
}

//0 corresponds to push, 1 corresponds to pull
void read_pull(void *pull) {

	*((uint8_t *)(pull)) = 0;
}
//END OF FUNCTIONS TO READ VALUES --------------------------------

