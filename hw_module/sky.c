//This include specifies the corresponding platform
#include "sky.h"

//Initialising struct, used to fill records with relevant data
struct template_rec set_fields(uint8_t, uint16_t, uint16_t, uint32_t, void(*sens_val)(void*));

//Every data record to be sent has to be read by a specified function
void read_temp(void*);
void read_humid(void*);
void read_light_photo(void*);
void read_light_total(void*);
void read_battery(void*);
void read_time(void*);
void read_id(void*);
void read_pull(void*);

//Main datastructure, struct array of type template_rec
struct template_rec records[NUM_ENTRIES];

//Values are defined in [platform].h
struct template_rec *init_template() {

	records[0] = set_fields(E_BIT_TEMP, ELEMENT_ID_TEMP, LEN_TEMP, ENTERPRISE_ID_TEMP, &read_temp);
	records[1] = set_fields(E_BIT_HUMID, ELEMENT_ID_HUMID, LEN_HUMID, ENTERPRISE_ID_HUMID, &read_humid);
	records[2] = set_fields(E_BIT_LIGHT_PHOTO, ELEMENT_ID_LIGHT_PHOTO, LEN_LIGHT_PHOTO, ENTERPRISE_ID_LIGHT_PHOTO, &read_light_photo);
	records[3] = set_fields(E_BIT_LIGHT_TOTAL, ELEMENT_ID_LIGHT_TOTAL, LEN_LIGHT_TOTAL, ENTERPRISE_ID_LIGHT_TOTAL, &read_light_total);
	records[4] = set_fields(E_BIT_BATTERY, ELEMENT_ID_BATTERY, LEN_BATTERY, ENTERPRISE_ID_BATTERY, &read_battery);
	records[5] = set_fields(E_BIT_TIME, ELEMENT_ID_TIME, LEN_TIME, ENTERPRISE_ID_TIME, &read_time);
	records[6] = set_fields(E_BIT_ID, ELEMENT_ID_ID, LEN_ID, ENTERPRISE_ID_ID, &read_id);
	records[7] = set_fields(E_BIT_PULL, ELEMENT_ID_PULL, LEN_PULL, ENTERPRISE_ID_PULL, &read_pull);

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

	#ifdef SKY
	SENSORS_ACTIVATE(sht11_sensor);
	*((uint16_t*)(temp)) = sht11_sensor.value(SHT11_SENSOR_TEMP);
	SENSORS_DEACTIVATE(sht11_sensor);
	#endif
}

void read_humid(void* humid) {
	#ifdef SKY
	SENSORS_ACTIVATE(sht11_sensor);
	*((uint16_t*)(humid)) = sht11_sensor.value(SHT11_SENSOR_HUMIDITY);
	SENSORS_DEACTIVATE(sht11_sensor);
	#endif
}

void read_light_photo(void* light_photo) {
	#ifdef SKY
	SENSORS_ACTIVATE(light_sensor);
	*((uint16_t*)(light_photo)) = light_sensor.value(LIGHT_SENSOR_PHOTOSYNTHETIC);
	SENSORS_DEACTIVATE(light_sensor);
	#endif
}

void read_light_total (void* light_total) {
	#ifdef SKY
	SENSORS_ACTIVATE(light_sensor);
	*((uint16_t*)(light_total)) = light_sensor.value(LIGHT_SENSOR_TOTAL_SOLAR);
	SENSORS_DEACTIVATE(light_sensor);
	#endif
}

void read_battery(void* bat) {
	#ifdef SKY
	SENSORS_ACTIVATE(battery_sensor);
	*((uint16_t*)(bat)) = battery_sensor.value(0);
	SENSORS_DEACTIVATE(battery_sensor);
	#endif
}

void read_time(void* time) {
	#ifdef SKY
	*((uint32_t*)(time)) = clock_seconds ();
	#endif
}

//last two octets of the MAC address
void read_id(void* id) {
	#ifdef SKY
	*((int16_t*)(id)) = uip_lladdr.addr[6] << 8;
	*((int16_t*)(id)) |= uip_lladdr.addr[7];
	#endif
}

//0 corresponds to push, 1 corresponds to pull
void read_pull(void *pull) {
	#ifdef SKY
	*((uint8_t *)(pull)) = 0;
	#endif
}
//END OF FUNCTIONS TO READ VALUES --------------------------------

