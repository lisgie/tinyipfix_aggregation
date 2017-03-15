#ifndef COMM_H_
#define COMM_H_

#define SERVICE_ID 100

#define BORDER_COMM 0
#define AGGREGATOR_COMM 1
#define COLLECTOR_COMM 2

#include "../apps/servreg-hack/servreg-hack.h"

#include "net/ip/uip.h"
#include "net/ipv6/uip-ds6.h"
#include "net/ip/uip-debug.h"

#include "net/ip/simple-udp.h"

#define UDP_PORT_CLIENT_SERV 1234
#define UDP_PORT_SERV_BORDER 1111

void init_system();
int msg_send(int,const void*,uint16_t);

#endif /* NETWORKING_H_ */
