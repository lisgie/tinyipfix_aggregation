CONTIKI_PROJECT = aggregator
all: $(CONTIKI_PROJECT)

APPS=servreg-hack

CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"

CONTIKI = /home/livio/workspace/contiki

HW_MODULE_DIR = ../hw_module
TINYIPFIX_DIR = ../TinyIPFIX
NETWORKING_DIR = ../networking

include $(HW_MODULE_DIR)/Makefile.hw_module
include $(TINYIPFIX_DIR)/Makefile.tinyipfix
include $(NETWORKING_DIR)/Makefile.networking

CONTIKI_WITH_IPV6 = 1
include $(CONTIKI)/Makefile.include
