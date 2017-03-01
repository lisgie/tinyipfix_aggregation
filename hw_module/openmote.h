#ifndef OPENMOTE_H_
#define OPENMOTE_H_

#define NUM_ENTRIES 6

//SHT12 Temperature Sensor
#define ELEMENT_ID_TEMP 0x80B0
#define ENTERPRISE_ID_TEMP 0x12345678
#define E_BIT_TEMP 1
#define LEN_TEMP 2

//SHT12 Humidity Sensor
#define ELEMENT_ID_HUMID 0x80B1
#define ENTERPRISE_ID_HUMID 0x12345678
#define E_BIT_HUMID 1
#define LEN_HUMID 2

//MAX44009 Light Sensor
#define ELEMENT_ID_LIGHT 0x80B2
#define ENTERPRISE_ID_LIGHT 0x12345678
#define E_BIT_LIGHT 1
#define LEN_LIGHT 2

//Internal clock
#define ELEMENT_ID_TIME 0x80B3
#define ENTERPRISE_ID_TIME 0x12345678
#define E_BIT_TIME 1
#define LEN_TIME 4

//Node ID, last two octets of IPv6 address - redundant
#define ELEMENT_ID_ID 0x80B4
#define ENTERPRISE_ID_ID 0x12345678
#define E_BIT_ID 1
#define LEN_ID 2

//Possible future extension, pull flag
#define ELEMENT_ID_PULL 0x80B5
#define ENTERPRISE_ID_PULL 0x12345678
#define E_BIT_PULL 1
#define LEN_PULL 1

//This is needed as changeable header option for TinyIPFIX messages
#define EXTENDED_HEADER_SEQ 0
#define EXTENDED_HEADER_SET_ID 0

//needs to be globally accessible
struct template_rec *init_template();

#endif /* OPENMOTE_H_ */

