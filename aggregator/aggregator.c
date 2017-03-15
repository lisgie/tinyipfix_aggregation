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


#include "net/rpl/rpl.h"

#include <stdio.h>
#include <string.h>

#include "../TinyIPFIX/tinyipfix.h"
//might be needed for data aggregation, make it cleaner later
#include "../hw_module/sky.h"

#include "dev/leds.h"

#include "aggregator.h"
#include "aggr.h"
#include "comm.h"

#define TEMPLATE_INTERVAL 10

#define SEND_INTERVAL		(10 * CLOCK_SECOND)
#define SEND_TIME		(random_rand() % (SEND_INTERVAL))

uint32_t data_aggr_state;
uint16_t template_size;

uip_ipaddr_t border_router;
uint8_t buf[256];

uint8_t *template_buf;

uint16_t extraction_within_payload = 0, target_field_len;

uint8_t count = 0;


uint8_t payload[] = {
		0x08,0x14,0x03,0x19,0x9a,
		0x03,0x22,0x09,0xd7,0x08,
		0xb3,0x0d,0x55,0x00,0x00,
		0x00,0x0f,0x71,0x8a,0x00
};


//at the end we need to fill the header, so start not at the beginning to avoid copying memory
uint16_t insidebuf_count = MSG_HEADER_SIZE;
uint32_t seq_num = 0;

PROCESS(unicast_receiver_process, "Unicast receiver example process");
AUTOSTART_PROCESSES(&unicast_receiver_process);

//maybe drop the datalen, because can be extracted from the header itself
void message_aggregation(const uint8_t *data, uint16_t datalen) {


}

void data_aggregation(const uint8_t *data, uint16_t datalen) {


}


PROCESS_THREAD(unicast_receiver_process, ev, data)
{
  static struct etimer template_timer;
  uint8_t *tmp_buffer;
  uint16_t i, position, element_id, loc_field_len = 0;
  uint16_t end_of_block;

  PROCESS_BEGIN();

  etimer_set(&template_timer, CLOCK_SECOND*TEMPLATE_INTERVAL);

  template_buf = get_aggr_template();

  printf("0x%x, ", template_buf[1]);

  template_size = (template_buf[0] & 0x3);
  template_size <<= 8;
  template_size |= template_buf[1];

  init_system();

  while(1) {
    PROCESS_WAIT_EVENT();
	if (etimer_expired (&template_timer)) {

		leds_on(LEDS_RED);
		clock_delay(1000);
		leds_off(LEDS_RED);

		msg_send(BORDER_COMM, template_buf, template_size);

		etimer_reset(&template_timer);
	}
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
