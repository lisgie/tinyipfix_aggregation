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

#include "sys/node-id.h"

#include "simple-udp.h"
#include "servreg-hack.h"

#include "../TinyIPFIX/tinyipfix.h"
#include "dev/leds.h"

#include <stdio.h>
#include <string.h>

#define TEMPLATE_INTERVAL 17
#define DATA_INTERVAL 5

#define DEGREE_OF_AGGREGATION 1

#define UDP_PORT 1234
#define SERVICE_ID 190

/*---------------------------------------------------------------------------*/
PROCESS(unicast_sender_process, "Unicast sender example process");
AUTOSTART_PROCESSES(&unicast_sender_process);
/*---------------------------------------------------------------------------*/
static void
receiver(struct simple_udp_connection *c,
         const uip_ipaddr_t *sender_addr,
         uint16_t sender_port,
         const uip_ipaddr_t *receiver_addr,
         uint16_t receiver_port,
         const uint8_t *data,
         uint16_t datalen)
{
  printf("Data received on port %d from port %d with length %d\n",
         receiver_port, sender_port, datalen);
}
/*---------------------------------------------------------------------------*/

PROCESS_THREAD(unicast_sender_process, ev, data)
{

	static struct etimer data_timer, template_timer;

	uip_ipaddr_t *addr;
	uint8_t *buffer;
	uint8_t i;

	static struct simple_udp_connection aggregator_comm;

	PROCESS_BEGIN();

	etimer_set(&template_timer, CLOCK_SECOND*TEMPLATE_INTERVAL);
	etimer_set(&data_timer, CLOCK_SECOND*DATA_INTERVAL);
	servreg_hack_init();

	simple_udp_register(&aggregator_comm, UDP_PORT,
                    NULL, UDP_PORT, receiver);

	initialize_tinyipfix(DEGREE_OF_AGGREGATION);

	while(1) {
		PROCESS_WAIT_EVENT();

		if (etimer_expired (&template_timer)) {

			buffer = get_template();

			if( (buffer = get_template()) == NULL) {
				//debug(LEDS_RED | LEDS_BLUE);
				continue;
			}

			leds_on (LEDS_BLUE);
			clock_delay (1000);
			leds_off (LEDS_BLUE);

		    addr = servreg_hack_lookup(SERVICE_ID);
		    if(addr != NULL) {
		    	simple_udp_sendto(&aggregator_comm, buffer, buffer[1], addr);
			} else {			}

			etimer_reset(&template_timer);
		} else if(etimer_expired(&data_timer)) {

			if( (buffer = get_data()) == NULL) {
				//debug(LEDS_RED | LEDS_GREEN);
				continue;
			}

			leds_on (LEDS_GREEN);
			clock_delay (1000);
			leds_off (LEDS_GREEN);

		    addr = servreg_hack_lookup(SERVICE_ID);
		    if(addr != NULL) {
		    	simple_udp_sendto(&aggregator_comm, buffer, buffer[1], addr);
		    } else {		    }

			etimer_reset(&data_timer);
		}
	}
	PROCESS_END();
}
/*---------------------------------------------------------------------------*/
