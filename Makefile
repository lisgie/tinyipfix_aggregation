CONTIKI_PROJECT = client server
all: $(CONTIKI_PROJECT)

CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"

CONTIKI = /home/livio/workspace/contiki

#NETWORKING_DIR = networking

#include $(NETWORKING_DIR)/Makefile.networking

CONTIKI_WITH_IPV6 = 1
include $(CONTIKI)/Makefile.include
