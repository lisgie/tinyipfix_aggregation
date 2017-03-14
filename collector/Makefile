CONTIKI_PROJECT = collector
all: $(CONTIKI_PROJECT)

APPS=servreg-hack

CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\" -DSKY

CONTIKI = /home/livio/workspace/contiki-2.7

HW_MODULE_DIR = ../hw_module
TINYIPFIX_DIR = ../TinyIPFIX

include $(HW_MODULE_DIR)/Makefile.hw_module
include $(TINYIPFIX_DIR)/Makefile.tinyipfix

WITH_UIP6=1
UIP_CONF_IPV6=1
UIP_CONF_IPV6_RPL=1

CONTIKI_WITH_IPV6 = 1
include $(CONTIKI)/Makefile.include