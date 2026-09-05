TOPDIR = ../..

include $(TOPDIR)/make.header

INCLUDE += -I$(TOPDIR)/validation/include
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

PYTHON_HOME=/opt/rh/rh-python35/root/usr
INCLUDE += -I$(PYTHON_HOME)/include/python3.5m

JAVA_JDK_HOME=/workspace/svc_quant_dev/library/jdk1.8.0_121
INCLUDE += -I$(JAVA_JDK_HOME)/include
INCLUDE += -I$(JAVA_JDK_HOME)/include/linux

R_HOME=/workspace/svc_quant_dev/library/R
INCLUDE += -I$(R_HOME)/lib64/R/include


#CFLAGS += -wd80,470,811,858,1224,2358
# -Wno-deprecated added because SWIG requires us to add throw() specification to API functions,
# which is a deprecated feature in C++11 and above
CFLAGS += -fopenmp -Wno-reorder -Wno-deprecated
CFLAGS += -fPIC
CFLAGS += $(DEBUG)

OUTPUT = $(TOPDIR)/lib

## AQ library dependencies
LIBS += -lvalidation -letrading -lcalibration -lmodels -lmath

## Boost Library dependencies
LIBS += -L$(BOOST_HOME)/linux_x86_64/lib -lboost_chrono -lboost_atomic -lboost_filesystem -lboost_program_options
LIBS += -lboost_date_time -lboost_random -lboost_regex -lboost_thread -lboost_system

## Embed the path to the Boost shared libraries into our exe, so that
## the shared library path can be resolved at runtime
LIBS += -Wl,-rpath $(BOOST_HOME)/linux_x86_64/lib

## Quantlib library dependencies
QLIB_LIB_HOME=$(QLIB_HOME)/ql/.libs
LIBS += -L$(QLIB_LIB_HOME) -lQuantLib
LIBS += -Wl,-rpath $(QLIB_LIB_HOME)

## AAD Adept Library dependency
LIBS += -L$(ADEPT_HOME)/lib -ladept
LIBS += -Wl,-rpath $(ADEPT_HOME)/lib


## Also link in openmp support
LIBS += -fopenmp

SWIG_HOME=$(THIRD_PARTY)/swig-4.0.0

RESOURCE_CONFIG=resource/config
SRC_AQ_CONFIG=../../../$(RESOURCE_CONFIG)

APIS = python_api python_shared java_api java_shared csharp_api csharp_shared r_api r_shared

all:
	for api in $(APIS); \
	do \
		$(MAKE) $$api || exit; \
	done


PYTHON_API=$(OUTPUT)/Python
PYTHON_RESOURCE_CONFIG=$(PYTHON_API)/$(RESOURCE_CONFIG)
PYTHON_GENERATED_FILES=$(PYTHON_API)/GeneratedFiles
PYTHON_LIB_SO = _AQ_API.so


ReleasePython: python_api python_shared

