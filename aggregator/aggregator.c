#include "contiki.h"
#include "lib/random.h"
#include "sys/ctimer.h"
#include "sys/etimer.h"

#include "net/rpl/rpl.h"

#include <stdio.h>
#include <string.h>

#include "../TinyIPFIX/tinyipfix.h"

#include "aggregator.h"
#include "aggr.h"
#include "comm.h"

#define TEMPLATE_INTERVAL 5

#define SEND_INTERVAL		(10 * CLOCK_SECOND)
#define SEND_TIME		(random_rand() % (SEND_INTERVAL))

uint16_t template_size;
uint8_t *template_buf;

PROCESS(unicast_receiver_process, "Unicast receiver example process");
AUTOSTART_PROCESSES(&unicast_receiver_process);

PROCESS_THREAD(unicast_receiver_process, ev, data)
{
  static struct etimer template_timer;
  PROCESS_BEGIN();
  etimer_set(&template_timer, CLOCK_SECOND*TEMPLATE_INTERVAL);
  template_buf = get_aggr_template();

  init_system();

  while(1) {
    PROCESS_WAIT_EVENT();
	if (etimer_expired (&template_timer)) {
		msg_send(BORDER_COMM, template_buf, template_size);
		etimer_reset(&template_timer);
	}
  }
  PROCESS_END();
}
