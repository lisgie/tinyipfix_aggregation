/*
 * Copyright (c) 2011, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 */

#include "contiki.h"
#include "lib/random.h"
#include "sys/ctimer.h"
#include "sys/etimer.h"
#include "net/ip/uip.h"
#include "net/ipv6/uip-ds6.h"
#include "net/ip/uip-debug.h"

#include "simple-udp.h"
#include "servreg-hack.h"

#include "net/rpl/rpl.h"

#include <stdio.h>
#include <string.h>

#include "aggregator.h"
#include "../TinyIPFIX/tinyipfix.h"
//might be needed for data aggregation, make it cleaner later
#include "../hw_module/sky.h"

#include "dev/leds.h"

#define TEMPLATE_INTERVAL 10

#define UDP_PORT 1234
#define SERVICE_ID 190

#define MAX_TEMPLATE_SIZE 512

#define SEND_INTERVAL		(10 * CLOCK_SECOND)
#define SEND_TIME		(random_rand() % (SEND_INTERVAL))

//0 - Message Aggr., 1 - Data Aggr.
#define AGGREGATION_MODE 1

static struct simple_udp_connection unicast_connection;
static struct simple_udp_connection border_conn;

uint32_t data_aggr_state;
uint16_t template_size;

uip_ipaddr_t border_router;
uint8_t buf[256];

uint8_t template_buf[MAX_TEMPLATE_SIZE];

uint16_t extraction_within_payload = 0, target_field_len;

uint8_t count = 0;

//at the end we need to fill the header, so start not at the beginning to avoid copying memory
uint16_t insidebuf_count = MSG_HEADER_SIZE;
uint32_t seq_num = 0;

PROCESS(unicast_receiver_process, "Unicast receiver example process");
AUTOSTART_PROCESSES(&unicast_receiver_process);

//maybe drop the datalen, because can be extracted from the header itself
void message_aggregation(const uint8_t *data, uint16_t datalen) {

	struct tinyipfix_packet *header_and_payload;

	//I guess we have to copy the data because data is const
	header_and_payload = split_packet(data);

	count++;

	memcpy(buf+insidebuf_count,header_and_payload->payload,datalen-MSG_HEADER_SIZE);
	insidebuf_count += (datalen-MSG_HEADER_SIZE);

	if(count == DEGREE_OF_MSG_AGGREGATION) {
		//construct new header
		build_msg_header(buf,DATA_SET_ID,insidebuf_count,seq_num);
		simple_udp_sendto(&border_conn,buf,insidebuf_count,&border_router);
		leds_on(LEDS_YELLOW);
		clock_delay(1000);
		leds_off(LEDS_YELLOW);
		insidebuf_count = MSG_HEADER_SIZE;
		count = 0;
	} else {
		leds_on(LEDS_RED);
		clock_delay(1000);
		leds_off(LEDS_RED);
	}

	return;
}

void data_aggregation(const uint8_t *data, uint16_t datalen) {

	struct tinyipfix_packet* header_and_payload;
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

		printf("\n\n");
		for(i = 0; i < buf[1]; i++) {
			printf("0x%x, ", buf[i]);
		}
		printf("\n\n");
		simple_udp_sendto(&border_conn,buf,buf[1],&border_router);
		data_aggr_state = 0;
		count = 0;
	}
}


static void
receiver(struct simple_udp_connection *c,
         const uip_ipaddr_t *sender_addr,
         uint16_t sender_port,
         const uip_ipaddr_t *receiver_addr,
         uint16_t receiver_port,
         const uint8_t *data,
         uint16_t datalen) {

	//we only want data sets and discard template sets
	uint8_t template_check = data[0];
	template_check &= 0xfc; //we need to discard the last two bits, 0xfc = 11111100
	if(template_check == 0x04)
		return;

	if(AGGREGATION_MODE == 0)
		message_aggregation(data, datalen);
	else if(AGGREGATION_MODE == 1)
		data_aggregation(data, datalen);

	seq_num++;
	return;
}

