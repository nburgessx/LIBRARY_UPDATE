RH_VER=$(shell cat /etc/redhat-release)
ifeq ($(findstring release 7,$(RH_VER)), release 7)
IS_RH7 = 1
else
IS_RH7 = 0
endif

all:
	@if [ ! -d lib ]; then mkdir lib; fi
	@if [ ! -d bin ]; then mkdir bin; fi
	@if [ -d math/src ]; then (cd math/src; $(MAKE) $@); fi
	@if [ -d common/src ]; then (cd common/src; $(MAKE) $@); fi
ifeq "$(IS_RH7)" "1"
	@if [ -d ft/src ]; then (cd ft/src; $(MAKE) $@); fi
endif
	@if [ -d ir/src ]; then (cd ir/src; $(MAKE) $@); fi
	@if [ -d irserver/src ]; then (cd irserver/src; $(MAKE) $@); fi
	@if [ -d credit/src ]; then (cd credit/src; $(MAKE) $@); fi
	@if [ -d mdci/src ]; then (cd mdci/src; $(MAKE) $@); fi
	@if [ -d maus/src ]; then (cd maus/src; $(MAKE) $@); fi
	@if [ -d libext/src ]; then (cd libext/src; $(MAKE) $@); fi
	@if [ -d irxlsutility ]; then (cd irxlsutility/src; $(MAKE) $@); fi
	@if [ -d client ]; then (cd client; $(MAKE) $@); fi
	@if [ -d forward_fx_client ]; then (cd forward_fx_client; $(MAKE) $@); fi
	@if [ -d utility_server ]; then (cd utility_server; $(MAKE) $@); fi
	@if [ -d utility_server_client_main ]; then (cd utility_server_client_main; $(MAKE) $@); fi
	@if [ -d xva_lib/src ]; then (cd xva_lib/src; $(MAKE) $@); fi
	@if [ -d xva_calib/src ]; then (cd xva_calib/src; $(MAKE) $@); fi
	@if [ -d xva_std/src ]; then (cd xva_std/src; $(MAKE) $@); fi
	@if [ -d xva_server/src ]; then (cd xva_server/src; $(MAKE) $@); fi
	@if [ -d xva_single_client ]; then (cd xva_single_client; $(MAKE) $@); fi
	@if [ -d dev_utils/src ]; then (cd dev_utils/src; $(MAKE) $@); fi

clean:
	@if [ -d math/src ]; then (cd math/src; $(MAKE) $@); fi
	@if [ -d common/src ]; then (cd common/src; $(MAKE) $@); fi
ifeq "$(IS_RH7)" "1"
	@if [ -d ft/src ]; then (cd ft/src; $(MAKE) $@); fi
endif
	@if [ -d ir/src ]; then (cd ir/src; $(MAKE) $@); fi
	@if [ -d irserver/src ]; then (cd irserver/src; $(MAKE) $@); fi
	@if [ -d credit/src ]; then (cd credit/src; $(MAKE) $@); fi
	@if [ -d mdci/src ]; then (cd mdci/src; $(MAKE) $@); fi
	@if [ -d maus/src ]; then (cd maus/src; $(MAKE) $@); fi
	@if [ -d libext/src ]; then (cd libext/src; $(MAKE) $@); fi
	@if [ -d client ]; then (cd client; $(MAKE) $@); fi
	@if [ -d forward_fx_client ]; then (cd forward_fx_client; $(MAKE) $@); fi
	@if [ -d utility_server ]; then (cd utility_server; $(MAKE) $@); fi
	@if [ -d utility_server_client_main ]; then (cd utility_server_client_main; $(MAKE) $@); fi
	@if [ -d xva_lib/src ]; then (cd xva_lib/src; $(MAKE) $@); fi
	@if [ -d xva_calib/src ]; then (cd xva_calib/src; $(MAKE) $@); fi
	@if [ -d xva_std/src ]; then (cd xva_std/src; $(MAKE) $@); fi
	@if [ -d xva_server/src ]; then (cd xva_server/src; $(MAKE) $@); fi
	@if [ -d xva_single_client ]; then (cd xva_single_client; $(MAKE) $@); fi
	@if [ -d dev_utils/src ]; then (cd dev_utils/src; $(MAKE) $@); fi
	@if [ -d doc ]; then rm -rf doc; fi

doc:
	@if [ ! -d doc ]; then mkdir doc; fi
	doxygen
