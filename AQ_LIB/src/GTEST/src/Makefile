
TOPDIR = ../..

include $(TOPDIR)/make.header

INCLUDE += -I$(TOPDIR)/GTEST/include
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

GTEST_HOME=$(THIRD_PARTY)/googletest-release-1.7.0
INCLUDE += -I$(GTEST_HOME)/include


# CFLAGS += -wd80,470,811,858,1224,2358

CFLAGS += -fopenmp -Wno-reorder
CFLAGS += $(DEBUG)


OUTPUT = $(TOPDIR)/lib

## MLIB library dependencies
LIBS += -lvalidation_api -letrading -lmodels -lmath

## QuantLib: Resolve any further dependencies from etrading
LIBS += $(QLIB_LIB)

## Boost Library dependencies
LIBS += -L$(BOOST_HOME)/linux_x86_64/lib -lboost_chrono -lboost_atomic -lboost_filesystem -lboost_program_options
LIBS += -lboost_date_time -lboost_random -lboost_regex -lboost_thread -lboost_system

## Embed the path to the Boost shared libraries into our exe, so that
## the shared library path can be resolved at runtime
LIBS += -Wl,-rpath $(BOOST_HOME)/linux_x86_64/lib

## AAD Adept Library dependency
LIBS += $(ADEPT_HOME)/lib/libadept.a

## Google Test framework dependency
LIBS += $(GTEST_HOME)/make/gtest_main.a

## Also link in openmp support
LIBS += -fopenmp

EXEC = GTEST.exe



include $(TOPDIR)/make.footer