static uip_ipaddr_t *
set_global_address(void)
{
  static uip_ipaddr_t ipaddr;
  int i;
  uint8_t state;

  uip_ip6addr(&ipaddr, UIP_DS6_DEFAULT_PREFIX, 0,0,0,0,0,0,0);
  uip_ds6_set_addr_iid(&ipaddr, &uip_lladdr);
  uip_ds6_addr_add(&ipaddr, 0, ADDR_AUTOCONF);

  printf("IPv6 addresses: ");
  for(i = 0; i < UIP_DS6_ADDR_NB; i++) {
    state = uip_ds6_if.addr_list[i].state;
    if(uip_ds6_if.addr_list[i].isused &&
       (state == ADDR_TENTATIVE || state == ADDR_PREFERRED)) {
      uip_debug_ipaddr_print(&uip_ds6_if.addr_list[i].ipaddr);
      printf("\n");
    }
  }

  return &ipaddr;
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(unicast_receiver_process, ev, data)
{
  uip_ipaddr_t *ipaddr;
  static struct etimer template_timer;
  uint8_t *tmp_buffer;
  uint16_t i, position, element_id, loc_field_len = 0;
  uint16_t end_of_block;

  PROCESS_BEGIN();

  etimer_set(&template_timer, CLOCK_SECOND*TEMPLATE_INTERVAL);

  if(AGGREGATION_MODE == 0) {
	  initialize_tinyipfix(DEGREE_OF_MSG_AGGREGATION);
  	  memcpy(template_buf, get_template(), MAX_TEMPLATE_SIZE);
  } else if(AGGREGATION_MODE == 1) {
	  initialize_tinyipfix(1);

	  tmp_buffer = get_template();
	  //modify the header such that the desired template field is in there
	  //we need to count the length until we get our desired value
	  position = MSG_HEADER_SIZE+SET_HEADER_SIZE;
	  end_of_block = position;
	  for(i = 0; i < SENSOR+1; i++) {
		  position = end_of_block;
		  //we need to check if enterprise bit is set
		  element_id = tmp_buffer[position];
		  element_id <<= 8;
		  element_id |= tmp_buffer[position+1];
		  if( (element_id | 0x8000) == element_id) {
			  end_of_block = position+8;
		  } else {
			  end_of_block = position+4;
		  }

		  extraction_within_payload += loc_field_len;
		  loc_field_len = tmp_buffer[position+2];
		  loc_field_len <<= 8;
		  loc_field_len |= tmp_buffer[position+3];
	  }
	  //our target field len is loc_field_len;
	  target_field_len = loc_field_len;

	  //if I switch the sequence here, it doesn't work, why?
	  memcpy(template_buf+MSG_HEADER_SIZE+SET_HEADER_SIZE,get_template()+position, end_of_block-position);
	  memcpy(template_buf, get_template(), MSG_HEADER_SIZE+SET_HEADER_SIZE);

	  //template_buf[] stores length
	  template_buf[1] = MSG_HEADER_SIZE+SET_HEADER_SIZE+(end_of_block-position);
	  //set header
	  template_buf[MSG_HEADER_SIZE+2+1] = 0x01;
  }

  template_size = (template_buf[0] & 0x3);
  template_size <<= 8;
  template_size |= template_buf[1];

  uip_ip6addr(&border_router,UIP_DS6_DEFAULT_PREFIX,0,0,0,0,0,0,0);
  servreg_hack_init();
  ipaddr = set_global_address();

  servreg_hack_register(SERVICE_ID, ipaddr);

  simple_udp_register(&unicast_connection, UDP_PORT,
                      NULL, UDP_PORT, receiver);

  simple_udp_register(&border_conn, 40001,
                      NULL, 40001, NULL);

  while(1) {
    PROCESS_WAIT_EVENT();
	if (etimer_expired (&template_timer)) {

		leds_on(LEDS_RED);
		clock_delay(1000);
		leds_off(LEDS_RED);

		simple_udp_sendto(&border_conn, template_buf, template_size, &border_router);

		etimer_reset(&template_timer);
	}
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
