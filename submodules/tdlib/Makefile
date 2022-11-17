#
# SPDX-License-Identifier: GPL-2.0-only
#

SDIR_SUBMODULES__TDLIB := $(BASE_DIR)/submodules/tdlib
TDLIB_BUILD_DIR := $(SDIR_SUBMODULES__TDLIB)/td/build
TDLIB_INSTALL_DIR := $(TDLIB_BUILD_DIR)/install

TDLIB_OBJ := \
	$(TDLIB_INSTALL_DIR)/lib/libtdjson_static.a \
	$(TDLIB_INSTALL_DIR)/lib/libtdjson_private.a \
	$(TDLIB_INSTALL_DIR)/lib/libtdclient.a \
	$(TDLIB_INSTALL_DIR)/lib/libtdcore.a \
	$(TDLIB_INSTALL_DIR)/lib/libtdapi.a \
	$(TDLIB_INSTALL_DIR)/lib/libtdactor.a \
	$(TDLIB_INSTALL_DIR)/lib/libtdnet.a \
	$(TDLIB_INSTALL_DIR)/lib/libtddb.a \
	$(TDLIB_INSTALL_DIR)/lib/libtdsqlite.a \
	$(TDLIB_INSTALL_DIR)/lib/libtdutils.a

TDLIB_SO := \
	$(TDLIB_INSTALL_DIR)/lib/libtdpacked.so

LIB_LDFLAGS += \
	-L$(TDLIB_INSTALL_DIR)/lib/ -ltdpacked

obj-deps := $(TDLIB_SO)

INCLUDE_DIR += \
	-I$(TDLIB_INSTALL_DIR)/include

$(SDIR_SUBMODULES__TDLIB)/td/build/Makefile:
	mkdir -pv $(TDLIB_BUILD_DIR) && \
	cd $(TDLIB_BUILD_DIR) && \
	CC=$(CC) CXX=$(CXX) cmake -DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_INSTALL_PREFIX:PATH=$(TDLIB_INSTALL_DIR) ..;

$(TDLIB_OBJ): $(SDIR_SUBMODULES__TDLIB)/td/build/Makefile
	cd $(TDLIB_BUILD_DIR) && \
	VERBOSE=1 cmake --build . -j$(nproc) && \
	VERBOSE=1 cmake --build . --target install -j$(nproc);

$(TDLIB_SO): $(SDIR_SUBMODULES__TDLIB)/tdpacked.cpp
	$(CXX) -fpic -fPIC -shared -I$(TDLIB_INSTALL_DIR)/include \
		$(SDIR_SUBMODULES__TDLIB)/tdpacked.cpp \
		-o $(@) $(TDLIB_OBJ) -lcrypto -lssl -lz