python_api:
	mkdir -p $(PYTHON_RESOURCE_CONFIG)
	mkdir -p $(PYTHON_GENERATED_FILES)
	rm -f swig*wrap.*
	$(SWIG_HOME)/bin/swig -v -c++ -outdir $(PYTHON_GENERATED_FILES) -o swig_Python3_wrap.cxx -python -py3 swig_Python.i
	rsync -av --exclude=".*" $(SRC_AQ_CONFIG)/* $(PYTHON_RESOURCE_CONFIG)
	cp $(PYTHON_GENERATED_FILES)/*.py $(PYTHON_API)

## Create shared library
python_shared: $(OBJS) swig_Python3_wrap.o
	$(CC) -shared $(OBJS) swig_Python3_wrap.o $(LIBS) -o $(PYTHON_API)/$(PYTHON_LIB_SO)

## Build the swig wrapper
swig_Python3_wrap.o: swig_Python3_wrap.cxx
	$(CC) $(CFLAGS) $(INCLUDE) -c $<

JAVA_API=$(OUTPUT)/Java
JAVA_RESOURCE_CONFIG=$(JAVA_API)/$(RESOURCE_CONFIG)
JAVA_GENERATED_FILES=$(JAVA_API)/GeneratedFiles
JAVA_LIB_SO = libAQ_API.so


ReleaseJava: java_api java_shared

java_api:
	mkdir -p $(JAVA_RESOURCE_CONFIG)
	mkdir -p $(JAVA_GENERATED_FILES)
	rm -f swig*wrap.*
	$(SWIG_HOME)/bin/swig -v -c++ -outdir $(JAVA_GENERATED_FILES) -o swig_Java_wrap.cxx -package com.algoquantlib -java swig_JAVA.i
	rsync -av --exclude=".*" $(SRC_AQ_CONFIG)/* $(JAVA_RESOURCE_CONFIG)

## Create shared library	
java_shared: $(OBJS) swig_Java_wrap.o
	$(CC) -shared $(OBJS) swig_Java_wrap.o $(LIBS) -o $(JAVA_API)/$(JAVA_LIB_SO)

## Build the swig wrapper
swig_Java_wrap.o: swig_Java_wrap.cxx
	$(CC) $(CFLAGS) $(INCLUDE) -c $<


R_API=$(OUTPUT)/R
R_RESOURCE_CONFIG=$(R_API)/$(RESOURCE_CONFIG)
R_GENERATED_FILES=$(R_API)/GeneratedFiles
R_LIB_SO = AQ_API.so

ReleaseR: r_api r_shared

r_api: CFLAGS += -DSWIG_R
r_api:
	mkdir -p $(R_RESOURCE_CONFIG)
	mkdir -p $(R_GENERATED_FILES)
	rm -f swig*wrap.*
	rm -f *.o  # ...because we need to rebuild all cpp files with different flags for R
	$(SWIG_HOME)/bin/swig -v -c++ -outdir $(R_GENERATED_FILES) -o swig_R_wrap.cxx -r swig_R.i
	rsync -av --exclude=".*" $(SRC_AQ_CONFIG)/* $(R_RESOURCE_CONFIG)
	cp $(R_GENERATED_FILES)/*.R $(R_API)

## Create shared library
r_shared: CFLAGS += -DSWIG_R
r_shared: $(OBJS) swig_R_wrap.o
	$(CC) -shared $(OBJS) swig_R_wrap.o $(LIBS) -o $(R_API)/$(R_LIB_SO)

## Build the swig wrapper
swig_R_wrap.o: swig_R_wrap.cxx
	$(CC) $(CFLAGS) $(INCLUDE) -c $<


CSHARP_API=$(OUTPUT)/CSharp
CSHARP_RESOURCE_CONFIG=$(CSHARP_API)/$(RESOURCE_CONFIG)
CSHARP_GENERATED_FILES=$(CSHARP_API)/GeneratedFiles
CSHARP_LIB_SO = libAQ_API.so


ReleaseCSharp: csharp_api csharp_shared

csharp_api:
	mkdir -p $(CSHARP_RESOURCE_CONFIG)
	mkdir -p $(CSHARP_GENERATED_FILES)
	rm -f swig*wrap.*
	$(SWIG_HOME)/bin/swig -v -c++ -outdir $(CSHARP_GENERATED_FILES) -o swig_CSharp_wrap.cxx -csharp swig_CSharp.i
	rsync -av --exclude=".*" $(SRC_AQ_CONFIG)/* $(CSHARP_RESOURCE_CONFIG)

## Create shared library	
csharp_shared: $(OBJS) swig_CSharp_wrap.o
	$(CC) -shared $(OBJS) swig_CSharp_wrap.o $(LIBS) -o $(CSHARP_API)/$(CSHARP_LIB_SO)

## Build the swig wrapper
swig_CSharp_wrap.o: swig_CSharp_wrap.cxx
	$(CC) $(CFLAGS) $(INCLUDE) -c $<


include $(TOPDIR)/make.footer

## Add a bespoke clean target
clean:
	rm -f $(OBJS)
	rm -rf $(OUTPUT)/Python $(OUTPUT)/Java $(OUTPUT)/CSharp $(OUTPUT)/R

