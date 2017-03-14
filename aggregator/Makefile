CONTIKI_PROJECT = aggregator
all: $(CONTIKI_PROJECT)

APPS=servreg-hack

CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\" -DOPENMOTE

CONTIKI = /home/livio/workspace/contiki

HW_MODULE_DIR = ../hw_module
TINYIPFIX_DIR = ../TinyIPFIX

include $(HW_MODULE_DIR)/Makefile.hw_module
include $(TINYIPFIX_DIR)/Makefile.tinyipfix

CONTIKI_WITH_IPV6 = 1
include $(CONTIKI)/Makefile.include
