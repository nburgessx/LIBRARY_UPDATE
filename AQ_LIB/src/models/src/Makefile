TOPDIR = ../..

include $(TOPDIR)/make.header

THIRD_PARTY=/workspace/svc_quant_dev/library

BOOST_HOME = $(THIRD_PARTY)/boost_1_70_0
INCLUDE += -I$(BOOST_HOME)

CFLAGS += -fpermissive
CFLAGS += $(DEBUG)

OUTPUT = $(TOPDIR)/lib
LIBA = libmodels.a

include $(TOPDIR)/make.footer

