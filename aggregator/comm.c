#include "comm.h"
#include "aggregator.h"

uip_ipaddr_t self_addr;
uip_ipaddr_t border_router;

static struct simple_udp_connection unicast_connection;
static struct simple_udp_connection border_conn;

void init_networking() {
	uip_ip6addr(&border_router,UIP_DS6_DEFAULT_PREFIX,0,0,0,0,0,0,0);

	uip_ip6addr(&self_addr, UIP_DS6_DEFAULT_PREFIX, 0,0,0,0,0,0,0);
	uip_ds6_set_addr_iid(&self_addr, &uip_lladdr);
	uip_ds6_addr_add(&self_addr, 0, ADDR_AUTOCONF);

	simple_udp_register(&unicast_connection, UDP_PORT_CLIENT_SERV,
	                      NULL, UDP_PORT_CLIENT_SERV, receiver);

	simple_udp_register(&border_conn, UDP_PORT_SERV_BORDER,
	                      NULL, UDP_PORT_SERV_BORDER, NULL);
}

void servreg() {
	servreg_hack_init();
	servreg_hack_register(SERVICE_ID, self_addr);
}

static void receiver(struct simple_udp_connection *c,
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

int msg_send(struct simple_udp_connection *c,
        const void *data, uint16_t datalen,
        const uip_ipaddr_t *to) {

	return simple_udp_sendto(c,*data,datalen,*to);
}


