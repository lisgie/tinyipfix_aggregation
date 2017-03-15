CONTIKI_PROJECT = aggregator
all: $(CONTIKI_PROJECT)

APPS=servreg-hack

MSG_AGGR=0
DATA_AGGR=1

AGGR_MODE=$(DATA_AGGR)

ifeq ($(AGGR_MODE),$(MSG_AGGR))
PROJECT_SOURCEFILES += msg_aggr.c
else ifeq ($(AGGR_MODE),$(DATA_AGGR))
PROJECT_SOURCEFILES += data_aggr.c
endif

CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\" -DOPENMOTE -DCHOSEN_AGGR=$(AGGR_MODE)

CONTIKI = /home/livio/workspace/contiki

HW_MODULE_DIR = ../hw_module
TINYIPFIX_DIR = ../TinyIPFIX

include $(HW_MODULE_DIR)/Makefile.hw_module
include $(TINYIPFIX_DIR)/Makefile.tinyipfix
PROJECT_SOURCEFILES += comm.c

CONTIKI_WITH_IPV6 = 1
include $(CONTIKI)/Makefile.include
