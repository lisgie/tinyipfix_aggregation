#include "contiki.h"
#include "lib/random.h"
#include "sys/ctimer.h"
#include "sys/etimer.h"

#include "net/rpl/rpl.h"
#include "../TinyIPFIX/tinyipfix.h"

#include "aggregator.h"
#include "comm.h"

#define TEMPLATE_INTERVAL 37

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
    	clock_delay(random_rand()%4);
		msg_send(BORDER_COMM, template_buf, extr_len_from_header(template_buf[0]));
		etimer_reset(&template_timer);
	}
  }
  PROCESS_END();
}
