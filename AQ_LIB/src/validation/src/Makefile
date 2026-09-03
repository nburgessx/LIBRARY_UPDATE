
TOPDIR = ../..

include $(TOPDIR)/make.header

INCLUDE += -I$(TOPDIR)/validation_api/include
INCLUDE += -I$(TOPDIR)/etrading/include

THIRD_PARTY=/workspace/svc_quant_dev/library

BOOST_HOME = $(THIRD_PARTY)/boost_1_70_0
INCLUDE += -I$(BOOST_HOME)

RAPID_JSON_HOME=$(THIRD_PARTY)/rapidjson-master
INCLUDE += -I$(RAPID_JSON_HOME)/include

ADEPT_HOME=$(THIRD_PARTY)/adept-1.1
INCLUDE += -I$(ADEPT_HOME)/include

EIGEN_HOME=$(THIRD_PARTY)/eigen-eigen-b30b87236a1b
INCLUDE += -I$(EIGEN_HOME)

CFLAGS += -fpermissive -Wno-reorder

# CFLAGS += -wd80,470,858,1224,2358

CFLAGS += -fopenmp
CFLAGS += $(DEBUG)

OUTPUT = $(TOPDIR)/lib
LIBA = libvalidation_api.a

include $(TOPDIR)/make.footer

